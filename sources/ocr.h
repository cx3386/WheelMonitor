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
	Mat get_show_plate(); //!< 返回用于该车轮用于展示的plate，有可能为空(mat.empty)，尺寸未归一（于设定值之间）。
	/// return ocr total match size, should be called after get_final_result();
	inline int size() const { return ocrDetectCount; }
	//! 重置ocr的连续miss计数器
	inline void resetOcr() { nContinuousMissCount = 5; }

private:
	const ConfigHelper* configHelper;
	int deviceIndex;
	const OcrProfile* ocrProfile; // after deviceIndex/configHelper init. depend on the order in declaration

	bool isDebug = false; ///< 调试标志位，显示过程图像
	Mat pattern[10]; ///< number character image samples, 0~9

	string predictAnNum();
	uint lastNum = 0;
	int nContinuousMissCount = 5; //!< 每miss（识别）一次，则+1。用于预测序号：如果连续miss5次以上，则不再给出预测数字。

	vector<Mat> good_plates; //!< 能识别出3位数字的plates
	vector<Mat> soso_plates; //!< 能正确识别出（任意）数字的plates
	vector<Mat> other_plates; //!< 其他截取到的plates

	vector<string> result;
	int ocrDetectCount = 0;
	/// 输入RGB/GRAY格式的帧，输出GRAY格式的号码牌
	vector<Mat> detect_plate(Mat frame);

	/**
	 * \brief 识别号码牌中的数字，如果得到3个，则放入容器result。
	 *
	 * \param Mat plate GRAY格式的号码牌
	 * \return 识别到的数字个数
	 * \retval 0 无数字，差结果
	 * \retval 3 全数字,好结果
	 * \retval 1~2 可识别一些数字，soso结果
	 */
	int recognize(Mat plate);
	/// 字符分割，输入二值化的号码牌，输出单个字符的位置与图像
	vector<CharSegment> find_ch(Mat binaryImg);
	/// 用于将号码牌二值化
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
