#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, const char *argv[]) {

    // 画像ファイルの読み込み
    cv::Mat sourceImage = cv::imread("color1.jpg", cv::IMREAD_COLOR);
    cv::Mat sourceImage2 = cv::imread("color2.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data == 0) { // 画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);

    // 画像格納用オブジェクト"resultImage"の生成
    cv::Mat resultImage = cv::Mat(sourceImage.size(), CV_8UC3);

    // ウィンドウの生成と移動
    cv::namedWindow("Source");
    cv::moveWindow("Source", 0, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 400, 0);

    // sourceImageとsourceImage2を市松模様にして、resultImageに格納
    for (int y = 0; y < sourceImage.rows; y++) {
        for (int x = 0; x < sourceImage.cols; x++) {
            if ((x / (sourceImage.cols/4) + y / (sourceImage.rows/4)) % 2 == 0) {
                resultImage.at<cv::Vec3b>(y, x) = sourceImage.at<cv::Vec3b>(y, x);
            } else {
                resultImage.at<cv::Vec3b>(y, x) = sourceImage2.at<cv::Vec3b>(y, x);
            }
        }
    }
    // ウィンドウへの画像の表示
    cv::imshow("Source", sourceImage);
    cv::imshow("Result", resultImage);

    // キー入力待ち
    cv::waitKey(0);

    // 画像の保存
    cv::imwrite("result.jpg", resultImage);

    return 0;
}
