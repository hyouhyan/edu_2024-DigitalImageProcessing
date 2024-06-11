#include <iostream>
#include <opencv2/opencv.hpp>

int main(void){
    cv::Mat sourceImage = cv::imread("./src/ferarri.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data==0) {
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);

    cv::Mat backgroundImage = cv::imread("./src/milano.jpg", cv::IMREAD_COLOR);
    if (backgroundImage.data==0) {
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", backgroundImage.cols, backgroundImage.rows);

    cv::Mat frameImage = cv::Mat::zeros(sourceImage.size(), CV_8UC3);

    // ferarri.jpgの拡大率と角度を保存する変数
    double scale = 1.0;
    double angle = 0.0;

    cv::VideoWriter rec("./dst/dip08_kadai0_k22047.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, frameImage.size());

    // 10秒の動画を作成するためのループ
    for (int i = 0; i < 300; i++) {
        // milamo.jpgを背景にする
        frameImage = backgroundImage.clone();

        // ferarri.jpgの拡大率と角度を変更してframeImageに描画
        cv::Mat rotateMat = cv::getRotationMatrix2D(cv::Point2f(sourceImage.cols/2, sourceImage.rows/2), angle, scale);
        cv::Mat rotatedImage;
        cv::warpAffine(sourceImage, rotatedImage, rotateMat, frameImage.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0,0,0));

        // フェラーリ画像を背景画像に重ね合わせ
        int x = (backgroundImage.cols - rotatedImage.cols) / 2;
        int y = (backgroundImage.rows - rotatedImage.rows) / 2;
        for (int r = 0; r < rotatedImage.rows; ++r) {
            for (int c = 0; c < rotatedImage.cols; ++c) {
                cv::Vec3b color = rotatedImage.at<cv::Vec3b>(r, c);
                if (color != cv::Vec3b(0, 0, 0)) {
                    frameImage.at<cv::Vec3b>(y + r, x + c) = color;
                }
            }
        }

        // 画像を動画に保存
        rec << frameImage;

        // 拡大率・角度を少し小さくする
        scale *= 0.99;
        angle += 1.0;
    }

    

    return 0;
}