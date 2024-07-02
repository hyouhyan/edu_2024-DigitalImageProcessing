// g++ main.cpp -std=c++11 `pkg-config --cflags --libs opencv4` -framework OpenGL -framework GLUT -Wno-deprecated

#include <iostream>
#include <opencv2/opencv.hpp>
#include <GLUT/glut.h>

// グローバル変数
cv::VideoCapture capture;
int rows = 24, cols = 32;
int winW, winH;
float angles[24][32];

int imageWidth=720, imageHeight=405;
cv::Size imageSize(imageWidth, imageHeight);  //画像サイズ
cv::Mat originalImage;  //ビデオキャプチャ用
cv::Mat frameImage(imageSize, CV_8UC3);  //3チャンネル

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
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat col[4] = {223/255.0, 132/255.0, 2/255.0, 1.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);
    col[0] = 0.9; col[1] = 0.9; col[2] = 0.9; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_AMBIENT, col);
}

// OpenGL描画
void displayGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 1000.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    
    GLfloat pos0[] = {0.0, 0.0, 2000.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    
    glTranslatef(-cols * 10.0f, -rows * 10.0f, 0.0f);
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            glPushMatrix();
            glTranslatef(j * 20.0f, i * 20.0f, 0.0f);
            glRotatef(angles[i][j], 1.0f, 0.0f, 0.0f);
            glColor3f(0.5f, 0.3f, 0.2f);
            glBegin(GL_QUADS);
            glVertex3f(-10.0f, -10.0f, 0.0f);
            glVertex3f(10.0f, -10.0f, 0.0f);
            glVertex3f(10.0f, 10.0f, 0.0f);
            glVertex3f(-10.0f, 10.0f, 0.0f);
            glEnd();
            glPopMatrix();
        }
    }
    
    glutSwapBuffers();
}

// OpenCVのフレームを取得し角度を計算
void updateAngles() {
    capture >> originalImage;
    if (frameImage.empty()) return;

    cv::resize(originalImage, frameImage, imageSize);

    // OpenCVウィンドウに表示
    cv::imshow("Camera", frameImage);

    //フレームを上下反転
    cv::flip(frameImage, frameImage, 0);
    
    cv::resize(frameImage, frameImage, cv::Size(cols, rows));
    cv::cvtColor(frameImage, frameImage, cv::COLOR_BGR2GRAY);
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int brightness = frameImage.at<uchar>(i, j);
            angles[i][j] = brightness / 255.0f * 90.0f;
        }
    }
}

// タイマーコールバック
void timer(int value) {
    updateAngles();
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

    cv::namedWindow("Camera");
    
    glutDisplayFunc(displayGL);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(1000 / 30, timer, 0);
    
    glutMainLoop();
    return 0;
}
