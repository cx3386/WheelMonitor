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
 * \brief ƥ������ͼ�����ת���ǶȺ�ƥ��ͼ��ԣ�ƥ��ɹ�����1
 *
 * \param cv::Mat src1 ���� ǰһ֡ͼ��
 * \param cv::Mat src2 ���� ��һ֡ͼ��
 * \param cv::Mat & img_matches ��� ƥ��ͼ���
 * \param double & angle ��� ת���Ƕ�
 */
bool RobustMatcher::match(Mat src1, Mat src2, Mat& img_matches, double& angle)
{
	Mat mask1 = getMask(src1.size()), mask2 = getMask(src2.size());
	// // 0. Scale image2 to image1(1:1), so the scaling factor is 1
	// 1a. ���ORB����
	auto detector = ORB::create();
	vector<KeyPoint> keypoints1, keypoints2;
	detector->detect(src1, keypoints1, mask1);
	detector->detect(src2, keypoints2, mask2);
	qDebug() << "keypoints" << keypoints1.size() << keypoints2.size();

	// ***********if no keypoint detect then return
	if (keypoints1.empty() || keypoints2.empty())
		return false;
	// 1b. ��ȡORB������
	Mat descriptors1, descriptors2;
	detector->compute(src1, keypoints1, descriptors1);
	detector->compute(src2, keypoints2, descriptors2);
	// 2. ƥ��kNN(k=2)
	auto matcher = BFMatcher::create(NORM_HAMMING, true);//crosscheck=true
	vector<vector<DMatch>> matches; // vector of matches (up to 2 per entry)
	matcher->knnMatch(descriptors1, descriptors2, matches, 2); // return 2 nearest neighbors
	qDebug() << "knnmatch" << matches.size();
	// 3. RatioTest ������һ��ƥ����ڶ���ƥ��֮��ľ����㹻Сʱ������Ϊ����һ��ƥ�䡣
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
	// 6. ����ƥ����
	drawMatches(src1, keypoints1, src2, keypoints2,
		goodMatches, img_matches, Scalar::all(-1), Scalar::all(-1),
		vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	// 5. �ж���Чƥ�����Ƿ���ڵ���10��(2���Ϳ��Խ��м���)
	if (goodMatches.size() < 10) return false;
	// 7. �������任����
	vector<Point2f> obj, scene;
	for (auto & mat : goodMatches) {
		//-- Get the keypoints from the good matches
		obj.push_back(keypoints1[mat.queryIdx].pt);
		scene.push_back(keypoints2[mat.trainIdx].pt);
	}
	Mat M = estimateAffinePartial2D(obj, scene);
	if (M.data == nullptr) return false;
	// 8. ������ת�Ƕȣ�����
	auto sin = M.at<double>(1, 0); // sin*s
	auto cos = M.at<double>(0, 0); // cos*s
	angle = atan(sin / cos); // cv:˳ʱ��Ϊ��
	return true;
}

/** \brief get a white ring mask for the srcImg.
* ͼ���С��Դͼ����ͬ���⾶���ھ��ڴ��趨
* \param cv::Size size Դͼ��С
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