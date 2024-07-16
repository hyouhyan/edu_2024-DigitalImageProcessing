//g++ dip13.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main(int argc, char* argv[])
{
    //ビデオキャプチャの初期化
    cv::VideoCapture capture("./src/colorful.mp4");  //ビデオファイルをオープン
    if (capture.isOpened()==0) {  //オープンに失敗した場合
        printf("Capture not found\n");
        return -1;
    }
    
    //画像格納用インスタンス準備
    cv::Mat frameImage;  //ビデオキャプチャ用
    int width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Size imageSize(width, height);  //ビデオ画像サイズ
    printf("imageSize = (%d, %d)\n", width, height);  //ビデオ画像サイズ表示

    cv::Mat firstFrameImage;

    //画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::namedWindow("Result");
    cv::namedWindow("FirstFrame");

    // 出力用画像の準備
    cv::Mat outputImage = cv::Mat::zeros(height, width, CV_8UC3);

    // 動画出力用のvideoWriter
    cv::VideoWriter writer("./dst/dip13_kadai3.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, imageSize);

    //動画処理用無限ループ
    while (1) {
        //ビデオキャプチャから1フレーム"frameImage"に取り込み
        capture >> frameImage;
        //ビデオが終了したら無限ループから脱出
        if (frameImage.data==NULL) {
            break;
        }

        if (firstFrameImage.data == NULL) {
            firstFrameImage = frameImage.clone();
            outputImage = frameImage.clone();
        }

        // 初期フレームとの差分を検出
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                cv::Vec3b framePixel = frameImage.at<cv::Vec3b>(y, x);
                cv::Vec3b firstFramePixel = firstFrameImage.at<cv::Vec3b>(y, x);
                cv::Vec3b outputPixel = outputImage.at<cv::Vec3b>(y, x);

                if((abs(outputPixel[0] - framePixel[0]) < 15 && abs(outputPixel[1] - framePixel[1]) < 15 && abs(outputPixel[2] - framePixel[2]) < 15)) {
                    outputImage.at<cv::Vec3b>(y, x) = framePixel;
                } else {
                    outputImage.at<cv::Vec3b>(y, x) = cv::Vec3b(100, 100, 100);
                }
            }
        }


        //画像表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Result", outputImage);
        cv::imshow("FirstFrame", firstFrameImage);

        //動画出力
        writer << outputImage;

        //キー入力待ち
        int key = cv::waitKey(20);
        //[Q]が押されたら無限ループ脱出
        if (key=='q')
            break;
    }

    // Resultを出力
    cv::imwrite("./dst/dip13_kadai3_result.jpg", outputImage);
    
    //終了処理
    //カメラ終了
    capture.release();
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
