#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, char* argv[])
{
    // ビデオキャプチャの初期化
    cv::VideoCapture capture("./src/swingcar.mov");  //ビデオファイルをオープン
    if (capture.isOpened()==0) {
        std::cerr << "Capture not found\n";
        return -1;
    }
    
    // 画像格納用インスタンス準備
    int w = capture.get(cv::CAP_PROP_FRAME_WIDTH);  //captureから動画横サイズ取得
    int h = capture.get(cv::CAP_PROP_FRAME_HEIGHT);  //captureから動画縦サイズ取得
    std::cout << "Frame size: " << w << " x " << h << std::endl;

    cv::Size imageSize(w, h);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3);  //3チャンネル
    cv::Mat grayImage(imageSize, CV_8UC1);  //1チャンネル
    cv::Mat edgeImage(imageSize, CV_8UC1);  //1チャンネル
    cv::Mat binImage(imageSize, CV_8UC3);  //3チャンネル
    
    // 画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Edge");
    cv::moveWindow("Edge", 100, 100);
    
    // ハフ変換用変数
    std::vector<cv::Point2f> lines; //ρ,θ の組で表現される直線群

    // ビデオライタ生成
    // h264 でエンコード
    cv::VideoWriter rec("./dst/dip09_kadai2.mp4", cv::VideoWriter::fourcc('h','2','6','4'), 20, frameImage.size());

    
    // 動画処理用無限ループ
    while (1) {
        // ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;

        // フレームが正しく読み込まれたか確認
        if (originalImage.empty()) {
            std::cerr << "Failed to capture frame\n";
            break;
        }

        // オリジナルのサイズが意図しない場合は、サイズの確認を行う
        if (originalImage.size() != imageSize) {
            std::cerr << "Frame size does not match the expected size\n";
            continue; // サイズが違う場合は処理をスキップ
        }


        // "originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);
        
        // "frameImage"からグレースケール画像"grayImage"を生成
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);
        
        // "grayImage"からエッジ画像"edgeImage"を生成
        cv::Canny(grayImage, edgeImage, 120, 160, 3); //ケニーのエッジ検出アルゴリズム

        
        // "edgeImage"に直線検出ハフ変換を施して，閾値(250)以上の投票数を得た直線群(ρ,θ)を"lines"に格納
        cv::HoughLines(edgeImage, lines, 1, M_PI/180, 250);

        int count=0;
        float theta2 = 0.0;
        cv::Point2f center = cv::Point2f(frameImage.cols/2, frameImage.rows/2);
        
        // ハフ変換結果表示
        // 検出された直線の数("lines.size()")と閾値(100)の小さい方の数だけ繰り返し
        for (int i=0; i<std::min(lines.size(), size_t(200)); i++) {
            // 直線パラメータ：(ρ,θ) → 直線数式：a(x-x0)=b(y-y0) → 2 端点"p1"，"p2"を計算
            cv::Point2f line = lines[i]; // "lines"から直線(ρ,θ)を 1 組取り出し
            float rho = line.x; // "ρ"
            float theta = line.y; // "θ"
            double a = cos(theta); // "θ"から"a"を計算
            double b = sin(theta); // "θ"から"b"を計算
            double x0 = a * rho; // 直線上の 1 点 p0(x0, y0)の"x0"を計算
            double y0 = b * rho; // 直線上の 1 点 p0(x0, y0)の"y0"を計算
            cv::Point p1, p2; // 直線描画用の端点"p1"，"p2"
            p1.x = x0-1000*b; // "p1"の x 座標の計算
            p1.y = y0+1000*a; // "p1"の y 座標の計算
            p2.x = x0+1000*b; // "p2"の x 座標の計算
            p2.y = y0-1000*a; // "p2"の y 座標の計算

            theta = theta*180/M_PI-90;
            theta2 += theta;
            count++;

            // "p1"と"p2"を結ぶ線分を描画
            // cv::line(frameImage, p1, p2, cv::Scalar(0, 0, 255), 2, 8, 0);
        }

        cv::Mat rotateMat = cv::getRotationMatrix2D(center, theta2/count, 1.0);
        cv::warpAffine(frameImage, frameImage, rotateMat, binImage.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0,0,0));

        
        // "frameImage"，"edgeImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Edge", edgeImage);
        
        // キー入力待ち
        int key = cv::waitKey(10);
        // [Q]が押されたら無限ループ脱出
        if (key=='q') break;

        rec << frameImage;  //ビデオライタに画像出力
    }
    
    // 終了処理
    // カメラ終了
    capture.release();
    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
