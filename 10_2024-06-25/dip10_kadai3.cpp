// g++ dip10_gaussian_filter.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

// 配列の象限入れ替え用関数の宣言
void ShiftDFT(const cv::Mat& src_arr, cv::Mat& dst_arr);

int main(int argc, const char* argv[]){
    // 画像の読み込み
    // cv::Mat sourceImg = cv::imread("./src/nymegami.jpg", cv::IMREAD_GRAYSCALE);
    cv::Mat sourceImg = cv::imread("./src/lenna_g.jpg", cv::IMREAD_GRAYSCALE);
    if (sourceImg.empty()) {
        std::cerr << "Error loading image" << std::endl;
        return -1;
    }

    // 画像を保存
    cv::imwrite("./dst/dip10_kadai3/original.jpg", sourceImg);

    // 作業用配列領域の宣言
    cv::Mat cxMatrix(sourceImg.size(), CV_64FC2); //複素数用(実数 2 チャンネル)
    cv::Mat ftMatrix(sourceImg.size(), CV_64FC2); //複素数用(実数 2 チャンネル)
    cv::Mat spcMatrix(sourceImg.size(), CV_64FC1); //スペルトルデータ(実数)
    
    cv::Mat spcImg(sourceImg.size(), CV_8UC1); //スペクトル画像(自然数)
    cv::Mat resultImg(sourceImg.size(), CV_8UC1); //逆変換画像(自然数)

    // 原画像を複素数(実数部と虚数部)の 2 チャンネル配列として表現．虚数部はゼロ
    cv::Mat imgMatrix[] = {cv::Mat_<double>(sourceImg), cv::Mat::zeros(sourceImg.size(),CV_64FC1)};
    // 実数部と虚数部を一組にした 2 チャンネル画像 cxMatrix を生成
    cv::merge(imgMatrix, 2, cxMatrix);

    // フーリエ変換
    cv::dft(cxMatrix, ftMatrix);
    ShiftDFT(ftMatrix, ftMatrix);

    double sigma = 0.008;  // ガウシアンフィルタの標準偏差
    cv::Point center2 = cv::Point(ftMatrix.cols / 2, ftMatrix.rows / 2);
    for (int i = 0; i < ftMatrix.rows; ++i) {
        for (int j = 0; j < ftMatrix.cols; ++j) {
            double radius = std::pow(j - center2.x, 2.0) + std::pow(i - center2.y, 2.0);
            // double weight =  std::exp(-2.0 * M_PI * M_PI * std::pow(radius, 2.0) / (2.0 * std::pow(sigma, 2.0)));
            double weight =  1 - std::exp(-2.0 * M_PI * M_PI * std::pow(sigma, 2.0) * radius);
            ftMatrix.at<cv::Vec2d>(i, j)[0] *= weight;
            ftMatrix.at<cv::Vec2d>(i, j)[1] *= weight;
        }
    }

    // フーリエスペクトル"spcMatrix"の計算
    cv::split(ftMatrix, imgMatrix);
    cv::magnitude(imgMatrix[0], imgMatrix[1], spcMatrix);

    // フーリエスペクトルからフーリエスペクトル画像を生成
    spcMatrix += cv::Scalar::all(1);
    cv::log(spcMatrix, spcMatrix);
    cv::normalize(spcMatrix, spcImg, 0, 255, cv::NORM_MINMAX, CV_8U);

    // フーリエ逆変換
    ShiftDFT(ftMatrix, ftMatrix);
    cv::idft(ftMatrix, cxMatrix);
    cv::split(cxMatrix, imgMatrix);
    cv::normalize(imgMatrix[0], resultImg, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::imwrite("./dst/dip10_kadai3/highpass_spatial.jpg", spcImg);

    // ハイパスフィルタには2値化処理を行う
    cv::threshold(resultImg, resultImg, 110, 255, cv::THRESH_BINARY_INV);

    cv::imwrite("./dst/dip10_kadai3/highpass_result.jpg", resultImg);


    // ローパスフィルタ

    // フーリエ変換
    cv::dft(cxMatrix, ftMatrix);
    ShiftDFT(ftMatrix, ftMatrix);

    sigma = 0.008;  // ガウシアンフィルタの標準偏差
    center2 = cv::Point(ftMatrix.cols / 2, ftMatrix.rows / 2);
    for (int i = 0; i < ftMatrix.rows; ++i) {
        for (int j = 0; j < ftMatrix.cols; ++j) {
            double radius = std::pow(j - center2.x, 2.0) + std::pow(i - center2.y, 2.0);
            // double weight =  std::exp(-2.0 * M_PI * M_PI * std::pow(radius, 2.0) / (2.0 * std::pow(sigma, 2.0)));
            double weight =  std::exp(-2.0 * M_PI * M_PI * std::pow(sigma, 2.0) * radius);
            ftMatrix.at<cv::Vec2d>(i, j)[0] *= weight;
            ftMatrix.at<cv::Vec2d>(i, j)[1] *= weight;
        }
    }

    // フーリエスペクトル"spcMatrix"の計算
    cv::split(ftMatrix, imgMatrix);
    cv::magnitude(imgMatrix[0], imgMatrix[1], spcMatrix);

    // フーリエスペクトルからフーリエスペクトル画像を生成
    spcMatrix += cv::Scalar::all(1);
    cv::log(spcMatrix, spcMatrix);
    cv::normalize(spcMatrix, spcImg, 0, 255, cv::NORM_MINMAX, CV_8U);

    // フーリエ逆変換
    ShiftDFT(ftMatrix, ftMatrix);
    cv::idft(ftMatrix, cxMatrix);
    cv::split(cxMatrix, imgMatrix);
    cv::normalize(imgMatrix[0], resultImg, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::imwrite("./dst/dip10_kadai3/lowpass_spatial.jpg", spcImg);
    cv::imwrite("./dst/dip10_kadai3/lowpass_result.jpg", resultImg);


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