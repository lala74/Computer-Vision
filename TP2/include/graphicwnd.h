/*
Copyright (c) 2017 Julien Mille
INSA Centre Val de Loire
Laboratoire d'Informatique Fondamentale et Appliquée de Tours
*/

#ifndef GRAPHICWND_H
#define GRAPHICWND_H

#include <opencv2/core.hpp>

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

#include <sstream>
#include <iostream>
#include <vector>

#include "dialogparams.h"

using namespace std;

class CImage2DWnd;

class CGraphicWndFrame : public QFrame
{
    friend class CImage2DWnd;

  protected:
    CImage2DWnd *pParentWnd;

  public:
    CGraphicWndFrame(QWidget *pParent=0):QFrame(pParent)
    {
        pParentWnd = NULL;
    }

  protected:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void paintEvent(QPaintEvent *);
};

class CImage2DWnd : public QMainWindow
{
    friend class CGraphicWndFrame;

  // Member variables
  protected:
    CGraphicWndFrame *pFrame;
    QScrollArea *scrollArea;
    double scaleFactor;

    cv::Mat imgInput;
    QImage qimage, qimageZoom;

    cv::Point piMouseLastPress; // Position at which the mouse was last pressed
    cv::Point piMouseCurrent;
    bool bLeftButtonPressed;
    bool bRightButtonPressed;

    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
    QMenu *viewMenu;

    // Look-up table between values and colors
	// Colors are in BGR format
	vector<pair<float, cv::Vec3f> > listValuesColors;

    // For vector-valued arrays (in BGR format)
    cv::Vec3f fMin, fMax;

  // Member functions
  public:

    // Create a new window to show a given image
    static CImage2DWnd *Show(const cv::Mat &, const char *strWindowName="Image");

  public:
    CImage2DWnd(int iWidth=200, int iHeight=200, const char *strWindowName="Image");
    ~CImage2DWnd();

    virtual void SetImage(const cv::Mat &);

    void ClearValuesColorList() {listValuesColors.clear();}
    void AddValueColorList(float fValue, unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue)
    {
        pair<float, cv::Vec3f> valueAndColor;

        valueAndColor.first = fValue;
        valueAndColor.second = cv::Vec3f((float)ucRed, (float)ucGreen, (float)ucBlue);

        listValuesColors.push_back(valueAndColor);
    }
    cv::Vec3b GetColorFromLUT(float) const;
    void SetMin(float blue, float green, float red) {fMin = cv::Vec3f(blue,green,red);}
    void SetMax(float blue, float green, float red) {fMax = cv::Vec3f(blue,green,red);}

 protected:
    virtual void handleMousePress(int, int, Qt::MouseButton);
    virtual void handleMouseRelease(int, int, Qt::MouseButton);
    virtual void handleMouseMove(int, int);

    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();

    void createActions();
    void createMenus();
    void updateActions();

    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    virtual void keyPressEvent(QKeyEvent *);
    virtual void MakeQImage();
    virtual void SaveQImage(const char *) const;

    virtual void scaleImage(double factor);

    virtual string GetStatusBarString() const;
    virtual void UpdateTextStatusBar();
};

// Display a sequence of images
// Walk along the sequence by pressing keys A and Z
class CListImage2DWnd : public CImage2DWnd
{
  // Member variables
  protected:
    // Sequence of images
    // Each image has a name, which will be displayed in the window status bar
    vector<pair<cv::Mat, string>> vectImg;

    // Index of image to be displayed
    unsigned int idxCurrent;

  // Member functions
  public:
    // Constructor
    CListImage2DWnd(int iWidth=200, int iHeight=200, const char *strWindowName="Image"):CImage2DWnd(iWidth, iHeight, strWindowName)
    {
        idxCurrent = 0;
    }

    // Add an image at the end of the sequence
    // Params: [in] image, [in] image name
    void AddImage(const cv::Mat &, const char *);

    // Empty the sequence of images
    void ClearImages();

    // Return size of sequence
    virtual unsigned int GetNbImages() const;

    // Set the index of the image to displayed
    virtual void SetCurrentImage(unsigned int);

    // The SetImage member function inherited from CImage2DWnd is disabled
    virtual void SetImage(const cv::Mat &) {}

  protected:
    // Make string with current image name and status string
    virtual string GetStatusBarString() const;

    // Handle keyboard
    // If A is pressed, current image index is decreased
    // If Z is pressed, current image index is increased
    // Otherwise, keyboard handling is passed to base class CImage2DWnd
    virtual void keyPressEvent(QKeyEvent *);
};

#endif
