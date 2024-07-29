//g++ dip14.cpp -std=c++11 -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv4` -Wno-deprecated
#include <iostream> //入出力関連ヘッダ
#include <GLUT/glut.h> //OpenGL
#include <math.h> //数学関数
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ
#include <random>

// Shapeクラスの追加
class Shape {
public:
  std::vector<cv::Point3f> points; // 形状を定義する点の集合
  cv::Point3f velocity; // この形状全体の速度

  // コンストラクタ
  Shape(const std::vector<cv::Point3f>& points_, const cv::Point3f& velocity_) : points(points_), velocity(velocity_) {}
};

//・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
//関数名の宣言
void initCV(void);
void initGL(void);
void display(void);
void reshape(int w, int h);
void timer(int value);
void mouse(int button, int state, int x, int y); void motion(int x, int y);
void keyboard(unsigned char key, int x, int y); void initCV(void);
void mouseCallback(int event, int x, int y, int flags, void *userdata);

//・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
//グローバル変数
double eDist, eDegX, eDegY; //視点極座標
double theta = 0.0;
double delta = 1.0; // 回転の速度
double fr = 30.0; //フレームレート 

int mX, mY, mState, mButton; //マウス座標
int rotFlag = 1; // 回転フラグ

cv::Mat originalImage, frameImage; //画像格納用
cv::VideoCapture capture; //ビデオキャプチャ 
cv::Size imageSize; //画像サイズ用

// 追加したグローバル変数
cv::Mat prevImage; // 前のフレームを保存する変数
std::vector<cv::Point2f> prevFeatures, currFeatures; // 前のフレームと現在のフレームの特徴点
float threshold = 10.0;  // 閾値を定義（この値は適宜調整）

std::deque<int> gestureQueue; // 連続したジェスチャを記録するためのキュー
int gestureCountThreshold = 15; //連続するフレーム数を定義

std::vector<cv::Point> mouse_points; //マウスの座標を記録するための配列

std::vector<cv::Point3f> transferred_points; // Points transferred to 3D space

std::vector<Shape> shapes; // shapesをShapeのベクターに変更

std::vector<float> shapeSpeeds;

std::random_device rnd;
std::mt19937 mt(rnd());
std::uniform_int_distribution<> RandX(0.0, 700.0);
std::uniform_int_distribution<> RandY(0.0, 100.0);
std::uniform_int_distribution<> RandZ(-500.0, 500.0);
std::uniform_int_distribution<> RandV(30.0, 80.0);
std::uniform_real_distribution<> RandSP(1.0, 10.0);
std::uniform_real_distribution<> RandColor(0.0, 1.0);

int maxX = 400;
int minX = -400;
int maxY = 200;
int minY = 10;
int maxZ = 400;
int minZ = -400;

//・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
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

//・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
//OpenCV初期設定処理
void initCV(void)
{
  //①ビデオキャプチャの初期化
  capture = cv::VideoCapture(0); //カメラ0番をオープン
  if (capture.isOpened()==0) { //オープンに失敗した場合
    printf("Capture not found\n");
    return;
  }

  //②画像格納用インスタンス準備
  int imageWidth=720, imageHeight=405;
  imageSize = cv::Size(imageWidth, imageHeight); //画像サイズ
  frameImage = cv::Mat(imageSize, CV_8UC3); //3チャンネル

  prevImage = cv::Mat(imageSize, CV_8UC1);
  prevFeatures.clear();

  shapeSpeeds.push_back(RandSP(mt));  // Generate speed for the new shape

  //③画像表示用ウィンドウの生成
  cv::namedWindow("Frame");
  cv::moveWindow("Frame", 0, 0);

  //マウスコールバック関数のウィンドウへの登録
  cv::setMouseCallback("Frame", mouseCallback);
}

//・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
//OpenGL 初期設定処理
void initGL(void) {
  //初期設定
  glutInitWindowSize(600, 400);  //ウィンドウサイズ指定
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE); //ディスプレイモード設定

  //OpenGLウィンドウ生成
  glutInitWindowPosition(imageSize.width, 0);
  glutCreateWindow("GL");
  
  //ウィンドウ消去色設定
  glClearColor(0.9, 0.95, 1.0, 1.0);

  //機能有効化
  glEnable(GL_DEPTH_TEST); //デプスバッファ 
  glEnable(GL_NORMALIZE); //法線ベクトル正規化
  glEnable(GL_LIGHTING); //陰影付け 
  glEnable(GL_LIGHT0); //光源0

  //光原0の設定
  GLfloat col[4]; //パラメータ(RGBA)
  glEnable(GL_LIGHT0); //光源0
  col[0] = 0.9; col[1] = 0.9; col[2] = 0.9; col[3] = 1.0; 
  glLightfv(GL_LIGHT0,GL_DIFFUSE,col); //光源0の拡散反射の強度 
  glLightfv(GL_LIGHT0,GL_SPECULAR,col); //光源0の鏡面反射の強度 
  col[0] = 0.05; col[1] = 0.05; col[2] = 0.05; col[3] = 1.0; 
  glLightfv(GL_LIGHT0,GL_AMBIENT,col); //光源0の環境光の強度

  //コールバック関数
  glutDisplayFunc(display); //ディスプレイコールバック関数の指定 
  glutReshapeFunc(reshape); //リシェイプコールバック関数の指定 
  glutMouseFunc(mouse); //マウスクリックコールバック関数の指定 
  glutMotionFunc(motion);//マウスドラッグコールバック関数の指定 
  glutKeyboardFunc(keyboard); //キーボードコールバック関数の指定 
  glutTimerFunc(1000/fr, timer, 0); //タイマーコールバック関数の指定

  //視点極座標初期値
  eDist = 1500; eDegX = 10.0; eDegY = 0.0;
  
  glLineWidth(3.0);
}

//・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
//ディスプレイコールバック関数
void display()
{
  // ------------------------------- OpenCV --------------------------------
  //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
  capture >> originalImage;

  //ビデオが終了したら無限ループから脱出
  if (originalImage.data==NULL) {
    exit(0);
  }

  //"originalImage"をリサイズして"frameImage"生成
  cv::resize(originalImage, frameImage, imageSize);

  // -----------------------------------------------------------------
  // ジェスチャー検出ここから
  // -----------------------------------------------------------------
  cv::Mat gray;
  cv::cvtColor(frameImage, gray, cv::COLOR_BGR2GRAY);

  if(!prevFeatures.empty()){
    std::vector<uchar> status;
    std::vector<float> err;
    cv::calcOpticalFlowPyrLK(prevImage, gray, prevFeatures, currFeatures, status, err);

    int countLeft = 0;
    int countRight = 0;
    for(int i=0; i<prevFeatures.size(); ++i){
      if(status[i]){
        float diffX = currFeatures[i].x - prevFeatures[i].x;
        if(diffX > threshold){
          countRight++;
        } else if(diffX < -threshold){
          countLeft++;
        }
      }
    }

    if(countRight > countLeft){
      gestureQueue.push_back(1);
    } else if(countLeft > countRight){
      gestureQueue.push_back(-1);
    }
    // キューが一定の大きさ以上になったら、先頭の要素を削除
    if(gestureQueue.size() > gestureCountThreshold){
      gestureQueue.pop_front();
    }
    // キューの全要素が同じ方向のジェスチャであるかを確認
    if(gestureQueue.size() == gestureCountThreshold){
      int first = gestureQueue.front();
      bool allSame = std::all_of(gestureQueue.begin(), gestureQueue.end(), [first](int i){ return i == first; });
      if(allSame){
        if(first == 1){
          std::cout << "右にスワイプしました" << std::endl;
          if(!mouse_points.empty()){
            // Add mouse points to a new shape
            std::vector<cv::Point3f> newShape;
            float zz = RandZ(mt);  // Generate z coordinate for the entire shape
            float xx = RandX(mt);  // Generate x coordinate offset for the entire shape
            float yy = RandY(mt);  // Generate y coordinate offset for the entire shape
            for(auto &pt: mouse_points){
              newShape.push_back(cv::Point3f(pt.x-xx, imageSize.height - pt.y + yy, zz));
            }
            // shapes.push_back(newShape);
            cv::Point3f velocity(RandV(mt), RandV(mt), RandV(mt));
            shapes.push_back(Shape(newShape, velocity));
            mouse_points.clear(); // Clear the drawn figure
          }
        } else{
          std::cout << "左にスワイプしました" << std::endl;
          mouse_points.clear();
        }
        gestureQueue.clear();
      }
    }
  }

  cv::goodFeaturesToTrack(gray, prevFeatures, 1000, 0.01, 10);
  gray.copyTo(prevImage);
  // -----------------------------------------------------------------
  // ジェスチャー検出ここまで
  // -----------------------------------------------------------------

  // 描いた図形を描画（cv::line等で）
  for (size_t i = 1; i < mouse_points.size(); ++i) {
    cv::line(frameImage, mouse_points[i-1], mouse_points[i], cv::Scalar(0, 255, 0), 2);
  }

  //(b)"frameImage"の表示
  cv::imshow("Frame", frameImage);

  // ------------------------------- OpenGL --------------------------------
  
  GLfloat col[4]; //色設定用

  //ウィンドウ内消去
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //行列初期化
  glLoadIdentity();

  //視点座標の計算
  double ex = eDist*cos(eDegX*M_PI/180.0)*sin(eDegY*M_PI/180.0);
  double ey = eDist*sin(eDegX*M_PI/180.0);
  double ez = eDist*cos(eDegX*M_PI/180.0)*cos(eDegY*M_PI/180.0);

  //視点視線の設定
  gluLookAt(ex, ey, ez, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); //変換行列に視野変換行列を乗算

  //光源0の位置指定
  GLfloat pos0[] = {200.0, 700.0, 200.0, 0.0}; //(x, y, z, 0(平行光源)/1(点光源))
  glLightfv(GL_LIGHT0, GL_POSITION, pos0);

  //-------------------- 地面 --------------------
  //色設定
  col[0] = 0.5; col[1] = 1.0; col[2] = 0.5; // (0.5, 1.0, 0.5) : RGB
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col); //拡散反射係数
  col[0] = 1.0; col[1] = 1.0; col[2] = 1.0; col[3] = 1.0;
  glMaterialfv(GL_FRONT, GL_SPECULAR, col);
  glMaterialf(GL_FRONT, GL_SHININESS, 64); //ハイライト係数
  glPushMatrix(); //行列一時保存
  glScaled(1000, 1, 1000); //拡大縮小
  glutSolidCube(1.0); //立方体の配置
  glPopMatrix(); //行列復帰

  for(auto &shape : shapes){
    if(!shape.points.empty()) {
      //色設定
      col[0] = RandColor(mt); col[1] = RandColor(mt); col[2] = RandColor(mt); // RGBをランダムに設定
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col); //拡散反射係数
      col[0] = 1.0; col[1] = 1.0; col[2] = 1.0; col[3] = 1.0;
      glMaterialfv(GL_FRONT, GL_SPECULAR, col);
      glMaterialf(GL_FRONT, GL_SHININESS, 64); //ハイライト係数

      //一連の線で繋いで描画
      glBegin(GL_LINE_STRIP); //線を描くための開始点を設定

      for (const auto &point : shape.points) {
        glVertex3f(point.x, point.y, point.z); //頂点座標を設定
        printf("%f %f %f\n", point.x, point.y, point.z);
      }

      glEnd(); //線の描画終了
    }
  }

  //描画実行
  glutSwapBuffers();
}

//・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
//タイマーコールバック関数 
void timer(int value) {
  if(rotFlag){
    eDegY += delta;
  }
  
  theta += delta;
  
  // shape全体をx軸、y軸、z軸に沿って動かす
  for(auto &shape : shapes){
    // 最初の点について範囲チェックを行い、範囲外なら速度を反転
    if (shape.points[0].x > maxX || shape.points[0].x < minX){
        shape.velocity.x = -1 * shape.velocity.x;
    }
    if (shape.points[0].y > maxY || shape.points[0].y < minY){
        shape.velocity.y = -1 * shape.velocity.y;
    }
    if (shape.points[0].z > maxZ || shape.points[0].z < minZ){
        shape.velocity.z = -1 * shape.velocity.z;
    }

    // 全ての点を同じ速度で動かす
    for(auto &point : shape.points){
      point.x += shape.velocity.x;
      point.y += shape.velocity.y;
      point.z += shape.velocity.z;
    }
  }
  
  glutPostRedisplay();  //ディスプレイイベント強制発生
  glutTimerFunc(1000/fr, timer, 0);  //タイマー再設定
}

//・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
//リシェイプコールバック関数 
void reshape(int w, int h) {
  glViewport(0, 0, w, h); //ウィンドウ全体が描画対象
  glMatrixMode(GL_PROJECTION); //投影変換行列を計算対象に設定
  glLoadIdentity(); //行列初期化
  gluPerspective(30.0, (double)w/(double)h, 1.0, 10000.0); //変換行列に透視投影を乗算 
  glMatrixMode(GL_MODELVIEW); //モデルビュー変換行列を計算対象に設定
}

//・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
//マウスクリックコールバック関数
void mouse(int button, int state, int x, int y){
  if (state==GLUT_DOWN) {
    //マウス情報格納
    mX = x; mY = y;
    mState = state; mButton = button;
  } 
}

//・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
//マウスドラッグコールバック関数 
void motion(int x, int y){
  if (mButton==GLUT_RIGHT_BUTTON) {
    //マウスの移動量を角度変化量に変換
    eDegY = eDegY+(mX-x)*0.5; //マウス横方向→水平角
    eDegX = eDegX+(y-mY)*0.5; //マウス縦方向→垂直角
  }
  //マウス座標格納
  mX = x; mY = y; 
}

//・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
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

//・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・・
// マウスコールバック関数 in a window made by OpenCV
void mouseCallback(int event, int x, int y, int flags, void *userdata)
{
  // マウスの座標を出力
  std::cout << "x=" << x << ", y=" << y << " ";

  // イベントの種類を出力
  switch (event) {
    case cv::EVENT_MOUSEMOVE:
      std::cout << "マウスが動いた";
      if (flags & cv::EVENT_FLAG_LBUTTON) {  // 左ボタンが押されている間
        mouse_points.push_back(cv::Point(x, y));
      }
      break;
    case cv::EVENT_LBUTTONDOWN:
      // クリックした座標を配列に追加
      mouse_points.push_back(cv::Point(x, y));
      break;
    case cv::EVENT_RBUTTONDOWN:
      std::cout << "右ボタンを押した";
      break;
    case cv::EVENT_LBUTTONUP:
      std::cout << "左ボタンを離した";
      break;
    case cv::EVENT_RBUTTONUP:
      std::cout << "右ボタンを離した";
      break;
    case cv::EVENT_RBUTTONDBLCLK:
      std::cout << "右ボタンをダブルクリック";
      break;
    case cv::EVENT_LBUTTONDBLCLK:
      std::cout << "左ボタンをダブルクリック";
      break;
  }

  // マウスボタンと特殊キーの押下状態を出力
  std::string str;
  if (flags & cv::EVENT_FLAG_ALTKEY) {
    str += "Alt "; // ALTキーが押されている
  }
  if (flags & cv::EVENT_FLAG_CTRLKEY) {
    str += "Ctrl "; // Ctrlキーが押されている
  }
  if (flags & cv::EVENT_FLAG_SHIFTKEY) {
    str += "Shift "; // Shiftキーが押されている
  }
  if (flags & cv::EVENT_FLAG_LBUTTON) {
    str += "左ボタン "; // マウスの左ボタンが押されている
  }
  if (flags & cv::EVENT_FLAG_RBUTTON) {
    str += "右ボタン"; // マウスの右ボタンが押されている
  }
  
  if (!str.empty()) {
    std::cout << " 押下: " << str;
  }
  
  std::cout << std::endl;
}