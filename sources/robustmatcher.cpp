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
}

RobustMatcher::~RobustMatcher()
{
	// A Ptr<T> pretends to be a pointer to an object of type T.
	// The object will be automatically cleaned up once all Ptr instances pointing to it are destroyed.
	// SO there is no need to call delete detector and matcher.
	// \see std::shared_ptr since C++11
}

bool RobustMatcher::match(cv::Mat src1, cv::Mat src2, double maskOuterRatio, double maskInnerRatio, cv::Mat &img_matches, double &angle)
{
	int ro1, ri1, ro2, ri2;
	Mat mask1 = getMask(src1.size(), maskOuterRatio, maskInnerRatio, ro1, ri1);
	Mat mask2 = getMask(src2.size(), maskOuterRatio, maskInnerRatio, ro2, ri2);
	auto detector = ORB::create();
	vector<KeyPoint> keypoints1, keypoints2;
	// 1a. Detection of the ORB features
	detector->detect(src1, keypoints1, mask1);
	detector->detect(src2, keypoints2, mask2);
	// ***********if no keypoint detect then return
	if (keypoints1.empty() || keypoints2.empty())
		return false;
	// 1b. Extraction of the ORB descriptors
	Mat descriptors1, descriptors2;
	detector->compute(src1, keypoints1, descriptors1);
	detector->compute(src2, keypoints2, descriptors2);

	// 2. Match the two image descriptors
	auto matcher = BFMatcher::create(NORM_HAMMING);
	// from image 1 to image 2 based on k nearest neighbours (with k=2)
	vector<vector<DMatch>> matches1; // vector of matches (up to 2 per entry)
	matcher->knnMatch(descriptors1, descriptors2, matches1, 2); // return 2 nearest neighbors
	// from image 2 to image 1 based on k nearest neighbours (with k=2)
	vector<vector<DMatch>> matches2; // vector of matches (up to 2 per entry)
	matcher->knnMatch(descriptors2, descriptors1, matches2, 2); // return 2 nearest neighbors

	// 3. Remove matches for which NN ratio > threshold
	// clean image 1 -> image 2 matches
	ratioTest(matches1);
	// clean image 2 -> image 1 matches
	ratioTest(matches2);

	// 4. Remove non-symmetrical matches
	vector<DMatch> matches;
	symmetryTest(matches1, matches2, matches);

	// 5. 判断有效匹配点对是否大于等于10个(2个就可以进行计算)
	if (matches.size() < 10) {
		//qDebug("the match point less than 2");
		return false;
	}
	// 绘制匹配结果
	drawMatches(src1, keypoints1, src2, keypoints2,
		matches, img_matches, Scalar::all(-1), Scalar::all(-1),
		vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	circle(img_matches, Point(mask1.cols / 2, mask1.rows / 2), ro1, Scalar(0, 0, 255), 1, LINE_AA);
	circle(img_matches, Point(mask1.cols / 2, mask1.rows / 2), ri1, Scalar(0, 255, 0), 1, LINE_AA);
	circle(img_matches, Point(mask1.cols + mask2.cols / 2, mask2.rows / 2), ro2, Scalar(0, 0, 255), 1, LINE_AA);
	circle(img_matches, Point(mask1.cols + mask2.cols / 2, mask2.rows / 2), ri2, Scalar(0, 255, 0), 1, LINE_AA);

	// 6. 求仿射变换
	vector<Point2f> obj;
	vector<Point2f> scene;
	for (int i = 0; i < matches.size(); i++) {
		//-- Get the keypoints from the good matches
		obj.push_back(keypoints1[matches[i].queryIdx].pt);
		scene.push_back(keypoints2[matches[i].trainIdx].pt);
	}
	Mat M = estimateAffinePartial2D(obj, scene);
	if (M.data == nullptr)
		return false;
	//计算旋转角度，弧度
	auto sin_alpha = M.at<double>(1, 0); // sin*s
	auto cos_alpha = M.at<double>(0, 0); // cos*s
	angle = atan(sin_alpha / cos_alpha); // cv:顺时针为正
	return true;
}

void RobustMatcher::symmetryTest(const vector<vector<DMatch>>& matches1, const vector<vector<DMatch>>& matches2, vector<DMatch>& symMatches)
{
	// for all matches image 1 -> image 2
	for (vector<vector<DMatch>>::
		const_iterator matchIterator1
		= matches1.begin();
		matchIterator1 != matches1.end(); ++matchIterator1) {
		// ignore deleted matches
		if (matchIterator1->size() < 2)
			continue;
		// for all matches image 2 -> image 1
		for (vector<vector<DMatch>>::
			const_iterator matchIterator2
			= matches2.begin();
			matchIterator2 != matches2.end();
			++matchIterator2) {
			// ignore deleted matches
			if (matchIterator2->size() < 2)
				continue;
			// Match symmetry test
			if ((*matchIterator1)[0].queryIdx == (*matchIterator2)[0].trainIdx && (*matchIterator2)[0].queryIdx == (*matchIterator1)[0].trainIdx) {
				// add symmetrical match
				symMatches.push_back(
					DMatch((*matchIterator1)[0].queryIdx,
					(*matchIterator1)[0].trainIdx,
						(*matchIterator1)[0].distance));
				break; // next match in image 1 -> image 2
			}
		}
	}
}

/** \brief kNN
 * \param matches input matches
 * \return the removed points number
 */
int RobustMatcher::ratioTest(vector<vector<DMatch>>& matches)
{
	int removed = 0;
	// for all matches
	for (vector<vector<DMatch>>::iterator
		matchIterator
		= matches.begin();
		matchIterator != matches.end(); ++matchIterator) {
		// if 2 NN has been identified
		if (matchIterator->size() > 1) {
			// check distance ratio
			if ((*matchIterator)[0].distance / (*matchIterator)[1].distance > 0.65f) {
				matchIterator->clear(); // remove match
				removed++;
			}
		}
		else { // does not have 2 neighbors
			matchIterator->clear(); // remove match
			removed++;
		}
	}
	return removed;
}

/** \brief get a white ring mask for the srcImg.
* 图像为一白色圆环， 图像大小与源图像相同，外径与内径在此设定
* \param cv::Size size 源图大小
* \param outerRatio 外圆半径占比
* \param innerRatio 内圆半径占比
* \param ro 得到的外圆半径
* \param ri 得到的内圆半径
*/
cv::Mat RobustMatcher::getMask(cv::Size size, double outerRatio, double innerRatio, int &ro, int &ri) const
{
	cv::Mat mask = cv::Mat::zeros(size, CV_8UC1);
	int r = size.height / 2;
	ro = r * outerRatio;
	ri = r * innerRatio;
	cv::circle(mask, cv::Point(r, r), ro, cv::Scalar(255), -1, 8);
	//bitwise_not(mask1, mask1);
	cv::circle(mask, cv::Point(r, r), ri, cv::Scalar(0), -1, 8);
	return mask;
}