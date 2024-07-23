#include <iostream>
#include <GLUT/glut.h>
#include <opencv2/opencv.hpp>

// 関数名の宣言
void initGL(void);
void display(void);
void reshape(int w, int h);
void timer(int value);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void initCV(void);

// グローバル変数
double eDist, eDegX, eDegY;
int mX, mY, mState, mButton;
int winW, winH;
double fr = 30.0;
cv::VideoCapture capture;
cv::Size imageSize;
cv::Mat prevGray, gray, frame;
std::vector<cv::Point2f> prevPoints, currPoints;
std::vector<uchar> status;
std::vector<float> err;
double theta = 0.0;
double delta = 1.0;
int rotFlag = 1;

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);

    // OpenCV初期設定処理
    initCV();

    // OpenGL初期設定処理
    initGL();

    // イベント待ち無限ループ
    glutMainLoop();

    return 0;
}

void initCV(void)
{
    capture = cv::VideoCapture(0);
    if (!capture.isOpened()) {
        std::cerr << "Capture not found" << std::endl;
        exit(1);
    }

    imageSize = cv::Size(640, 480);
    capture >> frame;
    cv::cvtColor(frame, prevGray, cv::COLOR_BGR2GRAY);
    cv::goodFeaturesToTrack(prevGray, prevPoints, 100, 0.01, 10);

    cv::namedWindow("Frame");
}

void initGL(void)
{
    glutInitWindowSize(600, 400);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow("GL");

    glClearColor(0.9, 0.95, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat col[4];
    glEnable(GL_LIGHT0);
    col[0] = 0.9; col[1] = 0.9; col[2] = 0.9; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);
    col[0] = 0.05; col[1] = 0.05; col[2] = 0.05; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_AMBIENT, col);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(1000/fr, timer, 0);

    eDist = 1500; eDegX = 10.0; eDegY = 0.0;

    glLineWidth(3.0);
}

void display()
{
    capture >> frame;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::calcOpticalFlowPyrLK(prevGray, gray, prevPoints, currPoints, status, err);

    double totalMotionX = 0.0;
    for (size_t i = 0; i < prevPoints.size(); i++) {
        if (status[i]) {
            totalMotionX += currPoints[i].x - prevPoints[i].x;
        }
    }


    if (300 < totalMotionX && totalMotionX < 1000) {
        std::cout << "右にスワイプしました" << std::endl;
        std::cout << "totalMotionX: " << totalMotionX << std::endl;
    } else if (-1000 < totalMotionX && totalMotionX < -300) {
        std::cout << "左にスワイプしました" << std::endl;
        std::cout << "totalMotionX: " << totalMotionX << std::endl;
    }

    cv::swap(prevGray, gray);
    cv::goodFeaturesToTrack(prevGray, prevPoints, 100, 0.01, 10);

    cv::imshow("Frame", frame);
    cv::waitKey(1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    double ex = eDist*cos(eDegX*M_PI/180.0)*sin(eDegY*M_PI/180.0);
    double ey = eDist*sin(eDegX*M_PI/180.0);
    double ez = eDist*cos(eDegX*M_PI/180.0)*cos(eDegY*M_PI/180.0);

    gluLookAt(ex, ey, ez, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    GLfloat pos0[] = {200.0, 700.0, 200.0, 0.0};
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);

    GLfloat col[4];
    col[0] = 0.5; col[1] = 1.0; col[2] = 0.5;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    col[0] = 1.0; col[1] = 1.0; col[2] = 1.0; col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);
    glPushMatrix();
    glScaled(1000, 1, 1000);
    glutSolidCube(1.0);
    glPopMatrix();

    glutSwapBuffers();
}

void timer(int value)
{
    if(rotFlag){
        eDegY += delta;
    }

    theta += delta;

    glutPostRedisplay();
    glutTimerFunc(1000/fr, timer, 0);
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, (double)w/(double)h, 1.0, 10000.0);
    glMatrixMode(GL_MODELVIEW);
}

void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        mX = x; mY = y;
        mState = state; mButton = button;
    }
}

void motion(int x, int y)
{
    if (mButton == GLUT_RIGHT_BUTTON) {
        eDegY = eDegY + (mX - x) * 0.5;
        eDegX = eDegX + (y - mY) * 0.5;
    }

    mX = x; mY = y;
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 'q':
        case 'Q':
        case 27:
            exit(0);
    }
}
