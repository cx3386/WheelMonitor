#include "stdafx.h"
#include "RobustMatcher.h"

using namespace std;
using namespace cv;

RobustMatcher::RobustMatcher()
	:ratio(0.65f)
{
	detector = ORB::create(1500);
	matcher = DescriptorMatcher::create("BruteForce-Hamming");
}


RobustMatcher::~RobustMatcher()
{
	//未释放 detector & matcher
}

bool RobustMatcher::match(Mat & image1, Mat & image2, Mat & mask1, Mat &mask2, Mat& img_matches, double& angle)
{
	vector<KeyPoint> keypoints1, keypoints2;
	// 1a. Detection of the ORB features
	detector->detect(image1, keypoints1, mask1);
	detector->detect(image2, keypoints2, mask2);
	// ***********if no keypoint detect then return
	if (keypoints1.empty() || keypoints2.empty())
		return false;
	// 1b. Extraction of the ORB descriptors
	Mat descriptors1, descriptors2;
	detector->compute(image1, keypoints1, descriptors1);
	detector->compute(image2, keypoints2, descriptors2);

	// 2. Match the two image descriptors
	// from image 1 to image 2 based on k nearest neighbours (with k=2)
	vector<vector<DMatch> > matches1; // vector of matches (up to 2 per entry)
	matcher->knnMatch(descriptors1, descriptors2, matches1, 2); // return 2 nearest neighbours
	// from image 2 to image 1 based on k nearest neighbours (with k=2)
	vector<vector<DMatch> > matches2; // vector of matches (up to 2 per entry)
	matcher->knnMatch(descriptors2, descriptors1, matches2, 2); // return 2 nearest neighbours

	// 3. Remove matches for which NN ratio > threshold
	// clean image 1 -> image 2 matches
	ratioTest(matches1);
	// clean image 2 -> image 1 matches
	ratioTest(matches2);

	// 4. Remove non-symmetrical matches
	vector<DMatch>matches;
	symmetryTest(matches1, matches2, matches);

	// 5. 判断有效匹配点对是否大于等于10个(2个就可以进行计算)
	if (matches.size() < 10)
	{
		//qDebug("the match point less than 2");
		return false;
	}
	drawMatches(image1, keypoints1, image2, keypoints2,
		matches, img_matches, Scalar::all(-1), Scalar::all(-1),
		vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

	// 6. 求仿射变换
	vector<Point2f> obj;
	vector<Point2f> scene;
	for (int i = 0; i < matches.size(); i++) {
		//-- Get the keypoints from the good matches
		obj.push_back(keypoints1[matches[i].queryIdx].pt);
		scene.push_back(keypoints2[matches[i].trainIdx].pt);
	}
	Mat M = estimateRigidTransform(obj, scene, false);
	if (M.data == NULL)
		return false;
	//计算旋转角度
	angle = -asin(M.at<double>(0, 1)) / M_PI * 180;
	return true;
}

void RobustMatcher::symmetryTest(const vector<vector<DMatch>>& matches1, const vector<vector<DMatch>>& matches2, vector<DMatch>& symMatches)
{
	// for all matches image 1 -> image 2
	for (vector<vector<DMatch> >::
		const_iterator matchIterator1
		= matches1.begin();
		matchIterator1 != matches1.end(); ++matchIterator1) {
		// ignore deleted matches
		if (matchIterator1->size() < 2)
			continue;
		// for all matches image 2 -> image 1
		for (vector<vector<DMatch> >::
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

int RobustMatcher::ratioTest(vector<vector<DMatch>>& matches)
{
	int removed = 0;
	// for all matches
	for (vector<vector<DMatch> >::iterator
		matchIterator
		= matches.begin();
		matchIterator != matches.end(); ++matchIterator) {
		// if 2 NN has been identified
		if (matchIterator->size() > 1) {
			// check distance ratio
			if ((*matchIterator)[0].distance / (*matchIterator)[1].distance > ratio) {
				matchIterator->clear(); // remove match
				removed++;
			}
		}
		else { // does not have 2 neighbours
			matchIterator->clear(); // remove match
			removed++;
		}
	}
	return removed; //返回被删除的点数量
}

cv::Mat RobustMatcher::getMask(cv::Size size, int Ro, int Ri)
{
	cv::Mat mask = cv::Mat::zeros(size, CV_8UC1);
	cv::circle(mask, cv::Point(mask.rows / 2, mask.cols / 2), Ro, cv::Scalar(255), -1, 8);
	//bitwise_not(mask1, mask1);
	cv::circle(mask, cv::Point(mask.rows / 2, mask.cols / 2), Ri, cv::Scalar(0), -1, 8);
	return mask;
}
