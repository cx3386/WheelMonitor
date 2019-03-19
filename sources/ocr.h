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
	CharSegment(Mat i, Rect p)
		: img(i)
		, pos(p)
	{
	}
};

class ConfigHelper;
struct OcrProfile;
class OCR : public QObject {
	Q_OBJECT
public:
	OCR(const ConfigHelper* _configHelper, int _deviceIndex, QObject* parent = Q_NULLPTR);
	void core_ocr(Mat src); //= detect_plate() + recogenize()
	string get_final_result();
	Mat get_show_plate(); //!< �������ڸó�������չʾ��plate���п���Ϊ��(mat.empty)���ߴ�δ��һ�����趨ֵ֮�䣩��
	/// return ocr total match size, should be called after get_final_result();
	inline int size() const { return ocrDetectCount; }
	//! ����ocr������miss������
	inline void resetOcr() { nContinuousMissCount = 5; }

private:
	const ConfigHelper* configHelper;
	int deviceIndex;
	const OcrProfile* ocrProfile; // after deviceIndex/configHelper init. depend on the order in declaration

	bool isDebug = false; ///< ���Ա�־λ����ʾ����ͼ��
	Mat pattern[10]; ///< number character image samples, 0~9

	string predictAnNum();
	uint lastNum = 0;
	int nContinuousMissCount = 5; //!< ÿmiss��ʶ��һ�Σ���+1������Ԥ����ţ��������miss5�����ϣ����ٸ���Ԥ�����֡�

	vector<Mat> good_plates; //!< ��ʶ���3λ���ֵ�plates
	vector<Mat> soso_plates; //!< ����ȷʶ��������⣩���ֵ�plates
	vector<Mat> other_plates; //!< ������ȡ����plates

	vector<string> result;
	int ocrDetectCount = 0;
	/// ����RGB/GRAY��ʽ��֡�����GRAY��ʽ�ĺ�����
	vector<Mat> detect_plate(Mat frame);

	/**
	 * \brief ʶ��������е����֣�����õ�3�������������result��
	 *
	 * \param Mat plate GRAY��ʽ�ĺ�����
	 * \return ʶ�𵽵����ָ���
	 * \retval 0 �����֣�����
	 * \retval 3 ȫ����,�ý��
	 * \retval 1~2 ��ʶ��һЩ���֣�soso���
	 */
	int recognize(Mat plate);
	/// �ַ��ָ�����ֵ���ĺ����ƣ���������ַ���λ����ͼ��
	vector<CharSegment> find_ch(Mat binaryImg);
	/// ���ڽ������ƶ�ֵ��
	Mat binaryProcess(const Mat& in);

	//plates

	/**
	 * \brief return the num character 0~9, -1 if fail, comparing with pattern.
	 *
	 * \param Mat charImg the unit char image
	 */
	int sampleComparison(Mat charImg);
	float oudistance(Mat a, Mat b) const;
	static bool isInside(cv::Rect rect1, cv::Rect rect2)
	{
		return (rect1 == (rect1 & rect2));
	}
};

#endif
