#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, const char * argv[])
{
	//ビデオファイル"movie.mov"を取り込み
    cv::VideoCapture capture("movie.mov");  //指定したビデオファイルをオープン
    //ビデオファイルがオープンできたかどうかをチェック
    if (capture.isOpened()==0) {
        printf("Specified video not found\n");
        return -1;
    }
    
    //フレームの大きさを取得
    int width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    printf("FRAME SIZE = (%d %d)\n", width, height);
		
    //②画像格納用インスタンス準備
    cv::Mat frameImage;
    cv::Mat hsvImage;
    cv::Mat resultImage;
    
    //動画ファイルの書き出し準備
    //ビデオライタの生成
//    cv::VideoWriter rec("rec.mpg", CV_FOURCC('P','I','M','1'), 30, cv::Size(width,height));
    
    //③ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", width, 0);
		
	//動画像処理無限ループ：「ビデオキャプチャから1フレーム取り込み」→「画像処理」→「表示」の繰り返し
	while (1) {
        //④カメラから1フレーム読み込み（ストリーム入力）
        capture >> frameImage;
        if(frameImage.data == NULL) break;
		
        //⑤画像処理
		//色空間変換(BGR -> HSV)
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);
		
		//色の抽出
		for (int y=0; y<frameImage.rows; y++) {
			for (int x=0; x<frameImage.cols; x++) {
                cv::Vec3b s = hsvImage.at<cv::Vec3b>(y, x);
				//色相(H)と彩度(S)の値を用いてボール抽出
				if (s[0]>60 && s[0]<80 && s[1]>100) {
				}
				else {
					s[0] = 0; s[1] = 0; s[2] = 0;
				}
                hsvImage.at<cv::Vec3b>(y, x) = s;
			}
		}
		
		//色空間変換(HSV -> BGR)
        cv::cvtColor(hsvImage, resultImage, cv::COLOR_HSV2BGR);
		
        //⑥ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Result", resultImage);
        
        //動画ファイルの書き出し
//        rec << resultImage;  //ビデオライタに画像出力
        
        //⑦キー入力待ち
        char key = cv::waitKey(30);  //30ミリ秒待機
        if(key == 'q') break;
	}
	
	//メッセージを出力して終了
	printf("Finished\n");
	return 0;
}

