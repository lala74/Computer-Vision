/*
Copyright (c) 2017 Julien Mille
INSA Centre Val de Loire
Laboratoire d'Informatique Fondamentale et Appliquée de Tours
*/

#include <opencv2/core.hpp>
#include <string>

using namespace std;

// Convert OpenCV format to string
// Param: [in] OpenCV type (like CV_32S, CV_32F, CV_8UC3, ...)
string type2str(int);

// 2D Gaussian kernel
// Params: [out] gaussian kernel, [in] standard deviation, [in] (optional) half-size of mask
// Size of mask is computed as 2k+1, where k is the half-size in parameter, if k is different from -1
// If the half-size is -1, the size of the mask is computed from the standard deviation
void setGaussian(cv::Mat &, float, int halfSize=-1);

// First derivatives of Gaussian
// Params: [out] derxx, [out] deryy, [out] derxy, [in] standard deviation, [in] (optional) half-size of mask
// Size of mask is computed as 2k+1, where k is the half-size in parameter, if k is different from -1
// If the half-size is -1, the size of the mask is computed from the standard deviation
void setGaussianDer1(cv::Mat &, cv::Mat &, float, int halfSize=-1);

// Second derivatives of Gaussian
// Params: [out] derxx, [out] deryy, [out] derxy, [in] standard deviation, [in] (optional) half-size of mask
// Size of mask is computed as 2k+1, where k is the half-size in parameter, if k is different from -1
// If the half-size is -1, the size of the mask is computed from the standard deviation
void setGaussianDer2(cv::Mat &, cv::Mat &, cv::Mat &, float, int halfSize=-1);

// Remove non-maxima points
// Let I and O be the input and output image, respectively
// O[x,y] = I[x,y] if (x,y) is a local maxima
//          0 otherwise
// Params: [in] input image in CV_32F format, [out] output image in CV_32F format, [in] neighborhood (4 or 8)
void removeNonMaxima(const cv::Mat &, cv::Mat &, int neighborhood=4);

// Detect local extrema
// of type CV_8U (0->not an extremum, 1->local minimum, 2->local maximum)
// O[x,y] = 0 if (x,y) is not an extremum
//          1 if (x,y) is a local minimum
//          2 if (x,y) is a local maximum
// Output image is of type CV_8U
// Params: [in] input image, [out] output image , [in] neighborhood (4 or 8)
void localExtrema(const cv::Mat &, cv::Mat &, int neighborhood=4);

// Draw a straight cross in image
// Params: [in-out] image, [in] cross center
void drawCross(cv::Mat &, const cv::Point &, int idxColor=0);

// Estimate image value at real coordinates by bilinear interpolation
// Image is supposed to have real components (CV_32F..., CV_64F...)
// Params: [in] image, [in] point with real coordinates
template <typename T> T interp_bilinear(const cv::Mat &img, const cv::Point2f &pt)
{
    assert(pt.x>=0 && pt.y>=0 && pt.x<=img.cols-2 && pt.y<=img.rows-2);

    float dx, dy;
    int xi, yi;
    T *p;
    T interp;

    xi = (int)pt.x;
    yi = (int)pt.y;
    dx = pt.x-floor(pt.x);
    dy = pt.y-floor(pt.y);

    // Get address of nearest element with lower integer coordinates
    p = (T *)img.data + yi*img.cols + xi;

    interp = (T)(
        (1.0f-dx)*(1.0f-dy) * p[0] +
        dx*(1.0f-dy)        * p[1] +
        (1.0f-dx)*dy        * p[img.cols] +
        dx*dy               * p[img.cols+1]);

    return interp;
}
