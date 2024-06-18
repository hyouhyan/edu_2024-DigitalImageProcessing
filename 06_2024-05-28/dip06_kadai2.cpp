#include <iostream>           //入出力関連ヘッダ
#include <math.h>             //数学関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, const char *argv[]) {
    // ①ルートディレクトリの画像ファイル"col.jpg"を読み込んで"sourceImage"に格納
    cv::Mat sourceImage = cv::imread("./src/kadai.jpg", cv::IMREAD_COLOR);
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

    //画像の背景色を取得
    cv::Vec3b bgColor = sourceImage.at<cv::Vec3b>(0, 0);


    // 輪郭データ"contour"を順次描画
    for (int i = 0; i < contours.size(); i++) {
        // 円形度を計算
        circularity = 4.0 * M_PI * cv::contourArea(contours[i]) / (cv::arcLength(contours[i], true) * cv::arcLength(contours[i], true));

        // 円形度が0.8以上の図形のみをcircleContoursに格納
        // 面積が1000以上
        if (circularity >= 0.8 && cv::contourArea(contours[i]) >= 1000){
            circleContours.push_back(contours[i]);
            circleCount++;

            // 背景色で塗りつぶしを行う
            cv::drawContours(sourceImage, circleContours, -1, bgColor, -1);
        }
    }

    // countourImageをグレースケールに変換
    cv::cvtColor(sourceImage, grayImage, cv::COLOR_BGR2GRAY);

    // 2値化
    cv::threshold(grayImage, binImage, 50, 255, cv::THRESH_BINARY);

    //膨張収縮処理
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::erode(binImage, binImage, element, cv::Point(-1, -1), 1);
    cv::dilate(binImage, binImage, element, cv::Point(-1, -1), 1);

    cv::dilate(binImage, binImage, element, cv::Point(-1, -1), 2);
    cv::erode(binImage, binImage, element, cv::Point(-1, -1), 2);

    tmpImage = binImage.clone();

    cv::findContours(binImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

    for (int i = 0; i < contours.size(); i++) {
        // 円形度を計算
        circularity = 4.0 * M_PI * cv::contourArea(contours[i]) / (cv::arcLength(contours[i], true) * cv::arcLength(contours[i], true));

        printf("circularity: %f\n", circularity);

        // 円形度が0.8以上の図形のみをcircleContoursに格納
        if (circularity >= 0.8  && cv::contourArea(contours[i]) >= 1000 ) {
            circleContours.push_back(contours[i]);
            circleCount++;

            // 背景色で塗りつぶしを行う
            cv::drawContours(sourceImage, circleContours, -1, bgColor, -1);
        }
    }

    // 輪郭データ"circleContour"を順次描画
    for (int i = 0; i < circleContours.size(); i++) {
        // "contourImage"に"contours"の i 番目の領域の輪郭を描画(白色，線幅2，8連結)
        cv::drawContours(contourImage, circleContours, i, cv::Scalar(255, 255, 255), 2, 8);

        printf("円%d: Length=%.0f, Area=%.0f\n", i + 1, cv::arcLength(circleContours[i], true), cv::contourArea(circleContours[i]));

        //binImageを塗りつぶす
        cv::drawContours(binImage, circleContours, i, cv::Scalar(255, 255, 255), -1);
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
    cv::imwrite("./dst/dip06_kadai2_result.jpg", contourImage);

    // binImageの出力
    cv::imwrite("./dst/dip06_kadai2_bin.jpg", binImage);

    // ⑦キー入力があるまでここでストップ
    cv::waitKey(0);

    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
