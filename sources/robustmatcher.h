#pragma once
#include <opencv.hpp>

class RobustMatcher
{
public:
	RobustMatcher();
	~RobustMatcher();
	bool match(cv::Mat src1, cv::Mat src2, double maskOuterRatio, double maskInnerRatio, cv::Mat &img_matches, double &angle);
private:
	void symmetryTest(
		const std::vector<std::vector<cv::DMatch>> &matches1,
		const std::vector<std::vector<cv::DMatch>> &matches2,
		std::vector<cv::DMatch> &symMatches);
	int ratioTest(std::vector<std::vector<cv::DMatch>> &matches);
	cv::Mat getMask(cv::Size size, double outerRatio, double innerRatio, int &ro, int &ri) const;
};
