//(OpenCV4) g++ dip07-2.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip07-2.cpp `pkg-config --cflags --libs opencv`

#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, char *argv[])
{
    //ビデオキャプチャを初期化して，映像を取り込む
    cv::VideoCapture capture("./src/water1.mov");  //指定したビデオファイルをオープン
    if (capture.isOpened()==0) {
        printf("Camera not found\n");
        return -1;
    }
    //フレームサイズ取得
    int width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    printf("Frame Size = (%d %d)\n", width, height);
    cv::Size imageSize(width, height);  //フレームと同じ画像サイズ定義

    //船画像"face.jpg"の読み込み
    cv::Mat shipImage = cv::imread("./src/ship.jpg", cv::IMREAD_COLOR);

    //画像格納用インスタンス準備
    cv::Mat frameImage;
    cv::Mat recImage = cv::Mat(cv::Size(width/2, height/2), CV_8UC3);
    
    //オプティカルフロー準備
    cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::MAX_ITER|cv::TermCriteria::EPS, 30, 0.01);  //終了条件
    cv::Mat presentImage(imageSize, CV_8UC1), priorImage(imageSize, CV_8UC1);  //現フレーム濃淡画像，前フレーム濃淡画像
    std::vector<cv::Point2f> presentFeature, priorFeature;  //現フレーム対応点，前フレーム追跡点
    std::vector<unsigned char> status;  //処理用
    std::vector<float> errors;  //処理用

    //船の初期位置
    cv::Point2f shipPoint(130, 190);

    //ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Ship");
    cv::moveWindow("Ship", width, 0);

    //船画像の表示
    cv::imshow("Ship", shipImage);
    
    //追跡点の設定（適当に決めた5つの点）
    priorFeature.push_back(cv::Point2f(width/2.0-20, height/2.0));
    priorFeature.push_back(cv::Point2f(width/2.0-10, height/2.0));
    priorFeature.push_back(cv::Point2f(width/2.0, height/2.0));
    priorFeature.push_back(cv::Point2f(width/2.0+10, height/2.0));
    priorFeature.push_back(cv::Point2f(width/2.0+20, height/2.0));

    //ビデオライタ生成(ファイル名，コーデック(mp4v/mov)，フレームレート，フレームサイズ)
    cv::VideoWriter rec("./dst/dip08_kadai2_k22047.mp4", cv::VideoWriter::fourcc('m','p','4','v'), 30, cv::Size(width, height));

    //動画像処理無限ループ
    int fid = 0;
    while (1) {
        //===== カメラから1フレーム読み込み =====
        capture >> frameImage;
        if(frameImage.data == NULL) break;
        
        //===== オプティカルフロー =====
        cv::cvtColor(frameImage, presentImage, cv::COLOR_BGR2GRAY);  //現フレーム濃淡画像"presentImage"を生成
        //cv::goodFeaturesToTrack(priorImage, priorFeature, 1000, 0.01, 1);  //前フレーム追跡点"priorFeature"生成
        int opCnt = priorFeature.size();  //追跡点の個数
        //オプティカルフローの計算と描画
        if (opCnt>0) {  //追跡点が存在する場合
            //"priorImage"と"presentImage"を用いて，追跡点"priorFeature"に対応する現フレーム点"presentFeature"を取得
            cv::calcOpticalFlowPyrLK(priorImage, presentImage, priorFeature, presentFeature, status, errors, cv::Size(10,10), 4, criteria);
            //オプティカルフロー描画
            for(int i=0; i<opCnt; i++){
                cv::Point pt1 = cv::Point(priorFeature[i]);  //前フレーム追跡点
                cv::Point pt2 = cv::Point(presentFeature[i]);  //現フレーム対応点
                cv::line(frameImage, pt1, pt2, cv::Scalar(0,0,255), 2, 8);  //pt1とpt2を結ぶ直線を描画
            }
        }
        presentImage.copyTo(priorImage);  //"priorImage"を"presentImage"で更新
        
        //船の位置に円を表示
        cv::circle(frameImage, shipPoint, 5, cv::Scalar(0,255,0), -1, 8);

        //ウィンドウに画像表示
        cv::imshow("Frame", frameImage);

        //キー入力待ち
        char key = cv::waitKey(20);  //20ミリ秒待機
        if(key == 'q') break;

        //動画ファイル書き出し
        //rec << frameImage;  //ビデオライタに画像出力
    }
    
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}

