#pragma once

class OutlierDetection
{
public:
	OutlierDetection();
	~OutlierDetection();
	void grubbs(std::vector<double>& v);	//grubbs criterion
	void pauta(std::vector<double>& v);	//pauta criterion
	double mean(std::vector<double>& v);	//arithmetical mean
	double stDev(std::vector<double>& v);

private:
	const double grubbs95[28] = { 1.153, 1.463, 1.672, 1.822, 1.938, 2.032, 2.110, 2.176, 2.234, 2.285, 2.331, 2.371, 2.409, 2.443, 2.475, 2.501, 2.532, 2.557, 2.580, 2.603, 2.624, 2.644, 2.663, 2.681, 2.698, 2.714, 2.730, 2.745 };//3~30
};
