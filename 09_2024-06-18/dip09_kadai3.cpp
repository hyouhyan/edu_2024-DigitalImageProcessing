#include <iostream>
#include <opencv2/opencv.hpp>

// 直線同士の交点を計算する関数
cv::Point2f computeIntersection(cv::Vec2f line1, cv::Vec2f line2) {
    float rho1 = line1[0], theta1 = line1[1];
    float rho2 = line2[0], theta2 = line2[1];
    float a1 = cos(theta1), b1 = sin(theta1);
    float a2 = cos(theta2), b2 = sin(theta2);
    float x = (b2 * rho1 - b1 * rho2) / (a1 * b2 - a2 * b1);
    float y = (a1 * rho2 - a2 * rho1) / (a1 * b2 - a2 * b1);
    return cv::Point2f(x, y);
}

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

        std::vector<cv::Point2f> cardCorners;
        for (size_t i = 0; i < lines.size(); i++) {
            for (size_t j = i + 1; j < lines.size(); j++) {
                cv::Point2f pt = computeIntersection(lines[i], lines[j]);
                if (pt.x >= 0 && pt.x < w && pt.y >= 0 && pt.y < h) {
                    cardCorners.push_back(pt);
                }
            }
        }

        if (cardCorners.size() >= 4) {
            std::sort(cardCorners.begin(), cardCorners.end(), [](const cv::Point2f& a, const cv::Point2f& b) {
                return a.x < b.x || (a.x == b.x && a.y < b.y);
            });

            cv::Point2f tl = cardCorners[0];
            cv::Point2f tr = cardCorners[1];
            cv::Point2f br = cardCorners[2];
            cv::Point2f bl = cardCorners[3];

            cv::circle(frameImage, tl, 5, cv::Scalar(0, 255, 0), -1);
            cv::circle(frameImage, tr, 5, cv::Scalar(0, 255, 0), -1);
            cv::circle(frameImage, br, 5, cv::Scalar(0, 255, 0), -1);
            cv::circle(frameImage, bl, 5, cv::Scalar(0, 255, 0), -1);

            cv::line(frameImage, tl, tr, cv::Scalar(255, 0, 0), 2);
            cv::line(frameImage, tr, br, cv::Scalar(255, 0, 0), 2);
            cv::line(frameImage, br, bl, cv::Scalar(255, 0, 0), 2);
            cv::line(frameImage, bl, tl, cv::Scalar(255, 0, 0), 2);
        }

        cv::imshow("Frame", frameImage);
        cv::imshow("Edge", edgeImage);

        int key = cv::waitKey(10);
        if (key == 'q') break;
    }

    capture.release();
    std::cout << "Finished" << std::endl;
    return 0;
}
