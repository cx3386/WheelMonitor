#pragma once
#include <opencv.hpp>

class RobustMatcher
{
public:
	RobustMatcher();
	~RobustMatcher();
	bool match(cv::Mat src1, cv::Mat src2, cv::Mat &img_matches, double &angle);
private:
	cv::Mat getMask(cv::Size) const;
};
