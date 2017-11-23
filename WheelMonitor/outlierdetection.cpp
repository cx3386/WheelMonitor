#include "stdafx.h"
#include "outlierdetection.h"

using namespace std;
OutlierDetection::OutlierDetection()
{
}

OutlierDetection::~OutlierDetection()
{
}

void OutlierDetection::grubbs(std::vector<double> &v)
{
	auto size0 = v.size();
	if (size0 < 3)
	{
		return;
	}
	auto gp = grubbs95[size0 - 3]; //critical G
	sort(v.begin(), v.end());
	auto xm = mean(v);
	auto xd = stDev(v);
	auto gn = (v.back() - xm) / xd;
	auto gn1 = (xm - v.front()) / xd;
	if ((gn > gn1) && (gn > gp))
	{
		v.pop_back(); //xn is outlier, delete the last element
	}
	else if ((gn1 > gn) && (gn1 > gp))
	{
		v.erase(v.begin()); //x1 is outlier, delete the first element
	}
	if (v.size() == size0)
	{
		return;
	}
	grubbs(v);
}

double OutlierDetection::mean(std::vector<double> &v)
{
	auto sum = std::accumulate(v.begin(), v.end(), 0.0);
	return sum / v.size();
}

double OutlierDetection::stDev(std::vector<double> &v)
{
	double accum = 0;
	std::for_each(v.begin(), v.end(), [&](const double d) {
		accum += (d - mean(v)) * (d - mean(v));
	});
	return sqrt(accum / (v.size() - 1));
}