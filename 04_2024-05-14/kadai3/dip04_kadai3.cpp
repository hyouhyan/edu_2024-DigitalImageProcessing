#include <iostream>
#include <opencv2/opencv.hpp>

const int PATTERN_SIZE = 4;
const int GRAY_LEVELS = 17;

// 濃度パターンを動的に生成
void generatePatterns(unsigned char patterns[GRAY_LEVELS][PATTERN_SIZE][PATTERN_SIZE]) {
    for (int level = 0; level < GRAY_LEVELS; ++level) {
        int numWhitePixels = (level * PATTERN_SIZE * PATTERN_SIZE) / (GRAY_LEVELS - 1);
        int count = 0;
        
        for (int y = 0; y < PATTERN_SIZE; ++y) {
            for (int x = 0; x < PATTERN_SIZE; ++x) {
                if (count < numWhitePixels) {
                    patterns[level][y][x] = 255;
                } else {
                    patterns[level][y][x] = 0;
                }
                count++;
            }
        }
    }
}

int main (int argc, const char* argv[])
{
    int width = 640, height = 480;
    
    cv::VideoCapture capture(0);  // カメラ0番をオープン
    if (capture.isOpened() == 0) {
        printf("Camera not found\n");
        return -1;
    }
    
    cv::Mat captureImage;
    cv::Mat frameImage(cv::Size(width, height), CV_8UC3);
    cv::Mat grayImage(cv::Size(width, height), CV_8UC1);
    cv::Mat smallGrayImage(cv::Size(width / 4, height / 4), CV_8UC1);
    cv::Mat resultImage(cv::Size(width, height), CV_8UC1);
    cv::Mat recImage(cv::Size(width, height), CV_8UC3);
    
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Gray");
    cv::moveWindow("Gray", width, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", width, height);
    
    cv::VideoWriter rec("rec.mp4", cv::VideoWriter::fourcc('M','P','4','V'), 30, recImage.size());
    
    // 濃度パターンの生成
    unsigned char patterns[GRAY_LEVELS][PATTERN_SIZE][PATTERN_SIZE];
    generatePatterns(patterns);
    
    while (1) {
        capture >> captureImage;
        if (captureImage.data == NULL) break;
        
        cv::resize(captureImage, frameImage, frameImage.size());
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);
        
        // グレースケール画像を1/4のサイズに縮小
        cv::resize(grayImage, smallGrayImage, smallGrayImage.size());
        
        // 濃度パターン法によるハーフトーニング処理
        for (int j = 0; j < smallGrayImage.rows; j++) {
            for (int i = 0; i < smallGrayImage.cols; i++) {
                unsigned char s = smallGrayImage.at<unsigned char>(j, i);
                int level = s * (GRAY_LEVELS - 1) / 255;
                
                for (int y = 0; y < PATTERN_SIZE; y++) {
                    for (int x = 0; x < PATTERN_SIZE; x++) {
                        resultImage.at<unsigned char>(j * PATTERN_SIZE + y, i * PATTERN_SIZE + x) = patterns[level][y][x];
                    }
                }
            }
        }
        
        cv::imshow("Frame", frameImage);
        cv::imshow("Gray", grayImage);
        cv::imshow("Result", resultImage);
        
        cv::cvtColor(resultImage, recImage, cv::COLOR_GRAY2BGR);
        rec << recImage;
        
        char key = cv::waitKey(20);
        if (key == 'q') break;
    }
    
    return 0;
}
