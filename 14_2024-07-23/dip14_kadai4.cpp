#include <iostream>  
#include <GLUT/glut.h>  
#include <math.h>  
#include <opencv2/opencv.hpp>  

void initGL(void);
void display(void);
void reshape(int w, int h);
void timer(int value);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void initCV(void);
void mouseCallback(int event, int x, int y, int flags, void *userdata);

// グローバル変数
double eDist, eDegX, eDegY;  
int mX, mY, mState, mButton;  
int winW, winH;  
double fr = 30.0;  
cv::VideoCapture capture;  
cv::Size imageSize;  
cv::Mat originalImage, frameImage, grayImage, prevGray;  

std::vector<cv::Point2f> prevPoints, currPoints;
std::vector<uchar> status;
std::vector<float> err;

double theta = 0.0;
double delta = 1.0;  
int rotFlag = 1;  
std::vector<cv::Point> points;  // マウスクリック座標の格納用配列
std::vector<cv::Point3f> gl_points;  // マウスクリック座標の格納用配列

// main関数
int main(int argc, char* argv[])
{
    // OpenGL初期化
    glutInit(&argc, argv);

    // OpenCV初期設定処理
    initCV();

    // OpenGL初期設定処理
    initGL();
    
    // イベント待ち無限ループ
    glutMainLoop();
    
    return 0;
}

// OpenCV初期設定処理
void initCV(void)
{
    // ビデオキャプチャの初期化
    capture = cv::VideoCapture(0);  // カメラ0番をオープン
    if (capture.isOpened() == 0) {  // オープンに失敗した場合
        printf("Capture not found\n");
        return;
    }
    
    // 画像格納用インスタンス準備
    int imageWidth = 720, imageHeight = 405;
    imageSize = cv::Size(imageWidth, imageHeight);  // 画像サイズ
    frameImage = cv::Mat(imageSize, CV_8UC3);  // 3チャンネル

    capture >> originalImage;
    cv::resize(originalImage, frameImage, imageSize);
    cv::cvtColor(frameImage, prevGray, cv::COLOR_BGR2GRAY);
    cv::goodFeaturesToTrack(prevGray, prevPoints, 100, 0.01, 10);

    // 画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    
    // マウスコールバック関数のウィンドウへの登録
    cv::setMouseCallback("Frame", mouseCallback);
}

// OpenGL初期設定処理
void initGL(void)
{
    // 初期設定
    glutInitWindowSize(600, 400);  // ウィンドウサイズ指定
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);  // ディスプレイモード設定
    
    // OpenGLウィンドウ生成
    glutInitWindowPosition(imageSize.width, 0);
    glutCreateWindow("GL");
    
    // ウィンドウ消去色設定
    glClearColor(0.9, 0.95, 1.0, 1.0);
    
    // 機能有効化
    glEnable(GL_DEPTH_TEST);  // デプスバッファ
    glEnable(GL_NORMALIZE);  // 法線ベクトル正規化
    glEnable(GL_LIGHTING);  // 陰影付け
    glEnable(GL_LIGHT0);  // 光源０

    // 光原０の設定
    GLfloat col[4];  // パラメータ(RGBA)
    glEnable(GL_LIGHT0);  // 光源0
    col[0] = 0.9; col[1] = 0.9; col[2] = 0.9; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);  // 光源0の拡散反射の強度
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);  // 光源0の鏡面反射の強度
    col[0] = 0.05; col[1] = 0.05; col[2] = 0.05; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_AMBIENT, col);  // 光源0の環境光の強度

    // コールバック関数
    glutDisplayFunc(display);  // ディスプレイコールバック関数の指定
    glutReshapeFunc(reshape);  // リシェイプコールバック関数の指定
    glutMouseFunc(mouse);  // マウスクリックコールバック関数の指定
    glutMotionFunc(motion);  // マウスドラッグコールバック関数の指定
    glutKeyboardFunc(keyboard);  // キーボードコールバック関数の指定
    glutTimerFunc(1000 / fr, timer, 0);  // タイマーコールバック関数の指定
    
    // 視点極座標初期値
    eDist = 1500; eDegX = 10.0; eDegY = 0.0;
    
    glLineWidth(3.0);
}

// ディスプレイコールバック関数
void display()
{
    // OpenCV
    // (a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
    capture >> originalImage;
    // ビデオが終了したら無限ループから脱出
    if (originalImage.data == NULL) {
        exit(0);
    }
    // "originalImage"をリサイズして"frameImage"生成
    cv::resize(originalImage, frameImage, imageSize);

    // 描いた図形を描画（cv::line等で）
    for (size_t i = 1; i < points.size(); ++i) {
        cv::line(frameImage, points[i - 1], points[i], cv::Scalar(0, 255, 0), 2);
    }
    
    // (b)"frameImage"の表示
    cv::imshow("Frame", frameImage);

    cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);
    cv::calcOpticalFlowPyrLK(prevGray, grayImage, prevPoints, currPoints, status, err);
    
    double totalMotionX = 0.0;
    for (size_t i = 0; i < prevPoints.size(); i++) {
        if (status[i]) {
            totalMotionX += currPoints[i].x - prevPoints[i].x;
        }
    }


    cv::swap(prevGray, grayImage);
    cv::goodFeaturesToTrack(prevGray, prevPoints, 100, 0.01, 10);


    // OpenGL
    GLfloat col[4];  // 色設定用
    
    // ウィンドウ内消去
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 行列初期化
    glLoadIdentity();
    
    // 視点座標の計算
    double ex = eDist * cos(eDegX * M_PI / 180.0) * sin(eDegY * M_PI / 180.0);
    double ey = eDist * sin(eDegX * M_PI / 180.0);
    double ez = eDist * cos(eDegX * M_PI / 180.0) * cos(eDegY * M_PI / 180.0);
    
    // 視点視線の設定
    gluLookAt(ex, ey, ez, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);  // 変換行列に視野変換行列を乗算
    
    // 光源0の位置指定
    GLfloat pos0[] = {200.0, 700.0, 200.0, 0.0};  // (x, y, z, 0(平行光源)/1(点光源))
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    
    // 地面
    // 色設定
    col[0] = 0.5; col[1] = 1.0; col[2] = 0.5;  // (0.5, 1.0, 0.5) : RGB
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);  // 拡散反射係数
    col[0] = 1.0; col[1] = 1.0; col[2] = 1.0; col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);  // ハイライト係数
    glPushMatrix();  // 行列一時保存
    glScaled(1000, 1, 1000);  // 拡大縮小
    glutSolidCube(1.0);  // 立方体の配置
    glPopMatrix();  // 行列復帰

    // if (400 < totalMotionX && totalMotionX < 1000) {
    //     std::cout << "右にスワイプしました" << std::endl;
    //     std::cout << "totalMotionX: " << totalMotionX << std::endl;
    //     // マウスクリック座標をOpenGL座標に変換
    //     gl_points.clear();
    //     for (size_t i = 0; i < points.size(); i++) {
    //         double x = (double)points[i].x / imageSize.width * 2000 - 1000;
    //         double y = (double)(imageSize.height - points[i].y) / imageSize.height * 2000 - 1000;
    //         gl_points.push_back(cv::Point3f(x, y, 0));
    //     }
    //     points.clear();
    // } else if (-1000 < totalMotionX && totalMotionX < -500) {
    //     std::cout << "左にスワイプしました" << std::endl;
    //     std::cout << "totalMotionX: " << totalMotionX << std::endl;
    //     points.clear();
    // }

    //色設定
    col[0] = 0.5; col[1] = 1.0; col[2] = 0.5;  // (0.5, 1.0, 0.5) : RGB
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);  //拡散反射係数
    col[0] = 1.0; col[1] = 1.0; col[2] = 1.0; col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);  //ハイライト係数
    glPushMatrix();  //行列一時保存
    glBegin(GL_LINE_STRIP);
    for(int i=0; i<gl_points.size(); i++) {
        glVertex3f(gl_points[i].x/4, gl_points[i].y/4, 0);
    }
    glEnd();
    glPopMatrix();  //行列復帰
    
    // 描画実行
    glutSwapBuffers();
}

// タイマーコールバック関数
void timer(int value)
{
    if (rotFlag) {
        eDegY += delta;
    }
    
    theta += delta;
    
    glutPostRedisplay();  // ディスプレイイベント強制発生
    glutTimerFunc(1000 / fr, timer, 0);  // タイマー再設定
}

// リシェイプコールバック関数
void reshape(int w, int h)
{
    winW = w;  // ウィンドウサイズ格納
    winH = h;
    
    // ビューポート変換行列設定
    glViewport(0, 0, w, h);
    
    // 透視投影変換行列設定
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 10000.0);
    glMatrixMode(GL_MODELVIEW);
}

// マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    mX = x;
    mY = y;
    mState = state;
    mButton = button;
}

// マウスドラッグコールバック関数
void motion(int x, int y)
{
    if (mButton == GLUT_LEFT_BUTTON) {  // 左ボタンのドラッグ
        eDegY += (double)(x - mX) * 0.5;  // 視点水平回転角度
        eDegX += (double)(y - mY) * 0.5;  // 視点垂直回転角度
    }
    mX = x;
    mY = y;
}

// キーボードコールバック関数
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 'q':  // 終了
            exit(0);
        case ' ':  // 回転開始/停止
            rotFlag = 1 - rotFlag;
            break;
        case 'x':
            gl_points.clear();
            for (size_t i = 0; i < points.size(); i++) {
                double x = (double)points[i].x / imageSize.width * 2000 - 1000;
                double y = (double)(imageSize.height - points[i].y) / imageSize.height * 2000 - 1000;
                gl_points.push_back(cv::Point3f(x, y, 0));
            }
            points.clear();
            break;
        case 'c':
            points.clear();
            break;
        default:
            break;
    }
}

// OpenCVマウスコールバック関数
void mouseCallback(int event, int x, int y, int flags, void *userdata)
{
    if (event == cv::EVENT_MOUSEMOVE && flags & cv::EVENT_FLAG_LBUTTON) {  // 左ボタンクリック
        points.push_back(cv::Point(x, y));  // クリック座標を追加
    }
}
