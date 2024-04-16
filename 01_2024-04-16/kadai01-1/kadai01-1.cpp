#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, const char* argv[])
{
    cv::Mat sourceImage = cv::imread("photo.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data==0) { //画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows); //横幅と高さ
    
    //インスタンスの生成
    cv::Mat binImage;

    //グレースケール変換
    cv::Mat grayImage;
    cv::cvtColor(sourceImage, grayImage, cv::COLOR_BGR2GRAY);

    //2値化
    cv::threshold(grayImage, binImage, 128, 255, cv::THRESH_BINARY);

    cv::imshow("Source Image", sourceImage);
    cv::imshow("Gray Image", grayImage);
    cv::imshow("Binary Image", binImage);

    cv::waitKey(0);

    
    return 0;
}
