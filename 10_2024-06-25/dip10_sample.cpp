//g++ dip10.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

//配列の象限入れ替え用関数の宣言
void ShiftDFT(const cv::Mat& src_arr, cv::Mat& dst_arr);

int main(int argc, const char* argv[])
{
    //①原画像のグレースケール画像を"sourceImg"に格納
    cv::Mat sourceImg = cv::imread("nymegami.jpg", cv::IMREAD_GRAYSCALE);
    if (sourceImg.data==0) { //ファイルが見つからないときはメッセージを表示して終了
        printf("Source not found\n");
        exit(0);
    }
    
    //作業用配列領域、描画用画像領域の宣言
    cv::Mat cxMatrix(sourceImg.size(), CV_64FC2); //複素数用(実数 2 チャンネル)
    cv::Mat ftMatrix(sourceImg.size(), CV_64FC2); //複素数用(実数 2 チャンネル)
    cv::Mat spcMatrix(sourceImg.size(), CV_64FC1); //スペルトルデータ(実数)
    cv::Mat spcImg(sourceImg.size(), CV_8UC1); //スペクトル画像(自然数)
    cv::Mat resultImg(sourceImg.size(), CV_8UC1); //逆変換画像(自然数)
    
    //ウィンドウ生成
    //原画像
    cv::namedWindow("sourceImg");
    cv::moveWindow("sourceImg", 0, 0);
    //フーリエスペクトル画像
    cv::namedWindow("spcImg");
    cv::moveWindow("spcImg", sourceImg.cols, 0);
    //逆変換画像
    cv::namedWindow("Result");
    cv::moveWindow("Result", sourceImg.cols*2, 0);
    
    //②原画像を複素数(実数部と虚数部)の 2 チャンネル配列(画像)として表現．虚数部はゼロ
    cv::Mat imgMatrix[] = {cv::Mat_<double>(sourceImg), cv::Mat::zeros(sourceImg.size(),CV_64FC1)};
    //実数部と虚数部を一組にした 2 チャンネル画像 cxMatrix を生成
    cv::merge(imgMatrix, 2, cxMatrix);
    
    //③フーリエ変換
    //フーリエ変換の実施（cxMatrix → ftMatrix）
    cv::dft(cxMatrix, ftMatrix);
    //配列の象限入れ替え（低周波成分が画像中央部、高周波成分が画像周辺部
    ShiftDFT(ftMatrix, ftMatrix);
    
    //④フーリエスペクトル"spcMatrix"の計算
    //ftMatrix を実数部 imgMatrix[0] と虚数部 imgMatrix[1] に分解
    cv::split(ftMatrix, imgMatrix);
    //フーリエスペクトル各要素を計算して spcMatrix に格納(spc = sqrt(re^2+im^2))
    cv::magnitude(imgMatrix[0], imgMatrix[1], spcMatrix);
    
    //⑤フーリエスペクトルからフーリエスペクトル画像を生成
    //表示用にフーリエスペクトル spcMatrix の各要素の対数をとる(log(1+spc))
    spcMatrix += cv::Scalar::all(1);
    cv::log(spcMatrix, spcMatrix);
    //フーリエスペクトルを 0〜255 にスケーリングしてフーリエスペクトル画像 spcImg にコピー
    cv::normalize(spcMatrix, spcImg, 0, 255, cv::NORM_MINMAX, CV_8U);
    
    //⑥フーリエ逆変換
    //配列の象限入れ替え（低周波成分が画像周辺部、高周波成分が画像中央部）
    ShiftDFT(ftMatrix, ftMatrix);
    //フーリエ逆変換の実施（ftMatrix → cxMatrix）
    //cv::idft(ftMatrix, cxMatrix);
    cv::idft(ftMatrix, cxMatrix);
    //cxMatrix を実数部(imgMatrix[0])と虚数部(imgMatrix[1])に分解
    cv::split(cxMatrix, imgMatrix);
    //実数部(imgMatrix[0])を 0〜255 にスケーリングして resultImg にコピー
    cv::normalize(imgMatrix[0], resultImg, 0, 255, cv::NORM_MINMAX, CV_8U);
    
    //⑦各画像を表示
    //原画像
    cv::imshow("sourceImg", sourceImg);
    //フーリエスペクトル画像
    cv::imshow("spcImg", spcImg);
    //逆変換画像
    cv::imshow("Result", resultImg);
    
    //⑧キー入力されたら，ウィンドウと画像格納用領域を破棄して終了
    cv::waitKey(0);
    
    return 0;
}

//画像の象限入れ替え用関数
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
