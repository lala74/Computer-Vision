/*
Copyright (c) 2017 Julien Mille
INSA Centre Val de Loire
Laboratoire d'Informatique Fondamentale et Appliquée de Tours
*/

#include "opencv_utility.h"
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <math.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

string type2str(int type)
{
    string r;

    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch ( depth ) {
        case CV_8U:  r = "8U"; break;
        case CV_8S:  r = "8S"; break;
        case CV_16U: r = "16U"; break;
        case CV_16S: r = "16S"; break;
        case CV_32S: r = "32S"; break;
        case CV_32F: r = "32F"; break;
        case CV_64F: r = "64F"; break;
        default:     r = "User"; break;
    }

    r += "C";
    r += (chans+'0');

    return r;
}

void setGaussian(cv::Mat &g, float sigma, int halfSize)
{
 	int fullSize;
	float x2, y2;
	float f2Sigma2, fDen;
	float *pG;
	cv::Point p;

    if (halfSize==-1)
        halfSize = max(1, (int)(4.0*sigma));
    fullSize = 2*halfSize+1;

	g.create(fullSize, fullSize, CV_32F);

    f2Sigma2 = 2.0*sigma*sigma;
	fDen = 2.0*M_PI*sigma*sigma;

	pG = (float *)g.data;

	for (p.y=-halfSize;p.y<=halfSize;p.y++)
	{
		y2 = (float)(p.y*p.y);
		for (p.x=-halfSize;p.x<=halfSize;p.x++)
		{
			x2 = (float)(p.x*p.x);
			*pG++ = expf(-(x2+y2)/f2Sigma2)/fDen;
		}
	}
}

void setGaussianDer1(cv::Mat &derX, cv::Mat &derY, float sigma, int halfSize)
{
	int fullSize;
	float x2, y2;
	float f2Sigma2, fDen;
	float *pDerX, *pDerY;
	cv::Point p;

    if (halfSize==-1)
        halfSize = max(1, (int)(4.0*sigma));
    fullSize = 2*halfSize+1;

	derX.create(fullSize, fullSize, CV_32F);
    derY.create(fullSize, fullSize, CV_32F);

    f2Sigma2 = 2.0*sigma*sigma;
	fDen = -2.0*pow(sigma, 4)*M_PI;

	pDerX = (float *)derX.data;
	pDerY = (float *)derY.data;

	for (p.y=-halfSize;p.y<=halfSize;p.y++)
	{
		y2 = (float)(p.y*p.y);
		for (p.x=-halfSize;p.x<=halfSize;p.x++)
		{
			x2 = (float)(p.x*p.x);
			*pDerX++ = expf(-(x2+y2)/f2Sigma2)*((float)p.x)/fDen;
			*pDerY++ = expf(-(x2+y2)/f2Sigma2)*((float)p.y)/fDen;
		}
	}
}

void setGaussianDer2(cv::Mat &derXX, cv::Mat &derYY, cv::Mat &derXY, float sigma, int halfSize)
{
	int fullSize;
	float x2, y2;
	float f2Sigma2, fDen, fExp;
	float *pDerXX, *pDerYY, *pDerXY;
	cv::Point p;

    if (halfSize==-1)
        halfSize = max(1, (int)(4.0*sigma));
    fullSize = 2*halfSize+1;

	derXX.create(fullSize, fullSize, CV_32F);
    derYY.create(fullSize, fullSize, CV_32F);
    derXY.create(fullSize, fullSize, CV_32F);

    f2Sigma2 = 2.0*sigma*sigma;
	fDen = pow(sigma, 4)*M_PI;

	pDerXX = (float *)derXX.data;
	pDerYY = (float *)derYY.data;
	pDerXY = (float *)derXY.data;

	for (p.y=-halfSize;p.y<=halfSize;p.y++)
	{
		y2 = (float)(p.y*p.y);
		for (p.x=-halfSize;p.x<=halfSize;p.x++)
		{
			x2 = (float)(p.x*p.x);
			fExp = expf(-(x2+y2)/f2Sigma2);

			*pDerXX++ = fExp*(x2/f2Sigma2-0.5)/fDen;
			*pDerYY++ = fExp*(y2/f2Sigma2-0.5)/fDen;
			*pDerXY++ = fExp*(float)(p.x*p.y)/(f2Sigma2*fDen);
		}
	}
}

void removeNonMaxima(const cv::Mat &in, cv::Mat &out, int neighborhood)
{
	cv::Point p;
	vector<cv::Point> neighbors;

	assert(neighborhood==4 || neighborhood==8);

	if (neighborhood==8)
        neighbors = vector<cv::Point>({ {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1} });
	else
        neighbors = vector<cv::Point>({ {-1,0},{1,0},{0,-1},{0,1}});

	unsigned int i;
	bool isMax;

	assert(in.type()==CV_32F);
	out = cv::Mat::zeros(in.size(), CV_32F);

	for (p.y=1;p.y<in.rows-1;p.y++)
	{
		for (p.x=1;p.x<in.cols-1;p.x++)
		{
            isMax = true;
            for (i=0; i<neighbors.size() && isMax; i++)
                isMax = (in.at<float>(p+neighbors[i])<=in.at<float>(p));
            if (isMax)
                out.at<float>(p) = in.at<float>(p);
		}
	}
}


void localExtrema(const cv::Mat &in, cv::Mat &out, int neighborhood)
{
	cv::Point p;
	vector<cv::Point> neighbors;

	assert(neighborhood==4 || neighborhood==8);

	if (neighborhood==8)
        neighbors = vector<cv::Point>({ {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1} });
	else
        neighbors = vector<cv::Point>({ {-1,0},{1,0},{0,-1},{0,1}});

	unsigned int i;
	bool isMax, isMin;

	assert(in.type()==CV_32F);
	out = cv::Mat::zeros(in.size(), CV_8U);

	for (p.y=1;p.y<in.rows-1;p.y++)
	{
		for (p.x=1;p.x<in.cols-1;p.x++)
		{
            isMin = true;
            for (i=0; i<neighbors.size() && isMin; i++)
                isMin = (in.at<float>(p+neighbors[i])>in.at<float>(p));
            if (isMin)
                out.at<uchar>(p) = 1;
            else {
                isMax = true;
                for (i=0; i<neighbors.size() && isMax; i++)
                    isMax = (in.at<float>(p+neighbors[i])<in.at<float>(p));
                if (isMax)
                    out.at<uchar>(p) = 2;
            }
		}
	}
}

void drawCross(cv::Mat &out, const cv::Point &pt, int idxColor)
{
    int s = 3;
    cv::Scalar sc;

    if (idxColor==0)
        sc = cv::Scalar(0,0,255); // Red
    else
        sc = cv::Scalar(255,0,0); // Blue

    cv::line(out, pt+cv::Point(-s,0), pt+cv::Point(s,0), sc, 1);
    cv::line(out, pt+cv::Point(0,-s), pt+cv::Point(0,s), sc, 1);
}
