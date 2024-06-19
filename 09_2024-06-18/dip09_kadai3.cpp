#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

bool isParallel(float theta1, float theta2, float angleThreshold = CV_PI / 180 * 10) {
    return std::abs(theta1 - theta2) < angleThreshold || std::abs(theta1 - theta2 - CV_PI) < angleThreshold;
}

bool isPerpendicular(float theta1, float theta2, float angleThreshold = CV_PI / 180 * 5) {
    float angleDiff = std::abs(theta1 - theta2);
    return std::abs(angleDiff - CV_PI / 2) < angleThreshold || std::abs(angleDiff - 3 * CV_PI / 2) < angleThreshold;
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

        // 直線の検出
        cv::HoughLines(edgeImage, lines, 1, CV_PI / 180, 60);

        // 極めて隣接した直線を複数検出しないようにする
        std::vector<cv::Vec2f> uniqueLines;
        for (int i = 0; i < lines.size(); i++) {
            cv::Vec2f line = lines[i];
            float rho = line[0], theta = line[1];
            bool isUnique = true;
            for (int j = 0; j < uniqueLines.size(); j++) {
                cv::Vec2f uniqueLine = uniqueLines[j];
                float uniqueRho = uniqueLine[0], uniqueTheta = uniqueLine[1];
                if (std::abs(rho - uniqueRho) < 20 && std::abs(theta - uniqueTheta) < CV_PI / 180 * 10) {
                    isUnique = false;
                    break;
                }
            }
            if (isUnique) uniqueLines.push_back(line);
        }

        // 並行な直線を検出して色分けする
        std::vector<std::pair<cv::Vec2f, cv::Vec2f>> parallelPairs;
        for (int i = 0; i < uniqueLines.size(); i++) {
            for (int j = i + 1; j < uniqueLines.size(); j++) {
                if (isParallel(uniqueLines[i][1], uniqueLines[j][1])) {
                    parallelPairs.push_back(std::make_pair(uniqueLines[i], uniqueLines[j]));
                }
            }
        }

        // 最も距離の遠い並行な直線ペアを探す
        double maxDistance = 0;
        std::pair<cv::Vec2f, cv::Vec2f> furthestPair;
        for (const auto& pair : parallelPairs) {
            double distance = std::abs(pair.first[0] - pair.second[0]);
            if (distance > maxDistance) {
                maxDistance = distance;
                furthestPair = pair;
            }
        }

        // 最も距離の遠い並行な直線ペアを描画
        if (maxDistance > 0) {
            for (const auto& line : {furthestPair.first, furthestPair.second}) {
                float rho = line[0], theta = line[1];
                double a = cos(theta), b = sin(theta);
                double x0 = a * rho, y0 = b * rho;
                cv::Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * a));
                cv::Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * a));
                cv::line(frameImage, pt1, pt2, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
            }
        }

        // 緑色の線と垂直に交わる直線を検出
        std::vector<cv::Vec2f> perpendicularLines;
        if (maxDistance > 0) {
            for (const auto& line : uniqueLines) {
                if (isPerpendicular(furthestPair.first[1], line[1]) || isPerpendicular(furthestPair.second[1], line[1])) {
                    perpendicularLines.push_back(line);
                }
            }
        }

        // 垂直に交わる直線ペアのうち最も距離の遠いペアを探す
        double maxPerpendicularDistance = 0;
        std::pair<cv::Vec2f, cv::Vec2f> furthestPerpendicularPair;
        for (int i = 0; i < perpendicularLines.size(); i++) {
            for (int j = i + 1; j < perpendicularLines.size(); j++) {
                double distance = std::abs(perpendicularLines[i][0] - perpendicularLines[j][0]);
                if (distance > maxPerpendicularDistance) {
                    maxPerpendicularDistance = distance;
                    furthestPerpendicularPair = std::make_pair(perpendicularLines[i], perpendicularLines[j]);
                }
            }
        }

        // 最も距離の遠い垂直な直線ペアを青色で描画
        if (maxPerpendicularDistance > 0) {
            for (const auto& line : {furthestPerpendicularPair.first, furthestPerpendicularPair.second}) {
                float rho = line[0], theta = line[1];
                double a = cos(theta), b = sin(theta);
                double x0 = a * rho, y0 = b * rho;
                cv::Point pt1(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * a));
                cv::Point pt2(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * a));
                cv::line(frameImage, pt1, pt2, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
            }
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
