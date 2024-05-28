#include <iostream>           //入出力関連ヘッダ
#include <math.h>             //数学関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, const char *argv[]) {
    // ①ルートディレクトリの画像ファイル"col.jpg"を読み込んで"sourceImage"に格納
    cv::Mat sourceImage = cv::imread("kadai.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data == 0) { // 画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);

    // ②画像格納用インスタンスの生成
    cv::Mat grayImage(sourceImage.size(),
                      CV_8UC1); // グレースケール画像用（1チャンネル）
    cv::Mat binImage(sourceImage.size(), CV_8UC1); // ２値画像用（1チャンネル）
    cv::Mat contourImage(sourceImage.size(),
                         CV_8UC3); // 輪郭表示画像用（3チャンネル）

    // ③原画像をグレースケール画像に、グレースケール画像を２値画像に変換
    //"sourceImage"をグレースケール画像に変換して"grayImage"に出力
    cv::cvtColor(sourceImage, grayImage, cv::COLOR_BGR2GRAY);
    //"grayImage"を2値化して"grayImage"に出力
    cv::threshold(grayImage, binImage, 50, 255, cv::THRESH_BINARY);
    //"sourceImage"のコピーを"contourImage"に出力
    contourImage = sourceImage.clone();

    //膨張収縮処理
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::dilate(binImage, binImage, element, cv::Point(-1, -1), 1);
    cv::erode(binImage, binImage, element, cv::Point(-1, -1), 4);


    // ④輪郭点格納用配列、輪郭の階層用配列の確保
    // 輪郭点格納用配列
    std::vector<std::vector<cv::Point>> contours;
    std::vector<std::vector<cv::Point>> circleContours;

    // ⑤"binImage"からの輪郭抽出処理
    // findContours()では入力画像が壊れるので、tmpImage
    // に一時退避。壊れても良いなら不要。
    cv::Mat tmpImage = binImage.clone();

    // 輪郭抽出処理.輪郭ごとに輪郭画素位置を"counter"に格納
    cv::findContours(binImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

    double circularity;

    int circleCount = 0;

    // 輪郭データ"contour"を順次描画
    for (int i = 0; i < contours.size(); i++) {
        // 円形度を計算
        circularity = 4.0 * M_PI * cv::contourArea(contours[i]) / (cv::arcLength(contours[i], true) * cv::arcLength(contours[i], true));


        // 円形度が一定以上かつ、一定面積以上の図形のみを描画
        if (circularity >= 0.8) {
            cv::drawContours(contourImage, contours, i, cv::Scalar(255, 255, 255), 5, 8);
            circleCount++;
            printf("円%d: Length = %.0f, Area = %.0f\n", circleCount, cv::arcLength(contours[i], true), cv::contourArea(contours[i]));
        }
    }
    // ⑥ウィンドウを生成して各画像を表示
    // 原画像
    cv::namedWindow("Source");       // ウィンドウの生成
    cv::moveWindow("Source", 0, 50); // ウィンドウの表示位置の指定
    cv::imshow("Source", sourceImage); // ウィンドウに画像を表示
                                       // グレースケール(2値化)
    cv::namedWindow("Gray");         // ウィンドウの生成
    cv::moveWindow("Gray", 150, 50); // ウィンドウの表示位置の指定
    cv::imshow("Gray", grayImage);   // ウィンドウに画像を表示
                                   // 輪郭画像(原画像に輪郭を追加)
    cv::namedWindow("Contour");         // ウィンドウの生成
    cv::moveWindow("Contour", 300, 50); // ウィンドウの表示位置の指定
    cv::imshow("Contour", contourImage); // ウィンドウに画像を表示

    cv::namedWindow("Binary");         // ウィンドウの生成
    cv::moveWindow("Binary", 450, 50); // ウィンドウの表示位置の指定
    cv::imshow("Binary", binImage); // ウィンドウに画像を表示

    // 画像の出力
    cv::imwrite("dip06_kadai2_result.jpg", contourImage);

    // ⑦キー入力があるまでここでストップ
    cv::waitKey(0);

    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
