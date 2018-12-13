#include "stdafx.h"

#include "outlierhelper.h"
#include <algorithm>
#include <numeric>
#include <vector>

const double OutlierHelper::grubbs95[98] = {
    1.153,
    1.463,
    1.672,
    1.822,
    1.938,
    2.032,
    2.110,
    2.176,
    2.234,
    2.285,
    2.331,
    2.371,
    2.409,
    2.443,
    2.475,
    2.501,
    2.532,
    2.557,
    2.580,
    2.603,
    2.624,
    2.644,
    2.663,
    2.681,
    2.698,
    2.714,
    2.730,
    2.745,
    2.759,
    2.773,
    2.786,
    2.799,
    2.811,
    2.823,
    2.835,
    2.846,
    2.857,
    2.866,
    2.877,
    2.887,
    2.896,
    2.905,
    2.914,
    2.923,
    2.931,
    2.940,
    2.948,
    2.956,
    2.963,
    2.971,
    2.978,
    2.986,
    2.992,
    3.000,
    3.006,
    3.013,
    3.019,
    3.025,
    3.032,
    3.037,
    3.044,
    3.049,
    3.055,
    3.061,
    3.066,
    3.071,
    3.076,
    3.082,
    3.087,
    3.092,
    3.098,
    3.102,
    3.107,
    3.111,
    3.117,
    3.121,
    3.125,
    3.130,
    3.134,
    3.139,
    3.143,
    3.147,
    3.151,
    3.155,
    3.160,
    3.163,
    3.167,
    3.171,
    3.174,
    3.179,
    3.182,
    3.186,
    3.189,
    3.193,
    3.196,
    3.201,
    3.204,
    3.207,
}; //3~100
using namespace std;
void OutlierHelper::removeOutliers(std::vector<double>& v) const
{
    sort(v.begin(), v.end());
    if (v.size() > 100) {
        pauta(v);
    } else {
        grubbs(v);
    }
}

void OutlierHelper::grubbs(std::vector<double>& v) const
{
    auto size0 = v.size();
    if (size0 < 3) {
        return;
    }
    auto gp = grubbs95[size0 - 3]; //critical G
    //auto xm = mean(v);
    auto xm = median(v);
    auto xd = stDev(v);
    auto gn = (v.back() - xm) / xd;
    auto gn1 = (xm - v.front()) / xd;
    if ((gn > gn1) && (gn > gp)) {
        v.pop_back(); //xn is outlier, delete the last element
    } else if ((gn1 > gn) && (gn1 > gp)) {
        v.erase(v.begin()); //x1 is outlier, delete the first element
    } else {
        return; //detect no suspicious value
    }
    grubbs(v);
}

void OutlierHelper::pauta(std::vector<double>& v) const
{
    auto size0 = v.size();
    if (size0 <= 10) {
        grubbs(v);
    }
    auto xm = mean(v);
    auto xd = stDev(v);
    bool isErase = false;
    for (auto it = v.begin(); it != v.end();) {
        if (fabs(xm - *it) >= 3 * xd) {
            it = v.erase(it);
            isErase = true;
        } else {
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
    if (size % 2) {
        return v.at((size - 1) / 2);
    } else {
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