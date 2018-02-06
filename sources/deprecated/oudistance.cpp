//ace 自适应对比度均衡研究

#include "stdafx.h"
#include <stdio.h>  
#include <iostream>  
 
#include <stdlib.h>  
#include <string.h>  
#include <limits.h>  

#include "string"  
#include "vector"  
#include <windows.h>  
#include <opencv2/legacy/legacy.hpp>  
#include <opencv2/nonfree/nonfree.hpp>//opencv_nonfree模块：包含一些拥有专利的算法，如SIFT、SURF函数源码。   
#include "opencv2/core/core.hpp"  
#include "opencv2/features2d/features2d.hpp"  
#include "opencv2/highgui/highgui.hpp"  
#include <opencv2/nonfree/features2d.hpp>  
#include "math.h"
#include <time.h>



using namespace cv;
using namespace std;




void drawDetectLines(Mat& image, const vector<Vec4i>& lines, Scalar & color)
{
	// 将检测到的直线在图上画出来
	vector<Vec4i>::const_iterator it = lines.begin();
	while (it != lines.end())
	{
		Point pt1((*it)[0], (*it)[1]);
		Point pt2((*it)[2], (*it)[3]);
		line(image, pt1, pt2, color, 2); //  线条宽度设置为2
		++it;
	}
}

struct tm getTime()
{
	time_t timep;
	struct tm now_time;

	time(&timep);

	localtime_s(&now_time, &timep);
	//stringstream ss;
	//ss << now_time.tm_mon + 00 << "_" << now_time.tm_mday + 00 <<
	//	"_" << now_time.tm_hour + 00 << "_" << now_time.tm_min + 00 << "_" << now_time.tm_sec + 00;
	return now_time;
}

bool time_interval(struct tm now_time, struct tm last_time){
	int now = now_time.tm_hour * 3600 + now_time.tm_min * 60 + now_time.tm_sec;
	int last = last_time.tm_hour * 3600 + last_time.tm_min * 60 + last_time.tm_sec;
	if (now - last <= 5)
		return false;
	else 
		return true;

}

void get_final_result(){
	vector<string> result;
	result.push_back("020");
	result.push_back("019");
	result.push_back("018");
	result.push_back("019");
	result.push_back("014");
	result.push_back("019");
	result.push_back("014");
	result.push_back("016");
	result.push_back("019");
	result.push_back("019");
	//result.clear();

	vector<string> temp = result;
	sort(temp.begin(), temp.end());
	temp.erase(unique(temp.begin(), temp.end()), temp.end());//temp是排序后剔除相同元素的result
	vector<int> result_count;
	
	for (int i = 0; i < temp.size(); i++){
		
		result_count.push_back(count(result.begin(), result.end(), temp[i]));
	}
	int index = 0;
	for (int i = 0; i < result_count.size(); i++){//
		if (result_count[i]>result_count[index]){
			index = i;
		}
	}
	string final_result = temp[index];
	cout << final_result << endl;
	for (int i = 0; i < temp.size(); i++){
		cout << "element " << temp[i] << "	count " << result_count[i] << endl;
	}
	
}

Mat cameraUndistort(Mat src){
	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
	cameraMatrix.at<double>(0, 0) = 1.043590111319692e+03;
	cameraMatrix.at<double>(0, 1) = 1.372867556826876;
	cameraMatrix.at<double>(0, 2) = 9.718281655923367e+02;
	cameraMatrix.at<double>(1, 0) = 0;
	cameraMatrix.at<double>(1, 1) = 1.044022268512396e+03;
	cameraMatrix.at<double>(1, 2) = 5.189080037899544e+02;
	cameraMatrix.at<double>(2, 0) = 0;
	cameraMatrix.at<double>(2, 1) = 0;
	cameraMatrix.at<double>(2, 2) = 1;


	Mat distCoeffs = Mat::zeros(5, 1, CV_64F);
	distCoeffs.at<double>(0, 0) = -0.204422240471869;
	distCoeffs.at<double>(1, 0) = 0.031336696740248;
	distCoeffs.at<double>(2, 0) = -6.004717303426694e-04;
	distCoeffs.at<double>(3, 0) = 0.001281258711954;
	distCoeffs.at<double>(4, 0) = 0;


	Mat view, rview, map1, map2;
	Size imageSize = src.size();
	
	initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
		getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
		imageSize, CV_16SC2, map1, map2);

	Mat srcCalibration;
	remap(src, srcCalibration, map1, map2, INTER_LINEAR);
	return srcCalibration;
}


void detectPlates(){
	int count;
	int total_count = 0;
	for (int FileId = 2; FileId < 5; FileId++)
	{
		count = 0;
		stringstream filename;
		filename << "C://Users//Administrator//Desktop//test_avi//test (" << FileId << ").avi";

		VideoCapture capture;
		//capture.open(filename.str());
		capture.open("C://Users//Administrator//Desktop//1026_2.avi");
		struct tm last_time = getTime();

		//  判断视频流读取是否正确
		if (!capture.isOpened())
		{
			std::cout << "fail to open video!" << std::endl;
			getchar();
			return ;

		}

		Mat image;
		capture >> image;

		while (!image.empty()){

			Mat fullscreen;
			double fx = 1920.0 / image.cols;
			double fy = 1080.0 / image.rows;

			resize(image, fullscreen, Size(0, 0), fx, fy);
			//resize(image, fullscreen, Size(1280, 720), 0, 0, CV_INTER_LINEAR);

			//imshow("full", fullscreen);

			Rect re(1100, 0, 450, 400);
			Mat win_gray;

			Mat window(fullscreen, re);
			imshow("window", window);
			cvtColor(window, win_gray, CV_BGR2GRAY);

			//方法1  canny的两个阈值都比较低
			Mat gauss;
			GaussianBlur(win_gray, gauss, Size(7, 7), 0, 0);
			Mat img_canny;
			Canny(gauss, img_canny, 40, 120, 3, true);

			////方法2  canny的两个阈值都比较高
			//Mat img_canny;
			//Canny(win_gray, img_canny, 125, 350);
			//threshold(img_canny, img_canny, 128, 255, THRESH_BINARY_INV);

			imshow("img_canny", img_canny);


			//vector<cv::Vec4i> lines;
			//// 检测直线，最小投票为90，线条不短于50，间隙不小于10
			//HoughLinesP(contours, lines, 5, (CV_PI / 180)*15, 80, 10, 0);
			//drawDetectLines(image, lines, Scalar(0, 255, 0));

			//Find contours of possibles plates
			vector< vector< Point> > contours;
			findContours(img_canny,
				contours, // a vector of contours
				CV_RETR_EXTERNAL, // retrieve the external contours
				CV_CHAIN_APPROX_NONE); // all pixels of each contours

			//Start to iterate to each contour founded
			vector<vector<Point> >::iterator itc = contours.begin();
			vector<RotatedRect> rects;

			Mat result;
			Mat fliter;
			window.copyTo(result);
			window.copyTo(fliter);
			//cv::drawContours(result, contours,
			//	-1, // draw all contours
			//	cv::Scalar(255, 0, 0), // in blue
			//	1); // with a thickness of 1

			//Remove patch that are no inside limits of aspect ratio and area.    
			while (itc != contours.end()) {

				//Create bounding rect of object
				RotatedRect mr = minAreaRect(Mat(*itc));

				Rect re = boundingRect(Mat(*itc));
				//rectangle(result, re, Scalar(0, 255, 0));
				rectangle(result, re, Scalar(0, 255, 0));


				if (//re.x > 50 && re.x < 160 && 
					re.y>190 && re.y < 250 &&
					re.width>110 && re.width<140 && re.height>58 && re.height<75){  // testfornumber   (re.x > 20 && re.x < 30 && re.y>105 && re.y < 120 && re.width>95 && re.width<120 && re.height>100 && re.height<110)     re.x > 20 && re.x < 52 && re.y>55 && re.y < 85 &&

					struct tm now_time = getTime();
					if (true){//time_interval(now_time, last_time)
						rectangle(fliter, re, Scalar(0, 255, 0));

						last_time = now_time;
						stringstream ss;
						ss << now_time.tm_hour + 00 << "_" << now_time.tm_min + 00 << "_" << now_time.tm_sec + 00;
						Mat hmp(result, re);


						total_count++;
						stringstream save_path;
						save_path << "E://postgradu//ComputerVision//project//对比度//难检测的号码牌//" << total_count << ".jpg";

						imwrite(save_path.str(), hmp);
						//imshow(ss.str(), hmp);
						//cout << "hmp detected	" << ss.str() << "	" << re.x << "," << re.y << endl;
						count++;
					}
				}

				++itc;
				//if (!verifySizes(mr)){
				//	itc = contours.erase(itc);
				//}
				//else{
				//	++itc;
				//	rects.push_back(mr);
				//}

			}
			////纵向合并
			//Mat ppt_frame;
			//ppt_frame.push_back(window);
			//ppt_frame.push_back(fliter);
			//imshow("combine", ppt_frame);

			//横向合并
			//Mat img_merge;
			//Size size(window.cols * 4, MAX(window.rows, window.rows));
			//img_merge.create(size, CV_MAKETYPE(window.depth(), 3));
			//img_merge = Scalar::all(0);
			//Mat outImg_1, outImg_2, outImg_3, outImg_4;

			//Mat temp;
			//temp.create(img_canny.size(), CV_MAKETYPE(window.depth(), 3));
			//cvtColor(img_canny, temp, CV_GRAY2BGR);


			//2.在新建合并图像中设置感兴趣区域
			//outImg_1 = img_merge(Rect(0, 0, window.cols, window.rows));
			//outImg_2 = img_merge(Rect(window.cols, 0, window.cols, window.rows));
			//outImg_3 = img_merge(Rect(window.cols*2, 0, window.cols, window.rows));
			//outImg_4 = img_merge(Rect(window.cols*3, 0, window.cols, window.rows));

			////3.将待拷贝图像拷贝到感性趣区域中
			//window.copyTo(outImg_1);
			//temp.copyTo(outImg_2);
			//result.copyTo(outImg_3);
			//fliter.copyTo(outImg_4);

			//imshow("横向合并", img_merge);

			//ppt << result;

			//imshow("without_fliter", result);
			//imshow("with_fliter", fliter);
			waitKey(1);
			capture >> image;
		}
		cout << "test(" << FileId << ")" << "	 " << count << "	" << "hmps have been detected" << endl;

	}
}

int main()
{	
	//get_final_result();
	for (int FileId = 2; FileId < 5; FileId++)
	{
		stringstream filename;
		filename << "C://Users//Administrator//Desktop//test_avi//test (" << FileId << ").avi";

		VideoCapture capture;
		//capture.open(filename.str());
		capture.open("E://postgradu//HighBitRate.avi");
		struct tm last_time = getTime();

		//  判断视频流读取是否正确
		if (!capture.isOpened())
		{
			std::cout << "fail to open video!" << std::endl;
			getchar();
			return 0;

		}

		Mat image;
		capture >> image;
		cvtColor(image, image, CV_BGR2GRAY);
		while (!image.empty()){
			Mat image_fullscreen;
			double fx = 1920.0 / image.cols;
			double fy = 1080.0 / image.rows;

			resize(image, image_fullscreen, Size(0, 0), fx, fy);
			Mat imageCalibrated;
			imageCalibrated = cameraUndistort(image_fullscreen);
			//imshow("src", image_fullscreen);
			Rect re(371, 143, 1440, 810);
			Mat image_undistort(imageCalibrated, re);
			//imshow("undistort", imageCalibrated);
			//imshow("undistort", image_undistort);

			////纵向合并
			Mat ppt_frame;

			double fx_inppt = 640.0 / image.cols;
			double fy_inppt = 360.0 / image.rows;

			Mat src_inppt;
			resize(image, src_inppt, Size(0, 0), fx_inppt, fy_inppt);

			Mat undistort_inppt;
			double fx_inppt_1 = 640.0 / image_undistort.cols;
			double fy_inppt_1 = 360.0 / image_undistort.rows;
			resize(image_undistort, undistort_inppt, Size(0, 0), fx_inppt_1, fy_inppt_1);

			ppt_frame.push_back(src_inppt);
			ppt_frame.push_back(undistort_inppt);
			imshow("combine", ppt_frame);

			waitKey(1);
			capture >> image;
			cvtColor(image, image, CV_BGR2GRAY);

		}
	}

	return 0;
}