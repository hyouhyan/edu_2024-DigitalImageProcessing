#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

// 顔色の変更
void change_face_color(cv::Mat& faceImage, cv::Mat& hsvImage, cv::Rect rect, cv::Vec3b afterColor)
{
    // 色解析しやすいようにHSV色空間に変換
    cv::cvtColor(faceImage, hsvImage, cv::COLOR_BGR2HSV);

    for(int j=rect.y; j<rect.y+rect.height; j++) {
		if(j<0 || j>= hsvImage.rows) continue;
        for(int i=rect.x; i<rect.x+rect.width; i++) {
			if(i<0 || i>= hsvImage.cols) continue;
            cv::Vec3b s = hsvImage.at<cv::Vec3b>(j, i);
			// hsvImage.at<cv::Vec3b>(j, 2*rect.x+rect.width-i) = s;
            // 肌色領域のみ変換
            if(s[0]> 0 && s[0]< 15 &&
                s[1]>60 && s[1]<130 &&
                s[2]>70 && s[2]<200)
            {
                if(afterColor[0] != -1) {
                    s[0] = afterColor[0];
                }
                if(afterColor[1] != -1) {
                    s[1] = afterColor[1];
                }
                if(afterColor[2] != -1) {
                    s[2] = afterColor[2];
                }
                
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
        std::vector<cv::Rect> faces;

        // ③画像中から検出対象の情報を取得する
        faceClassifier.detectMultiScale(frameImage, faces, 1.1, 3, 0, cv::Size(20,20));

        // ④顔領域の検出
        for (int i = 0; i < faces.size(); i++) {
            // 検出情報から顔の位置情報を取得
            cv::Rect face = faces[i];
            // 大きさによるチェック。
            if(face.width*face.height < 150*150){
                continue; // 小さい矩形は採用しない
            }

            cv::Rect tmp_face = face;
            face.x = tmp_face.x-25;
            face.y = tmp_face.y-80;
            face.width = tmp_face.width+50;
            face.height = tmp_face.height+100;
            
            // ⑤画像の加工
            change_face_color(frameImage, hsvImage, face);
            
            // 取得した顔の位置情報に基づき、矩形描画を行う
            cv::rectangle(frameImage,
                cv::Point(face.x, face.y),
                cv::Point(face.x + face.width, face.y + face.height),
                CV_RGB(255, 0, 0),
                3, cv::LINE_AA);
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
