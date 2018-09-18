#pragma once
#include <opencv.hpp>

class RobustMatcher
{
public:
	RobustMatcher();

	~RobustMatcher();

	/**
	 * \brief 匹配两幅图像，输出转动角度和匹配图像对，匹配成功返回1
	 *
	 * \param cv::Mat src1 输入 前一帧图像
	 * \param cv::Mat src2 输入 后一帧图像
	 * \param cv::Mat & img_matches 输出 匹配图像对
	 * \param double & angle 输出 转动角度
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
