#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, const char *argv[]) {

    // 画像ファイルの読み込み
    cv::Mat sourceImage = cv::imread("./src/color1.jpg", cv::IMREAD_COLOR);
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

    // 画像の画素単位の読み込み・処理・書き込み
    cv::Vec3b s; // 画素値格納用("s[0]":B, "s[1]":G, "s[2]":R)
    for (int y = 0; y < sourceImage.rows; y++) {
        for (int x = 0; x < sourceImage.cols; x++) {
            s = sourceImage.at<cv::Vec3b>(y, x); //"sourceImage"の画素(x,y)の画素値を読み込んで"s"に格納
            float mx = sourceImage.cols/2.0;
            float my = sourceImage.rows/2.0;
            // 座標(mx, my)と画像中の各点の座標(x, y)との距離の r を計算する
            int r = sqrt((x - mx) * (x - mx) + (y - my) * (y - my));
            cv::Vec3i s1;
            s1[0] = s[0] - r/2;
            s1[1] = s[1] - r/2;
            s1[2] = s[2] - r/2;
            s = s1;
            resultImage.at<cv::Vec3b>(y, x) = s; //"resultImage"の画素(x,y)に画素値"s"を書き込み
        }
    }
    // ウィンドウへの画像の表示
    cv::imshow("Source", sourceImage);
    cv::imshow("Result", resultImage);

    // キー入力待ち
    cv::waitKey(0);

    // 画像の保存
    cv::imwrite("./dst/dip02_kadai1.jpg", resultImage);

    return 0;
}
