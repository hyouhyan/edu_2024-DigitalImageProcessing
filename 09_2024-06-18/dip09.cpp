/*
g++ dip09.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
g++ dip09.cpp `pkg-config --cflags --libs opencv`
 */

#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, char* argv[])
{
    //①ビデオキャプチャの初期化
    cv::VideoCapture capture("scene_ait.mov");  //ビデオファイルをオープン
    if (capture.isOpened()==0) {
        printf("Capture not found\n");
        return -1;
    }
    
    //②画像格納用インスタンス準備
    int w = capture.get(cv::CAP_PROP_FRAME_WIDTH);  //captureから動画横サイズ取得
    int h = capture.get(cv::CAP_PROP_FRAME_HEIGHT);  //captureから動画縦サイズ取得
    cv::Size imageSize(w, h);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3);  //3チャンネル
    cv::Mat grayImage(imageSize, CV_8UC1);  //1チャンネル
    cv::Mat edgeImage(imageSize, CV_8UC1);  //1チャンネル
    
    //③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Edge");
    cv::moveWindow("Edge", 100, 100);
    
    //④ハフ変換用変数

    
    //⑤動画処理用無限ループ
    while (1) {
        //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        //ビデオが終了したら巻き戻し
        if (originalImage.data==NULL) {
            capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);
        
        //(b)"frameImage"からグレースケール画像"grayImage"を生成
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);
        
        //(c)"grayImage"からエッジ画像"edgeImage"を生成

        
        //(d)"edgeImage"に直線検出ハフ変換を施して，閾値(250)以上の投票数を得た直線群(ρ,θ)を"lines"に格納

        
        //(e)ハフ変換結果表示
        //検出された直線の数("lines.size()")と閾値(100)の小さい方の数だけ繰り返し

        
        //(f)"frameImage"，"edgeImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Edge", edgeImage);
        
        //(g)キー入力待ち
        int key = cv::waitKey(10);
        //[Q]が押されたら無限ループ脱出
        if (key=='q')
            break;
    }
    
    //⑥終了処理
    //カメラ終了
    capture.release();
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
