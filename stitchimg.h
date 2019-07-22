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
    string loctn;
};

Point2f intersection(cnrs corner);

class stitchImg
{
public:
    stitchImg();

    ~stitchImg();

    void undistortImg(Mat &img);

    bool stictch();

    Mat *getQRcode(){return &QRcode;}

    void QRcodeRead();

    struct comparePix
    {
        inline bool operator() (const pair<double, Point> &a, const pair<double,Point> &b)
        {
            return (a.first < b.first);
        }
    };
private:
    qrcode *codeLocn(Mat &img);

    cnrs findCrns(Mat &img);

    qrcode topl;
    qrcode topr;
    qrcode botl;
    qrcode botr;
    vector<Point> dst_corners;
    Mat QRcode;
    cnrs *QRcorner;
};

#endif // STITCHIMG_H
