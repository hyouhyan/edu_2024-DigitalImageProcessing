// g++ dip02.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, const char *argv[]) {

    // ①画像ファイルの読み込み
    cv::Mat sourceImage = cv::imread("color2.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data == 0) { // 画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);

    // ③ウィンドウの生成と移動
    cv::namedWindow("Source");
    cv::moveWindow("Source", 0, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 400, 0);

    // cv::cvtColor()関数で画像を HSV 画像に変換
    // hsvImage
    cv::Mat hsvImage;
    cv::cvtColor(sourceImage, hsvImage, cv::COLOR_BGR2HSV);

    // 色相(H)成分:s[0]，彩度 (S)成分:s[1]，明度(V)成分:s[2]の値を調べて緑色の部分だけを残した HSV 画像を生成
    cv::Mat resultImage;
    resultImage = hsvImage.clone();
    for (int y = 0; y < hsvImage.rows; y++) {
        for (int x = 0; x < hsvImage.cols; x++) {
            cv::Vec3b s = hsvImage.at<cv::Vec3b>(y, x);
            if (!(65 < s[0] && s[0] < 85)) {
                resultImage.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
            }
        }
    }

    // cv::cvtColor()関数で画像を BGR 画像に変換
    cv::cvtColor(resultImage, resultImage, cv::COLOR_HSV2BGR);

    // ⑤ウィンドウへの画像の表示
    cv::imshow("Source", sourceImage);
    cv::imshow("Result", resultImage);

    // ⑥キー入力待ち
    cv::waitKey(0);

    // ⑦画像の保存
    cv::imwrite("result.jpg", resultImage);

    return 0;
}
