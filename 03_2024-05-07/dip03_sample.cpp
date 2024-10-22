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
    cv::Mat frameImage = cv::Mat(cv::Size(width, height), CV_8UC3);
    cv::Mat grayImage;

    //ファイル出力用
    cv::Mat recImage = cv::Mat(cv::Size(width, height), CV_8UC3);

    //ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0,0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 0,height);

    //ビデオライタ生成
    cv::VideoWriter rec("rec.mpg", cv::VideoWriter::fourcc('P','I','M','1'), 30, recImage.size());

    //動画処理無限ループ
    while(1){
        //カメラから1フレーム読み込んでcaptureImageに格納（CV_8UC3）
        capture >> captureImage;

        //captureImageをframeImageに合わせてサイズ変換して格納
        cv::resize(captureImage, frameImage, frameImage.size());

        //画像処理
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);

        //ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Result", grayImage);

        //動画ファイルの書き出し
        //動画用3チャンネル画像生成
        cv::cvtColor(grayImage, recImage, cv::COLOR_GRAY2BGR);
        //ビデオライタに画像出力
        rec << recImage;

        //キー入力待ち
        char key = cv::waitKey(30);
        if(key == 'q') break;
    }
    
    
    return 0;
}
