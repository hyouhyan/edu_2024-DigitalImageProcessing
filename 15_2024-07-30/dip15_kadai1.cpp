//(OpenCV4) g++ -std=c++11 dip15b.cpp -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv4` -Wno-deprecated
//(OpenCV3) g++ dip15b.cpp -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv` -Wno-deprecated

/*
 （ヒント）
 ０．各パーツを大雑把にモデリング
		各パーツを描画する関数を作ると良い（細かいモデリングをした時にdisplay関数を見やすくするため）．
 １．画像座標系からCG座標系への座標変換を立式する
 　　そして，画像↔︎CG座標変換を行う関数image2cg()を作る
		cv::Rect image2cg(cv::Rect p)
			入力は画像座標p，戻り値はCG座標．
			cv::Rect pは矩形の左上座標(p.x,p.y)と矩形のサイズ（p.with, p.height)をメンバに持つ
			x, yが座標値なのでこれを変換する．width, heightはそのままで良い
 ２．今はCGの顔の位置は固定だが，CGの顔の位置が認識した顔の位置になるように（連動して動くように）してみよう．
 ３．連動できたら，他のパーツも同じようにする
 ４．顔を傾けたらCGの顔も傾くようにする
	左右の眼の位置関係に着目すれば，何度回転しているか分かる．タンジェントが使えそう．
	ただし，左右の眼はそれぞれ1個のみ認識されるように工夫すること
 ５．各パーツを丁寧にモデリングし直し
 */

#include <iostream>  //入出力関連ヘッダ
#include <GLUT/glut.h>  //OpenGL関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

//関数名の宣言
void initCV(void);  // OpenCVの初期化
void initGL(void);  // OpenGLの初期化
void display(void);  // 描画関数
void timer(int value);  //タイマー関数
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);

//グローバル変数
double eDist, eDegX, eDegY;  //視点極座標
int mX, mY, mState, mButton;  //マウス座標
int winW, winH;  //ウィンドウサイズ
double fr = 30.0;  //フレームレート
cv::VideoCapture capture;  // 映像キャプチャ用変数
cv::Mat originalImage, frameImage, hsvImage, tempImage;
cv::Size imageSize(640, 360);

cv::CascadeClassifier faceClassifier;  // 顔認識用分類器
cv::CascadeClassifier left_eyeClassifier;  // 左目認識用分類器
cv::CascadeClassifier right_eyeClassifier;  // 右目認識用分類器
cv::CascadeClassifier mouthClassifier;  // 口認識用分類器
cv::CascadeClassifier noseClassifier;  // 鼻認識用分類器


//main関数
int main(int argc, char* argv[])
{
    //OpenGL初期化
    glutInit(&argc, argv);
    
    //OpenCV初期設定処理
    initCV();
    
    //OpenGL初期設定処理
    initGL();
    
    //イベント待ち無限ループ
    glutMainLoop();
    
    return 0;
}

//OpenCV初期設定処理
void initCV()
{
	//カメラキャプチャの初期化
	// capture = cv::VideoCapture(0);
    capture = cv::VideoCapture("./src/face.mov");
	if (capture.isOpened()==0) {
		//カメラが見つからないときはメッセージを表示して終了
		printf("Camera not found\n");
		exit(1);
	}
	capture >> originalImage;
	cv::resize(originalImage, frameImage, imageSize);

	//OpenCVウィンドウ生成
	cv::namedWindow("Frame");
	
	//顔検出器の読み込み
	faceClassifier.load("./src/haarcascade_frontalface_default.xml");
    left_eyeClassifier.load("./src/haarcascade_lefteye_2splits.xml");
    right_eyeClassifier.load("./src/haarcascade_righteye_2splits.xml");
    mouthClassifier.load("./src/haarcascade_mcs_mouth.xml");
    noseClassifier.load("./src/haarcascade_mcs_nose.xml");
}

//OpenGL初期設定処理
void initGL(void)
{
    //初期設定
    glutInitWindowSize(imageSize.width, imageSize.height);  //ウィンドウサイズ指定
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);  //ディスプレイモード設定
    
    //OpenGLウィンドウ生成
    glutCreateWindow("CG");
    
    //ウィンドウ消去色設定
    glClearColor(0.0, 0.0, 0.2, 1.0);
    
    //機能有効化
    glEnable(GL_DEPTH_TEST);  //デプスバッファ
    glEnable(GL_NORMALIZE);  //法線ベクトル正規化
    glEnable(GL_LIGHTING);  //陰影付け
    
    //光原設定
    GLfloat col[4];  //パラメータ(RGBA)
    glEnable(GL_LIGHT0);  //光源0
    col[0] = 0.9; col[1] = 0.9; col[2] = 0.9; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);  //光源0の拡散反射の強度
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);  //光源0の鏡面反射の強度
    col[0] = 0.05; col[1] = 0.05; col[2] = 0.05; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_AMBIENT, col);  //光源0の環境光の強度

    //コールバック関数
    glutDisplayFunc(display);  //ディスプレイコールバック関数の指定
    glutReshapeFunc(reshape);  //リシェイプコールバック関数の指定
    glutMouseFunc(mouse);  //マウスクリックコールバック関数の指定
    glutMotionFunc(motion);  //マウスドラッグコールバック関数の指定
    glutKeyboardFunc(keyboard);  //キーボードコールバック関数の指定
    glutTimerFunc(1000/fr, timer, 0);  //タイマーコールバック関数の指定
    
    //視点極座標初期値
    eDist = 1200; eDegX = 0.0; eDegY = 0.0;
}

//ディスプレイコールバック関数
void display(void)
{
    //------------------------------CV------------------------------
    //ビデオキャプチャから1フレーム画像取得
	capture >> originalImage;
	cv::resize(originalImage, frameImage, imageSize);

    //検出情報を受け取るための配列を用意する
	std::vector<cv::Rect> faces;
    std::vector<cv::Rect> left_eyes;
    std::vector<cv::Rect> right_eyes;
    std::vector<cv::Rect> mouths;
    std::vector<cv::Rect> noses;

    //画像中から検出対象の情報を取得する
	faceClassifier.detectMultiScale(frameImage, faces, 1.1, 3, 0, cv::Size(20,20));  //顔
    left_eyeClassifier.detectMultiScale(frameImage, left_eyes, 1.1, 3, 0, cv::Size(20,20));  //左目
    right_eyeClassifier.detectMultiScale(frameImage, right_eyes, 1.1, 3, 0, cv::Size(20,20));  //右目
    mouthClassifier.detectMultiScale(frameImage, mouths, 1.1, 3, 0, cv::Size(20,20));  //口
    noseClassifier.detectMultiScale(frameImage, noses, 1.1, 3, 0, cv::Size(20,20));  //鼻

    //顔
	for (int i=0; i<faces.size(); i++) {
        //検出情報から位置情報を取得
		cv::Rect face = faces[i];
        //大きさによるチェック。
		if(face.width*face.height<100*100){
            continue;  //小さい矩形は採用しない
        }
        //取得した位置情報に基づき矩形描画
		cv::rectangle(frameImage, cv::Point(face.x, face.y), cv::Point(face.x+face.width, face.y+face.height), CV_RGB(255,0,0), 2, 8);
    }

    //左目
    for (int i=0; i<left_eyes.size(); i++) {
        //検出情報から位置情報を取得
        cv::Rect left_eye = left_eyes[i];
        //大きさによるチェック。
        if(left_eye.width*left_eye.height<10*10){
            continue;  //小さい矩形は採用しない
        }
        //取得した位置情報に基づき矩形描画
        cv::rectangle(frameImage, cv::Point(left_eye.x, left_eye.y), cv::Point(left_eye.x+left_eye.width, left_eye.y+left_eye.height), CV_RGB(0,255,0), 2, 8);
    }

    //右目
    for (int i=0; i<right_eyes.size(); i++) {
        //検出情報から位置情報を取得
        cv::Rect right_eye = right_eyes[i];
        //大きさによるチェック。
        if(right_eye.width*right_eye.height<10*10){
            continue;  //小さい矩形は採用しない
        }
        //取得した位置情報に基づき矩形描画
        cv::rectangle(frameImage, cv::Point(right_eye.x, right_eye.y), cv::Point(right_eye.x+right_eye.width, right_eye.y+right_eye.height), CV_RGB(0,255,0), 2, 8);
    }

    //口
    for (int i=0; i<mouths.size(); i++) {
        //検出情報から位置情報を取得
        cv::Rect mouth = mouths[i];
        //大きさによるチェック。
        if(mouth.width*mouth.height<10*10){
            continue;  //小さい矩形は採用しない
        }
        //取得した位置情報に基づき矩形描画
        cv::rectangle(frameImage, cv::Point(mouth.x, mouth.y), cv::Point(mouth.x+mouth.width, mouth.y+mouth.height), CV_RGB(0,0,255), 2, 8);
    }

    //鼻
    for (int i=0; i<noses.size(); i++) {
        //検出情報から位置情報を取得
        cv::Rect nose = noses[i];
        //大きさによるチェック。
        if(nose.width*nose.height<10*10){
            continue;  //小さい矩形は採用しない
        }
        //取得した位置情報に基づき矩形描画
        cv::rectangle(frameImage, cv::Point(nose.x, nose.y), cv::Point(nose.x+nose.width, nose.y+nose.height), CV_RGB(255,255,0), 2, 8);
    }
    
    //フレーム画像表示
    cv::imshow("Frame", frameImage);
    
    //------------------------------CG------------------------------
    GLfloat col[4];  //色設定用
    
    //ウィンドウ内消去
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //行列初期化
    glLoadIdentity();
    
    //視点座標の計算
    double ex = eDist*cos(eDegX*M_PI/180.0)*sin(eDegY*M_PI/180.0);
    double ey = eDist*sin(eDegX*M_PI/180.0);
    double ez = eDist*cos(eDegX*M_PI/180.0)*cos(eDegY*M_PI/180.0);
    
    //視点視線の設定
    gluLookAt(ex, ey, ez, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);  //変換行列に視野変換行列を乗算
    
    //光源0の位置指定
    GLfloat pos0[] = {200.0, 700.0, 200.0, 1.0};  //(x, y, z, 0(平行光源)/1(点光源))
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    
    //--------------------顔--------------------
    //色設定
    col[0] = 1.0; col[1] = 0.8; col[2] = 0.5;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col);  //拡散反射係数
    glMaterialfv(GL_FRONT, GL_AMBIENT, col);  //環境光反射係数
    col[0] = 0.5; col[1] = 0.5; col[2] = 0.5; col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);  //ハイライト係数
    glPushMatrix();  //行列一時保存
    glTranslated(0.0, 0.0, 0.0);  //中心座標
    glScaled(250.0, 250.0, 60.0);  //拡大縮小
    glutSolidCube(1.0);  //立方体の配置
    glPopMatrix();  //行列復帰

    //描画実行
    glutSwapBuffers();
}

//タイマーコールバック関数
void timer(int value)
{
	glutPostRedisplay();  //ディスプレイイベント強制発生
	glutTimerFunc(1000/fr, timer, 0);  //タイマー再設定
}

//リサイズコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);  //ウィンドウ全体が描画対象
    glMatrixMode(GL_PROJECTION);  //投影変換行列を計算対象に設定
    glLoadIdentity();  //行列初期化
    gluPerspective(30.0, (double)w/(double)h, 1.0, 10000.0);  //変換行列に透視投影を乗算
    glMatrixMode(GL_MODELVIEW);  //モデルビュー変換行列を計算対象に設定
	
	winW = w; winH = h;
}

//マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    if (state==GLUT_DOWN) {
        //マウス情報格納
        mX = x; mY = y;
        mState = state; mButton = button;
    }
}

//マウスドラッグコールバック関数
void motion(int x, int y)
{
    if (mButton==GLUT_RIGHT_BUTTON) {
        //マウスの移動量を角度変化量に変換
        eDegY = eDegY+(mX-x)*0.5;  //マウス横方向→水平角
        eDegX = eDegX+(y-mY)*0.5;  //マウス縦方向→垂直角
    }
    
    //マウス座標格納
    mX = x; mY = y;
}

//キーボードコールバック関数(key:キーの種類，x,y:座標)
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 'q':
        case 'Q':
        case 27:
            exit(0);
    }
}
