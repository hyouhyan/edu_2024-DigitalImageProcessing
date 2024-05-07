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
    //mp4
    cv::VideoWriter rec("rec.mp4", cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30, recImage.size());

    //2値化画像の保存用
    cv::Mat binImage;

    //動画処理を10秒間だけ行う
    for(int i = 0; i < 300; i++){
        //カメラから1フレーム読み込んでcaptureImageに格納（CV_8UC3）
        capture >> captureImage;

        //captureImageをframeImageに合わせてサイズ変換して格納
        cv::resize(captureImage, frameImage, frameImage.size());

        //画像処理
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);

        //2値化
        cv::threshold(grayImage, binImage, 128, 255, cv::THRESH_BINARY);

        //ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Result", grayImage);

        //動画ファイルの書き出し
        //2値化画像をカラー画像に変換
        cv::cvtColor(binImage, recImage, cv::COLOR_GRAY2BGR);
        //ビデオライタに画像出力
        rec << recImage;

        //キー入力待ち
        char key = cv::waitKey(20);
        if(key == 'q') break;
    }
    
    
    return 0;
}
