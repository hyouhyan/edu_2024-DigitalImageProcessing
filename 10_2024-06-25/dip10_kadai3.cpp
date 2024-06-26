// g++ dip10_gaussian_filter.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

// 配列の象限入れ替え用関数の宣言
void ShiftDFT(const cv::Mat& src_arr, cv::Mat& dst_arr);
void applyGaussianLowPassFilter(cv::Mat& ftMatrix, double sigma);
void applyGaussianHighPassFilter(cv::Mat& ftMatrix, double sigma);
cv::Mat calculateMagnitude(const cv::Mat& complexImg);

int main(int argc, const char* argv[])
{
    // 画像の読み込み
    cv::Mat sourceImg = cv::imread("./src/nymegami.jpg", cv::IMREAD_GRAYSCALE);
    if (sourceImg.empty()) {
        std::cerr << "Error loading image" << std::endl;
        return -1;
    }

    // 画像を保存
    cv::imwrite("./dst/dip10_kadai3/original.jpg", sourceImg);

    // 作業用配列領域の宣言
    cv::Mat cxMatrix(sourceImg.size(), CV_64FC2); // 複素数用(実数 2 チャンネル)
    cv::Mat ftMatrix(sourceImg.size(), CV_64FC2); // 複素数用(実数 2 チャンネル)
    cv::Mat resultImg(sourceImg.size(), CV_8UC1); // 逆変換画像(自然数)

    // 原画像を複素数(実数部と虚数部)の 2 チャンネル配列として表現．虚数部はゼロ
    cv::Mat imgMatrix[] = {cv::Mat_<double>(sourceImg), cv::Mat::zeros(sourceImg.size(),CV_64FC1)};
    // 実数部と虚数部を一組にした 2 チャンネル画像 cxMatrix を生成
    cv::merge(imgMatrix, 2, cxMatrix);

    // フーリエ変換
    cv::dft(cxMatrix, ftMatrix);
    ShiftDFT(ftMatrix, ftMatrix);

    // ローパスフィルタの適用
    double sigma = 30.0;
    applyGaussianLowPassFilter(ftMatrix, sigma);
    cv::Mat lowPassFreqImg = calculateMagnitude(ftMatrix);
    cv::imwrite("./dst/dip10_kadai3/lowpass_freq.jpg", lowPassFreqImg);

    // フーリエ逆変換
    ShiftDFT(ftMatrix, ftMatrix);
    cv::idft(ftMatrix, cxMatrix);
    cv::split(cxMatrix, imgMatrix);
    cv::normalize(imgMatrix[0], resultImg, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::imwrite("./dst/dip10_kadai3/lowpass_spatial.jpg", resultImg);

    // ハイパスフィルタの適用
    cv::dft(cxMatrix, ftMatrix);  // フーリエ変換し直す
    ShiftDFT(ftMatrix, ftMatrix);
    applyGaussianHighPassFilter(ftMatrix, sigma);
    cv::Mat highPassFreqImg = calculateMagnitude(ftMatrix);
    cv::imwrite("./dst/dip10_kadai3/highpass_freq.jpg", highPassFreqImg);

    // フーリエ逆変換
    ShiftDFT(ftMatrix, ftMatrix);
    cv::idft(ftMatrix, cxMatrix);
    cv::split(cxMatrix, imgMatrix);
    cv::normalize(imgMatrix[0], resultImg, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::imwrite("./dst/dip10_kadai3/highpass_spatial.jpg", resultImg);

    return 0;
}

// 画像の象限入れ替え用関数
void ShiftDFT(const cv::Mat& src_arr, cv::Mat& dst_arr)
{
    int cx = src_arr.cols / 2;
    int cy = src_arr.rows / 2;

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

// ガウシアンローパスフィルタの適用
void applyGaussianLowPassFilter(cv::Mat& ftMatrix, double sigma)
{
    int rows = ftMatrix.rows;
    int cols = ftMatrix.cols;
    int centerX = cols / 2;
    int centerY = rows / 2;

    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < rows; j++) {
            double distance = sqrt(pow(i - centerX, 2) + pow(j - centerY, 2));
            double value = exp(-(distance * distance) / (2 * sigma * sigma));
            ftMatrix.at<cv::Vec2d>(j, i) *= value;
        }
    }
}

// ガウシアンハイパスフィルタの適用
void applyGaussianHighPassFilter(cv::Mat& ftMatrix, double sigma)
{
    int rows = ftMatrix.rows;
    int cols = ftMatrix.cols;
    int centerX = cols / 2;
    int centerY = rows / 2;

    for (int i = 0; i < cols; i++) {
        for (int j = 0; j < rows; j++) {
            double distance = sqrt(pow(i - centerX, 2) + pow(j - centerY, 2));
            double value = 1.0 - exp(-(distance * distance) / (2 * sigma * sigma));
            ftMatrix.at<cv::Vec2d>(j, i) *= value;
        }
    }
}

// 複素数画像の振幅（マグニチュード）を計算
cv::Mat calculateMagnitude(const cv::Mat& complexImg)
{
    cv::Mat planes[2];
    cv::split(complexImg, planes);
    cv::Mat mag;
    cv::magnitude(planes[0], planes[1], mag);
    cv::normalize(mag, mag, 0, 255, cv::NORM_MINMAX, CV_8U);
    return mag;
}
