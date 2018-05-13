#pragma once
#include <opencv.hpp>
#include "QMetaType"
struct ImProfile
{
  private:
	int *interv;

  public:
	ImProfile() = default;
	ImProfile(int &framInterv) { interv = &framInterv; }
	inline double angle2Speed() const { return 60 * (0.650 / 2) / (*interv / 25.0); }
	double warningRatio = 0.05;
	double alarmRatio = 0.10;
	int radius_min = 250;
	int radius_max = 200;
	int gs1 = 9;
	int gs2 = 9;
	double dp = 2;
	double minDist = 180;
	double param1 = 200;
	double param2 = 100;
	bool sensorTriggered = false;
	cv::Rect roiRect = cv::Rect(220, 0, 800, 720);
};
Q_DECLARE_METATYPE(ImProfile)

QDataStream &operator<<(QDataStream &out, const ImProfile &p);
QDataStream &operator>>(QDataStream &in, ImProfile &p);
