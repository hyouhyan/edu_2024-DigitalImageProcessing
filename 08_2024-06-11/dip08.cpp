//(OpenCV4) g++ dip08.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main(int argc, const char* argv[])
{
    //①画像ファイルの読み込み
    //画像ファイル"ferarri.jpg"を読み込んで，画像データ"sourceImage"に格納
    cv::Mat sourceImage = cv::imread("ferarri.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data==0) {  //画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);
    
    //②画像表示用ウィンドウの生成
    cv::namedWindow("Translate");
    
    //③3チャンネル画像"translateImage"の確保（画像ファイルと同サイズ）
    //"sourceImage"と同サイズ・3チャンネル・ゼロで初期化
    cv::Mat translateImage = cv::Mat::zeros(sourceImage.size(), CV_8UC3);
    
    //④回転移動行列"rotateMat"の生成
    cv::Point2f center = cv::Point2f(sourceImage.cols/2, sourceImage.rows/2);  //回転中心
    double angle = -45.0;  //回転角度
    double scale = 1.0;  //拡大率
    cv::Mat rotateMat = cv::getRotationMatrix2D(center, angle, scale); //行列生成

    //行列要素表示(確認用)
    printf("%f %f %f\n", rotateMat.at<double>(0, 0), rotateMat.at<double>(0, 1), rotateMat.at<double>(0, 2));
    printf("%f %f %f\n", rotateMat.at<double>(1, 0), rotateMat.at<double>(1, 1), rotateMat.at<double>(1, 2));
    
    //⑤"sourceImage"に回転移動"rotateMat"を施して"translateImage"に張り付け
    //値が決定されない画素は黒で塗りつぶし．
    cv::warpAffine(sourceImage, translateImage, rotateMat, translateImage.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0,0,0));
    
    //⑥"translateImage"の表示
    cv::imshow("Translate", translateImage);
    
    //⑦キー入力待ち
    cv::waitKey(0);
    
    return 0;
}
