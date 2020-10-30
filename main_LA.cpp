#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#include "graph.h"
#include "graphicwnd.h"
#include "opencv_utility.h"

using namespace std;

#ifdef WIN32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

float segmentation(float noeud, float seuil) {
    if (noeud >= seuil) {
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    cv::Mat img, seg;

    img = cv::imread("./images/A_40x40_noise.png", cv::IMREAD_GRAYSCALE);
    if (img.data==nullptr)
    {
        cout<<"Failed to open image"<<endl;
        return -1;
    }

    GraphFlow gf;
    // TODO 1 : create a flow graph for binary segmentation of img
    // Since we deal with a binary segmentation on a grayscale image,
    // Unary data terms can be set to U_i(0) = f_i and U_i(1)=1-f_i
    // so that class 0 gathers dark pixels and class 1 gathers bright pixels

    // Add code here
    // ...
    int numNodes;
    int cols, rows;
    int c,r;
    int index;
    float value;

    float seuil = 0.5;
    float alpha = 2.5;

    cols = img.cols;
    rows = img.rows;
    numNodes = cols * rows;

    gf.setNbNodes(numNodes);
    for(r=0; r<rows; r++) {
        for(c=0; c<cols; c++) {
            // Calculer U pour chaque pixel
//            cout<<"r: "<<r<<" c: "<<c<<endl;
            index = r*cols + c;
            value = img.at<float>(r,c);
            // Class 0
            gf.connectSourceToNode(index, value);
            // Class 1
            gf.connectNodeToSink(index, 1 - value);

            // Calculer V pour chaque pixel
            if (r == 0) {
                if (segmentation(value, seuil) != segmentation(img.at<float>(r+1,c), seuil)) {
                    gf.connectNodes(index, index + cols, alpha);
                }
            } else if (r == rows-1) {
                if (segmentation(value, seuil) != segmentation(img.at<float>(r-1,c), seuil)) {
                    gf.connectNodes(index, index - cols, alpha);
                }
            } else {
                if (segmentation(value, seuil) != segmentation(img.at<float>(r+1,c), seuil)) {
                    gf.connectNodes(index, index + cols, alpha);
                }
                if (segmentation(value, seuil) != segmentation(img.at<float>(r-1,c), seuil)) {
                    gf.connectNodes(index, index - cols, alpha);
                }
            }

            if (c == 0) {
                if (segmentation(value, seuil) != segmentation(img.at<float>(r,c+1), seuil)) {
                    gf.connectNodes(index, index + 1, alpha);
                }
            } else if (c == cols-1) {
                if (segmentation(value, seuil) != segmentation(img.at<float>(r,c-1), seuil)) {
                    gf.connectNodes(index, index - 1, alpha);
                }
            } else {
                if (segmentation(value, seuil) != segmentation(img.at<float>(r,c+1), seuil)) {
                    gf.connectNodes(index, index + 1, alpha);
                }
                if (segmentation(value, seuil) != segmentation(img.at<float>(r,c-1), seuil)) {
                    gf.connectNodes(index, index - 1, alpha);
                }
            }
            cout<<"row = "<<r<<" col : "<<c<<endl;
        }
    }


    // Compute maximum flow
    cout<<"Hello1"<<endl;
    gf.FordFulkerson();
    cout<<"Hello"<<endl;

    // Compute the s/t cut given the maximum flow
    vector<unsigned int> S;
    gf.cutFromSource(S);

    seg.create(img.size(), CV_8U);
    seg.setTo(0);

    // TODO 3 : create segmentation image from the S set
    // Add code here
    // ...

    CImage2DWnd::Show(seg, "Seg");

    return app.exec();
}
