#include "stitchimg.h"

stitchImg::stitchImg()
{
    dst_corners.push_back(Point(0,0));
    dst_corners.push_back(Point(0,200));
    dst_corners.push_back(Point(200,200));
    dst_corners.push_back(Point(200,0));

    QRcode= cv::Mat::zeros(400, 400, CV_8UC3);
}

stitchImg::~stitchImg(){
    delete QRcorner;
    QRcorner = nullptr;
}

void stitchImg::undistortImg(Mat &img)
{
    // Determine code type and pixel location:
    // type - is code the top left, top right,
    // bottom left, bottom right segment
    qrcode *pQr = codeLocn(img);        

    Mat dst_img;
    if (pQr != nullptr){
        // Undistort the image segment
        vector<Point> src_corners{Point((*pQr).corners.tl),Point((*pQr).corners.bl),
                             Point((*pQr).corners.br),Point((*pQr).corners.tr)};
        Mat h = findHomography(src_corners,dst_corners);
        warpPerspective(img, dst_img, h, Size(200,200));
    } else{
        return;
    }
    //imshow("unwarped", dst_img);
    //waitKey(0);

    if ((*pQr).loctn == "topl")
        dst_img.copyTo(topl.code);
    else if ((*pQr).loctn == "topr")
        dst_img.copyTo(topr.code);
    else if ((*pQr).loctn == "botl")
        dst_img.copyTo(botl.code);
    else if ((*pQr).loctn == "botr")
        dst_img.copyTo((botr.code));
}

qrcode *stitchImg::codeLocn(Mat& img)
{
    // Choose colour of boarder to extract from image
    Vec3b bgrPixel(10, 255, 10);
    Mat3b bgr (bgrPixel);
    int thresh = 5;
    Scalar minBGR = Scalar(bgrPixel.val[0] - thresh, bgrPixel.val[1] - thresh, bgrPixel.val[2] - thresh);
    Scalar maxBGR = Scalar(bgrPixel.val[0] + thresh, min(bgrPixel.val[1] + thresh, 255), bgrPixel.val[2] + thresh);
    Mat maskBGR, resultBGR, resultBGR_blur;
    inRange(img, minBGR, maxBGR, maskBGR);
    bitwise_and(img,img,resultBGR,maskBGR);

    // Find the centroid of the boarder and QR code to determine the location of the
    // QR code
    Mat gray;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    GaussianBlur(resultBGR,resultBGR_blur,Size(3,3),0,0);
    cvtColor(resultBGR_blur,gray,COLOR_BGR2GRAY);
    inRange(gray,20,255,gray);
    Mat element = getStructuringElement(0, Size(3,3), Point(-1,-1));
    //erode(gray,gray,element);
    //imshow("inRange",gray);
    //waitKey(0);
    findContours(gray,contours,hierarchy,RETR_TREE,CHAIN_APPROX_SIMPLE,Point(0,0));
    Moments muiPad, muQR;
    bool iPadcontour = false, QRcontour = false;
    for (unsigned int i = 0; i < hierarchy.size(); i++){
        if ((hierarchy[i][2] > -1) && (hierarchy[i][3] < 0)){
            muiPad = moments(contours[i],false);
            iPadcontour = true;
        }else if ((hierarchy[i][3] > -1) && (hierarchy[i][2] < 0)){
            muQR = moments(contours[i],false);
            QRcontour = true;
        }
    }
    if (!iPadcontour || !QRcontour)
        return nullptr;

    Point2f QRcentroid = Point2f(muQR.m10/muQR.m00, muQR.m01/muQR.m00);
    Point2f iPadcentroid = Point2f(muiPad.m10/muiPad.m00, muiPad.m01/muiPad.m00);;

    /*Scalar white(255,255,255);
    circle(gray,QRcentroid,2,white,-1);
    circle(gray,iPadcentroid,2,white,-1);
    imshow("Centroid",gray);
    waitKey(0);*/

    // Find corners of image
    QRcorner = new cnrs;
    Mat dst, dst_norm, dst_norm_scaled;
    cornerHarris(gray, dst, 6, 3, 0.04);
    normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
    convertScaleAbs( dst_norm, dst_norm_scaled );
    inRange(dst_norm_scaled,100, 255, dst_norm_scaled);
    dilate(dst_norm_scaled,dst_norm_scaled,element);
    //imshow("Edge detection", dst_norm_scaled);
    //waitKey(0);
    /*int k = 50;
    kLargestHeap fps(dst_norm_scaled,k);
    for (int i = 0; i < dst_norm_scaled.cols*dst_norm_scaled.rows; i++){
        fps.swapRoot(dst_norm_scaled,i);
    }

    for (int i = 0; i < k; i++){

    }*/
    vector<vector<Point>> Contours;
    vector<Vec4i> Hierarchy;
    findContours(dst_norm_scaled,Contours,Hierarchy,RETR_TREE,CHAIN_APPROX_SIMPLE,Point(0,0));
    Mat dstBGR;//(dst_norm_scaled.size(),CV_8UC3);
    cvtColor(dst_norm_scaled,dstBGR,COLOR_GRAY2BGR);
    vector<Moments> mu(Contours.size());
    for (unsigned int i = 0; i < Contours.size(); i++){
        mu[i] = moments(Contours[i],false);
        drawContours(dstBGR,Contours,i,Scalar(0,0,255),2,8);
    }

    vector<Point2f> mc(Contours.size());
    for (unsigned int i = 0; i < Contours.size(); i++){
        mc[i] = Point2f(mu[i].m10/mu[i].m00, mu[i].m01/mu[i].m00);
    }

    //imshow("Corners",dstBGR);
    //waitKey(0);

    // Only take corners corresponding to rQR code (closest to QR centroid)
    vector<pair<double,Point>> pixels;
    for (int i = 0; i < Contours.size(); i++){
        double dist = SQ(mc[i].x - QRcentroid.x) + SQ(mc[i].y - QRcentroid.y);
        pixels.push_back(make_pair(dist,Point(mc[i].x,mc[i].y)));
    }
    sort(pixels.begin(), pixels.end(),comparePix());
    int crnrsFound[4] = {0};
    int j = 0;
    while (((crnrsFound[0] == 0) || (crnrsFound[1] == 0) || (crnrsFound[2] == 0)
           || (crnrsFound[3] == 0)) && j<8 ){
        Point pix = Point(pixels[j].second.x,pixels[j].second.y);
        if (pix.x < QRcentroid.x){
            if ((pix.y < QRcentroid.y) && (crnrsFound[0] == 0)){
                QRcorner->tl = pix;
                crnrsFound[0] = 1;
            }else if ((pix.y > QRcentroid.y) && (crnrsFound[1] == 0)){
                QRcorner->bl = pix;
                crnrsFound[1] = 1;
            }
        }else{
            if ((pix.y < QRcentroid.y) && (crnrsFound[2] == 0) ){
                QRcorner->tr = pix;
                crnrsFound[2] = 1;
            }else if ((pix.y > QRcentroid.y) && (crnrsFound[3] == 0)){
                QRcorner->br = pix;
                crnrsFound[3] = 1;
            }
        }
        j++;
    }
    //cnrs iPadcorner;
    //QRcentroid = intersection(QRcorner);
    //iPadcentroid = intersection(iPadcorner);

    if (QRcentroid.x < iPadcentroid.x){
        if (QRcentroid.y < iPadcentroid.y){
            //botl.code = img;
            topl.corners = *QRcorner;
            topl.loctn = "topl";
            return &topl;
        }else{
            //topl.code = img;
            botl.corners = *QRcorner;
            botl.loctn = "botl";
            return &botl;
        }
    }else{
        if (QRcentroid.y < iPadcentroid.y){
            //botr.code = img;
            topr.corners = *QRcorner;
            topr.loctn = "topr";
            return &topr;
        }else{
            //topr.code = img;
            botr.corners = *QRcorner;
            botr.loctn = "botr";
            return &botr;
        }
    }
}

bool stitchImg::stictch()
{
    if((!topl.code.empty()) && (!topr.code.empty()) &&
            (!botl.code.empty()) && (!botr.code.empty())){
        Rect toplROI(0,0,200,200), toprROI(200,0,200,200), botlROI(0,200,200,200), botrROI(200,200,200,200);
        (topl.code).copyTo(QRcode(toplROI));
        topr.code.copyTo(QRcode(toprROI));
        botl.code.copyTo(QRcode(botlROI));
        botr.code.copyTo(QRcode(botrROI));
        return true;
    }else{
        return false;
    }
}

void stitchImg::QRcodeRead()
{
   //QRCodeDetector qrDetector = QRCodeDetector::QRCodeDetector();
}

Point2f intersection(cnrs corner)
{
    // Line connecting tl and br corners
    double a1 = corner.tl.y - corner.br.y;
    double b1 = corner.br.x - corner.tl.x;
    double c1 = a1*(corner.br.x) + b1*(corner.br.y);

    // Line connecting bl and tr corners
    double a2 = corner.tr.y - corner.bl.y;
    double b2 = corner.bl.x - corner.tr.x;
    double c2 = a2*(corner.bl.x) + b2*(corner.bl.y);

    double determinant = a1*b2 - a2*b1;

    if (determinant == 0)
    {
        // The lines are parallel
        return Point2f(FLT_MAX, FLT_MAX);
    }else
    {
        double x = (b2*c1 - b1*c2)/determinant;
        double y = (a1*c2 - a2*c1)/determinant;
        return Point2f(x,y);
    }

}
