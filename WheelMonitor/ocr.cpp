#include "stdafx.h"
#include "ocr.h"

CharSegment::CharSegment() {}
CharSegment::CharSegment(Mat i, Rect p) {
	img = i;
	pos = p;
}

ocr_parameters::ocr_parameters() {}

ocr_parameters ocr::p;

ocr::ocr() {//加载样本
	charSize = 20;
	string local = QCoreApplication::applicationDirPath().append("/ocr_pattern/").toStdString();
	for (int i = 0; i < 10; i++) {
		stringstream patternname;
		//patternname << "E://postgradu//ComputerVision//project/hmpsb_9.20//hmpsb_9.20//char//pattern//1012char20//" << i << ".jpg";
		patternname << local << i << ".jpg";

		pattern[i] = imread(patternname.str(), -1);

		if (pattern[i].empty())
		{
			//qDebug()<< "Could not open or find pattern!" ;
		}
	}
	//qDebug() << "patterns loaded";
	resetOcr();
}

void ocr::resetOcr() {
	unit_char.clear();
	result.clear();
	final_result.clear();
	//qDebug() << "ocr initial complete";
}

vector<Mat> ocr::detect_plate(Mat frame) {
	vector<Mat> output;
	Mat fullscreen;
	double fx = 1920.0 / frame.cols;
	double fy = 1080.0 / frame.rows;
	resize(frame, fullscreen, Size(0, 0), fx, fy);//归一到全屏
	//resize(frame, fullscreen, Size(1920, 1080), 0, 0, CV_INTER_LINEAR);

	Rect re(1100, 0, 450, 400);
	Mat window(fullscreen, re);
	//Mat win_gray;
	//cvtColor(window, win_gray, CV_BGR2GRAY);//如果输入是bgr则需要这两行

	if (debug)
		imshow("window", window);

	//方法1  canny的两个阈值都比较低
	Mat gauss;
	GaussianBlur(window, gauss, Size(7, 7), 0, 0);
	Mat img_canny;
	Canny(gauss, img_canny, 40, 120, 3, true);

	////方法2  canny的两个阈值都比较高
	//Mat img_canny;
	//Canny(win_gray, img_canny, 125, 350);
	//threshold(img_canny, img_canny, 128, 255, THRESH_BINARY_INV);
	//if (debug)
	//	imshow("img_canny", img_canny);

	vector< vector< Point> > contours;
	findContours(img_canny,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // retrieve the external contours
		CV_CHAIN_APPROX_NONE); // all pixels of each contours

	//Start to iterate to each contour founded
	vector<vector<Point> >::iterator itc = contours.begin();
	//vector<RotatedRect> rects;

	Mat fliter;
	window.copyTo(fliter);

	while (itc != contours.end()) {
		//Create bounding rect of object
		//RotatedRect mr = minAreaRect(Mat(*itc));
		Rect re = boundingRect(Mat(*itc));
		//rectangle(result, re, Scalar(0, 255, 0));

		if (//re.x > p.plate_x_min && re.x < p.plate_x_max &&
			re.y > p.plate_y_min && re.y < p.plate_y_max &&
			re.width > p.plate_width_min && re.width < p.plate_width_max &&
			re.height > p.plate_height_min && re.height < p.plate_height_max) {
			rectangle(fliter, re, Scalar(0, 255, 0));
			Mat hmp(fliter, re);
			output.push_back(hmp);
			stringstream filename;
			string local = QCoreApplication::applicationDirPath().append("/ocr_pattern/").toStdString();
			filename << local << getTime() << "hmp" << ".jpg";
			imwrite(filename.str(), hmp);
		}
		++itc;
	}
	return output;
}

void ocr::recognize(Mat in)
{
	unit_char.clear();
	Mat threshold = preprocess(in);
	//imshow("prepro", threshold);
	unit_char = find_ch(threshold);

	int count = 0;
	stringstream out;
	for (int i = 0; i < unit_char.size(); i++) {
		//Preprocess each char for all images have same sizes
		Mat StandardChar = processChar(unit_char[i].img);
		//稳定后注释掉
		stringstream filename;
		string local = QCoreApplication::applicationDirPath().append("/ocr_pattern/").toStdString();
		filename << local << getTime() << "_" << i << ".jpg";
		imwrite(filename.str(), StandardChar);

		int temp = judge(StandardChar);
		if (temp >= 0) {
			out << temp;
			count++;
		}
	}
	if (count < 3) {//不能识别的号码牌直接return,不直接判定unit_char.size()是因为杂质可能被判定为unit_char,但judge()可以一定程度上分辨杂质和真数字
		//qDebug() << "get one invalide result" ;
		return;
	}
	else {
		result.push_back(out.str());
		//qDebug() << "get one result, result size come to" << result.size();

		return;
	}
}

Mat ocr::preprocess(Mat src) {
	Mat binary = src;
	//imshow("src", src);

	for (int i = 0; i < src.rows; i++) {
		//cout << i << endl;
		for (int j = 0; j < src.cols; j++) {
			int t = src.at<uchar>(i, j);

			if (t < 180) {
				binary.at<uchar>(i, j) = 0;
			}
			else binary.at<uchar>(i, j) = 255;
		}
	}
	Mat img_threshold;
	threshold(binary, img_threshold, 90, 255, cv::THRESH_BINARY);

	//imshow("s", img_threshold);
	//waitKey();

	return img_threshold;
}

vector<CharSegment> ocr::find_ch(Mat img_threshold) {
	vector<CharSegment> output;

	Mat img_contours;
	img_threshold.copyTo(img_contours);
	//Find contours of possibles characters
	vector< vector< Point> > contours;
	findContours(img_contours,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // retrieve the external contours
		CV_CHAIN_APPROX_NONE); // all pixels of each contours

	// Draw blue contours on a white image
	cv::Mat result;
	img_threshold.copyTo(result);
	cvtColor(result, result, CV_GRAY2RGB);
	cv::drawContours(result, contours,
		-1, // draw all contours
		cv::Scalar(255, 0, 0), // in blue
		1); // with a thickness of 1
	//Start to iterate to each contour founded
	//imshow("contour", result);

	vector<vector<Point> >::iterator itc = contours.begin();

	Rect re[5];
	int index = 0;

	while (itc != contours.end()) {
		//Create bounding rect of object
		Rect mr = boundingRect(Mat(*itc));
		rectangle(result, mr, Scalar(0, 255, 0));
		if (mr.width >= p.num_width_min && mr.width <= p.num_width_max &&
			mr.height >= p.num_height_min && mr.height <= p.num_height_max) {//筛选有效的框***   mr.width >= 20 && mr.width <= 35 && mr.height >= 20 && mr.height <= 50
			re[index] = mr;
			Mat num_cut(img_threshold, re[index]);
			output.push_back(CharSegment(num_cut, re[index]));
			index++;
		}
		++itc;
	}
	cout << index << endl;

	sort(output.begin(), output.end(), CharSegment::LcNum);
	return output;
}

Mat ocr::processChar(Mat in) {
	//Remap image
	int h = in.rows;
	int w = in.cols;
	Mat transformMat = Mat::eye(2, 3, CV_32F);
	int m = max(w, h);
	transformMat.at<float>(0, 2) = m / 2 - w / 2;
	transformMat.at<float>(1, 2) = m / 2 - h / 2;

	Mat warpImage(m, m, in.type());
	warpAffine(in, warpImage, transformMat, warpImage.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(0));

	Mat out;
	resize(warpImage, out, Size(charSize, charSize));

	return out;
}

int ocr::judge(Mat test) {
	float dis[10];
	int index = 0;
	cout << "数组";
	for (int i = 0; i < 10; i++) {
		dis[i] = oudistance(pattern[i], test);
		cout << dis[i] << "  ";
	}
	cout << endl;

	double temp = dis[0];
	for (int i = 0; i < 10; i++) {
		if (dis[i] < temp) {
			temp = dis[i];
			index = i;
		}
	}
	if (dis[index] < 2400) {
		return index;
	}
	else {
		return -1;
	}
}

float ocr::oudistance(Mat pattern, Mat test) {
	float distance = 0;
	for (int i = 0; i < pattern.cols; i++) {
		for (int j = 0; j < pattern.rows; j++) {
			distance += (pattern.at<uchar>(i, j) - test.at<uchar>(i, j))*(pattern.at<uchar>(i, j) - test.at<uchar>(i, j));
		}
	}
	distance = sqrt(distance);
	return distance;
}

string ocr::getTime()
{
	time_t timep;
	struct tm now_time;

	time(&timep);

	localtime_s(&now_time, &timep);
	stringstream ss;
	ss << now_time.tm_mon + 00 << "_" << now_time.tm_mday + 00 <<
		"_" << now_time.tm_hour + 00 << "_" << now_time.tm_min + 00 << "_" << now_time.tm_sec + 00;
	return ss.str();
}

void ocr::generate_pattern(Mat in) {
	Mat frame;
	in.copyTo(frame);
	vector<Mat> plates = detect_plate(frame);
	if (debug)
		cout << "find plates" << plates.size();
	for (int j = 0; j < plates.size(); j++) {
		Mat threshold = preprocess(plates[j]);

		vector<CharSegment> ch = find_ch(threshold);

		for (int i = 0; i < ch.size(); i++) {
			//Preprocess each char for all images have same sizes
			Mat StandardChar = processChar(ch[i].img);
			stringstream filename;
			string local = QCoreApplication::applicationDirPath().append("/ocr_pattern/").toStdString();
			filename << local << getTime() << "_" << i << ".jpg";
			imwrite(filename.str(), StandardChar);
		}
	}
}

void ocr::core_ocr(Mat src) {
	Mat frame;
	src.copyTo(frame);
	vector<Mat> plates = detect_plate(frame);
	if (debug) cout << "find plates " << plates.size();
	for (int j = 0; j < plates.size(); j++) {
		recognize(plates[j]);
		if (debug) cout << "find num " << unit_char.size();
	}
}
void ocr::get_final_result() {
	if (result.empty())
		return;
	vector<string> temp = result;
	sort(temp.begin(), temp.end());
	temp.erase(unique(temp.begin(), temp.end()), temp.end());//temp是排序后剔除相同元素的result
	vector<int> result_count;
	for (int i = 0; i < temp.size(); i++) {
		result_count.push_back(count(result.begin(), result.end(), temp[i]));
	}
	int index = 0;
	for (int i = 0; i < result_count.size(); i++) {
		if (result_count[i] > result_count[index]) {
			index = i;
		}
	}
	final_result = temp[index];
}