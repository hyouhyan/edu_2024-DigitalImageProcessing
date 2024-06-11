//(OpenCV4) g++ dip08-1.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, const char * argv[])
{
    //ビデオファイル"colball.mov"を取り込み
    cv::VideoCapture capture("colball.mov");
    if (capture.isOpened()==0) {
        printf("No video\n");
        return -1;
    }
    //フレームサイズ取得
    int width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    printf("Frame Size = (%d %d)\n", width, height);
    
    //合成用画像"face.jpg"の読み込み
    cv::Mat compImage = cv::imread("face.jpg", cv::IMREAD_COLOR);
    if (compImage.data==0) {
        printf("No image\n");
        exit(0);
    }
    printf("Image Size = (%d, %d)\n", compImage.cols, compImage.rows);

    //画像格納用インスタンス準備
    cv::Mat frameImage, hsvImage;
    cv::Mat binImage(cv::Size(width, height), CV_8UC1);;  //領域用
    
    //ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Image");
    cv::moveWindow("Image", width, 0);
    cv::namedWindow("Bin");
    cv::moveWindow("Bin", 0, height);

    //領域膨張収縮用構造要素
    cv::Mat element1 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3,3));
    cv::Mat element2 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5));

    //合成用画像の表示
    cv::imshow("Image", compImage);

    //ビデオライタ生成(ファイル名，コーデック(mp4v/mov)，フレームレート，フレームサイズ)
    //cv::VideoWriter rec("rec.mov", cv::VideoWriter::fourcc('m','p','4','v'), 30, cv::Size(width, height));

    //動画像処理無限ループ
    while (1) {
        //カメラから1フレーム読み込み
        capture >> frameImage;
        if(frameImage.data == NULL) break;
        
        //色空間変換(BGR -> HSV)
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);
        
        //ボール領域画像リセット
        binImage = 0;
        
        //ボール領域抽出
        for (int y=0; y<height; y++) {
            for (int x=0; x<width; x++) {
                //"hsvImage"の画素(x,y)の画素値s取得
                cv::Vec3b s = hsvImage.at<cv::Vec3b>(y, x);
                
                //HSVの値を用いてボール抽出
                if (s[0]>90 && s[0]<110 && s[1]>120 && s[2]>64) {  //Blue
                    binImage.at<unsigned char>(y, x) = 255;
                }
                else if (s[0]>60 && s[0]<80 && s[1]>120 && s[2]>64) {  //Green
                    binImage.at<unsigned char>(y, x) = 255;
                }
                else if (((s[0]>0 && s[0]<5) || (s[0]>160 && s[0]<180)) && s[1]>110 && s[2]>64) {  //Red
                    binImage.at<unsigned char>(y, x) = 255;
                }
                else if (s[0]>20 && s[0]<30 && s[1]>160 && s[2]>64) {  //Yellow
                    binImage.at<unsigned char>(y, x) = 255;
                }
            }
        }
        
        //収縮膨張による各ボール領域のノイズ・穴除去
        cv::erode(binImage, binImage, element1, cv::Point(-1,-1), 1);  //収縮
        cv::dilate(binImage, binImage, element2, cv::Point(-1,-1), 2);  //膨張
        
        //フレーム画像および領域画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Bin", binImage);

        //キー入力待ち
        char key = cv::waitKey(20);  //20ミリ秒待機
        if(key == 'q') break;
        
        //動画ファイル書き出し
        //rec << frameImage;  //ビデオライタに画像出力
    }
    
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
