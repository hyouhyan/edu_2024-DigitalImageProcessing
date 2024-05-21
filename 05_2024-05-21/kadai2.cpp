//(OpenCV4) g++ -std=c++11 dip05.cpp `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip05.cpp `pkg-config --cflags --libs opencv`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

//アニメ調色変換関数の宣言
void myAnimeColor(const cv::Mat &src, cv::Mat &dst);
unsigned char lookupTable[3][256];

int main(int argc, char *argv[]) {
    //アニメ変換用ルックアップテーブル
    for (int i = 0; i < 256; i++) {
        // H
        lookupTable[0][i] = i;
        // S
        lookupTable[1][i] = i;
        // V
        if (i < 64)
            lookupTable[2][i] = 0;
        else if (i < 128)
            lookupTable[2][i] = 85;
        else if (i < 196)
            lookupTable[2][i] = 170;
        else
            lookupTable[2][i] = 255;
    }

    //ビデオキャプチャを初期化して，映像を取り込む
    // cv::VideoCapture capture(0);  //内臓カメラをオープン
    cv::VideoCapture capture("scene.mov"); //指定したビデオファイルをオープン
    //ビデオファイルがオープンできたかどうかをチェック
    if (capture.isOpened() == 0) {
        printf("Camera not found\n");
        return -1;
    }

    //画像格納用インスタンス準備
    int width = 720, height = 405;
    cv::Mat frameImage;
    cv::Mat originalImage(cv::Size(width, height), CV_8UC3);
    cv::Mat grayImage(cv::Size(width, height), CV_8UC1);
    cv::Mat resultImage(cv::Size(width, height), CV_8UC3);
    cv::Mat lapImage(cv::Size(width, height), CV_8UC1);
    cv::Mat recImage(cv::Size(width, height), CV_8UC3);
    cv::Mat animeImage(cv::Size(width, height), CV_8UC3);
    cv::Mat binImage(cv::Size(width, height), CV_8UC1);

    //画像表示用ウィンドウの生成
    cv::namedWindow("Original");
    cv::moveWindow("Original", 0, 50);
    cv::namedWindow("Gray");
    cv::moveWindow("Gray", 0, 200);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 200, 200);

    //線形空間フィルタの生成
    //線形空間フィルタの係数を要素とする1次元配列の作成(5x5の平均値フィルタ用)
    float fdata[] = {
        1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0,
        1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0,
        1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0,
        1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0,
        1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0, 1.0 / 25.0};

    //一次元配列の要素に基づき，線形空間フィルタをCvMat型の5x5行列"kernel"として生成
    cv::Mat kernel(cv::Size(5, 5), CV_32F, fdata);

    cv::VideoWriter rec("dip05_kadai2.mp4", cv::VideoWriter::fourcc('H','2','6','4'), 30, recImage.size());

    //動画像処理無限ループ：「ビデオキャプチャから1フレーム取り込み」→「画像処理」→「表示」の繰り返し
    while (1) {
        //ビデオキャプチャ"capture"から1フレームを取り込んで，"frameImage"に格納
        capture >> frameImage;
        //ビデオが終了したら無限ループから脱出
        if (frameImage.data == NULL) break;

        //"frameImage"をリサイズして"originalImage"に格納
        cv::resize(frameImage, originalImage, originalImage.size());

        //"frameImage"をグレースケールに変換して"grayImage"に格納
        cv::cvtColor(originalImage, grayImage, cv::COLOR_BGR2GRAY);

        //メディアンフィルタを適用
        cv::medianBlur(grayImage, grayImage, 3);

        //ラプラシアンフィルタを適用
        cv::Laplacian(grayImage, lapImage, CV_8U, 5);

        //2値化
        cv::threshold(lapImage, binImage, 128, 255, cv::THRESH_BINARY);

        //アニメ調色変換
        myAnimeColor(originalImage, animeImage);

        // 画像を走査し、binImageの白画素をresultImageにコピーし、黒画素の場合は、animeImageの画素をコピー
        for (int j = 0; j < originalImage.rows; j++) {
            for (int i = 0; i < originalImage.cols; i++) {
                if (binImage.at<unsigned char>(j, i) == 255) {
                    resultImage.at<cv::Vec3b>(j, i) = cv::Vec3b(0, 0, 0);
                } else {
                    resultImage.at<cv::Vec3b>(j, i) = animeImage.at<cv::Vec3b>(j, i);
                }
            }
        }

        



        //ウィンドウに画像表示
        cv::imshow("Original", originalImage);
        cv::imshow("Gray", grayImage);
        cv::imshow("Result", resultImage);
        cv::imshow("Anime", animeImage);

        // cv::cvtColor(resultImage, recImage, cv::COLOR_BGR2RGB);
        rec << resultImage;

        //[q]キーが押されたら無限ループから脱出
        int key = cv::waitKey(10);
        if (key == 'q')
            break;
    }

    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}

//アニメ調色変換関数（src：入力画像，dst：出力画像）
void myAnimeColor(const cv::Mat &src, cv::Mat &dst) {
    //作業用画像生成
    cv::Mat cImage(src.size(), CV_8UC3); // 3チャンネル

    //色変換
    cv::cvtColor(src, cImage, cv::COLOR_BGR2HSV); // RGB→HSV
    cv::Vec3b s;
    for (int j = 0; j < src.rows; j++) {
        for (int i = 0; i < src.cols; i++) {
            //ルックアップテーブルで各画素値変換
            s = cImage.at<cv::Vec3b>(j, i);
            s[0] = lookupTable[0][s[0]];
            s[1] = lookupTable[1][s[1]];
            s[2] = lookupTable[2][s[2]];
            cImage.at<cv::Vec3b>(j, i) = s;
        }
    }

    cv::cvtColor(cImage, dst, cv::COLOR_HSV2BGR); // HSV→RGB
}
