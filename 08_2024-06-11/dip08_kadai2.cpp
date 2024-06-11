#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ
#include <vector>
#include <numeric>
#include <cmath>

void removeGreenBackground(cv::Mat& image) {
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            cv::Vec3b color = image.at<cv::Vec3b>(y, x);
            if (color[1] > 200 && color[0] < 100 && color[2] < 100) { // 緑色の判定
                image.at<cv::Vec3b>(y, x)[0] = 0;
                image.at<cv::Vec3b>(y, x)[1] = 0;
                image.at<cv::Vec3b>(y, x)[2] = 0;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    // ビデオキャプチャを初期化して，映像を取り込む
    cv::VideoCapture capture("./src/water1.mov");  // 指定したビデオファイルをオープン
    if (!capture.isOpened()) {
        printf("Camera not found\n");
        return -1;
    }
    // フレームサイズ取得
    int width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    printf("Frame Size = (%d %d)\n", width, height);
    cv::Size imageSize(width, height);  // フレームと同じ画像サイズ定義

    // 船画像"ship.jpg"の読み込み
    cv::Mat shipImage = cv::imread("./src/ship.jpg", cv::IMREAD_COLOR);
    if (shipImage.empty()) {
        printf("No image\n");
        exit(0);
    }
    removeGreenBackground(shipImage); // 背景の緑色を削除

    // 画像格納用インスタンス準備
    cv::Mat frameImage;
    cv::Mat presentImage(imageSize, CV_8UC1), priorImage(imageSize, CV_8UC1);  // 現フレーム濃淡画像，前フレーム濃淡画像
    std::vector<cv::Point2f> presentFeature, priorFeature;  // 現フレーム対応点，前フレーム追跡点
    std::vector<unsigned char> status;  // 処理用
    std::vector<float> errors;  // 処理用

    // 船の初期位置
    cv::Point2f shipPoint(130, 190);
    cv::Point2f shipVelocity(0, 0); // 船の速度初期値

    // ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);

    // 船の近傍に追跡点を配置
    for (int i = -20; i <= 20; i += 10) {
        for (int j = -20; j <= 20; j += 10) {
            priorFeature.push_back(shipPoint + cv::Point2f(i, j));
        }
    }

    // mp4での動画書き出し
    // ファイルサイズを小さくするために、ビデオコーデックをH.264に指定
    cv::VideoWriter rec("./dst/dip08_kadai2_k22047.mp4", cv::VideoWriter::fourcc('H', '2', '6', '4'), 30, imageSize);

    // オプティカルフロー用終了条件
    cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 30, 0.01);

    // 動画像処理無限ループ
    while (true) {
        // カメラから1フレーム読み込み
        capture >> frameImage;
        if (frameImage.empty()) break;

        // オプティカルフロー
        cv::cvtColor(frameImage, presentImage, cv::COLOR_BGR2GRAY);  // 現フレーム濃淡画像"presentImage"を生成

        // 初回フレームでは初期化
        int opCnt = priorFeature.size();
        if (opCnt > 0) {
            cv::calcOpticalFlowPyrLK(priorImage, presentImage, priorFeature, presentFeature, status, errors, cv::Size(10, 10), 4, criteria);
            
            // オプティカルフローのベクトルを収集
            std::vector<cv::Point2f> flowVectors;
            for (int i = 0; i < opCnt; i++) {
                if (status[i]) {
                    flowVectors.push_back(presentFeature[i] - priorFeature[i]);
                }
            }

            // 平均と標準偏差を計算
            cv::Point2f mean(0, 0);
            for (const auto& vec : flowVectors) {
                mean += vec;
            }
            mean *= (1.0 / flowVectors.size());

            cv::Point2f stddev(0, 0);
            for (const auto& vec : flowVectors) {
                stddev.x += (vec.x - mean.x) * (vec.x - mean.x);
                stddev.y += (vec.y - mean.y) * (vec.y - mean.y);
            }
            stddev.x = std::sqrt(stddev.x / flowVectors.size());
            stddev.y = std::sqrt(stddev.y / flowVectors.size());

            // フィルタリング
            cv::Point2f aveOp(0, 0);
            int validPoints = 0;
            for (const auto& vec : flowVectors) {
                if (std::abs(vec.x - mean.x) <= 2 * stddev.x && std::abs(vec.y - mean.y) <= 2 * stddev.y) {
                    aveOp += vec;
                    validPoints++;
                }
            }
            if (validPoints > 0) {
                aveOp *= (1.0 / validPoints);
                shipVelocity = 0.5 * shipVelocity + 0.5 * aveOp; // 速度の更新
            }

            shipPoint += shipVelocity; // 位置の更新

            // オプティカルフローの描画
            for (int i = 0; i < opCnt; i++) {
                if (status[i]) {
                    cv::Point pt1 = cv::Point(priorFeature[i]);  //前フレーム追跡点
                    cv::Point pt2 = cv::Point(presentFeature[i]);  //現フレーム対応点
                    // cv::line(frameImage, pt1, pt2, cv::Scalar(0, 0, 255), 2, 8);  //pt1とpt2を結ぶ直線を描画
                }
            }
        }
        presentImage.copyTo(priorImage);  //"priorImage"を"presentImage"で更新
        priorFeature = presentFeature;  //"priorFeature"を"presentFeature"で更新

        //追跡点を更新（船の近傍に再配置）
        priorFeature.clear();
        for (int i = -20; i <= 20; i += 10) {
            for (int j = -20; j <= 20; j += 10) {
                priorFeature.push_back(shipPoint + cv::Point2f(i, j));
            }
        }

        // 船の回転角度を計算
        // 角度の範囲は[-45, 45]とする
        float angle = std::atan2(shipVelocity.y, shipVelocity.x) * 180 / M_PI;
        if (angle < -45) angle = -45;
        if (angle > 45) angle = 45;

        // 船画像のアフィン変換行列を計算
        cv::Mat rotationMatrix = cv::getRotationMatrix2D(cv::Point2f(shipImage.cols / 2, shipImage.rows / 2), angle, 1.0);
        cv::Mat rotatedShip;
        cv::warpAffine(shipImage, rotatedShip, rotationMatrix, shipImage.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));

        // 船の位置に円を表示
        // cv::circle(frameImage, shipPoint, 5, cv::Scalar(0, 255, 0), -1, 8);

        // 船画像の合成
        for (int y = 0; y < rotatedShip.rows; y++) {
            for (int x = 0; x < rotatedShip.cols; x++) {
                cv::Vec3b color = rotatedShip.at<cv::Vec3b>(y, x);
                if (color != cv::Vec3b(0, 0, 0)) {  // 黒色でないピクセルを背景画像にコピー
                    int fx = shipPoint.x - rotatedShip.cols / 2 + x;
                    int fy = shipPoint.y - rotatedShip.rows / 2 + y;
                    if (fx >= 0 && fx < frameImage.cols && fy >= 0 && fy < frameImage.rows) {
                        frameImage.at<cv::Vec3b>(fy, fx) = color;
                    }
                }
            }
        }

        // ウィンドウに画像表示
        cv::imshow("Frame", frameImage);

        // キー入力待ち
        char key = cv::waitKey(20);  //20ミリ秒待機
        if (key == 'q') break;

        // 動画ファイル書き出し
        rec << frameImage;  //ビデオライタに画像出力
    }

    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
