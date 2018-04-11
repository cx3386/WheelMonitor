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
	void resetOcr();//���vector
	string get_final_result();
	int size() const { return final_result_size; };  //valid after call get_final_result and before the next call of get_final_result

private:
	const ConfigHelper * configHelper;
	const OcrProfile * ocrProfile;
	int deviceIndex;

	int nContinuousMissCount = 5;
	uint lastNum = 0;
	bool isDbg = false;//���Ա�־λ����ʾ����ͼ��
	vector<string> result;
	int final_result_size = 0;
	vector<CharSegment> find_ch(Mat threshold);
	void generate_pattern(Mat in);//��������
	void recognize(Mat plate);//����GRAY��ʽ�ĺ����ƣ��õ�string��������result
	vector<Mat> detect_plate(Mat frame);//����RGB��ʽ��֡�����GRAY��ʽ�ĺ�����
	Mat preprocess(const Mat &in);
	int charSize = 20;
	Mat processChar(Mat in);
	int judge(Mat test);
	Mat pattern[10];
	float oudistance(Mat a, Mat b) const;
	vector<CharSegment> unit_char;
	string getTime() const;//���ڱ�����Ե��ļ�
};

#endif
