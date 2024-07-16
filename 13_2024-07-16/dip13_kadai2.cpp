//g++ dip13.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main(int argc, char* argv[])
{
    //ビデオキャプチャの初期化
    cv::VideoCapture capture("./src/senro.mov");  //ビデオファイルをオープン
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

    //画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::namedWindow("Result");

    // 出力用画像の準備
    cv::Mat outputImage = cv::Mat::zeros(height, width, CV_8UC3);

    // 画素の平均を求めるループ用のcount
    int count = 0;

    // 動画出力用のvideoWriter
    cv::VideoWriter writer("./dst/dip13_kadai2.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, imageSize);

    //動画処理用無限ループ
    while (1) {
        //ビデオキャプチャから1フレーム"frameImage"に取り込み
        capture >> frameImage;
        //ビデオが終了したら無限ループから脱出
        if (frameImage.data==NULL) {
            break;
        }

        // 画素の平均を求める
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                cv::Vec3b framePixel = frameImage.at<cv::Vec3b>(y, x);
                cv::Vec3b outputPixel = outputImage.at<cv::Vec3b>(y, x);

                cv::Vec3b s;

                // outputPixelにcountを重みとして乗算し、framePixelと加算
                s[0] = (outputPixel[0] * count + framePixel[0]) / (count + 1);
                s[1] = (outputPixel[1] * count + framePixel[1]) / (count + 1);
                s[2] = (outputPixel[2] * count + framePixel[2]) / (count + 1);

                outputImage.at<cv::Vec3b>(y, x) = s;
            }
        }

        count++;

        //画像表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Result", outputImage);

        //動画出力
        writer << outputImage;

        //キー入力待ち
        int key = cv::waitKey(20);
        //[Q]が押されたら無限ループ脱出
        if (key=='q')
            break;
    }

    // Resultを出力
    cv::imwrite("./dst/dip13_kadai2_result.jpg", outputImage);
    
    //終了処理
    //カメラ終了
    capture.release();
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
