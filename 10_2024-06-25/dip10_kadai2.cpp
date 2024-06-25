//g++ dip10_video.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

// 配列の象限入れ替え用関数の宣言
void ShiftDFT(const cv::Mat& src_arr, cv::Mat& dst_arr);
void applyHighPassFilter(cv::Mat& ftMatrix, double radius);
void applyBandPassFilter(cv::Mat& ftMatrix, double lowRadius, double highRadius);

int main(int argc, const char* argv[])
{
    // ビデオキャプチャの初期化
    cv::VideoCapture cap(0);  // カメラをオープン
    if (!cap.isOpened()) {
        std::cerr << "Error opening video stream" << std::endl;
        return -1;
    }

    // ビデオライターの初期化
    int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    cv::VideoWriter video("output.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, cv::Size(frame_width, frame_height), false);

    // フレーム処理ループ
    int frameCount = 0;
    while (frameCount < 100) {  // 10秒間 (10 fps * 10秒 = 100フレーム)
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            break;
        }

        // グレースケール変換
        cv::Mat grayFrame;
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

        // 作業用配列領域の宣言
        cv::Mat cxMatrix(grayFrame.size(), CV_64FC2); // 複素数用(実数 2 チャンネル)
        cv::Mat ftMatrix(grayFrame.size(), CV_64FC2); // 複素数用(実数 2 チャンネル)
        cv::Mat resultImg(grayFrame.size(), CV_8UC1); // 逆変換画像(自然数)
        
        // 原画像を複素数(実数部と虚数部)の 2 チャンネル配列として表現．虚数部はゼロ
        cv::Mat imgMatrix[] = {cv::Mat_<double>(grayFrame), cv::Mat::zeros(grayFrame.size(),CV_64FC1)};
        // 実数部と虚数部を一組にした 2 チャンネル画像 cxMatrix を生成
        cv::merge(imgMatrix, 2, cxMatrix);
        
        // フーリエ変換
        cv::dft(cxMatrix, ftMatrix);
        ShiftDFT(ftMatrix, ftMatrix);
        
        // ハイパスフィルタの適用
        double highRadius = 30.0;  // 適用する半径
        applyHighPassFilter(ftMatrix, highRadius);
        
        // フーリエ逆変換
        ShiftDFT(ftMatrix, ftMatrix);
        cv::idft(ftMatrix, cxMatrix);
        cv::split(cxMatrix, imgMatrix);
        cv::normalize(imgMatrix[0], resultImg, 0, 255, cv::NORM_MINMAX, CV_8U);
        
        // 2値化処理
        cv::Mat binaryImg;
        cv::threshold(resultImg, binaryImg, 128, 255, cv::THRESH_BINARY);

        // 結果をビデオに保存
        video.write(binaryImg);

        // フレーム数をカウント
        frameCount++;
    }

    // リソースの解放
    cap.release();
    video.release();

    return 0;
}

// 画像の象限入れ替え用関数
void ShiftDFT(const cv::Mat& src_arr, cv::Mat& dst_arr)
{
    int cx = src_arr.cols/2;
    int cy = src_arr.rows/2;
    
    cv::Mat q1(src_arr, cv::Rect(cx, 0, cx, cy));
    cv::Mat q2(src_arr, cv::Rect(0, 0, cx, cy));
    cv::Mat q3(src_arr, cv::Rect(0, cy, cx, cy));
    cv::Mat q4(src_arr, cv::Rect(cx, cy, cx, cy));
    
    cv::Mat tmp;
    q1.copyTo(tmp);
    q3.copyTo(q1);
    tmp.copyTo(q3);
    
    q2.copyTo(tmp);
    q4.copyTo(q2);
    tmp.copyTo(q4);
}

// ハイパスフィルタの適用
void applyHighPassFilter(cv::Mat& ftMatrix, double radius)
{
    cv::Vec2d s(0.0, 0.0);
    int rows = ftMatrix.rows;
    int cols = ftMatrix.cols;
    int centerX = cols / 2;
    int centerY = rows / 2;
    
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < rows; j++) {
            double distance = sqrt(pow(i - centerX, 2) + pow(j - centerY, 2));
            if (distance < radius) {
                ftMatrix.at<cv::Vec2d>(j, i) = s;
            }
        }
    }
}

// バンドパスフィルタの適用 (オプション)
void applyBandPassFilter(cv::Mat& ftMatrix, double lowRadius, double highRadius)
{
    cv::Vec2d s(0.0, 0.0);
    int rows = ftMatrix.rows;
    int cols = ftMatrix.cols;
    int centerX = cols / 2;
    int centerY = rows / 2;
    
    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < rows; j++) {
            double distance = sqrt(pow(i - centerX, 2) + pow(j - centerY, 2));
            if (distance < lowRadius || distance > highRadius) {
                ftMatrix.at<cv::Vec2d>(j, i) = s;
            }
        }
    }
}
