#include <iostream>
#include <opencv2/opencv.hpp>

int main (int argc, const char * argv[])
{
    cv::VideoCapture capture("movie.mov");
    if (capture.isOpened() == 0) {
        printf("Specified video not found\n");
        return -1;
    }
    
    int width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    printf("FRAME SIZE = (%d %d)\n", width, height);
    
    cv::Mat frameImage;
    cv::Mat hsvImage;
    cv::Mat resultImage;
    
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", width, 0);
    
    std::vector<cv::Point> trajectory;
    // cv::VideoWriter rec("rec.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, cv::Size(width, height));
    //mp4形式で保存
    cv::VideoWriter rec("dip06_kadai3.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30, cv::Size(width, height));
    
    while (1) {
        capture >> frameImage;
        if (frameImage.data == NULL) break;
        
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);
        
        cv::Mat mask;
        cv::inRange(hsvImage, cv::Scalar(60, 100, 100), cv::Scalar(80, 255, 255), mask);
        
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        cv::cvtColor(hsvImage, resultImage, cv::COLOR_HSV2BGR);
        
        cv::Point2f center;
        float radius;
        if (!contours.empty()) {
            std::vector<cv::Point> largestContour = contours[0];
            for (const auto& contour : contours) {
                if (cv::contourArea(contour) > cv::contourArea(largestContour)) {
                    largestContour = contour;
                }
            }
            cv::minEnclosingCircle(largestContour, center, radius);
            trajectory.push_back(center);
            cv::circle(frameImage, center, radius, cv::Scalar(255, 255, 255), 2);
            cv::circle(resultImage, center, radius, cv::Scalar(255, 255, 255), -1);
        }

        cv::cvtColor(resultImage, hsvImage, cv::COLOR_BGR2HSV);
        
        for (size_t i = 0; i < trajectory.size(); ++i) {
            int h = (i * 10) % 180; // 0-179 の範囲で色相を設定
            int s = 255; // 彩度を最大に設定
            int v = 255; // 明度を最大に設定
            cv::circle(hsvImage, trajectory[i], 5, cv::Scalar(h, s, v), -1);
        }
        
        cv::cvtColor(hsvImage, resultImage, cv::COLOR_HSV2BGR);
        
        cv::imshow("Frame", frameImage);
        cv::imshow("Result", resultImage);
        
        rec << resultImage;
        
        char key = cv::waitKey(30);
        if (key == 'q') break;
    }
    
    printf("Finished\n");
    return 0;
}
