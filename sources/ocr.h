#ifndef extract_h
#define extract_h

#include <QObject>
#include <opencv.hpp>

const char OCR_MISS[] = "MIS";

using namespace std;
using namespace cv;

struct CharSegment {
	Mat img;
	Rect pos;
	CharSegment(Mat i, Rect p) :img(i), pos(p) { }
};

class ConfigHelper;
struct OcrProfile;
class OCR :public QObject
{
	Q_OBJECT
public:
	OCR(const ConfigHelper * _configHelper, int _deviceIndex, QObject *parent = Q_NULLPTR);
	void core_ocr(Mat src);//= detect_plate() + recogenize()
	void resetOcr();//清空vector
	string get_final_result();
	int size() const { return final_result_size; };  //valid after call get_final_result and before the next call of get_final_result

private:
	const ConfigHelper * configHelper;
	const OcrProfile * ocrProfile;
	int deviceIndex;

	int nContinuousMissCount = 5;
	uint lastNum = 0;
	bool isDbg = false;//调试标志位，显示过程图像
	vector<string> result;
	int final_result_size = 0;
	vector<CharSegment> find_ch(Mat threshold);
	void generate_pattern(Mat in);//生成样本
	void recognize(Mat plate);//输入GRAY格式的号码牌，得到string放入容器result
	vector<Mat> detect_plate(Mat frame);//输入RGB格式的帧，输出GRAY格式的号码牌
	Mat preprocess(const Mat &in);
	int charSize = 20;
	Mat processChar(Mat in);
	int judge(Mat test);
	Mat pattern[10];
	float oudistance(Mat a, Mat b) const;
	vector<CharSegment> unit_char;
	string getTime() const;//用于保存调试的文件
};

#endif
