//g++ dip13.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main(int argc, char* argv[])
{
    //ビデオキャプチャの初期化
    cv::VideoCapture capture("./src/pantora.mp4");  //ビデオファイルをオープン
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
    cv::namedWindow("Binary");
    cv::namedWindow("Contour");

    //動画処理用無限ループ
    while (1) {
        //ビデオキャプチャから1フレーム"frameImage"に取り込み
        capture >> frameImage;
        //ビデオが終了したら無限ループから脱出
        if (frameImage.data==NULL) {
            break;
        }

        // 輪郭抽出の下準備
        // グレースケール画像に変換
        cv::Mat grayImage;
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);

        // 2値化
        cv::Mat binaryImage;
        cv::threshold(grayImage, binaryImage, 180, 255, cv::THRESH_BINARY);
        // 白黒反転
        cv::bitwise_not(binaryImage, binaryImage);

        // 収縮・膨張処理
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::erode(binaryImage, binaryImage, kernel, cv::Point(-1, -1), 10);
        cv::dilate(binaryImage, binaryImage, kernel, cv::Point(-1, -1), 10);

        cv::dilate(binaryImage, binaryImage, kernel, cv::Point(-1, -1), 10);
        cv::erode(binaryImage, binaryImage, kernel, cv::Point(-1, -1), 10);


        // 輪郭表示用画像
        cv::Mat contourImage = frameImage.clone();

        // 輪郭抽出
        std::vector<std::vector<cv::Point>> contours;

        // 画像の破壊を防ぐためにコピーを作成
        cv::Mat tmpImage = binaryImage.clone();
        cv::findContours(tmpImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

        // 輪郭描画
        for (int i = 0; i < contours.size(); i++) {
            // 面積が小さいものは無視
            if (cv::contourArea(contours[i]) > 1200) {
                
                
                cv::drawContours(contourImage, contours, i, cv::Scalar(0, 0, 255), 2);
            }
        }
        


        //画像表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Contour", contourImage);
        cv::imshow("Binary", binaryImage);

        //キー入力待ち
        int key = cv::waitKey(20);
        //[Q]が押されたら無限ループ脱出
        if (key=='q')
            break;
    }
    
    //終了処理
    //カメラ終了
    capture.release();
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
