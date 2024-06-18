#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, const char* argv[])
{
    cv::Mat sourceImage = cv::imread("./src/kadai01-2.png", cv::IMREAD_COLOR);
    if (sourceImage.data==0) { //画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows); //横幅と高さ
    
    // 単純にグレースケール化
    cv::Mat grayImage;
    cv::cvtColor(sourceImage, grayImage, cv::COLOR_BGR2GRAY);

    //RGBをそれぞれの画像に分解
    cv::Mat bgr[3];
    cv::split(sourceImage, bgr);
    cv::Mat blueImage = bgr[0];
    cv::Mat greenImage = bgr[1];
    cv::Mat redImage = bgr[2];

    //Gにおいて、閾値"1"で2値化
    cv::Mat binImage;
    cv::threshold(greenImage, binImage, 1, 255, cv::THRESH_BINARY);







    // 画像の表示
    cv::imshow("Source Image", sourceImage);
    cv::imshow("BIN Image", binImage);

    cv::waitKey(0);

    
    return 0;
}
