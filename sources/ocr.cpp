#include "stdafx.h"
#include "hikvideocapture.h"
#include "ocr.h"
#include "common.h"

//ocr_parameters OCR::p; //init static member of OCR

OCR::OCR() : charSize(20), nContinuousMissCount(0), tooMuchMiss(true), isDbg(false), lastNum(0), final_result_size(0)
{//加载样本
	for (int i = 0; i < 10; i++) {
		QString patternName = QString("%1/%2.jpg").arg(ocrPatternDirPath).arg(i);
		pattern[i] = imread(patternName.toStdString(), -1);

		if (pattern[i].empty())
		{
			qWarning() << "OCR: Could not open or find pattern[" << i << "]";
		}
	}
	resetOcr();
}

void OCR::resetOcr() {
	unit_char.clear();
	result.clear();
	//qDebug() << "ocr initial complete";
}

vector<Mat> OCR::detect_plate(Mat frame) {
	Mat fullscreen;
	resize(frame, fullscreen, Size(1920, 1080), 0, 0, CV_INTER_LINEAR);

	Rect re(1100, 100, 450, 380);
	Mat window(fullscreen, re);
	//Mat win_gray;
	//cvtColor(window, win_gray, CV_BGR2GRAY);//如果输入是bgr则需要这两行

	if (isDbg)
		imshow("window", window);

	//方法1  canny的两个阈值都比较低
	Mat gauss;
	GaussianBlur(window, gauss, Size(5, 5), 0, 0);
	Mat img_canny;
	Canny(gauss, img_canny, 40, 100, 3, true);

	////方法2  canny的两个阈值都比较高
	//Mat img_canny;
	//Canny(win_gray, img_canny, 125, 350);
	//threshold(img_canny, img_canny, 128, 255, THRESH_BINARY_INV);
	if (isDbg)
		imshow("img_canny", img_canny);

	vector< vector< Point> > contours;
	findContours(img_canny,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // retrieve the external contours
		CV_CHAIN_APPROX_NONE); // all pixels of each contours

	//vector<RotatedRect> rects;

	//Start to iterate to each contour founded
	vector<Mat> output;
	for (auto&& ct : contours)
	{
		//Create bounding rect of object
		//RotatedRect mr = minAreaRect(Mat(*itc));
		Rect re = boundingRect(Mat(ct));
		//rectangle(result, re, Scalar(0, 255, 0));

		if (//re.x > p.plate_x_min && re.x < p.plate_x_max &&
			re.y > p.plate_y_min && re.y < p.plate_y_max &&
			re.width > p.plate_width_min && re.width < p.plate_width_max &&
			re.height > p.plate_height_min && re.height < p.plate_height_max) {
			Mat hmp(window, re);
			output.push_back(hmp);
			//save hmp
			QString nowDate = QDate::currentDate().toString("yyyyMMdd");
			QString nowTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
			QString fullFilePath = QStringLiteral("%1/%2/%3hmp.jpg").arg(ocrDirPath).arg(nowDate).arg(nowTime);
			imwrite(fullFilePath.toStdString(), hmp);
		}
	}
	return output;
}

void OCR::recognize(Mat in)
{
	unit_char.clear();
	Mat threshold = preprocess(in);
	unit_char = find_ch(threshold);

	int count = 0;
	stringstream out;
	for (int i = 0; i < unit_char.size(); i++) {
		//Preprocess each char for all images have same sizes
		Mat StandardChar = processChar(unit_char[i].img);
		//稳定后注释掉
		QString nowDate = QDate::currentDate().toString("yyyyMMdd");
		QString nowTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
		QString fullFilePath = QStringLiteral("%1/%2/%3_%4.jpg").arg(ocrDirPath).arg(nowDate).arg(nowTime).arg(i);
		imwrite(fullFilePath.toStdString(), StandardChar);

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

cv::Mat OCR::preprocess(const Mat &in) {
	Mat binary;
	threshold(in, binary, 180, 255, cv::THRESH_BINARY);
	return binary;
}

vector<CharSegment> OCR::find_ch(Mat img_threshold) {
	//Find contours of possibles characters
	vector< vector< Point> > contours;
	findContours(img_threshold,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // retrieve the external contours
		CV_CHAIN_APPROX_NONE); // all pixels of each contours

	Rect re[5];
	vector<CharSegment> output;
	int i = 0;
	for (auto&& ct : contours) {		//Create bounding rect of object
		Rect mr = boundingRect(Mat(ct));
		if (mr.width >= p.num_width_min && mr.width <= p.num_width_max &&
			mr.height >= p.num_height_min && mr.height <= p.num_height_max) {//筛选有效的框***   mr.width >= 20 && mr.width <= 35 && mr.height >= 20 && mr.height <= 50
			re[i] = mr;
			Mat num_cut(img_threshold, re[i]);
			output.emplace_back(num_cut, re[i]);
			i++;
		}
	}
	cout << i << endl;

	sort(output.begin(), output.end(), [](auto X, auto Y) {return X.pos.x < Y.pos.x; });
	return output;
}

Mat OCR::processChar(Mat in) {
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

int OCR::judge(Mat test) {
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

float OCR::oudistance(Mat a, Mat b) const {
	float distance = 0;
	for (int i = 0; i < a.cols; i++) {
		for (int j = 0; j < a.rows; j++) {
			distance += (a.at<uchar>(i, j) - b.at<uchar>(i, j))*(a.at<uchar>(i, j) - b.at<uchar>(i, j));
		}
	}
	return sqrt(distance);
}

std::string OCR::getTime() const
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

void OCR::generate_pattern(Mat in) {
	auto plates = detect_plate(in);
	if (isDbg)
		cout << "find plates" << plates.size();
	for (auto&& pl : plates) {
		Mat threshold = preprocess(pl);
		auto chs = find_ch(threshold);
		int i = 0;
		for (auto& ch : chs) {
			//Preprocess each char for all images have same sizes
			Mat StandardChar = processChar(ch.img);
			QString nowDate = QDate::currentDate().toString("yyyyMMdd");
			QString nowTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
			QString fullFilePath = QStringLiteral("%1/%2/%3_%4.jpg").arg(ocrDirPath).arg(nowDate).arg(nowTime).arg(i);
			imwrite(fullFilePath.toStdString(), StandardChar);
			++i;
		}
	}
}

void OCR::core_ocr(Mat src) {
	vector<Mat> plates = detect_plate(src);
	if (isDbg) cout << "find plates " << plates.size();
	for (auto&& pl : plates)
	{
		recognize(pl);
		if (isDbg) cout << "find num " << unit_char.size();
	}
}

std::string OCR::get_final_result()
{
	final_result_size = result.size();
	map<string, int> keyList; //take result value as key and the count as value
	for (auto&& str : result) { keyList[str]++; }
	string key = "";
	int maxValue = 0;//遍历keyList找到value(count)最大的key
	for (auto&& mp : keyList) {
		if (mp.second >= maxValue) {
			maxValue = mp.second;
			key = mp.first;
		}
	}

	//if the key is null or the length != 3, means the result is wrong
	if (key.length() != 3)
	{
		//if miss for 5 times, no longer output predict num, output miss instead, until a real num is detected.
		if (++nContinuousMissCount >= 5)
		{
			tooMuchMiss = true;
		}
		if (tooMuchMiss)
		{
			key = "MISS";
		}
		else if (lastNum == 82)
		{
			key = "001";
		}
		else
		{
			key = QString("%1").arg(lastNum + 1, 3, 10, QChar('0')).toStdString();
		}
	}
	else
	{
		nContinuousMissCount = 0;
		tooMuchMiss = false;
	}
	lastNum = QString::fromStdString(key).toUInt();
	resetOcr();  //aferter output a result, reset the vectorsl
	return key;
}