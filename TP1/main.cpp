#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgcodecs.hpp>  // For cv::imread, cv::imwrite

#include "graphicwnd.h"
#include "mysift_LA.h"
#include "opencv_utility.h"

using namespace std;

#ifdef WIN32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    cv::Mat img1, img2;

    // This ratio is used to select good matches.
    // A match will be considered as "good" if its distance is below the distance
    // of the best match multiplied by this ratio
    float goodMatchRatio;

    //    img1 = cv::imread("./images/table01_640x480.jpg", cv::IMREAD_GRAYSCALE);
    img1 = cv::imread("./images/test1.jpg", cv::IMREAD_GRAYSCALE);
    if(img1.data == nullptr) {
        cout << "Cannot open ./images/bep1.jpg" << endl;
        return -1;
    }

    //    img2 = cv::imread("./images/table02_640x480.jpg", cv::IMREAD_GRAYSCALE);
    img2 = cv::imread("./images/test2.jpg", cv::IMREAD_GRAYSCALE);
    if(img1.data == nullptr) {
        cout << "Cannot open ./images/bep2.jpg" << endl;
        return -1;
    }

    vector<cv::KeyPoint> keypoints1, keypoints2;
    cv::Mat descriptors1, descriptors2;

    cv::Mat imgReal1, imgReal2;

    // Our home-made SIFT wants images with real values between 0 and 1
    img1.convertTo(imgReal1, CV_32F, 1.0 / 255.0);
    img2.convertTo(imgReal2, CV_32F, 1.0 / 255.0);

    MySIFT sift1;
    sift1.setImageBase(imgReal1);
    sift1.computeSIFTPoints();

    MySIFT sift2;
    sift2.setImageBase(imgReal2);
    sift2.computeSIFTPoints();

    keypoints1 = sift1.getKeypoints();
    keypoints2 = sift2.getKeypoints();

    descriptors1 = sift1.getDescriptors();
    descriptors2 = sift2.getDescriptors();

    //        return app.exec();

    goodMatchRatio = 1.5;
    //        goodMatchRatio = 1.3;

    // Brute-force (exhaustive) search: each descriptor of the first image
    // is compared to each descriptor of the second one (in terms of Euclidean
    // distance)
    cv::BFMatcher matcher;
    vector<cv::DMatch> matches;
    vector<cv::DMatch> good_matches;

    matcher.match(descriptors1, descriptors2, matches);
    if(matches.size() == 0) {
        cout << "No match!" << endl;
        return -1;
    }

    double max_dist = matches[0].distance;
    double min_dist = matches[0].distance;

    // Get the minimal and maximal distances
    for(int i = 0; i < descriptors1.rows; i++) {
        const double& dist = matches[i].distance;
        if(dist < min_dist) min_dist = dist;
        if(dist > max_dist) max_dist = dist;
    }
    cout << "Min distance: " << min_dist << endl;
    cout << "Max distance: " << max_dist << endl;

    // Extract "good" matches: (i.e. whose distance is less than min_dist x ratio)
    for(int i = 0; i < descriptors1.rows; i++) {
        if(matches[i].distance <= goodMatchRatio * min_dist) good_matches.push_back(matches[i]);
    }

    // Draw good matches...
    cv::Mat img_matches;
    cv::drawMatches(img1,
                    keypoints1,
                    img2,
                    keypoints2,
                    good_matches,
                    img_matches,
                    cv::Scalar::all(-1),
                    cv::Scalar::all(-1),
                    vector<char>(),
                    cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    // .. and show them
    CImage2DWnd::Show(img_matches, "Good Matches");

    return app.exec();
}
