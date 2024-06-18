//(OpenCV4) g++ -std=c++11 dip07a.cpp `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip07a.cpp `pkg-config --cflags --libs opencv`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, char* argv[])
{
    //①ビデオキャプチャの初期化
    cv::VideoCapture capture("./src/walking.mov");  //ビデオファイルをオープン
    if (capture.isOpened()==0) {
        printf("Camera not found\n");
        return -1;
    }
    
    //②画像格納用インスタンス準備
    cv::Size imageSize(720, 405);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3);
    cv::Mat backImage(imageSize, CV_8UC3);
    cv::Mat subImage(imageSize, CV_8UC3);
    cv::Mat subBinImage(imageSize, CV_8UC1);
    cv::Mat resultImage(imageSize, CV_8UC3);

    cv::Mat outputImage(imageSize, CV_8UC3);
    
    //③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Back");
    cv::moveWindow("Back", 50, 50);
    cv::namedWindow("Subtraction");
    cv::moveWindow("Subtraction", 100, 100);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 150, 150);
    cv::namedWindow("Output");
    cv::moveWindow("Output", 200, 200);

    cv::VideoWriter rec("./dst/dip07_kadai1.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, cv::Size(720, 405));
    
    //④動画処理用無限ループ
    while (1) {
        //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        //ビデオが終了したら無限ループから脱出
        if (originalImage.data==NULL) break;
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);
        
        //(b)"frameImage"と"backImage"との差分画像"subImage"の生成
        cv::absdiff(frameImage, backImage, subImage);

        //(b')"subImage"をグレースケール変換→しきい値処理した画像"subBinImage"を生成
        cv::cvtColor(subImage, subBinImage, cv::COLOR_BGR2GRAY);
        cv::threshold(subBinImage, subBinImage, 10, 255, cv::THRESH_BINARY);


        //(b")"frameImage"を"subBinImage"マスク付きで"resultImage"にコピー
        resultImage = cv::Scalar(0);
        frameImage.copyTo(resultImage, subBinImage);

        // outputImageにコピー
        resultImage.copyTo(outputImage);

        //subBinImageの描画部分の面積を計算してprintfで出力
        cv::Mat subBinImageCopy;
        subBinImage.copyTo(subBinImageCopy);
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(subBinImageCopy, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        double area = 0;
        for (int i = 0; i < contours.size(); i++) {
            area += cv::contourArea(contours[i]);
        }
        printf("area: %f\n", area);

        //画面中央に赤い円を描画
        cv::circle(outputImage, cv::Point(imageSize.width/2, imageSize.height/2), area/100, cv::Scalar(0, 0, 255), 5);
        
        //(c)"frameImage"，"backImage"，"subImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Back", backImage);
        cv::imshow("Subtraction", subBinImage);
        cv::imshow("Result", resultImage);
        cv::imshow("Output", outputImage);
        
        //(d)"frameImage"で"backImage"を更新
        frameImage.copyTo(backImage);
        
        //(e)キー入力待ち
        int key = cv::waitKey(20);

        rec << outputImage;

        //[Q]が押されたら無限ループ脱出
        if (key=='q') break;

        //[C]が押されたら"frameImage"で"backImage"を更新
        if (key=='c') frameImage.copyTo(backImage);
    }
    
    //⑤終了処理
    //カメラ終了
    capture.release();
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
