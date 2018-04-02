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
	void resetOcr();//���vector
	static ocr_parameters p;
	string get_final_result();
	int size() const { return final_result_size; };  //valid after call get_final_result and before the next call of get_final_result
private:
	bool tooMuchMiss;
	int nContinuousMissCount;
	uint lastNum;
	bool isDbg;//���Ա�־λ����ʾ����ͼ��
	vector<string> result;
	int final_result_size;
	vector<CharSegment> find_ch(Mat threshold);
	void generate_pattern(Mat in);//��������
	void recognize(Mat plate);//����GRAY��ʽ�ĺ����ƣ��õ�string��������result
	vector<Mat> detect_plate(Mat frame);//����RGB��ʽ��֡�����GRAY��ʽ�ĺ�����
	Mat preprocess(const Mat &in);
	int charSize;
	Mat processChar(Mat in);
	int judge(Mat test);
	Mat pattern[10];
	float oudistance(Mat a, Mat b) const;
	vector<CharSegment> unit_char;
	string getTime() const;//���ڱ�����Ե��ļ�
};

#endif
