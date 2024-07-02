#include <iostream>
#include <opencv2/opencv.hpp>
#include <GLUT/glut.h>
#include <vector>

// グローバル変数
int winW, winH;
bool lightOn = false;
GLfloat lightPos[4] = {0.0, 0.0, 0.0, 1.0};  // 光源の位置

// OpenCV関連 グローバス変数
cv::VideoCapture capture(0);

int imageWidth=720, imageHeight=405;
cv::Size imageSize(imageWidth, imageHeight);  //画像サイズ
cv::Mat originalImage;  //ビデオキャプチャ用
cv::Mat frameImage(imageSize, CV_8UC3);  //3チャンネル

struct Box {
    float x, y, z;
    float sizeX, sizeY, sizeZ;
};

std::vector<Box> boxes;

// OpenCV初期化
void initCV() {
    capture.open(0);
    if (!capture.isOpened()) {
        std::cerr << "Capture not found" << std::endl;
        exit(-1);
    }
}

// OpenGL初期設定
void initGL() {
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("GL");
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    
    // 20個の直方体をランダムに配置
    for (int i = 0; i < 20; ++i) {
        Box box;
        box.x = (rand() % 1000) - 500;
        box.y = (rand() % 1000) - 500;
        box.z = (rand() % 1000) - 500;
        box.sizeX = (rand() % 100) + 50;
        box.sizeY = (rand() % 100) + 50;
        box.sizeZ = (rand() % 100) + 50;
        boxes.push_back(box);
    }
}

// OpenGL描画
void displayGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 1000.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    if (lightOn) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    } else {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
    }

    for (const auto& box : boxes) {
        glPushMatrix();
        glTranslatef(box.x, box.y, box.z);
        glScalef(box.sizeX, box.sizeY, box.sizeZ);
        glColor3f(0.5f, 0.3f, 0.2f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
    
    glutSwapBuffers();
}

// OpenCVのフレームを取得し、光源を検出
void updateLight() {
    capture >> originalImage;
    if (frameImage.empty()) return;

    cv::resize(originalImage, frameImage, imageSize);
    cv::imshow("Frame", frameImage);

    cv::flip(frameImage, frameImage, 1);  // 左右反転
    cv::cvtColor(frameImage, frameImage, cv::COLOR_BGR2GRAY);
    cv::threshold(frameImage, frameImage, 200, 255, cv::THRESH_BINARY);
    
    // 光源の位置を検出
    cv::Moments m = cv::moments(frameImage, true);
    if (m.m00 > 0) {
        lightOn = true;
        lightPos[0] = (m.m10 / m.m00) * 4.0 - 800;
        lightPos[1] = (m.m01 / m.m00) * 4.0 - 600;
        lightPos[2] = 500.0;
    } else {
        lightOn = false;
    }

    // OpenCVウィンドウに表示
    cv::imshow("Camera", frameImage);
}

// タイマーコールバック
void timer(int value) {
    updateLight();
    glutPostRedisplay();
    glutTimerFunc(1000 / 30, timer, 0);
}

// リシェイプコールバック
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, (double)w / (double)h, 1.0, 10000.0);
    glMatrixMode(GL_MODELVIEW);
}

// キーボードコールバック
void keyboard(unsigned char key, int x, int y) {
    if (key == 'q' || key == 'Q' || key == 27) {
        capture.release();
        exit(0);
    }
}

// メイン関数
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    initCV();
    initGL();
    
    // OpenCVウィンドウを作成
    cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);

    glutDisplayFunc(displayGL);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(1000 / 30, timer, 0);
    
    glutMainLoop();
    return 0;
}
