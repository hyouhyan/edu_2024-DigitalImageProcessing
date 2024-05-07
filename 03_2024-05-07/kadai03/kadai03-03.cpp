//g++ dip03.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, const char* argv[])
{    
    //ファイルをオープン
    cv::VideoCapture capture("dance.mov");
    cv::VideoCapture capture2("landscape.mov");

    //ファイルがオープンできたかどうかをチェック
    if(capture.isOpened() == 0 || capture2.isOpened() == 0){
        printf("File not found\n");
        return -1;
    }

    //画像格納用インスタンス準備
    //処理画像サイズ
    int width = 640, height = 360;

    //キャプチャ用
    cv::Mat captureImage;
    cv::Mat capture2Image;

    //処理用
    cv::Mat frameImage = cv::Mat(cv::Size(width, height), CV_8UC3);
    cv::Mat frame2Image = cv::Mat(cv::Size(width, height), CV_8UC3);
    cv::Mat hsvImage;

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
        capture2 >> capture2Image;

        //captureImageをframeImageに合わせてサイズ変換して格納
        cv::resize(captureImage, frameImage, frameImage.size());
        cv::resize(capture2Image, frame2Image, frame2Image.size());

        //画像処理
        //hsvに変換
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);


        //フレームを走査し、グリーンバッグを削除
        //削除した部分にcapture2を埋め込む
        for(int y = 0; y < hsvImage.rows; y++){
            for(int x = 0; x < hsvImage.cols; x++){
                //hsvの値を取得
                cv::Vec3b hsv = hsvImage.at<cv::Vec3b>(y, x);
                //hsvの値をもとに、グリーンバッグを削除
                if(40 <= hsv[0] && hsv[0] <= 50 && 160 <= hsv[1]){
                    frameImage.at<cv::Vec3b>(y, x) = frame2Image.at<cv::Vec3b>(y, x);
                    recImage.at<cv::Vec3b>(y, x) = frame2Image.at<cv::Vec3b>(y, x);
                }else{
                    recImage.at<cv::Vec3b>(y, x) = frameImage.at<cv::Vec3b>(y, x);
                
                }
            }
        }

        //動画ファイルの書き出し
        //動画用3チャンネル画像生成
        // cv::cvtColor(hsvImage, recImage, cv::COLOR_HSV2BGR);
        //ビデオライタに画像出力
        rec << recImage;

        //ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Result", recImage);

        //キー入力待ち
        char key = cv::waitKey(20);
        if(key == 'q') break;
    }
    
    
    return 0;
}
