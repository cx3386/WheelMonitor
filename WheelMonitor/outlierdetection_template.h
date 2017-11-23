#pragma once

//T support basic data type, e.g. int, long, double
template<typename T> class OutlierDetection
{
public:
	OutlierDetection();
	~OutlierDetection();
	std::vector<T> Grubbs(std::vector<T>& v);
	double mean(std::vector<T>& v);	//arithmetical mean
	double stDev(std::vector<T>& v);

private:
	const double grubbs95[28] = { 1.153, 1.463, 1.672, 1.822, 1.938, 2.032, 2.110, 2.176, 2.234, 2.285, 2.331, 2.371, 2.409, 2.443, 2.475, 2.501, 2.532, 2.557, 2.580, 2.603, 2.624, 2.644, 2.663, 2.681, 2.698, 2.714, 2.730, 2.745 };//3~30
};

using namespace std;

template<typename T>
OutlierDetection<T>::OutlierDetection()
{
}

template<typename T>
OutlierDetection<T>::~OutlierDetection()
{
}

template<typename T>
std::vector<T> OutlierDetection<T>::Grubbs(std::vector<T>& v)
{
	sort(v.begin(), v.end());
	v.size();
	for (auto it = v.begin(); it != v.end(); ++it)
	{
		;
	}
	return v;
}

template<typename T>
double OutlierDetection<T>::mean(std::vector<T>& v)
{
	double sum = std::accumulate(begin(v), end(v), 0.0);
	return sum / v.size();
}

template<typename T>
double OutlierDetection<T>::stDev(std::vector<T>& v)
{
	double accum = 0;
	std::for_each(v.begin(), v.end(), [&](const double d) {
		accum += (d - mean(v))*(d - mean(v));
	});
	return sqrt(accum / (v.size() - 1));
}