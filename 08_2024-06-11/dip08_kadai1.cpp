#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

using namespace cv;
using namespace std;

//色でボールを検出して中心座標を求める関数
Point2f detectBallCenter(const Mat& binImage) {
    Moments m = moments(binImage, true);
    return Point2f(m.m10 / m.m00, m.m01 / m.m00);
}

int main (int argc, const char * argv[])
{
    //ビデオファイル"colball.mov"を取り込み
    VideoCapture capture("./src/colball.mov");
    if (!capture.isOpened()) {
        printf("No video\n");
        return -1;
    }
    //フレームサイズ取得
    int width = capture.get(CAP_PROP_FRAME_WIDTH);
    int height = capture.get(CAP_PROP_FRAME_HEIGHT);
    printf("Frame Size = (%d %d)\n", width, height);
    
    //合成用画像"face.jpg"の読み込み
    Mat compImage = imread("./src/face.jpg", IMREAD_COLOR);
    if (compImage.empty()) {
        printf("No image\n");
        exit(0);
    }
    printf("Image Size = (%d, %d)\n", compImage.cols, compImage.rows);

    //画像格納用インスタンス準備
    Mat frameImage, hsvImage;
    Mat binImageBlue, binImageGreen, binImageRed, binImageYellow;
    
    //ウィンドウの生成と移動
    namedWindow("Frame");
    moveWindow("Frame", 0, 0);
    namedWindow("Image");
    moveWindow("Image", width, 0);
    namedWindow("Bin");
    moveWindow("Bin", 0, height);

    //領域膨張収縮用構造要素
    Mat element1 = getStructuringElement(MORPH_ELLIPSE, Size(3,3));
    Mat element2 = getStructuringElement(MORPH_ELLIPSE, Size(5,5));

    //合成用画像の表示
    imshow("Image", compImage);

    VideoWriter rec("./dst/dip08_kadai1_k22047.mp4", VideoWriter::fourcc('m','p','4','v'), 30, Size(width, height));

    //動画像処理無限ループ
    while (true) {
        //カメラから1フレーム読み込み
        capture >> frameImage;
        if(frameImage.empty()) break;
        
        //色空間変換(BGR -> HSV)
        cvtColor(frameImage, hsvImage, COLOR_BGR2HSV);
        
        //ボール領域画像リセット
        binImageBlue = Mat::zeros(height, width, CV_8UC1);
        binImageGreen = Mat::zeros(height, width, CV_8UC1);
        binImageRed = Mat::zeros(height, width, CV_8UC1);
        binImageYellow = Mat::zeros(height, width, CV_8UC1);
        
        //ボール領域抽出
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Vec3b s = hsvImage.at<Vec3b>(y, x);
                
                //HSVの値を用いてボール抽出
                if (s[0] > 90 && s[0] < 110 && s[1] > 120 && s[2] > 64) {  //Blue
                    binImageBlue.at<unsigned char>(y, x) = 255;
                }
                else if (s[0] > 60 && s[0] < 80 && s[1] > 120 && s[2] > 64) {  //Green
                    binImageGreen.at<unsigned char>(y, x) = 255;
                }
                else if (((s[0] > 0 && s[0] < 5) || (s[0] > 160 && s[0] < 180)) && s[1] > 110 && s[2] > 64) {  //Red
                    binImageRed.at<unsigned char>(y, x) = 255;
                }
                else if (s[0] > 20 && s[0] < 30 && s[1] > 160 && s[2] > 64) {  //Yellow
                    binImageYellow.at<unsigned char>(y, x) = 255;
                }
            }
        }
        
        //収縮膨張による各ボール領域のノイズ・穴除去
        erode(binImageBlue, binImageBlue, element1, Point(-1,-1), 1);  //収縮
        dilate(binImageBlue, binImageBlue, element2, Point(-1,-1), 2);  //膨張

        erode(binImageGreen, binImageGreen, element1, Point(-1,-1), 1);  //収縮
        dilate(binImageGreen, binImageGreen, element2, Point(-1,-1), 2);  //膨張

        erode(binImageRed, binImageRed, element1, Point(-1,-1), 1);  //収縮
        dilate(binImageRed, binImageRed, element2, Point(-1,-1), 2);  //膨張

        erode(binImageYellow, binImageYellow, element1, Point(-1,-1), 1);  //収縮
        dilate(binImageYellow, binImageYellow, element2, Point(-1,-1), 2);  //膨張
        
        //各ボールの中心座標を計算
        Point2f blueCenter = detectBallCenter(binImageBlue);
        Point2f greenCenter = detectBallCenter(binImageGreen);
        Point2f redCenter = detectBallCenter(binImageRed);
        Point2f yellowCenter = detectBallCenter(binImageYellow);

        // ボールの中心座標が有効か確認
        if (blueCenter.x > 0 && blueCenter.y > 0 && greenCenter.x > 0 && greenCenter.y > 0 &&
            redCenter.x > 0 && redCenter.y > 0 && yellowCenter.x > 0 && yellowCenter.y > 0) {

            // 射影変換用の4点を指定
            vector<Point2f> srcPoints = { Point2f(0, 0), Point2f(compImage.cols, 0), Point2f(compImage.cols, compImage.rows), Point2f(0, compImage.rows) };
            vector<Point2f> dstPoints = { blueCenter, greenCenter, redCenter, yellowCenter };

            // 射影変換行列を計算
            Mat transformMatrix = getPerspectiveTransform(srcPoints, dstPoints);

            // フレームに画像を射影変換で合成
            Mat warpedImage;
            warpPerspective(compImage, warpedImage, transformMatrix, frameImage.size());

            // 半透明で合成
            addWeighted(frameImage, 0.7, warpedImage, 0.3, 0, frameImage);
        }

        // フレーム画像の表示
        imshow("Frame", frameImage);

        // 動画ファイル書き出し
        rec << frameImage;  //ビデオライタに画像出力

        // キー入力待ち
        // char key = waitKey(20);  //20ミリ秒待機
        // if(key == 'q') break;
    }
    
    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
