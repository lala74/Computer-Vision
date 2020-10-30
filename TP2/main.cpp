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

float zone(float noeud, float seuil) {
// fonction trouver les �tiquettes attribu�s aux pixels
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
    int numNodes;           // nombre des noeuds dans notre graphe (sauf noeud source et noeud puits)
    int cols, rows;         // nombre de colonne et rang�e d'image
    int c,r;                // indices pour colonne et rang�e
    int index;              // indice d'un noeud ~ d'un pixel de l'image dans la matrice 1D
    float value;            // valeur de pixel

    float seuil = 0.38;     // seuil pour les �tiquettes
    float alpha = 2.5;

    cv::Mat imgReal;
    img.convertTo(imgReal, CV_32F, 1.0/255.0);      // changer valeur des pixels � valeur r�elle entre 0 et 1

    cols = imgReal.cols;    // trouver le nombre de colonne d'image
    rows = imgReal.rows;    // trouver le nombre de rang�e d'image
    numNodes = cols * rows; // trouver le nombre des noeuds

    gf.setNbNodes(numNodes);

    cout<<"Build flow graph"<<endl;

    for(r=0; r<rows; r++) {
        for(c=0; c<cols; c++) {
            // Calculer U pour chaque pixel ~ la capacit� de l'arc entre le noeud source et le pixel

//            cout<<"r: "<<r<<" c: "<<c<<endl;
//            cout<<imgReal.at<float>(r,c)<<" ";

            index = r*cols + c;                         // indice d'un pixel de l'image dans la matrice 1D
            value = imgReal.at<float>(r,c);             // valeur de ce pixel

            gf.connectSourceToNode(index, value);       // Classe 0
            gf.connectNodeToSink(index, 1 - value);     // Classe 1

            // Calculer V pour chaque pixel
            if (r == 0) {
            // si on est � la premi�re rang�e
                if (zone(value, seuil) != zone(imgReal.at<float>(r+1,c), seuil)) {
                // V = alpha*(wi - wj) donc si deux pixel sont dans le m�me zone, on ne r�alise pas un arc
                    gf.connectNodes(index, index + cols, alpha);    // on r�alise un arc avec les pixels voisins en bas
                }
            } else if (r == rows-1) {
            // si on est � la derni�re rang�e
                if (zone(value, seuil) != zone(imgReal.at<float>(r-1,c), seuil)) {
                    gf.connectNodes(index, index - cols, alpha);    // on r�alise un arc avec les pixels voisins en haut
                }
            } else {
            // dans l'autre cas
                // on r�alise un arc avec les pixels voisins en haut et en bas
                if (zone(value, seuil) != zone(imgReal.at<float>(r+1,c), seuil)) {
                    gf.connectNodes(index, index + cols, alpha);
                }
                if (zone(value, seuil) != zone(imgReal.at<float>(r-1,c), seuil)) {
                    gf.connectNodes(index, index - cols, alpha);
                }
            }

            if (c == 0) {
            // si on est � la premi�re colonne
                if (zone(value, seuil) != zone(imgReal.at<float>(r,c+1), seuil)) {
                    gf.connectNodes(index, index + 1, alpha);       // on r�alise un arc avec les pixels voisins � droit
                }
            } else if (c == cols-1) {
            // si on est � la derni�re colonne
                if (zone(value, seuil) != zone(imgReal.at<float>(r,c-1), seuil)) {
                    gf.connectNodes(index, index - 1, alpha);       // on r�alise un arc avec les pixels voisins � gauche
                }
            } else {
            // dans l'autre cas
                // on r�alise un arc avec les pixels voisins � droit et � gauche
                if (zone(value, seuil) != zone(imgReal.at<float>(r,c+1), seuil)) {
                    gf.connectNodes(index, index + 1, alpha);
                }
                if (zone(value, seuil) != zone(imgReal.at<float>(r,c-1), seuil)) {
                    gf.connectNodes(index, index - 1, alpha);
                }
            }
//            cout<<"row = "<<r<<" col : "<<c<<endl;
        }
    }


    // Compute maximum flow
    cout<<"Ford Fulkerson flow graph"<<endl;
    gf.FordFulkerson();
    cout<<"Finish Ford Fulkerson flow graph"<<endl;

    // Compute the s/t cut given the maximum flow
    vector<unsigned int> S;
    gf.cutFromSource(S);

    seg.create(img.size(), CV_8U);
    seg.setTo(0);

    // TODO 3 : create segmentation image from the S set
    // Add code here
    // ...

    for (int i = 0; i < S.size(); i++) {
        // calculer l'indice de colonne et rang�e � partir de l'indice du matrice 1D
        c = S.at(i) % cols;
        r = S.at(i) / cols;
//        cout << S.at(i) <<" c: "<<c<<" r: "<<r<<endl;
        seg.at<uchar>(r,c) = 230;       // set value for pixel in S net ~ display the result
    }

    CImage2DWnd::Show(img, "Image Origin");
    CImage2DWnd::Show(seg, "Seg");

    return app.exec();
}
