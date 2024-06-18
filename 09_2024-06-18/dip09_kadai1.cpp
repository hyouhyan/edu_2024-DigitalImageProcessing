#include <iostream>
#include <opencv2/opencv.hpp>

int main (int argc, char* argv[]) {
    // ①ビデオキャプチャの初期化
    cv::VideoCapture capture("./src/balls.mov");  // ビデオファイルをオープン
    if (!capture.isOpened()) {
        std::cout << "Capture not found" << std::endl;
        return -1;
    }
    
    // ②画像格納用インスタンス準備
    int w = capture.get(cv::CAP_PROP_FRAME_WIDTH);  // captureから動画横サイズ取得
    int h = capture.get(cv::CAP_PROP_FRAME_HEIGHT);  // captureから動画縦サイズ取得
    cv::Size imageSize(w, h);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3);  // 3チャンネル
    cv::Mat grayImage(imageSize, CV_8UC1);  // 1チャンネル
    cv::Mat edgeImage(imageSize, CV_8UC1);  // 1チャンネル
    
    // ③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Edge");
    cv::moveWindow("Edge", 100, 100);

    // 動画ファイル保存用のVideoWriterを準備
    cv::VideoWriter outputVideo("./dst/output.mp4", 
                                cv::VideoWriter::fourcc('h', '2', '6', '4'), 
                                30, 
                                imageSize);

    if (!outputVideo.isOpened()) {
        std::cout << "Could not open the output video file for write" << std::endl;
        return -1;
    }
    
    // ④ハフ変換用変数
    std::vector<cv::Vec3f> circles;  //(cx, cy, r)で表現される円群
    
    // ⑤動画処理用無限ループ
    while (true) {
        // (a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        // ビデオが終了したら巻き戻し
        if (originalImage.empty()) {
            break; // 終了したらループを抜ける
        }
        // "originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);
        
        // (b)"frameImage"からグレースケール画像"grayImage"を生成
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);
        
        // (c)"grayImage"からエッジ画像"edgeImage"を生成
        cv::Canny(grayImage, edgeImage, 100, 200, 3); // ケニーのエッジ検出アルゴリズム
        
        // (d')"grayImage"に円検出ハフ変換を施して，しきい値(90)以上の得票数を得た円群(cx, cy, r)を"circles"に格納
        cv::HoughCircles(grayImage, circles, cv::HOUGH_GRADIENT, 1, 20, 60, 10, 12, 20);
        
        // (e')検出された円の数("circles.size()")としきい値(200)の小さい方の数だけ繰り返し
        for (size_t i = 0; i < std::min(circles.size(), static_cast<size_t>(200)); i++) {
            cv::Vec3f circle = circles[i];  //"circles"から円(cx, cy, r)を 1 組取り出し
            float x0 = circle[0]; // 円の中心座標(cx, cy)の x 座標"cx"
            float y0 = circle[1]; // 円の中心座標(cx, cy)の y 座標"cy"
            float r = circle[2]; // 円の半径"r"
            cv::circle(frameImage, cv::Point(cvRound(x0), cvRound(y0)), cvRound(r), cv::Scalar(0, 0, 255), 2, 8, 0); // 円の描画
        }

        // 検出された円の個数を表示
        std::string circleCountText = "Circles: " + std::to_string(circles.size());
        cv::putText(frameImage, circleCountText, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);

        // (f)"frameImage"，"edgeImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Edge", edgeImage);

        // フレームを書き込む
        outputVideo.write(frameImage);
        
        // (g)キー入力待ち
        int key = cv::waitKey(10);
        // [Q]が押されたら無限ループ脱出
        if (key == 'q') break;
    }
    
    // ⑥終了処理
    // カメラ終了
    capture.release();
    outputVideo.release();
    // メッセージを出力して終了
    std::cout << "Finished" << std::endl;
    return 0;
}
