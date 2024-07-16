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

    cv::Mat hsvImage;

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

        // 2値化
        cv::Mat binaryImage;
        cv::threshold(grayImage, binaryImage, 189, 255, cv::THRESH_BINARY_INV);


        // 収縮・膨張処理
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        
        //収縮
        cv::erode(binaryImage, binaryImage, cv::Mat(), cv::Point(-1, -1), 11); 
        //膨張
        cv::dilate(binaryImage, binaryImage, cv::Mat(), cv::Point(-1, -1), 20);
        //収縮
        cv::erode(binaryImage, binaryImage, cv::Mat(), cv::Point(-1, -1), 9);


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
                // 色抽出用座標
                cv::Point pixelColor(contours[i][0].x + 5, contours[i][0].y + 10);

                // 座標が画面端10%によっている時
                if (pixelColor.x < width * 0.2 || pixelColor.y < height * 0.2) continue;
                if (pixelColor.x > width * 0.8 || pixelColor.y > height * 0.8) continue;

                // 色判定
                cv::Vec3b hsvColor = hsvImage.at<cv::Vec3b>(pixelColor.y, pixelColor.x);

                // 輪郭の色を定義
                cv::Scalar color;

                // 円形度を計算
                double perimeter = cv::arcLength(contours[i], true);
                double area = cv::contourArea(contours[i]);
                double circularity = 4 * M_PI * area / (perimeter * perimeter);

                // 円形度による分類
                if(circularity > 0.82){
                    // レモンバウム
                    if (hsvColor[0] > 20 && hsvColor[0] < 40 && hsvColor[1] > 100 && hsvColor[2] > 100){
                        color = color_lemon_baum;
                    }else{
                        // チーズブッセ
                        color = color_cheese;
                    }
                }else if(circularity > 0.75){
                    // 厚切りチョコケーキ
                    if (hsvColor[1] < 50 && hsvColor[2] < 90){
                        color = color_thick_choco;
                    }else{
                        // バナナブレッド
                        color = color_banana_bread;
                    }
                }else{
                    // ごろごろチョコチップスコーン
                    color = color_goro_goro;
                }

                // 要素の面積と円形度を画面上に表示
                // cv::putText(contourImage, std::to_string(area), contours[i][0], cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);
                // cv::putText(contourImage, std::to_string(circularity), contours[i][0] + cv::Point(0, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);
                
                
                cv::drawContours(contourImage, contours, i, color, 2);

                // 画面左上(座標固定)にインジケーターを描画
                cv::Point indicatorPos;

                if (color == color_lemon_baum) indicatorPos = cv::Point(10, 10);
                if (color == color_cheese) indicatorPos = cv::Point(30, 10);
                if (color == color_thick_choco) indicatorPos = cv::Point(50, 10);
                if (color == color_banana_bread) indicatorPos = cv::Point(70, 10);
                if (color == color_goro_goro) indicatorPos = cv::Point(90, 10);

                cv::circle(contourImage, indicatorPos, 10, color, -1);

            }
        }
        


        //画像表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Contour", contourImage);
        cv::imshow("Binary", binaryImage);

        //動画出力
        writer << contourImage;


        //キー入力待ち
        int key = cv::waitKey(20);
        //[Q]が押されたら無限ループ脱出
        if (key=='q')
            break;

        // [S]が押されたら動画を一時停止
        if (key=='s')
            // 再度sが押されるまで無限ループ
            while (cv::waitKey(0)!='s');
    }
    
    //終了処理
    //カメラ終了
    capture.release();
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
