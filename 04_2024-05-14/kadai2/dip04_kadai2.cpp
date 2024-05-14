#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, const char* argv[])
{
    int width = 640, height = 480;
    
    //カメラの初期化
    cv::VideoCapture capture(0);  //カメラ0番をオープン
    //カメラがオープンできたかどうかをチェック
    if (capture.isOpened()==0) {
        printf("Camera not found\n");
        return -1;
    }
    
    //画像格納用インスタンス準備
    cv::Mat captureImage;  //キャプチャ用
    cv::Mat frameImage = cv::Mat(cv::Size(width,height), CV_8UC3);  //処理用
    cv::Mat grayImage(cv::Size(width,height), CV_8UC1);  //1チャンネル
    cv::Mat resultGImage(cv::Size(width,height), CV_8UC1);  //1チャンネル
    cv::Mat recImage(cv::Size(width,height), CV_8UC3);  //3チャンネル
    
    //ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", width, height);
    
    //ルックアップテーブルの作成
    unsigned char lookupTable[256];

    for(int i=0; i<256; i++){
        //hsvをアニメ調に変換するためのルックアップテーブルを作成
        lookupTable[i] = (i/43)*43;
    }
    
    //ビデオライタ生成(ファイル名，コーデック，フレームレート，フレームサイズ)
    cv::VideoWriter rec("rec.mp4", cv::VideoWriter::fourcc('M','P','4','V'), 30, recImage.size());

    //hsvイメージを格納するためのインスタンス
    cv::Mat hsvImage(cv::Size(width,height), CV_8UC3);
    
    //動画像処理無限ループ
    while (1) {
        //カメラから1フレームを" captureImage"に読み込み
        capture >> captureImage;
        if (captureImage.data==NULL) break;
        
        //"captureImage"をリサイズして"frameImage"に格納
        cv::resize(captureImage, frameImage, frameImage.size());
        
        //hsvに変換
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);
        
        //ルックアップテーブルを用いて、hsvを変換
        for(int j=0; j<height; j++){
            for(int i=0; i<width; i++){
                unsigned char h = hsvImage.at<cv::Vec3b>(j, i)[0];
                unsigned char s = hsvImage.at<cv::Vec3b>(j, i)[1];
                unsigned char v = hsvImage.at<cv::Vec3b>(j, i)[2];
                
                h = lookupTable[h];
                s = lookupTable[s];
                v = lookupTable[v];
                
                hsvImage.at<cv::Vec3b>(j, i)[0] = h;
                hsvImage.at<cv::Vec3b>(j, i)[1] = s;
                hsvImage.at<cv::Vec3b>(j, i)[2] = v;
            }
        }

        //hsvからbgrに変換
        cv::cvtColor(hsvImage, resultGImage, cv::COLOR_HSV2BGR);
        
        //ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Result", resultGImage);
        
        //動画ファイル書き出し
        // cv::cvtColor(resultGImage, recImage, cv::COLOR_BGR2RGB);  //BGR→RGB
        rec << resultGImage;  //ビデオライタに画像出力
        
        //キー入力待ち
        char key = cv::waitKey(20);  //20ミリ秒待機
        if (key=='q') break;
    }
    
    return 0;
}
