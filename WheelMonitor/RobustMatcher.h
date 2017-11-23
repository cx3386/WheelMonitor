#pragma once
#include <opencv.hpp>

class RobustMatcher
{
public:
	RobustMatcher();
	~RobustMatcher();
	cv::Mat getMask(cv::Size size, int Ro, int Ri);
	bool match(cv::Mat& image1, cv::Mat& image2, // input images
		cv::Mat& mask1, cv::Mat& mask2,	//input mask
		cv::Mat& img_matches,	//output img_matches
		double& angle);	// output angle

private:
	cv::Ptr<cv::FeatureDetector> detector;
	cv::Ptr<cv::DescriptorMatcher> matcher;
	float ratio;
	void symmetryTest(
		const std::vector<std::vector<cv::DMatch> >& matches1,
		const std::vector<std::vector<cv::DMatch> >& matches2,
		std::vector<cv::DMatch>& symMatches);
	int ratioTest(std::vector<std::vector<cv::DMatch> >& matches);
};
