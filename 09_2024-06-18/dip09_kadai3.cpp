#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

// #define DEBUG

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
    cv::Mat reticleImage;

    cv::namedWindow("Edge");
    cv::moveWindow("Edge", 100, 100);
    cv::namedWindow("Reticle");
    cv::moveWindow("Reticle", 200, 200);
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);

    std::vector<cv::Vec2f> lines;
    
    // ビデオライタ生成
    // h264 でエンコード
    cv::VideoWriter rec("./dst/dip09_kadai3.mp4", cv::VideoWriter::fourcc('a', 'v', 'c', '1'), 20, frameImage.size());

    while (true) {
        reticleImage = cv::Mat::zeros(imageSize, CV_8UC1);
        capture >> originalImage;

        #ifdef DEBUG
        if (originalImage.empty()) {
            capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }
        #endif

        // フレームが正しく読み込まれたか確認
        if (originalImage.empty()) {
            std::cerr << "Failed to capture frame\n";
            break;
        }

        // オリジナルのサイズが意図しない場合は、サイズの確認を行う
        if (originalImage.size() != imageSize) {
            std::cerr << "Frame size does not match the expected size\n";
            continue; // サイズが違う場合は処理をスキップ
        }

        // 最終的な長方形の辺を入れる
        std::vector<cv::Vec2f> rectangleLines;

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

                #ifdef DEBUG
                cv::line(frameImage, pt1, pt2, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
                #endif

                // 長方形の辺を保存
                rectangleLines.push_back(line);
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

                #ifdef DEBUG
                cv::line(frameImage, pt1, pt2, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
                #endif

                // 長方形の辺を保存
                rectangleLines.push_back(line);
            }
        }


        // 長方形の頂点(rectangleLinesの交点)を描画
        std::vector<cv::Point> rectanglePoints;
        if (rectangleLines.size() >= 4) {
            for (int i = 0; i < rectangleLines.size(); i++) {
                for (int j = i + 1; j < rectangleLines.size(); j++) {
                    float rho1 = rectangleLines[i][0], theta1 = rectangleLines[i][1];
                    float rho2 = rectangleLines[j][0], theta2 = rectangleLines[j][1];
                    double a1 = cos(theta1), b1 = sin(theta1);
                    double a2 = cos(theta2), b2 = sin(theta2);
                    double d = a1 * b2 - a2 * b1;
                    if (std::abs(d) < 1e-6) continue;
                    double x = (b2 * rho1 - b1 * rho2) / d;
                    double y = (a1 * rho2 - a2 * rho1) / d;
                    cv::Point point(cvRound(x), cvRound(y));
                    cv::circle(frameImage, point, 10, cv::Scalar(0, 255, 0), -1);
                    cv::circle(reticleImage, point, 10, cv::Scalar(255), -1);

                    rectanglePoints.push_back(point);
                }
            }
        }

        //reticleImageの輪郭を抽出
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(reticleImage, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // 各輪郭の中心同士を直線で結ぶ
        for (int i = 0; i < contours.size(); i++) {
            for (int j = i + 1; j < contours.size(); j++) {
                cv::Moments moments1 = cv::moments(contours[i]);
                cv::Moments moments2 = cv::moments(contours[j]);
                cv::Point center1(moments1.m10 / moments1.m00, moments1.m01 / moments1.m00);
                cv::Point center2(moments2.m10 / moments2.m00, moments2.m01 / moments2.m00);
                cv::line(frameImage, center1, center2, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
            }
        }



        cv::imshow("Frame", frameImage);
        cv::imshow("Edge", edgeImage);
        cv::imshow("Reticle", reticleImage);

        int key = cv::waitKey(10);
        if (key == 'q') break;

        rec << frameImage;  //ビデオライタに画像出力
    }

    capture.release();
    std::cout << "Finished" << std::endl;
    return 0;
}
