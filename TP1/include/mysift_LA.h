#ifndef MYSIFT_H
#define MYSIFT_H

#include <opencv2/core.hpp>
#include <vector>

class MySIFT
{
    // Member variables
protected:
    // Number of layers (scales) in multiscale representation
    int nbLayers;

    // Initial image
    cv::Mat base;

    float scaleFactor;

    float scaleBase;

    // Sequence of scales
    // The size of this vector should be nbLayers
    std::vector<float> scales;

    // Multiscale representation of image
    // The size of this vector should be nbLayers
    std::vector<cv::Mat> pyr;

    // Sequence of Difference-of-Gaussian images
    // The size of this vector should be nbLayers-1
    // dogpyr[i] = pyr[i+1]-pyr[i]
    std::vector<cv::Mat> dogpyr;

    float contrastThreshold;

    float edgeThreshold;

    // Number of bins of orientation histograms
    int nbBinsOrientation;

    // Number of bins of descriptor histograms
    int nbBinsDescriptor;

    // Number of histograms along one side
    // For a given keypoint, a descriptor is made up of descriptorWidth x descriptorWidth histograms
    int descriptorWidth;

    // Set of keypoints
    // Note: the 'octave' member of cv::KeyPoint is used as scale index!
    // The image corresponding to a keypoint kp is thus pyr[kp.octave]
    std::vector<cv::KeyPoint> keypoints;

    // Descriptors of all keypoints are stored in a single matrix
    // Number of rows = number of keypoints
    // Number of columns = number of features per keypoint
    cv::Mat descriptors;

    // Member functions
public:
    MySIFT(int nLayers = 30, float contrastThreshold = 0.02, float edgeThreshold = 10, float scaleFactor = 1.2);

    void setImageBase(const cv::Mat&);

    const std::vector<cv::KeyPoint>& getKeypoints();

    const cv::Mat& getDescriptors();

    // Detect SIFT points and compute their descriptors
    void computeSIFTPoints();

protected:
    void buildDoGScaleSpace();

    void findScaleSpaceExtrema();

    bool adjustLocalExtremum(const cv::Point3i&, cv::Point3i&) const;

    // Computes the dominant gradient orientation around a given keypoint,
    // using the histogram of gradient orientations
    // Sets the 'angle' field of the keypoint
    // Params: [in-out] keypoint
    void setDominantOrientation(cv::KeyPoint&) const;

    // Computes the SIFT descriptor around a given keypoint
    // from the histograms of gradient orientation,
    // Params: [in] keypoint, [out] pointer to array of features
    void computeDescriptor(const cv::KeyPoint&, float*);
};

#endif
