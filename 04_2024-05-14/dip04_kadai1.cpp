#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, const char* argv[])
{
    int width = 640, height = 480;
    
    //カメラの初期化
    cv::VideoCapture capture(0);  //カメラ0番をオープン
    //カメラがオープンできたかどうかをチェック
    if (capture.isOpened()==0) {
        printf("Camera not found\n");
        return -1;
    }
    
    //画像格納用インスタンス準備
    cv::Mat captureImage;  //キャプチャ用
    cv::Mat frameImage = cv::Mat(cv::Size(width,height), CV_8UC3);  //処理用
    cv::Mat grayImage(cv::Size(width,height), CV_8UC1);  //1チャンネル
    cv::Mat resultGImage(cv::Size(width,height), CV_8UC1);  //1チャンネル
    cv::Mat recImage(cv::Size(width,height), CV_8UC3);  //3チャンネル
    
    //ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Gray");
    cv::moveWindow("Gray", width, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", width, height);
    
    //ルックアップテーブルの作成
    unsigned char lookupTable[256];

    for(int i=0; i<256; i++){
        //6~8 階調にポスタリゼーション化するためのルックアップテーブルを作成
        lookupTable[i] = (i/43)*43;
    }
    
    //ビデオライタ生成(ファイル名，コーデック，フレームレート，フレームサイズ)
    cv::VideoWriter rec("./dst/dip04_kadai1.mp4", cv::VideoWriter::fourcc('M','P','4','V'), 30, recImage.size());
    
    //動画像処理無限ループ
    while (1) {
        //カメラから1フレームを" captureImage"に読み込み
        capture >> captureImage;
        if (captureImage.data==NULL) break;
        
        //"captureImage"をリサイズして"frameImage"に格納
        cv::resize(captureImage, frameImage, frameImage.size());
        
        //"frameImage"をグレースケールに変換して"grayImage"に格納
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);
        
        //"grayImage"の各画像を走査して，ルックアップテーブルに基づいて画素値変換して"resultGImage"に格納
        for(int j=0; j<height; j++){
            for(int i=0; i<width; i++){
                //座標(i,j)の画素値"s"取得
                unsigned char s = grayImage.at<unsigned char>(j, i);
                
                //ルックアップテーブルで画素値"s"を変換
                s = lookupTable[s];

                //変換後の画素値"s"を"resultImage"の座標(i,j)に格納
                resultGImage.at<unsigned char>(j, i) = s;
            }
        }
        
        //ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Gray", grayImage);
        cv::imshow("Result", resultGImage);
        
        //動画ファイル書き出し
        cv::cvtColor(resultGImage, recImage, cv::COLOR_GRAY2BGR);  //動画用3チャンネル画像生成
        rec << recImage;  //ビデオライタに画像出力
        
        //キー入力待ち
        char key = cv::waitKey(20);  //20ミリ秒待機
        if (key=='q') break;
    }
    
    return 0;
}
