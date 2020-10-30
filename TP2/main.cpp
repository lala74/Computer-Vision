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

    // Compute maximum flow
    gf.FordFulkerson();

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
