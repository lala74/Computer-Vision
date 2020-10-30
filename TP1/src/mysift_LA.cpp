#include "mysift_LA.h"

#include <iostream>
#include <opencv2/features2d.hpp>  // For cv::KeyPointsFilter
#include <opencv2/imgproc.hpp>

#include "graphicwnd.h"
#include "opencv_utility.h"

// #include <windows.h>

using namespace std;

MySIFT::MySIFT(int _nLayers, float _contrastThreshold, float _edgeThreshold, float _scaleFactor)
{
    nbLayers = _nLayers;
    contrastThreshold = _contrastThreshold;
    edgeThreshold = _edgeThreshold;
    scaleFactor = _scaleFactor;

    scaleBase = 1.0;
    nbBinsOrientation = 20;
    nbBinsDescriptor = 8;
    descriptorWidth = 4;
}

void MySIFT::setImageBase(const cv::Mat& img)
{
    assert(img.type() == CV_32F);
    base = img.clone();
}

const vector<cv::KeyPoint>& MySIFT::getKeypoints()
{
    return keypoints;
}

const cv::Mat& MySIFT::getDescriptors()
{
    return descriptors;
}

void MySIFT::buildDoGScaleSpace()
{
    assert(base.type() == CV_32F && base.data != nullptr);

    cout << "Build DoG scale-space..." << endl;

    // TODO 1 : build the 'scales', 'pyr' and 'dogpyr' members
    // Add the code HERE

    // Build the scales
    this->scales.push_back(this->scaleBase);
    for(int i = 1; i < this->nbLayers; i++) {
        this->scales.push_back(this->scales[i - 1] * this->scaleFactor);  // s[i] = n * s[i-1]
    }
    // Print scales
    //    for(int i=0; i<this->nbLayers; i++) {
    //        cout<<this->scales[i]<<endl;
    //    }

    // Build pyr
    for(int i = 0; i < nbLayers; i++) {
        cv::Mat img_dst;
        cv::GaussianBlur(base,
                         img_dst,
                         cv::Size(0, 0),
                         scales[i]);  // convolution entre l'image et le filtre Gaussian qui a �tendu = scale[i]
        pyr.push_back(img_dst);
    }

    // Build dogpyr
    for(int i = 0; i < this->nbLayers - 1; i++) {
        this->dogpyr.push_back(this->pyr[i + 1] - this->pyr[i]);
    }

    // Affiche multi �chelle
    stringstream ss;
    CListImage2DWnd* wnd_pyr = new CListImage2DWnd();
    CListImage2DWnd* wnd_dogpyr = new CListImage2DWnd();
    for(unsigned int i = 0; i < this->pyr.size(); i++) {
        ss.str("");
        ss.clear();
        ss << "pyr=" << i;
        wnd_pyr->AddImage(this->pyr[i], ss.str().c_str());
        if(i < this->dogpyr.size()) {
            ss.str("");
            ss.clear();
            ss << "dogpyr=" << i;
            wnd_dogpyr->AddImage(this->dogpyr[i], ss.str().c_str());
        }
    }
    wnd_pyr->show();
    wnd_dogpyr->show();
}

bool MySIFT::adjustLocalExtremum(const cv::Point3i& ptInit, cv::Point3i& ptRelocalized) const
{
    float xi = 0, xr = 0, xc = 0, contr = 0;
    int i;

    int r, c, layer;

    c = ptInit.x;
    r = ptInit.y;
    layer = ptInit.z;

    // width of border in which to ignore keypoints
    static const int SIFT_IMG_BORDER = 5;

    // maximum steps of keypoint interpolation before failure
    static const int SIFT_MAX_INTERP_STEPS = 20;

    for(i = 0; i < SIFT_MAX_INTERP_STEPS; i++) {
        // int idx = layer;
        const cv::Mat& img = dogpyr[layer];
        const cv::Mat& prev = dogpyr[layer - 1];
        const cv::Mat& next = dogpyr[layer + 1];

        cv::Vec3f dD((img.at<float>(r, c + 1) - img.at<float>(r, c - 1)) * 0.5,
                     (img.at<float>(r + 1, c) - img.at<float>(r - 1, c)) * 0.5,
                     (next.at<float>(r, c) - prev.at<float>(r, c)) * 0.5);

        float v2 = (float)img.at<float>(r, c) * 2;
        float dxx = (img.at<float>(r, c + 1) + img.at<float>(r, c - 1) - v2);
        float dyy = (img.at<float>(r + 1, c) + img.at<float>(r - 1, c) - v2);
        float dss = (next.at<float>(r, c) + prev.at<float>(r, c) - v2);
        float dxy = (img.at<float>(r + 1, c + 1) - img.at<float>(r + 1, c - 1) - img.at<float>(r - 1, c + 1) +
                     img.at<float>(r - 1, c - 1)) *
                    0.25;
        float dxs = (next.at<float>(r, c + 1) - next.at<float>(r, c - 1) - prev.at<float>(r, c + 1) +
                     prev.at<float>(r, c - 1)) *
                    0.25;
        float dys = (next.at<float>(r + 1, c) - next.at<float>(r - 1, c) - prev.at<float>(r + 1, c) +
                     prev.at<float>(r - 1, c)) *
                    0.25;

        cv::Matx33f H(dxx, dxy, dxs, dxy, dyy, dys, dxs, dys, dss);

        cv::Vec3f X = H.solve(dD, cv::DECOMP_LU);  // Solves system Hx = dD

        xi = -X[2];
        xr = -X[1];
        xc = -X[0];

        if(std::abs(xi) < 0.5f && std::abs(xr) < 0.5f && std::abs(xc) < 0.5f) break;

        if(std::abs(xi) > (float)(INT_MAX / 3) || std::abs(xr) > (float)(INT_MAX / 3) ||
           std::abs(xc) > (float)(INT_MAX / 3)) {
            return false;
        }

        // Rounds to the nearest integer
        c += cvRound(xc);
        r += cvRound(xr);
        layer += cvRound(xi);

        if(layer < 1 || layer > nbLayers - 3 || c < SIFT_IMG_BORDER || c >= img.cols - SIFT_IMG_BORDER ||
           r < SIFT_IMG_BORDER || r >= img.rows - SIFT_IMG_BORDER) {
            return false;
        }
    }

    // ensure convergence of interpolation
    if(i >= SIFT_MAX_INTERP_STEPS) {
        return false;
    }

    const cv::Mat& img = dogpyr[layer];
    const cv::Mat& prev = dogpyr[layer - 1];
    const cv::Mat& next = dogpyr[layer + 1];

    cv::Matx31f dD((img.at<float>(r, c + 1) - img.at<float>(r, c - 1)) * 0.5,
                   (img.at<float>(r + 1, c) - img.at<float>(r - 1, c)) * 0.5,
                   (next.at<float>(r, c) - prev.at<float>(r, c)) * 0.5);

    float t = dD.dot(cv::Matx31f(xc, xr, xi));

    contr = img.at<float>(r, c) + t * 0.5f;
    if(std::abs(contr) < contrastThreshold) return false;

    // Principal curvatures are computed using the trace and det of Hessian
    float v2 = img.at<float>(r, c) * 2.f;
    float dxx = (img.at<float>(r, c + 1) + img.at<float>(r, c - 1) - v2);
    float dyy = (img.at<float>(r + 1, c) + img.at<float>(r - 1, c) - v2);
    float dxy = (img.at<float>(r + 1, c + 1) - img.at<float>(r + 1, c - 1) - img.at<float>(r - 1, c + 1) +
                 img.at<float>(r - 1, c - 1));

    float tr = dxx + dyy;
    float det = dxx * dyy - dxy * dxy;

    if(det <= 0 || tr * tr * edgeThreshold >= (edgeThreshold + 1) * (edgeThreshold + 1) * det) return false;

    ptRelocalized.x = c;
    ptRelocalized.y = r;
    ptRelocalized.z = layer;

    return true;
}

// Detects features at extrema in DoG scale space.  Bad features are discarded
// based on contrast and ratio of principal curvatures.
void MySIFT::findScaleSpaceExtrema()
{
    vector<cv::Point3i> localExtrema;
    cv::KeyPoint kpt;

    localExtrema.clear();
    keypoints.clear();

    if(pyr.size() < 3) {
        cout << "ERROR: there should be at least 3 layers." << endl;
        return;
    }

    if(dogpyr.size() != pyr.size() - 1) {
        cout << "ERROR: number of layers in the DoG sequence does not match the multiscale sequence." << endl;
        return;
    }

    cout << "Search for local extrema..." << endl;

    // TODO 2: find local extrema in the DoG scale-space and add them into the 'localExtrema' vector
    // Local extrema will be stored as 3D points (the z component should be used for the scale index)
    // Add the code HERE
    float check_point;      // valeur du point au milieu
    float neightbor_point;  // valeur des voisinages
    cv::Mat img = base.clone();
    for(unsigned int i = 1; i < dogpyr.size() - 1;
        i++) {  // on ne cherche pas les points clefs sur le 1er image et le dernier image
        for(int x = 1; x < dogpyr[i].cols - 1; x++) {  // on ne cherche pas les points clefs sur le contour
            for(int y = 1; y < dogpyr[i].rows - 1; y++) {
                // value of check point
                check_point = dogpyr[i].at<float>(y, x);
                if(check_point > contrastThreshold) {
                    // Consider point(x,y) of image dogpyr[i]
                    int maxima = 0;  // nombre des points qui sont sup�rieur au checkpoint
                    int minima = 0;  // nombre des points qui sont inf�rieur au checkpoint
                    for(int img_index = -1; img_index <= 1;
                        img_index++) {  // comparer le check point avec les voisinages
                        for(int x_index = -1; x_index <= 1; x_index++) {
                            for(int y_index = -1; y_index <= 1; y_index++) {
                                // value of the neighbor point
                                neightbor_point = dogpyr[i + img_index].at<float>(y + y_index, x + x_index);
                                if(neightbor_point >= check_point) {
                                    maxima++;
                                }
                                if(neightbor_point <= check_point) {
                                    minima++;
                                }
                            }
                        }
                    }
                    if((maxima == 1) ||
                       (minima == 1)) {  // maxima == 1 car on comparer le chekcpoint avec lui-meme une fois
                        cv::Point3i _localExtrema;
                        _localExtrema.x = x;
                        _localExtrema.y = y;
                        _localExtrema.z = i;
                        localExtrema.push_back(_localExtrema);

                        cv::Point2i pt;  // cr�er un point pour dessiner un cross sur l'image
                        pt.x = x;
                        pt.y = y;
                        drawCross(img, pt);  // dessiner le local extrema sur l'image
                    }
                }
            }
        }
    }

    CListImage2DWnd* wnd = new CListImage2DWnd();
    wnd->AddImage(img, "Locaux Extrema");
    wnd->show();

    cout << localExtrema.size() << " local extrema found." << endl;

    // Adjust local extrema and remove non-well defined ones
    cv::Point3i ptRelocalized;
    for(const cv::Point3i& ptInit : localExtrema) {
        if(adjustLocalExtremum(ptInit, ptRelocalized)) {
            // The local extremum has passed the test, so let's create a new keypoint
            kpt.pt.x = ptRelocalized.x;
            kpt.pt.y = ptRelocalized.y;
            kpt.octave = ptRelocalized.z;
            kpt.size = scales[kpt.octave];
            kpt.angle = 0;  // For now...

            setDominantOrientation(kpt);

            keypoints.push_back(kpt);
        }
    }

    cout << keypoints.size() << " keypoint(s) found" << endl;
}

void MySIFT::setDominantOrientation(cv::KeyPoint& kpt) const
{
    int i, radius, x, y, nx, ny;
    float hist[nbBinsOrientation];
    const cv::Mat& img = pyr[kpt.octave];

    cv::Mat g;

    setGaussian(g, 1.5 * kpt.size);

    radius = g.rows / 2;  // calcul de la taille de la r�gion de collection d'orientation

    for(i = 0; i < nbBinsOrientation; i++) hist[i] = 0.0;

    cv::Point2f grad;

    for(y = -radius; y <= radius; y++) {
        ny = (int)kpt.pt.y + y;  // point voisinage � coordinate ny
        if(ny >= 1 && ny < img.rows - 1) {
            for(x = -radius; x <= radius; x++) {
                nx = (int)kpt.pt.x + x;  // point voisinage � coordinate nx
                if(nx >= 1 && nx < img.cols - 1) {
                    // TODO 3 : compute the gradient at (nx,ny) using centered finite difference
                    // Get its magnitude (norm) and orientation (angle), and update the right bin
                    // in histogram 'hist'
                    // The magnitude should be weighted by the Gaussian value g[x,y] (just as in a convolution)
                    // Tip: use std::atan2 to get the angle

                    // Add code here
                    float angle = 0;
                    float norm = 0;
                    float y_cor = 0;    // la d�riv� en axe y
                    float x_cor = 0;    // la d�riv� en axe x
                    int index_bin = 0;  // l'indice du bin � laquelle appartient le point (nx,ny)

                    y_cor = (img.at<float>(ny + 1, nx) - img.at<float>(ny - 1, nx)) / 2;  // la d�riv� en axe y
                    x_cor = (img.at<float>(ny, nx + 1) - img.at<float>(ny, nx - 1)) / 2;  // la d�riv� en axe x

                    // Calcul d'orientation
                    angle = std::atan2(y_cor, x_cor);
                    angle = angle * 180 / M_PI + 180;  // passer de radian � degree

                    // Calcul de magnitude
                    norm = std::sqrt(x_cor * x_cor + y_cor * y_cor);
                    norm *= g.at<float>(y + radius, x + radius);  // weight the magnitude by Gaussian value g[x,y]

                    index_bin =
                        (int)angle /
                        (360 / nbBinsOrientation);  // calcul de l'indice du bin � laquelle appartient le point (nx,ny)

                    hist[index_bin] += norm;  // mis � jour la valeur de l'histogram
                }
            }
        }
    }

    // TODO 4 : get the bin index with maximal value
    // and assign the corresponding angle to kpt.angle

    // Add code here
    int index_bin_max = 0;                    // choisir le 1er bin comme la valeur maximale
    for(i = 0; i < nbBinsOrientation; i++) {  // incr�menter � tous les bins
        if(hist[i] > hist[index_bin_max]) {   // si la valeur d'un bin est sup�rieur � la valeur maximale
            index_bin_max = i;                // on prends cet indice
        }
    }
    //    cout<<"Index Max : "<<index_bin_max<<"   "<<"Norm : "<<hist[index_bin_max]<<endl;
    kpt.angle = (index_bin_max)*2 * M_PI / nbBinsOrientation;  // passer l'angle dominant en degr�e
}

void MySIFT::computeDescriptor(const cv::KeyPoint& kpt, float* dst)
{
    cv::Point pt(cvRound(kpt.pt.x), cvRound(kpt.pt.y));
    float cos_t = std::cos(kpt.angle);
    float sin_t = std::sin(kpt.angle);
    float bins_per_rad = nbBinsDescriptor / (2.0 * M_PI);
    float exp_scale = -1.f / (descriptorWidth * descriptorWidth * 0.5f);
    float hist_width = 3.0 * kpt.size;
    int radius = cvRound(hist_width * std::sqrt(2.0) * /*1.4142135623730951f * */ (descriptorWidth + 1) * 0.5f);

    // threshold on magnitude of elements of descriptor vector
    static const float SIFT_DESCR_MAG_THR = 0.2;

    const cv::Mat& img = pyr[kpt.octave];

    // Clip the radius to the diagonal of the image to avoid autobuffer too large exception
    radius = std::min(radius, (int)std::sqrt(img.cols * img.cols + img.rows * img.rows));
    // cos_t /= hist_width;
    // sin_t /= hist_width;

    int i, j, k, len = (radius * 2 + 1) * (radius * 2 + 1),
                 histlen = (descriptorWidth + 2) * (descriptorWidth + 2) * (nbBinsDescriptor + 2);

    cv::AutoBuffer<float> buf(len * 5 + histlen);
    float *Mag = buf, *Ori = Mag + len, *W = Ori + len;
    float *RBin = W + len, *CBin = RBin + len, *hist = CBin + len;

    for(i = 0; i < descriptorWidth + 2; i++) {
        for(j = 0; j < descriptorWidth + 2; j++)
            for(k = 0; k < nbBinsDescriptor + 2; k++)
                hist[(i * (descriptorWidth + 2) + j) * (nbBinsDescriptor + 2) + k] = 0.;
    }

    cv::Point2f grad;
    for(i = -radius, k = 0; i <= radius; i++)
        for(j = -radius; j <= radius; j++) {
            // Calculate sample's histogram array coords rotated relative to ori.
            // Subtract 0.5 so samples that fall e.g. in the center of row 1 (i.e.
            // r_rot = 1.5) have full weight placed in row 1 after interpolation.
            float c_rot = (j * cos_t - i * sin_t) / hist_width;
            float r_rot = (j * sin_t + i * cos_t) / hist_width;
            float rbin = r_rot + descriptorWidth / 2 - 0.5f;
            float cbin = c_rot + descriptorWidth / 2 - 0.5f;
            int r = pt.y + i, c = pt.x + j;

            if(rbin > -1 && rbin < descriptorWidth && cbin > -1 && cbin < descriptorWidth && r > 0 &&
               r < img.rows - 1 && c > 0 && c < img.cols - 1) {
                grad.x = (img.at<float>(r, c + 1) - img.at<float>(r, c - 1)) * 0.5;
                grad.y = (img.at<float>(r + 1, c) - img.at<float>(r - 1, c)) * 0.5;

                RBin[k] = rbin;
                CBin[k] = cbin;

                // W[k] = (c_rot * c_rot + r_rot * r_rot)*exp_scale;
                W[k] = std::exp((i * i + j * j) * exp_scale / (hist_width * hist_width));

                Mag[k] = cv::norm(grad);
                Ori[k] = std::atan2(grad.y, grad.x);
                if(Ori[k] < 0) Ori[k] += 2.0 * M_PI;

                k++;
            }
        }

    len = k;

    // Trilinear interpolation
    for(k = 0; k < len; k++) {
        Ori[k] -= kpt.angle;
        if(Ori[k] < 0) Ori[k] += 2.0 * M_PI;

        float rbin = RBin[k], cbin = CBin[k];
        float obin = Ori[k] * bins_per_rad;
        float mag = Mag[k] * W[k];

        int r0 = cvFloor(rbin);
        int c0 = cvFloor(cbin);
        int o0 = cvFloor(obin);

        rbin -= r0;
        cbin -= c0;
        obin -= o0;

        // histogram update using tri-linear interpolation
        float v_r1 = mag * rbin, v_r0 = mag - v_r1;
        float v_rc11 = v_r1 * cbin, v_rc10 = v_r1 - v_rc11;
        float v_rc01 = v_r0 * cbin, v_rc00 = v_r0 - v_rc01;
        float v_rco111 = v_rc11 * obin, v_rco110 = v_rc11 - v_rco111;
        float v_rco101 = v_rc10 * obin, v_rco100 = v_rc10 - v_rco101;
        float v_rco011 = v_rc01 * obin, v_rco010 = v_rc01 - v_rco011;
        float v_rco001 = v_rc00 * obin, v_rco000 = v_rc00 - v_rco001;

        int idx = ((r0 + 1) * (descriptorWidth + 2) + c0 + 1) * (nbBinsDescriptor + 2) + o0;
        hist[idx] += v_rco000;
        hist[idx + 1] += v_rco001;
        hist[idx + (nbBinsDescriptor + 2)] += v_rco010;
        hist[idx + (nbBinsDescriptor + 3)] += v_rco011;
        hist[idx + (descriptorWidth + 2) * (nbBinsDescriptor + 2)] += v_rco100;
        hist[idx + (descriptorWidth + 2) * (nbBinsDescriptor + 2) + 1] += v_rco101;
        hist[idx + (descriptorWidth + 3) * (nbBinsDescriptor + 2)] += v_rco110;
        hist[idx + (descriptorWidth + 3) * (nbBinsDescriptor + 2) + 1] += v_rco111;
    }

    // finalize histogram, since the orientation histograms are circular
    for(i = 0; i < descriptorWidth; i++)
        for(j = 0; j < descriptorWidth; j++) {
            int idx = ((i + 1) * (descriptorWidth + 2) + (j + 1)) * (nbBinsDescriptor + 2);
            hist[idx] += hist[idx + nbBinsDescriptor];
            hist[idx + 1] += hist[idx + nbBinsDescriptor + 1];
            for(k = 0; k < nbBinsDescriptor; k++) dst[(i * descriptorWidth + j) * nbBinsDescriptor + k] = hist[idx + k];
        }

    // copy histogram to the descriptor,
    // apply hysteresis thresholding
    float nrm2 = 0;
    len = descriptorWidth * descriptorWidth * nbBinsDescriptor;

    for(k = 0; k < len; k++) nrm2 += dst[k] * dst[k];

    float thr = std::sqrt(nrm2) * SIFT_DESCR_MAG_THR;

    nrm2 = 0;
    for(i = 0; i < len; i++) {
        float val = std::min(dst[i], thr);
        dst[i] = val;
        nrm2 += val * val;
    }

    nrm2 = std::max(std::sqrt(nrm2), FLT_EPSILON);

    for(k = 0; k < len; k++) dst[k] /= nrm2;
}

void MySIFT::computeSIFTPoints()
{
    assert(base.data != nullptr);

    buildDoGScaleSpace();

    findScaleSpaceExtrema();

    cv::KeyPointsFilter::removeDuplicatedSorted(keypoints);

    int dsize = descriptorWidth * descriptorWidth * nbBinsDescriptor;

    descriptors.create((int)keypoints.size(), dsize, CV_32F);

    int i = 0;
    for(const cv::KeyPoint& kpt : keypoints) {
        computeDescriptor(kpt, (float*)descriptors.ptr(i));
        i++;
    }
}
