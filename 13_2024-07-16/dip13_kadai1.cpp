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

    // パンの種類を表す色を定義
    const cv::Scalar color_goro_goro(255, 255, 0); // シアン
    const cv::Scalar color_cheese(255, 0, 0);      // 青
    const cv::Scalar color_thick_choco(0, 255, 0); // 緑
    const cv::Scalar color_banana_bread(255, 0, 255); // マゼンタ
    const cv::Scalar color_lemon_baum(0, 0, 255);  // 赤
    
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

    //オプティカルフローに関する初期設定
    cv::Mat priorImage(imageSize, CV_8UC1);  //前フレーム画像
    cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::MAX_ITER|cv::TermCriteria::EPS, 20, 0.05);  //反復アルゴリズム停止基準
    std::vector<cv::Point2f> priorFeature, presentFeature;  //前フレームおよび現フレーム特徴点
    std::vector<unsigned char> status;  //作業用
    std::vector<float> errors;  //作業用

    cv::Mat hsvImage;
    std::vector<cv::Mat> hsvChannels;

    // ビデオライタ 
    cv::VideoWriter writer("dip13_kadai1.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, imageSize);

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
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);
        cv::split(hsvImage, hsvChannels);

        cv::goodFeaturesToTrack(grayImage, priorFeature, 300, 0.01, 10);

        // オプティカルフロー計算
        int opCnt = priorFeature.size(); //特徴点数

        if (opCnt > 0) {
            cv::calcOpticalFlowPyrLK(priorImage, grayImage, priorFeature, presentFeature, status, errors, cv::Size(10, 10), 3, criteria);

            // オプティカルフローの平均を計算
            cv::Point2f sum(0, 0);
            for (int i = 0; i < opCnt; i++) {
                // 極端に大きな値は無視
                if(presentFeature[i].x - priorFeature[i].x > 20 || presentFeature[i].y - priorFeature[i].y > 20) {
                    continue;
                }
                sum += presentFeature[i] - priorFeature[i];
            }
            cv::Point2f ave = sum / opCnt;

            // 画面中央に矢印を描画
            cv::arrowedLine(frameImage, cv::Point(width / 2, height / 2), cv::Point(width / 2 + ave.x, height / 2 + ave.y), cv::Scalar(0, 255, 0), 2);
        }

        // 2値化
        cv::Mat binaryImage;
        cv::threshold(grayImage, binaryImage, 185, 255, cv::THRESH_BINARY);

        cv::threshold(hsvChannels[1], binaryImage, 40, 255, cv::THRESH_BINARY);

        // 収縮・膨張処理
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        
        // 膨らます
        cv::dilate(binaryImage, binaryImage, kernel, cv::Point(-1, -1), 10);
        // 削る
        cv::erode(binaryImage, binaryImage, kernel, cv::Point(-1, -1), 10);

        // 削る
        cv::erode(binaryImage, binaryImage, kernel, cv::Point(-1, -1), 10);
        // 膨らます
        cv::dilate(binaryImage, binaryImage, kernel, cv::Point(-1, -1), 10);


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

                // 円形度を計算
                double perimeter = cv::arcLength(contours[i], true);
                double area = cv::contourArea(contours[i]);
                double circularity = 4 * M_PI * area / (perimeter * perimeter);

                // 要素の面積と円形度を画面上に表示
                cv::putText(contourImage, std::to_string(area), contours[i][0], cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);
                cv::putText(contourImage, std::to_string(circularity), contours[i][0] + cv::Point(0, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);

            }
        }
        


        //画像表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Contour", contourImage);
        cv::imshow("Binary", binaryImage);

        //動画出力
        writer << contourImage;

        priorImage = grayImage.clone();

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
