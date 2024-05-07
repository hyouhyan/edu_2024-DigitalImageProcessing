//g++ dip03.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, const char* argv[])
{    
    //カメラ0番をオープン
    cv::VideoCapture capture(0);

    //カメラがオープンできたかどうかをチェック
    if(capture.isOpened() == 0){
        printf("Camera not found\n");
        return -1;
    }

    //画像格納用インスタンス準備
    //処理画像サイズ
    int width = 640, height = 360;

    //キャプチャ用
    cv::Mat captureImage;

    //処理用
    cv::Mat frameImage = cv::mat(cv::Size(width, height), CV_8UC3);
    cv::mat grayImage;

    //③ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0,0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 0,height);

    
    //④カメラから1フレーム読み込んでcaptureImageに格納（CV_8UC3）
    
    
    //⑤captureImageをframeImageに合わせてサイズ変換して格納
    
    
    //⑥画像処理
    
    
    //⑦ウィンドウへの画像の表示
    
    
    //⑧キー入力待ち
    char key = cv::waitKey(0);
    
    
    return 0;
}
