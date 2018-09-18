#pragma once
#include <vector>

class OutlierHelper
{
public:
	OutlierHelper() = default;
	~OutlierHelper() = default;
	void removeOutliers(std::vector<double> &v) const;
	double mean(std::vector<double> v) const; //arithmetical mean
	double stDev(std::vector<double> v) const;

private:
	static const double grubbs95[98];
	double median(std::vector<double> v) const;																																																																																																																																																																									//assume the vector is sorted already
	void grubbs(std::vector<double> &v) const;																																																																																																																																																																									//grubbs criterion
	void pauta(std::vector<double> &v) const;																																																																																																																																																																									//pauta criterion
};
