#ifndef extract_h
#define extract_h

#include <QObject>
#include <opencv.hpp>

using namespace std;
using namespace cv;

struct CharSegment {
	Mat img;
	Rect pos;
	CharSegment(Mat i, Rect p) :img(i), pos(p) { }
};

class OCR :public QObject
{
public:
	OCR(QObject *parent = Q_NULLPTR);
	void core_ocr(Mat src);//= detect_plate() + recogenize()
	void resetOcr();//清空vector
	static ocr_parameters p;
	string get_final_result();
	int size() const { return final_result_size; };  //valid after call get_final_result and before the next call of get_final_result
private:
	bool tooMuchMiss;
	int nContinuousMissCount;
	uint lastNum;
	bool isDbg;//调试标志位，显示过程图像
	vector<string> result;
	int final_result_size;
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
