#include <cmath>
#include <algorithm>
#include <array>

#include "Common.h"

using namespace cv;
using namespace std;

void convertHSV(const Mat& image_)
{
	Mat_<Vec3b> image = image_;
	double B, G, R, V, H, diff;

	for(int i = 0; i < image.rows; ++i)
		for(int j = 0; j < image.cols; ++j)
		{
			B = image(i, j)[0];
			G = image(i, j)[1];
			R = image(i, j)[2];

			// V channel
			image(i, j)[2] = V = max(max(B, G), R);

			// S channel
			diff = V - min(min(B, G), R);
			image(i, j)[1] = (V != 0) ? (255 * diff / V) : 0;

			// H channel
			if(diff != 0)
			{
				if(V == B)
					H = 240 + 60 * (R-G)/diff;
				else if(V == G)
					H = 120 + 60 * (B-R)/diff;
				else
					H = 60 * (G-B)/diff;

				if(H < 0) H += 360;
				image(i, j)[0] = H / 2;
			}
		}
}

bool isLess(const Vec3b& v1, const Vec3b& v2)
{
	if(v1[0] < v2[0])
		return true;
	if(v1[0] > v2[0])
		return false;
	if(v1[1] < v2[1])
		return true;
	if(v1[1] > v2[1])
		return false;
	if(v1[2] < v2[2])
		return true;
	return false;
}


bool checkLessMin(uchar min, uchar max, uchar value)
{
	if(value >= min && value <= max)
		return true;
	return false;
}
bool checkLessMax(uchar min, uchar max, uchar value)
{
	if(value >= min || value <= max)
		return true;
	return false;
}

Mat selectColor(const Mat& image_, const Vec3b& minValues, const Vec3b& maxValues)
{
	Mat result_(image_.rows, image_.cols, CV_8UC3);
	Mat_<Vec3b> image = image_;
	Mat_<Vec3b> result = result_;
	bool(*check0)(uchar, uchar, uchar);
	bool(*check1)(uchar, uchar, uchar);
	bool(*check2)(uchar, uchar, uchar);

	if(minValues[0] <= maxValues[0])
		check0 = &checkLessMin;
	else
		check0 = &checkLessMax;
	if(minValues[1] <= maxValues[1])
		check1 = &checkLessMin;
	else
		check1 = &checkLessMax;
	if(minValues[2] <= maxValues[2])
		check2 = &checkLessMin;
	else
		check2 = &checkLessMax;

	for(int i = 0; i < image.rows; ++i)
		for(int j = 0; j < image.cols; ++j)
			if(check0(minValues[0], maxValues[0], image(i,j)[0]) &&
					check1(minValues[1], maxValues[1], image(i,j)[1]) &&
					check2(minValues[2], maxValues[2], image(i,j)[2]))
				result(i,j) = WHITE_VEC;
	return result;
}

Mat dilate(const Mat& image_, int iterations) {
	Mat result_(image_.rows, image_.cols, CV_8UC3);
	Mat_<Vec3b> image = image_;
	Mat_<Vec3b> result = result_;

	Mat_<Vec3b> tmp;
	hconcat(image, image.col(0), tmp);
	hconcat(tmp, tmp.col(tmp.cols-1), tmp);
	vconcat(tmp, tmp.row(0), tmp);
	vconcat(tmp, tmp.row(tmp.rows-1), tmp);

	for(int iter = 0; iter < iterations; ++iter)
		for(int i = 0; i < result.rows; ++i)
			for(int j = 0; j < result.cols; ++j) {
				array<Vec3b, 9> values = {tmp(i, j), tmp(i, j+1), tmp(i, j+2),
						tmp(i+1, j), tmp(i+1, j+1), tmp(i+1, j+2),
						tmp(i+2, j), tmp(i+2, j+1), tmp(i+2, j+2)};
				result(i, j) = *(max_element(values.begin(), values.end(), isLess));
			}
	return result;
}

Mat erode(const Mat& image_, int iterations) {
	Mat result_(image_.rows, image_.cols, CV_8UC3);
	Mat_<Vec3b> image = image_;
	Mat_<Vec3b> result = result_;

	Mat_<Vec3b> tmp;
	hconcat(image, image.col(0), tmp);
	hconcat(tmp, tmp.col(tmp.cols-1), tmp);
	vconcat(tmp, tmp.row(0), tmp);
	vconcat(tmp, tmp.row(tmp.rows-1), tmp);

	for(int iter = 0; iter < iterations; ++iter)
		for(int i = 0; i < result.rows; ++i)
			for(int j = 0; j < result.cols; ++j) {
				array<Vec3b, 9> values = {tmp(i, j), tmp(i, j+1), tmp(i, j+2),
						tmp(i+1, j), tmp(i+1, j+1), tmp(i+1, j+2),
						tmp(i+2, j), tmp(i+2, j+1), tmp(i+2, j+2)};
				result(i, j) = *(min_element(values.begin(), values.end(), isLess));
			}
	return result;
}

Mat morphologyClose(const Mat& image, int iterations) {
	return erode(dilate(image, iterations), iterations);
}

Mat morphologyOpen(const Mat& image, int iterations) {
	return dilate(erode(image, iterations), iterations);
}

Mat medianFilter(const Mat& image_, int iterations) {
	Mat result_(image_.rows, image_.cols, CV_8UC3);
	Mat_<Vec3b> image = image_;
	Mat_<Vec3b> result = result_;

	Mat_<Vec3b> tmp;
	hconcat(image, image.col(0), tmp);
	hconcat(tmp, tmp.col(tmp.cols-1), tmp);
	vconcat(tmp, tmp.row(0), tmp);
	vconcat(tmp, tmp.row(tmp.rows-1), tmp);

	for(int iter = 0; iter < iterations; ++iter)
		for(int i = 1; i < tmp.rows-1; ++i)
			for(int j = 1; j < tmp.cols-1; ++j) {
				array<Vec3b, 9> values = {tmp(i-1, j-1), tmp(i-1, j), tmp(i-1, j+1),
						tmp(i, j-1), tmp(i, j), tmp(i, j+1),
						tmp(i+1, j-1), tmp(i+1, j), tmp(i+1, j+1)};
				sort(values.begin(), values.end(), isLess);
				result(i-1, j-1) = values[5];
			}

	return result;
}
