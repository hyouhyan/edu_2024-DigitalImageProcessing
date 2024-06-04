//(OpenCV4) g++ -std=c++11 dip07a.cpp `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip07a.cpp `pkg-config --cflags --libs opencv`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, char* argv[])
{
    //①ビデオキャプチャの初期化
    cv::VideoCapture capture("walking.mov");  //ビデオファイルをオープン
    if (capture.isOpened()==0) {
        printf("Camera not found\n");
        return -1;
    }
    
    //②画像格納用インスタンス準備
    cv::Size imageSize(720, 405);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3);
    cv::Mat backImage(imageSize, CV_8UC3);
    cv::Mat subImage(imageSize, CV_8UC3);
    
    //③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Back");
    cv::moveWindow("Back", 50, 50);
    cv::namedWindow("Subtraction");
    cv::moveWindow("Subtraction", 100, 100);
    
    //④動画処理用無限ループ
    while (1) {
        //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        //ビデオが終了したら無限ループから脱出
        if (originalImage.data==NULL) break;
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);
        
        //(b)"frameImage"と"backImage"との差分画像"subImage"の生成
        
        //(c)"frameImage"，"backImage"，"subImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Back", backImage);
        cv::imshow("Subtraction", subImage);
        
        //(d)"frameImage"で"backImage"を更新
        
        //(e)キー入力待ち
        int key = cv::waitKey(20);
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
