//(OpenCV4) g++ -std=c++11 dip07b.cpp `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip07b.cpp `pkg-config --cflags --libs opencv`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, char * argv[])
{
    //①ビデオキャプチャの初期化
    cv::VideoCapture capture("./src/car.mov");  //ビデオファイルをオープン
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

    cv::VideoWriter rec("./dst/dip07_kadai2.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, imageSize);
    
    //⑤動画表示用無限ループ
    while (1) {
        //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        //ビデオが終了したら無限ループから脱出
        if (originalImage.data==NULL) break;
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);

        //frameImageをoutputImageにコピー
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
                
                float distance = sqrt((pt1.x - pt2.x)*(pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y));
                
                // 右向きの動きは白色、左向きの動きは赤色
                cv::Scalar color;
                if (pt1.x < pt2.x) {
                    color = cv::Scalar(255, 255, 255);
                } else {
                    color = cv::Scalar(0, 0, 255);
                    // 速度が遅いものは緑色で描画
                    if (distance < 28) {
                        color = cv::Scalar(0, 255, 0);
                    }
                }

                if(distance > 15 && distance < 50) {
                    cv::line(optImage, pt1, pt2, color, 1, 8);
                    cv::line(outputImage, pt1, pt2, color, 1, 8);
                }
            }

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
