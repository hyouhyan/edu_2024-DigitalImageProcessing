#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, const char * argv[])
{
	//①ルートディレクトリの画像ファイル"col.jpg"を読み込んで"sourceImage"に格納
    cv::Mat sourceImage = cv::imread("kadai.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data==0) {  //画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);
	
    //②画像格納用インスタンスの生成
    cv::Mat grayImage(sourceImage.size(), CV_8UC1);  //グレースケール画像用（1チャンネル）
    cv::Mat binImage(sourceImage.size(), CV_8UC1);  //２値画像用（1チャンネル）
    cv::Mat contourImage(sourceImage.size(), CV_8UC3);  //輪郭表示画像用（3チャンネル）
		
	//③原画像をグレースケール画像に、グレースケール画像を２値画像に変換
	//"sourceImage"をグレースケール画像に変換して"grayImage"に出力
    cv::cvtColor(sourceImage, grayImage, cv::COLOR_BGR2GRAY);
	//"grayImage"を2値化して"grayImage"に出力
    cv::threshold(grayImage, binImage, 50, 255, cv::THRESH_BINARY);
	//"sourceImage"のコピーを"contourImage"に出力
	contourImage = sourceImage.clone();
	
	//④輪郭点格納用配列、輪郭の階層用配列の確保
    //輪郭点格納用配列
    std::vector< std::vector<cv::Point> > contours;
	
    //⑤"binImage"からの輪郭抽出処理
	//findContours()では入力画像が壊れるので、tmpImage に一時退避。壊れても良いなら不要。 
    cv::Mat tmpImage = binImage.clone();
    
    //輪郭抽出処理.輪郭ごとに輪郭画素位置を"counter"に格納
    cv::findContours(binImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

    //輪郭データ"contour"を順次描画
    for(int i=0; i<contours.size(); i++){
        //"contourImage"に"contours"の i 番目の領域の輪郭を描画(白色，線幅2，8連結)
        //面積が 10000 以上の図形のみを描画
        if (cv::contourArea(contours[i]) >= 10000) cv::drawContours(contourImage, contours, i, cv::Scalar(255, 255, 255), 2, 8);
    }
	//⑥ウィンドウを生成して各画像を表示
	//原画像
    cv::namedWindow("Source");  //ウィンドウの生成
    cv::moveWindow("Source", 0, 50);  //ウィンドウの表示位置の指定
    cv::imshow("Source", sourceImage);  //ウィンドウに画像を表示
	//グレースケール(2値化)
    cv::namedWindow("Gray");  //ウィンドウの生成
    cv::moveWindow("Gray", 150, 50);  //ウィンドウの表示位置の指定
    cv::imshow("Gray", grayImage);  //ウィンドウに画像を表示
	//輪郭画像(原画像に輪郭を追加)
    cv::namedWindow("Contour");  //ウィンドウの生成
    cv::moveWindow("Contour", 300, 50);  //ウィンドウの表示位置の指定
    cv::imshow("Contour", contourImage);  //ウィンドウに画像を表示

    //画像の出力
    cv::imwrite("dip06_kadai1_result.jpg", contourImage);
	
	//⑦キー入力があるまでここでストップ
    cv::waitKey(0);

    //メッセージを出力して終了
	printf("Finished\n");
	return 0;
}
