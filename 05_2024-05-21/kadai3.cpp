#include <iostream>
#include <opencv2/opencv.hpp>

void myAnimeColor(const cv::Mat &src, cv::Mat &dst);
void drawShadingLines(cv::Mat &image, const cv::Mat &grayImage, int blockSize);

unsigned char lookupTable[3][256];

int main(int argc, char *argv[]) {
    for (int i = 0; i < 256; i++) {
        lookupTable[0][i] = i;
        lookupTable[1][i] = i;
        if (i < 64)
            lookupTable[2][i] = 0;
        else if (i < 128)
            lookupTable[2][i] = 85;
        else if (i < 196)
            lookupTable[2][i] = 170;
        else
            lookupTable[2][i] = 255;
    }

    cv::VideoCapture capture("scene.mov");
    if (!capture.isOpened()) {
        std::cerr << "Camera not found\n";
        return -1;
    }

    int width = 720, height = 405;
    cv::Mat frameImage, originalImage(cv::Size(width, height), CV_8UC3);
    cv::Mat grayImage(cv::Size(width, height), CV_8UC1);
    cv::Mat animeImage(cv::Size(width, height), CV_8UC3);
    cv::Mat resultImage(cv::Size(width, height), CV_8UC3);

    cv::VideoWriter rec("dip05_kadai2.mp4", cv::VideoWriter::fourcc('H', '2', '6', '4'), 30, originalImage.size());

    while (true) {
        capture >> frameImage;
        if (frameImage.empty()) break;

        cv::resize(frameImage, originalImage, originalImage.size());
        cv::cvtColor(originalImage, grayImage, cv::COLOR_BGR2GRAY);
        cv::medianBlur(grayImage, grayImage, 3);

        myAnimeColor(originalImage, animeImage);
        drawShadingLines(animeImage, grayImage, 4);

        cv::imshow("Original", originalImage);
        cv::imshow("Result", animeImage);

        rec << animeImage;

        int key = cv::waitKey(10);
        if (key == 'q') break;
    }

    std::cout << "Finished\n";
    return 0;
}

void myAnimeColor(const cv::Mat &src, cv::Mat &dst) {
    cv::Mat cImage(src.size(), CV_8UC3);
    cv::cvtColor(src, cImage, cv::COLOR_BGR2HSV);
    cv::Vec3b s;
    for (int j = 0; j < src.rows; j++) {
        for (int i = 0; i < src.cols; i++) {
            s = cImage.at<cv::Vec3b>(j, i);
            s[0] = lookupTable[0][s[0]];
            s[1] = lookupTable[1][s[1]];
            s[2] = lookupTable[2][s[2]];
            cImage.at<cv::Vec3b>(j, i) = s;
        }
    }
    cv::cvtColor(cImage, dst, cv::COLOR_HSV2BGR);
}

void drawShadingLines(cv::Mat &image, const cv::Mat &grayImage, int blockSize) {
    for (int y = 0; y < grayImage.rows; y += blockSize) {
        for (int x = 0; x < grayImage.cols; x += blockSize) {
            int blockWidth = std::min(blockSize, grayImage.cols - x);
            int blockHeight = std::min(blockSize, grayImage.rows - y);

            cv::Rect region(x, y, blockWidth, blockHeight);
            cv::Mat block = grayImage(region);
            double averageIntensity = cv::mean(block)[0];

            int numLines = 0;
            if (averageIntensity < 64) numLines = 3;
            else if (averageIntensity < 128) numLines = 2;
            else if (averageIntensity < 196) numLines = 1;

            for (int i = 0; i < numLines; ++i) {
                int offset = (i + 1) * blockWidth / 4;
                cv::line(image, cv::Point(x, y + offset), cv::Point(x + offset, y), cv::Scalar(0, 0, 0), 1);
                cv::line(image, cv::Point(x + blockWidth - offset, y + blockHeight), cv::Point(x + blockWidth, y + blockHeight - offset), cv::Scalar(0, 0, 0), 1);
            }
        }
    }
}
