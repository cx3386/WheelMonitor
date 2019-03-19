#pragma once

#include <QImage>
namespace cv
{
	class Mat;
}

namespace utils_cx
{
	QImage Mat2QImage(cv::Mat const& src);
}