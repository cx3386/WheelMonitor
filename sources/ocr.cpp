#include "stdafx.h"

#include "common.h"
#include "confighelper.h"
#include "ocr.h"

//ocr_parameters OCR::p; //init static member of OCR
 //!< patch: 2019/3/17 以下为新增了ocr参数，本应放入ocrprofile中
static const double plate_w_h_ratio_min = 1.63; //!< 车牌最小长宽比
static const double plate_w_h_ratio_max = 2.08; //!< 车牌最大长宽比
static const double plate_angle_max = 5;        //!< 车牌最大倾斜角

OCR::OCR(const ConfigHelper* _configHelper, int _deviceIndex, QObject* parent /*= Q_NULLPTR*/)
	: QObject(parent)
	, configHelper(_configHelper)
	, deviceIndex(_deviceIndex)
	, ocrProfile(&(configHelper->device[deviceIndex].ocrProfile))
{
	//加载样本
	for (int i = 0; i < 10; i++) {
		QString patternName = QString("%1/%2.jpg").arg(ocrPatternDirPath).arg(i);
		pattern[i] = imread(patternName.toStdString(), -1);

		if (pattern[i].empty()) {
			qWarning() << "OCR: Could not open or find pattern[" << i << "]";
		}
	}
}

void OCR::core_ocr(Mat src)
{
	vector<Mat> plates = detect_plate(src);//一般来说，一张图里只可能有一个plate，如果超过1个，则包含误识别
	if (isDebug)
		cout << "find plates " << plates.size();
	for (auto&& pl : plates)
	{
		//如果是内圈，翻转plate
		if (deviceIndex == 1) flip(pl, pl, 1);
		recognize(pl);
	}
}

/**
 * \brief 通过车牌的几何特征定位车牌
 *
 * \param Mat frame
 */
vector<Mat> OCR::detect_plate(Mat frame)
{
	const Mat& fullscreen = frame;//直接用1280*720
	//resize(frame, fullscreen, Size(1920, 1080), 0, 0, CV_INTER_LINEAR);

	//Rect re(1100, 100, 450, 380); 2019/3/13
	int w = ocrProfile->plate_x_max - ocrProfile->plate_x_min;
	int h = ocrProfile->plate_y_max - ocrProfile->plate_y_min;
	Rect roiRect(ocrProfile->plate_x_min, ocrProfile->plate_y_min, w, h);
	Mat window(fullscreen, roiRect);//取图中roi区域
	//Mat win_gray;
	//cvtColor(window, win_gray, CV_BGR2GRAY);//如果输入是bgr则需要这两行

	if (isDebug)
		imshow("window", window);

	//方法1  canny的两个阈值都比较低
	Mat gauss;
	GaussianBlur(window, gauss, Size(5, 5), 0, 0);
	Mat img_canny;
	Canny(gauss, img_canny, 40, 100, 3, true);

	////方法2  canny的两个阈值都比较高
	//Mat img_canny;
	//Canny(win_gray, img_canny, 125, 350);
	//threshold(img_canny, img_canny, 128, 255, THRESH_BINARY_INV);//二值化处理
	if (isDebug)
		imshow("img_canny", img_canny);

	vector<vector<Point>> contours;
	findContours(img_canny,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // retrieve the external contours
		CV_CHAIN_APPROX_NONE); // all pixels of each contours

	//vector<RotatedRect> rects;

	//Start to iterate to each contour founded
	Rect winRe(0, 0, window.cols, window.rows);
	vector<Mat> plates;
	for (auto&& ct : contours) {
		//Create bounding rect of object
		RotatedRect mr = minAreaRect(Mat(ct));
		//长宽比限制
		if ((double)mr.size.width / mr.size.height < plate_w_h_ratio_min
			|| (double)mr.size.width / mr.size.height > plate_w_h_ratio_max)
		{
			continue;
		}
		//角度限制
		if (fabs(mr.angle) > plate_angle_max)
		{
			continue;
		}

		Rect re = boundingRect(Mat(ct));
		//rectangle(result, re, Scalar(0, 255, 0));
		bool bInside = isInside(re, winRe);

		if (bInside
			&& re.width > ocrProfile->plate_width_min
			&& re.width < ocrProfile->plate_width_max
			&& re.height > ocrProfile->plate_height_min
			&& re.height < ocrProfile->plate_height_max
			&& (double)re.width / re.height >= plate_w_h_ratio_min
			&& (double)re.width / re.height <= plate_w_h_ratio_max)
		{
			Mat plate(window, re);
			plates.push_back(plate);
			//save hmp
			//patch:不再保存hmp  [1/2/2019 cx3386]
		}
	}
	return plates;
}

int OCR::recognize(Mat plate)
{
	auto unit_chars = find_ch(binaryProcess(plate));

	int count = 0;
	QString nums;
	int i = 0;
	QString date = QDate::currentDate().toString("yyyyMMdd");
	QString dateTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
	for (auto&& unit_char : as_const(unit_chars)) {
		Mat stdChar;
		resize(unit_char.img, stdChar, Size(20, 20));
		// save pattern
		QString fullFilePath = QStringLiteral("%1/%2/%3_%4_%5.jpg").arg(ocrDirPath).arg(date).arg(deviceIndex).arg(dateTime).arg(++i);
		imwrite(fullFilePath.toStdString(), stdChar);

		int num = sampleComparison(stdChar);
		if (num != -1)
			nums += QString("%1").arg(num);
	}
	// 根据识别到的数字个数，对该plate进行归类
	QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
	QString plateFilePath;
	switch (nums.size())
	{
	case 3:
		good_plates.push_back(plate);
		if (isDebug)
		{
			plateFilePath = QStringLiteral("%1/goodplate_%2.jpg").arg(ocrDirPath).arg(time);
			imwrite(plateFilePath.toStdString(), plate);
		}
		result.push_back(nums.toStdString());
		break;
	case 1:
	case 2:
		soso_plates.push_back(plate);
		if (isDebug)
		{
			plateFilePath = QStringLiteral("%1/sosoplate_%2.jpg").arg(ocrDirPath).arg(time);
			imwrite(plateFilePath.toStdString(), plate);
		}
		break;
	case 0:
	default:
		other_plates.push_back(plate);
		if (isDebug)
		{
			plateFilePath = QStringLiteral("%1/badplate_%2.jpg").arg(ocrDirPath).arg(time);
			imwrite(plateFilePath.toStdString(), plate);
		}
		break;
	}
	return nums.size();
}

cv::Mat OCR::binaryProcess(const Mat& in)
{
	Mat binary;
	threshold(in, binary, 180, 255, cv::THRESH_BINARY);
	return binary;
}

vector<CharSegment> OCR::find_ch(Mat binaryImg)
{
	//Find contours of possibles characters
	vector<vector<Point>> contours;
	findContours(binaryImg,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // retrieve the external contours
		CV_CHAIN_APPROX_NONE); // all pixels of each contours

	vector<CharSegment> unit_chars;
	for (auto&& ct : contours) { //Create bounding rect of object
		Rect mr = boundingRect(Mat(ct));
		if (mr.width >= ocrProfile->num_width_min
			&& mr.width <= ocrProfile->num_width_max
			&& mr.height >= ocrProfile->num_height_min
			&& mr.height <= ocrProfile->num_height_max)
		{ //筛选有效的框***   mr.width >= 20 && mr.width <= 35 && mr.height >= 20 && mr.height <= 50
			Mat num_cut(binaryImg, mr);
			unit_chars.emplace_back(num_cut, mr);
		}
	}

	// 从左往右对char排序
	sort(unit_chars.begin(), unit_chars.end(), [](auto X, auto Y) { return X.pos.x < Y.pos.x; });
	if (isDebug)
		cout << "find num " << unit_chars.size();
	return unit_chars;
}

int OCR::sampleComparison(Mat charImg)
{
	int minIndex = -1;
	// 如果所有匹配的相似度都未达到最低要求，则返回-1
	double minDis = 2400;
	for (int i = 0; i < 10; i++) {
		double dis = oudistance(pattern[i], charImg);
		// 如果相似度超过阈值，直接接受该匹配
		if (dis < 1000) {
			return i;
		}
		if (dis < minDis) {
			minDis = dis;
			minIndex = i;
		}
	}
	return minIndex;
}

float OCR::oudistance(Mat a, Mat b) const
{
	float distance = 0;
	for (int i = 0; i < a.cols; i++) {
		for (int j = 0; j < a.rows; j++) {
			distance += (a.at<uchar>(i, j) - b.at<uchar>(i, j)) * (a.at<uchar>(i, j) - b.at<uchar>(i, j));
		}
	}
	return sqrt(distance);
}

std::string OCR::get_final_result()
{
	map<string, int> keyMap; //take result value as key and the count as value
	for (auto&& str : result) {
		keyMap[str]++;
	}
	string num = "";
	int maxCount = 0; //遍历keyMap, 找到出现次数最多的num
	for (auto&& pair : keyMap) {
		if (pair.second > maxCount) {
			maxCount = pair.second;
			num = pair.first;
		}
	}

	if (num.empty()) { // no key is detected.
		num = predictAnNum(); // might return a miss
	}
	else { // a num is detected.
		nContinuousMissCount = 0;
		lastNum = QString::fromStdString(num).toUInt(); // record the num
	}
	ocrDetectCount = result.size();
	result.clear();
	return num;
}

cv::Mat OCR::get_show_plate()
{
	cv::Mat ret;
	if (!good_plates.empty())
	{
		ret = good_plates.at(good_plates.size() / 2);
		qDebug() << "good plate";
	}
	else if (!soso_plates.empty())
	{
		ret = soso_plates.at(good_plates.size() / 2);
		qDebug() << "soso plate";
	}
	else if (!other_plates.empty())
	{
		ret = other_plates.at(good_plates.size() / 2);
		qDebug() << "bad plate";
	}
	else
	{
	}
	good_plates.clear();
	soso_plates.clear();
	other_plates.clear();
	if (isDebug)
	{
		imshow("plate", ret);
		QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
		QString plateFilePath;
		plateFilePath = QStringLiteral("%1/ret_%2.jpg").arg(ocrDirPath).arg(time);
		imwrite(plateFilePath.toStdString(), ret);
	}
	return ret;
}

std::string OCR::predictAnNum()
{
	string num;
	// loop
	if (lastNum == 82) {
		num = "001";
	}
	else {
		num = QString("%1").arg(lastNum + 1, 3, 10, QChar('0')).toStdString();
	}
	lastNum = QString::fromStdString(num).toUInt();

	//if miss for 5 times, not predict
	if (++nContinuousMissCount > 5) {
		num = OCR_MISS;
	}
	return num;
}