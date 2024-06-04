//(OpenCV4) g++ -std=c++11 dip07b.cpp `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip07b.cpp `pkg-config --cflags --libs opencv`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, char * argv[])
{
    //①ビデオキャプチャの初期化
    cv::VideoCapture capture("room.mov");  //ビデオファイルをオープン
    //cv::VideoCapture capture(0);  //カメラをオープン
    if (capture.isOpened()==0) {
        printf("Camera not found\n");
        return -1;
    }
    
    //②画像格納用インスタンス準備
    cv::Size imageSize(720, 405);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3);
    cv::Mat optImage(imageSize, CV_8UC3);
    cv::Mat outputImage(imageSize, CV_8UC3);
    
    //③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("OpticalFlow");
    cv::moveWindow("OpticalFlow", 50, 0);
    cv::namedWindow("Output");
    cv::moveWindow("Output", 100, 0);
    
    //④オプティカルフローに関する初期設定
    cv::Mat priorImage(imageSize, CV_8UC1);  //前フレーム画像
    cv::Mat presentImage(imageSize, CV_8UC1);  //現フレーム画像
    cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::MAX_ITER|cv::TermCriteria::EPS, 20, 0.05);  //反復アルゴリズム停止基準
    std::vector<cv::Point2f> priorFeature, presentFeature;  //前フレームおよび現フレーム特徴点
    std::vector<unsigned char> status;  //作業用
    std::vector<float> errors;  //作業用

    cv::VideoWriter rec("dip07_kadai4.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, cv::Size(720, 405));
    
    //⑤動画表示用無限ループ
    while (1) {
        //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        //ビデオが終了したら無限ループから脱出
        if (originalImage.data==NULL) break;
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);

        frameImage.copyTo(outputImage);
        
        //(b)"frameImage"をグレースケール変換して"presentImage"を生成(現フレーム)
        cv::cvtColor(frameImage, presentImage, cv::COLOR_BGR2GRAY);
        
        //(c)"priorImage"から特徴点を抽出して"priorFeature[]"に出力
        cv::goodFeaturesToTrack(priorImage, priorFeature, 300, 0.01, 10);
        
        //(d)オプティカルフロー検出・描画
        int opCnt = priorFeature.size(); //特徴点数

        if (opCnt>0) {  //特徴点が存在する場合
            //前フレームの特徴点"priorFeature"から，対応する現フレームの特徴点"presentFeature"を検出
            cv::calcOpticalFlowPyrLK(priorImage, presentImage, priorFeature, presentFeature, status, errors, cv::Size(10,10), 4, criteria);

            //オプティカルフロー描画
            for(int i=0; i<opCnt; i++){
                cv::Point pt1 = cv::Point(priorFeature[i]); //前フレーム特徴点
                cv::Point pt2 = cv::Point(presentFeature[i]); //現フレーム特徴点
                cv::line(optImage, pt1, pt2, cv::Scalar(0,0,255), 1, 8); //直線描画
            }

            //全てのオプティカルフローの平均を計算
            cv::Point2f meanFlow(0, 0);
            for(int i=0; i<opCnt; i++){
                meanFlow += presentFeature[i] - priorFeature[i];
            }
            meanFlow /= opCnt;
            cv::Point2f center(imageSize.width/2, imageSize.height/2);
            //中心からのオプティカルフローを矢印で描画
            cv::arrowedLine(optImage, center, center - meanFlow*20, cv::Scalar(0,255,0), 2, 8);
            cv::arrowedLine(outputImage, center, center - meanFlow*20, cv::Scalar(0,255,0), 2, 8);

        }
        
        //(e)"frameImage"と"resultImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("OpticalFlow", optImage);
        cv::imshow("Output", outputImage);
        
        //(f)現フレームグレースケール画像"presentImage"を前フレームグレースケール画像"priorImage"にコピー
        presentImage.copyTo(priorImage);
        
        //(g)"optImage"をゼロセット
        optImage = cv::Scalar(0);

        rec << outputImage;
        
        //(h)キー入力待ち
        int key = cv::waitKey(20);
        //'q'が押されたら無限ループ脱出
        if (key=='q') {
            break;
        }
    }
    
    //⑥終了処理
    //カメラ終了
    capture.release();
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
