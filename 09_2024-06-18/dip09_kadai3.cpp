#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    cv::VideoCapture capture("./src/card.mov");
    if (!capture.isOpened()) {
        std::cout << "Capture not found" << std::endl;
        return -1;
    }

    int w = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int h = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Size imageSize(w, h);
    cv::Mat originalImage, frameImage(imageSize, CV_8UC3), grayImage(imageSize, CV_8UC1), edgeImage(imageSize, CV_8UC1);

    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Edge");
    cv::moveWindow("Edge", 100, 100);

    std::vector<cv::Vec2f> lines;

    while (true) {
        capture >> originalImage;
        if (originalImage.empty()) {
            capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }

        cv::resize(originalImage, frameImage, imageSize);
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);
        cv::Canny(grayImage, edgeImage, 120, 160, 3);
        cv::HoughLines(edgeImage, lines, 1, CV_PI / 180, 250);


        cv::imshow("Frame", frameImage);
        cv::imshow("Edge", edgeImage);

        int key = cv::waitKey(10);
        if (key == 'q') break;
    }

    capture.release();
    std::cout << "Finished" << std::endl;
    return 0;
}
