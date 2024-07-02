//g++ dip11_cv.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, char* argv[])
{
    //①ビデオキャプチャの初期化
    cv::VideoCapture capture(0);  //カメラ0番をオープン
    if (capture.isOpened()==0) {  //オープンに失敗した場合
        printf("Capture not found\n");
        return -1;
    }
    
    //②画像格納用インスタンス準備
    int imageWidth=720, imageHeight=405;
    cv::Size imageSize(imageWidth, imageHeight);  //画像サイズ
    cv::Mat originalImage;  //ビデオキャプチャ用
    cv::Mat frameImage(imageSize, CV_8UC3);  //3チャンネル

    //③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);

    //④動画処理用無限ループ
    while (1) {
        //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        //ビデオが終了したら無限ループから脱出
        if (originalImage.data==NULL) {
            break;
        }
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);
        
        //(b)"frameImage"の表示
        cv::imshow("Frame", frameImage);

        //(c)キー入力待ち
        int key = cv::waitKey(10);
        //[Q]が押されたら無限ループ脱出
        if (key=='q')
            break;
    }
    
    //⑤終了処理
    //カメラ終了
    capture.release();
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
