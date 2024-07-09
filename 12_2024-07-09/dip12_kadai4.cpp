#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

// 顔色の変更
void change_face_color(cv::Mat& faceImage, cv::Mat& hsvImage, cv::Rect rect)
{
    // 色解析しやすいようにHSV色空間に変換
    cv::cvtColor(faceImage, hsvImage, cv::COLOR_BGR2HSV);

    for(int j=rect.y; j<rect.y+rect.height; j++) {
		if(j<0 || j>= hsvImage.rows) continue;
        for(int i=rect.x; i<rect.x+rect.width; i++) {
			if(i<0 || i>= hsvImage.cols) continue;
            cv::Vec3b s = hsvImage.at<cv::Vec3b>(j, i);
			hsvImage.at<cv::Vec3b>(j, 2*rect.x+rect.width-i) = s;
            // 肌色領域のみ変換
            if(s[0]> 0 && s[0]< 45 &&
                s[1]>50 && s[1]<255 &&
                s[2]>50 && s[2]<255)
            {
                s[0] = 120;
                hsvImage.at<cv::Vec3b>(j, i) = s;
            }
        }
    }
    cv::cvtColor(hsvImage, faceImage, cv::COLOR_HSV2BGR);
}

//main関数
int main(int argc, char* argv[])
{
    //OpenCV初期設定処理
    //カメラキャプチャの初期化
    cv::VideoCapture capture("./src/face.mov");
    if (capture.isOpened()==0) {
        //カメラが見つからないときはメッセージを表示して終了
        printf("Camera not found\n");
        exit(1);
    }

    cv::Mat originalImage, frameImage, hsvImage, tempImage;
    cv::Size imageSize(720, 405);  // 画像サイズ
    cv::CascadeClassifier faceClassifier;  // 顔認識用分類器
    cv::CascadeClassifier eyeClassifier;  // 目認識用分類器

    //3チャンネル画像"hsvImage"と"tempImage"の確保（ビデオと同サイズ）
    hsvImage = cv::Mat(imageSize, CV_8UC3);
    tempImage = cv::Mat(imageSize, CV_8UC3);

    //OpenCVウィンドウ生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Face");
    cv::moveWindow("Face", imageSize.width, 0);

    // ①正面顔検出器の読み込み
    faceClassifier.load("./src/haarcascades/haarcascade_frontalface_default.xml");

    // 眼検出器の読み込み
    eyeClassifier.load("./src/haarcascades/haarcascade_mcs_eyepair_big.xml");

    // うさみみ画像の読み込み
    cv::Mat usamimiImage = cv::imread("./src/うさミミ.jpg", cv::IMREAD_UNCHANGED);


    
    while(1){
        //ビデオキャプチャから1フレーム画像取得
        capture >> originalImage;
        
        //ビデオが終了したら巻き戻し

        if(originalImage.data==NULL) {
            capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }

        cv::resize(originalImage, frameImage, imageSize);

        //フレーム画像表示
        cv::imshow("Frame", frameImage);

        // ②検出情報を受け取るための配列を用意する
        std::vector<cv::Rect> faces, eyes;

        // ③画像中から検出対象の情報を取得する
        faceClassifier.detectMultiScale(frameImage, faces, 1.1, 3, 0, cv::Size(20,20));
        eyeClassifier.detectMultiScale(frameImage, eyes, 1.1, 3, 0, cv::Size(10,10));

        // ④顔領域の検出
        for (int i = 0; i < faces.size(); i++) {
            // 検出情報から顔の位置情報を取得
            cv::Rect face = faces[i];
            // 大きさによるチェック。
            if(face.width*face.height < 100*100){
                continue; // 小さい矩形は採用しない
            }
            
            
            // 取得した顔の位置情報に基づき、矩形描画を行う
            cv::rectangle(frameImage,
                cv::Point(face.x, face.y),
                cv::Point(face.x + face.width, face.y + face.height),
                CV_RGB(255, 0, 0),
                3, cv::LINE_AA);
            
            // 顔の大きさに応じて、顔上部にうさみみを合成(白い部分は透過)
            cv::resize(usamimiImage, tempImage, cv::Size(face.width, face.height/2));
            for(int j=0; j<tempImage.rows; j++) {
                for(int i=0; i<tempImage.cols; i++) {
                    cv::Vec4b s = tempImage.at<cv::Vec4b>(j, i);
                    if(s[3] > 0) {
                        frameImage.at<cv::Vec3b>(face.y+j, face.x+i) = cv::Vec3b(s[0], s[1], s[2]);
                    }
                }
            }
        }

        // 眼領域の検出
        for (int i = 0; i < eyes.size(); i++) {
            // 検出情報から顔の位置情報を取得
            cv::Rect eye = eyes[i];
            // 取得した顔の位置情報に基づき、矩形描画を行う
            cv::rectangle(frameImage,
                cv::Point(eye.x, eye.y),
                cv::Point(eye.x + eye.width, eye.y + eye.height),
                CV_RGB(0, 255, 0),
                3, cv::LINE_AA
            );
        }


        
        //認識結果画像表示
        cv::imshow("Face", frameImage);
            
        char key = cv::waitKey(10);
        if(key == 'q'){
            break;
        }
    }
    
    return 0;
}
