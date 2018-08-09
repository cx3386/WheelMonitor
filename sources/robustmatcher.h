#pragma once
#include <opencv.hpp>

class RobustMatcher
{
  public:
	RobustMatcher();

	~RobustMatcher();

	cv::Mat getMask(cv::Size size, int Ro, int Ri) const;
	bool match(cv::Mat src1, cv::Mat src2, // input images
			   cv::Mat msk1, cv::Mat msk2, //input mask
			   cv::Mat &img_matches,	   //output img_matches
			   double &angle);			   // output angle

  private:
	cv::Ptr<cv::FeatureDetector> detector;
	cv::Ptr<cv::DescriptorMatcher> matcher;
	float ratio = 0.65f;
	void symmetryTest(
		const std::vector<std::vector<cv::DMatch>> &matches1,
		const std::vector<std::vector<cv::DMatch>> &matches2,
		std::vector<cv::DMatch> &symMatches);
	int ratioTest(std::vector<std::vector<cv::DMatch>> &matches);
};
