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
	string get_final_result();
	/// return ocr total match size, should be called after get_final_result();
	inline int size() const { return ocrDetectCount; }
	/// when stop process, reset ocr to miss ocr too much
	inline void resetOcr() { nContinuousMissCount = 5; }

private:
	const ConfigHelper * configHelper;
	int deviceIndex;
	const OcrProfile * ocrProfile; // after deviceIndex/configHelper init. depend on the order in declaration
	QString deviceMark;

	bool isDbg = false; ///< ���Ա�־λ����ʾ����ͼ��
	Mat pattern[10]; ///< number character image samples, 0~9

	string predictAnNum();
	uint lastNum = 0;
	int nContinuousMissCount = 5;

	vector<string> result;
	int ocrDetectCount = 0;
	/// ����RGB/GRAY��ʽ��֡�����GRAY��ʽ�ĺ�����
	vector<Mat> detect_plate(Mat frame);
	/// ����GRAY��ʽ�ĺ����ƣ��õ�num��������result
	void recognize(Mat plate);
	/// �ַ��ָ�����ֵ���ĺ����ƣ���������ַ���λ����ͼ��
	vector<CharSegment> find_ch(Mat binaryImg);
	/// ���ڽ������ƶ�ֵ��
	Mat binaryProcess(const Mat &in);
	/**
	 * \brief Preprocess char, make all char images have same sizes.
	 *
	 * \param Mat in a char unit
	 */
	Mat remapChar(Mat in);
	/**
	 * \brief return the num character 0~9, comparing with pattern.
	 *
	 * \param Mat charImg the unit char image
	 */
	int sampleComparison(Mat charImg);
	float oudistance(Mat a, Mat b) const;
};

#endif
