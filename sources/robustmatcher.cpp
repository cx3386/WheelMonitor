#include "stdafx.h"

#include "robustmatcher.h"
//#include <qmath.h> //use M_PI
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

using namespace std;
using namespace cv;

RobustMatcher::RobustMatcher()
{
	//detector = ORB::create();
	//matcher = DescriptorMatcher::create("BruteForce-Hamming");
}

RobustMatcher::~RobustMatcher()
{
	// A Ptr<T> pretends to be a pointer to an object of type T.
	// The object will be automatically cleaned up once all Ptr instances pointing to it are destroyed.
	// SO there is no need to call delete detector and matcher.
	// \see std::shared_ptr since C++11
}

/**
 * \brief 匹配两幅图像，输出转动角度和匹配图像对，匹配成功返回1
 *
 * \param cv::Mat src1 输入 前一帧图像
 * \param cv::Mat src2 输入 后一帧图像
 * \param cv::Mat & img_matches 输出 匹配图像对
 * \param double & angle 输出 转动角度
 */
bool RobustMatcher::match(Mat src1, Mat src2, Mat& img_matches, double& angle)
{
	Mat mask1 = getMask(src1.size()), mask2 = getMask(src2.size());
	// // 0. Scale image2 to image1(1:1), so the scaling factor is 1
	// 1a. 检测ORB特征
	auto detector = ORB::create();
	vector<KeyPoint> keypoints1, keypoints2;
	detector->detect(src1, keypoints1, mask1);
	detector->detect(src2, keypoints2, mask2);
	qDebug() << "keypoints" << keypoints1.size() << keypoints2.size();

	// ***********if no keypoint detect then return
	if (keypoints1.empty() || keypoints2.empty())
		return false;
	// 1b. 提取ORB描述子
	Mat descriptors1, descriptors2;
	detector->compute(src1, keypoints1, descriptors1);
	detector->compute(src2, keypoints2, descriptors2);
	// 2. 匹配kNN(k=2)
	auto matcher = BFMatcher::create(NORM_HAMMING, true);//crosscheck=true
	vector<vector<DMatch>> matches; // vector of matches (up to 2 per entry)
	matcher->knnMatch(descriptors1, descriptors2, matches, 2); // return 2 nearest neighbors
	qDebug() << "knnmatch" << matches.size();
	// 3. RatioTest 仅当第一个匹配与第二个匹配之间的距离足够小时，才认为这是一个匹配。
	const float minRatio = 1.f / 1.5f;
	vector<DMatch> goodMatches;
	for (const auto & mat : matches)
	{
		auto bestMatch = mat[0];
		auto betterMatch = mat[1];
		float ratio = bestMatch.distance / betterMatch.distance;
		if (ratio < minRatio)
			goodMatches.push_back(bestMatch);
	}
	// 6. 绘制匹配结果
	drawMatches(src1, keypoints1, src2, keypoints2,
		goodMatches, img_matches, Scalar::all(-1), Scalar::all(-1),
		vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	// 5. 判断有效匹配点对是否大于等于10个(2个就可以进行计算)
	if (goodMatches.size() < 10) return false;
	// 7. 计算仿射变换矩阵
	vector<Point2f> obj, scene;
	for (auto & mat : goodMatches) {
		//-- Get the keypoints from the good matches
		obj.push_back(keypoints1[mat.queryIdx].pt);
		scene.push_back(keypoints2[mat.trainIdx].pt);
	}
	Mat M = estimateAffinePartial2D(obj, scene);
	if (M.data == nullptr) return false;
	// 8. 计算旋转角度，弧度
	auto sin = M.at<double>(1, 0); // sin*s
	auto cos = M.at<double>(0, 0); // cos*s
	angle = atan(sin / cos); // cv:顺时针为正
	return true;
}

/** \brief get a white ring mask for the srcImg.
* 图像大小与源图像相同，外径与内径在此设定
* \param cv::Size size 源图大小
*
*/
cv::Mat RobustMatcher::getMask(cv::Size size) const
{
	cv::Mat mask = cv::Mat::zeros(size, CV_8UC1);
	int r = size.height / 2;
	int ro = r - 10;
	int ri = r / 2 + 15;
	cv::circle(mask, cv::Point(r, r), ro, cv::Scalar(255), -1, 8);
	//bitwise_not(mask1, mask1);
	cv::circle(mask, cv::Point(r, r), ri, cv::Scalar(0), -1, 8);
	return mask;
}