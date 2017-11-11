#ifndef extract_h
#define extract_h

#include <QObject>
#include <QCoreApplication>
#include <highgui.hpp>
#include <opencv.hpp>
#include <imgproc.hpp>
#include <iostream>
#include <core.hpp>
#include <sstream>
#include <string>
#include <vector>
#include "time.h"


using namespace std;
using namespace cv;

class ocr_parameters {
public:
	ocr_parameters();
	int plate_x_min;
	int plate_x_max;
	int plate_y_min;
	int plate_y_max;
	int plate_width_min;
	int plate_width_max;
	int plate_height_min;
	int plate_height_max;
	int num_width_min;
	int num_width_max;
	int num_height_min;
	int num_height_max;
};

class CharSegment{
public:
	CharSegment();
	CharSegment(Mat i, Rect p);
	Mat img;
	Rect pos;

	static inline bool LcNum(const CharSegment & X, const CharSegment & Y) { return X.pos.x < Y.pos.x; }
};

class ocr {
public:
	ocr();
	static ocr_parameters p;
	void resetOcr();//清空vector
	void core_ocr(Mat src);//= detect_plate() + recogenize()

	vector<Mat> detect_plate(Mat frame);//输入RGB格式的帧，输出GRAY格式的号码牌
	void recognize(Mat plate);//输入GRAY格式的号码牌，得到string放入容器result
	void generate_pattern(Mat in);//生成样本
	void get_final_result();

	Mat preprocess(Mat in);
	vector<CharSegment> find_ch(Mat threshold);
	int charSize;
	Mat processChar(Mat in);
	int judge(Mat test);
	Mat pattern[10];
	float oudistance(Mat a, Mat b);
	vector<CharSegment> unit_char;

	vector<string> result;
	string final_result;

	string getTime();//用于保存调试的文件
	bool debug = false;//调试标志位，显示过程图像



};

#endif