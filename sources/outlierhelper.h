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
	const double grubbs95[98] = {1.153, 1.463, 1.672, 1.822, 1.938, 2.032, 2.110, 2.176, 2.234, 2.285, 2.331, 2.371, 2.409, 2.443, 2.475, 2.501, 2.532, 2.557, 2.580, 2.603, 2.624, 2.644, 2.663, 2.681, 2.698, 2.714, 2.730, 2.745, 2.759, 2.773, 2.786, 2.799, 2.811, 2.823, 2.835, 2.846, 2.857, 2.866, 2.877, 2.887, 2.896, 2.905, 2.914, 2.923, 2.931, 2.940, 2.948, 2.956, 2.963, 2.971, 2.978, 2.986, 2.992, 3.000, 3.006, 3.013, 3.019, 3.025, 3.032, 3.037, 3.044, 3.049, 3.055, 3.061, 3.066, 3.071, 3.076, 3.082, 3.087, 3.092, 3.098, 3.102, 3.107, 3.111, 3.117, 3.121, 3.125, 3.130, 3.134, 3.139, 3.143, 3.147, 3.151, 3.155, 3.160, 3.163, 3.167, 3.171, 3.174, 3.179, 3.182, 3.186, 3.189, 3.193, 3.196, 3.201, 3.204, 3.207}; //3~100
	double median(std::vector<double> v) const;																																																																																																																																																																									//assume the vector is sorted already
	void grubbs(std::vector<double> &v) const;																																																																																																																																																																									//grubbs criterion
	void pauta(std::vector<double> &v) const;																																																																																																																																																																									//pauta criterion
};
