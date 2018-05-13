#include "outlierhelper.h"
#include <vector>
#include <algorithm>
#include <numeric>

using namespace std;

void OutlierHelper::removeOutliers(std::vector<double> &v) const
{
	sort(v.begin(), v.end());
	if (v.size() > 100)
	{
		pauta(v);
	}
	else
	{
		grubbs(v);
	}
}

void OutlierHelper::grubbs(std::vector<double> &v) const
{
	auto size0 = v.size();
	if (size0 < 3)
	{
		return;
	}
	auto gp = grubbs95[size0 - 3]; //critical G
	//auto xm = mean(v);
	auto xm = median(v);
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
	else
	{
		return; //detect no suspicious value
	}
	grubbs(v);
}

void OutlierHelper::pauta(std::vector<double> &v) const
{
	auto size0 = v.size();
	if (size0 <= 10)
	{
		grubbs(v);
	}
	auto xm = mean(v);
	auto xd = stDev(v);
	bool isErase = false;
	for (auto it = v.begin(); it != v.end();)
	{
		if (fabs(xm - *it) >= 3 * xd)
		{
			it = v.erase(it);
			isErase = true;
		}
		else
		{
			++it;
		}
	}
	if (!isErase)
	//if(v.size()==size0)
	{
		return;
	}
	pauta(v);
}

double OutlierHelper::mean(std::vector<double> v) const
{
	auto sum = std::accumulate(v.begin(), v.end(), 0.0);
	return sum / v.size();
}

double OutlierHelper::median(std::vector<double> v) const
{
	auto size = v.size();
	if (size % 2)
	{
		return v.at((size - 1) / 2);
	}
	else
	{
		return (v.at(size / 2) + v.at(size / 2 - 1)) / 2;
	}
}

double OutlierHelper::stDev(std::vector<double> v) const
{
	double accum = 0;
	std::for_each(v.begin(), v.end(), [&](const double d) {
		accum += (d - mean(v)) * (d - mean(v));
	});
	return sqrt(accum / (v.size() - 1));
}