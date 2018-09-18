#pragma once
#include <opencv.hpp>

class RobustMatcher
{
public:
	RobustMatcher();

	~RobustMatcher();

	/**
	 * \brief ƥ������ͼ�����ת���ǶȺ�ƥ��ͼ��ԣ�ƥ��ɹ�����1
	 *
	 * \param cv::Mat src1 ���� ǰһ֡ͼ��
	 * \param cv::Mat src2 ���� ��һ֡ͼ��
	 * \param cv::Mat & img_matches ��� ƥ��ͼ���
	 * \param double & angle ��� ת���Ƕ�
	 */
	bool match(cv::Mat src1, cv::Mat src2, cv::Mat &img_matches, double &angle);
private:
	cv::Mat getMask(cv::Size) const;
	cv::Ptr<cv::FeatureDetector> detector;
	cv::Ptr<cv::DescriptorMatcher> matcher;
	float ratio = 0.65f;
	void symmetryTest(
		const std::vector<std::vector<cv::DMatch>> &matches1,
		const std::vector<std::vector<cv::DMatch>> &matches2,
		std::vector<cv::DMatch> &symMatches);
	int ratioTest(std::vector<std::vector<cv::DMatch>> &matches);
};
