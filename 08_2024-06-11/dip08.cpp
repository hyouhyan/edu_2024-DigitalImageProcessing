//(OpenCV4) g++ dip08.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main(int argc, const char* argv[])
{
    //①画像ファイルの読み込み
    //画像ファイル"ferarri.jpg"を読み込んで，画像データ"sourceImage"に格納
    cv::Mat sourceImage = cv::imread("./src/ferarri.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data==0) {  //画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);
    
    //②画像表示用ウィンドウの生成
    cv::namedWindow("Translate");
    
    //③3チャンネル画像"translateImage"の確保（画像ファイルと同サイズ）
    //"sourceImage"と同サイズ・3チャンネル・ゼロで初期化
    cv::Mat translateImage = cv::Mat::zeros(sourceImage.size(), CV_8UC3);
    
    //④回転移動行列"rotateMat"の生成
    cv::Point2f original[4], translate[4];
    original[0] = cv::Point2f(0, 0); //A(オリジナル左上)
    original[1] = cv::Point2f(sourceImage.cols, 0); //B(オリジナル右上)
    original[2] = cv::Point2f(sourceImage.cols, sourceImage.rows); //C(オリジナル右下)
    original[3] = cv::Point2f(0, sourceImage.rows); //D(オリジナル左下)
    translate[0] = cv::Point2f(sourceImage.cols/4.0, sourceImage.rows/3.0); //A'(変換後左上)
    translate[1] = cv::Point2f(sourceImage.cols*3.0/4.0, sourceImage.rows/3.0); //B'(変換後右上)
    translate[2] = cv::Point2f(sourceImage.cols, sourceImage.rows); //C'(変換後右下)
    translate[3] = cv::Point2f(0, sourceImage.rows); //D'(変換後左下)
    cv::Mat persMat = cv::getPerspectiveTransform(original, translate); //行列生成

    //行列要素表示(確認用)
    printf("%f %f %f\n", rotateMat.at<double>(0, 0), rotateMat.at<double>(0, 1), rotateMat.at<double>(0, 2));
    printf("%f %f %f\n", rotateMat.at<double>(1, 0), rotateMat.at<double>(1, 1), rotateMat.at<double>(1, 2));
    printf("%f %f %f\n", persMat.at<double>(2,0), persMat.at<double>(2,1), persMat.at<double>(2,2));
    
    //⑤"sourceImage"に回転移動"rotateMat"を施して"translateImage"に張り付け
    //値が決定されない画素は黒で塗りつぶし．
    cv::warpPerspective(sourceImage, translateImage, persMat, translateImage.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0,0,0));
    
    //⑥"translateImage"の表示
    cv::imshow("Translate", translateImage);
    
    //⑦キー入力待ち
    cv::waitKey(0);
    
    return 0;
}
