#ifndef extract_h
#define extract_h

#include <opencv.hpp>

using namespace std;
using namespace cv;

struct ocr_parameters {
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

struct CharSegment {
	Mat img;
	Rect pos;
	CharSegment(Mat i, Rect p) :img(i), pos(p) { }
};

class OCR {
public:
	OCR();
	void core_ocr(Mat src);//= detect_plate() + recogenize()
	void resetOcr();//清空vector
	static ocr_parameters p;
	string get_final_result();
	int get_results_size() const;
private:
	uint lastNum;
	bool bDbg;//调试标志位，显示过程图像
	vector<string> result;
	vector<CharSegment> find_ch(Mat threshold);
	void generate_pattern(Mat in);//生成样本
	void recognize(Mat plate);//输入GRAY格式的号码牌，得到string放入容器result
	vector<Mat> detect_plate(Mat frame);//输入RGB格式的帧，输出GRAY格式的号码牌
	Mat preprocess(const Mat &in);
	int charSize;
	Mat processChar(Mat in);
	int judge(Mat test);
	Mat pattern[10];
	float oudistance(Mat a, Mat b) const;
	vector<CharSegment> unit_char;
	string getTime() const;//用于保存调试的文件
};

#endif
