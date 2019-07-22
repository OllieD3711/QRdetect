#ifndef STITCHIMG_H
#define STITCHIMG_H

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <bits/stdc++.h>
#include "klargestheap.h"


using namespace std;
using namespace cv;

struct cnrs {
    Point tl;
    Point tr;
    Point bl;
    Point br;
};

struct qrcode {
    Mat code;
    cnrs corners;
    Point2f loctn;
};

Point2f intersection(cnrs corner);

class stitchImg
{
public:
	stitchImg();
    stitchImg(bool debug);

    ~stitchImg();

    void undistortImg(Mat &img);

    bool stictch();

    Mat *getQRcode(){return &QRcode;}

    void QRcodeRead();

	// Takes contours and sorts them largest to smallest in terms of enclosed area
	struct sortContour
	{
		inline bool operator() (const vector<Point> &a, const vector<Point> &b)
		{
			double areaA = fabs(contourArea(Mat(a)));
			double areaB = fabs(contourArea(Mat(b)));
			return (areaA > areaB);
		}
	}

	// Takes pixel values and sorts them largest to smallest in terms of measurement parameter
    struct sortPointPair
    {
        inline bool operator() (const pair<int, Point> &a, const pair<int,Point> &b)
        {
            return (a.first > b.first);
        }
    };
private:
    bool codeLocn(Mat &img);
	bool drawDebug;
    cnrs findCrns(Mat &img);
    vector<qrcode> codeElements;
    vector<Point> dst_corners;
    Mat QRcode;
    cnrs QRcorner;
};

#endif // STITCHIMG_H
