/*
Copyright (c) 2017 Julien Mille
INSA Centre Val de Loire
Laboratoire d'Informatique Fondamentale et Appliquée de Tours
*/

#include "graphicwnd.h"

using namespace std;

/******************************************************************************
                             CGraphicWndFrame
******************************************************************************/

void CGraphicWndFrame::mousePressEvent(QMouseEvent* event)
{
    if(pParentWnd != NULL) pParentWnd->handleMousePress(event->x(), event->y(), event->button());
}

void CGraphicWndFrame::mouseReleaseEvent(QMouseEvent* event)
{
    if(pParentWnd != NULL) pParentWnd->handleMouseRelease(event->x(), event->y(), event->button());
}

void CGraphicWndFrame::mouseMoveEvent(QMouseEvent* event)
{
    if(pParentWnd != NULL) pParentWnd->handleMouseMove(event->x(), event->y());
}

void CGraphicWndFrame::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    if(pParentWnd != NULL) painter.drawImage(0, 0, pParentWnd->qimageZoom);
}

/******************************************************************************
                                 CImage2DWnd
******************************************************************************/

CImage2DWnd* CImage2DWnd::Show(const cv::Mat& imgNew, const char* strWindowName)
{
    CImage2DWnd* pWndNew;

    pWndNew = new CImage2DWnd(imgNew.cols, imgNew.rows, strWindowName);
    if(pWndNew != NULL) {
        pWndNew->SetImage(imgNew);
        pWndNew->show();
    }

    return pWndNew;
}

CImage2DWnd::CImage2DWnd(int iWidth, int iHeight, const char* strWindowName)
{
    pFrame = new CGraphicWndFrame;
    pFrame->setBackgroundRole(QPalette::Base);
    pFrame->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    pFrame->pParentWnd = this;

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(pFrame);
    setCentralWidget(scrollArea);

    createActions();
    createMenus();

    setWindowTitle(strWindowName);
    resize(iWidth, iHeight);

    scaleFactor = 1.0;
    bLeftButtonPressed = false;
    bRightButtonPressed = false;

    pair<float, cv::Vec3f> valueAndColor;

    listValuesColors.reserve(20);

    valueAndColor.first = 0.0f;
    valueAndColor.second = cv::Vec3f(0.0f, 0.0f, 0.0f);
    listValuesColors.push_back(valueAndColor);

    valueAndColor.first = 1.0f;
    valueAndColor.second = cv::Vec3f(255.0f, 255.0f, 255.0f);
    listValuesColors.push_back(valueAndColor);

    fMin = cv::Vec3f(-127.0, -127.0, -127.0);
    fMax = cv::Vec3f(127.0, 127.0, 127.0);
}

CImage2DWnd::~CImage2DWnd() {}

void CImage2DWnd::handleMousePress(int x, int y, Qt::MouseButton button)
{
    piMouseLastPress.x = x;
    piMouseLastPress.y = y;

    piMouseCurrent.x = x;
    piMouseCurrent.y = y;

    if(button == Qt::LeftButton)
        bLeftButtonPressed = true;
    else if(button == Qt::RightButton)
        bRightButtonPressed = true;

    UpdateTextStatusBar();
}

void CImage2DWnd::handleMouseRelease(int x, int y, Qt::MouseButton button)
{
    if(button == Qt::LeftButton)
        bLeftButtonPressed = false;
    else if(button == Qt::RightButton)
        bRightButtonPressed = false;

    piMouseCurrent.x = x;
    piMouseCurrent.y = y;

    UpdateTextStatusBar();
}

void CImage2DWnd::handleMouseMove(int x, int y)
{
    piMouseCurrent.x = x;
    piMouseCurrent.y = y;

    UpdateTextStatusBar();
}

string CImage2DWnd::GetStatusBarString() const
{
    cv::Point piCenter;
    cv::Vec3b vb;
    cv::Vec3i vi;
    cv::Vec3f vf;
    stringstream streamStatus;

    piCenter.x = (int)((float)piMouseCurrent.x / scaleFactor);
    piCenter.y = (int)((float)piMouseCurrent.y / scaleFactor);

    streamStatus << "Size=" << imgInput.size() << " (x,y)=" << piCenter;

    cv::Rect r = cv::Rect(cv::Point(0, 0), cv::Point(imgInput.cols - 1, imgInput.rows - 1));
    if(piCenter.inside(r)) {
        switch(imgInput.type()) {
            // Scalar types
            case CV_8U:
                streamStatus << "  Value=" << (int)imgInput.at<unsigned char>(piCenter) << " (uchar)";
                break;
            case CV_32S:
                streamStatus << "  Value=" << imgInput.at<int>(piCenter) << " (int)";
                break;
            case CV_32F:
                streamStatus << "  Value=" << imgInput.at<float>(piCenter) << " (float)";
                break;

            // Vector types
            case CV_8UC3:
                vb = imgInput.at<cv::Vec3b>(piCenter);
                streamStatus << "  [B,G,R]=" << vb << " (3 x uchar)";
                break;
            case CV_32SC3:
                vi = imgInput.at<cv::Vec3i>(piCenter);
                streamStatus << "  [B,G,R]=(" << vi << ") (3 x int)";
                break;
            case CV_32FC3:
                vf = imgInput.at<cv::Vec3f>(piCenter);
                streamStatus << "  [B,G,R]=" << vf << " (3 x float)";
                break;
            default:
                streamStatus << "  Value=Unsupported type";
        }
    }

    return streamStatus.str();
}

void CImage2DWnd::UpdateTextStatusBar()
{
    statusBar()->showMessage(GetStatusBarString().c_str());
}

void CImage2DWnd::SetImage(const cv::Mat& imgNew)
{
    imgInput = imgNew.clone();

    if(imgNew.type() == CV_32FC3) {
        float m = numeric_limits<float>::max();

        fMin = cv::Vec3f(m, m, m);
        fMax = cv::Vec3f(-m, -m, -m);

        for(auto it = imgNew.begin<cv::Vec3f>(); it != imgNew.end<cv::Vec3f>(); ++it) {
            for(int i = 0; i < 3; i++) {
                fMin.val[i] = min(fMin.val[i], (*it).val[i]);
                fMax.val[i] = max(fMax.val[i], (*it).val[i]);
            }
        }
    }

    MakeQImage();

    scaleImage(1.0);

    fitToWindowAct->setEnabled(true);
    updateActions();

    if(!fitToWindowAct->isChecked()) pFrame->adjustSize();
}

void CImage2DWnd::zoomIn()
{
    scaleImage(2.0);
}

void CImage2DWnd::zoomOut()
{
    scaleImage(0.5);
}

void CImage2DWnd::normalSize()
{
    scaleFactor = 1.0;
    scaleImage(1.0);
}

void CImage2DWnd::fitToWindow()
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if(!fitToWindow) normalSize();
    updateActions();
}

void CImage2DWnd::createActions()
{
    zoomInAct = new QAction(tr("Zoom &In (25%)"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setEnabled(false);
    connect(zoomInAct, &QAction::triggered, this, &CImage2DWnd::zoomIn);

    zoomOutAct = new QAction(tr("Zoom &Out (25%)"), this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setEnabled(false);
    connect(zoomOutAct, &QAction::triggered, this, &CImage2DWnd::zoomOut);

    normalSizeAct = new QAction(tr("&Normal Size"), this);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);
    connect(normalSizeAct, &QAction::triggered, this, &CImage2DWnd::normalSize);

    fitToWindowAct = new QAction(tr("&Fit to Window"), this);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));
    connect(fitToWindowAct, &QAction::triggered, this, &CImage2DWnd::fitToWindow);
}

void CImage2DWnd::createMenus()
{
    viewMenu = new QMenu(tr("&View"), this);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(normalSizeAct);
    viewMenu->addSeparator();
    viewMenu->addAction(fitToWindowAct);

    menuBar()->addMenu(viewMenu);
}

void CImage2DWnd::updateActions()
{
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void CImage2DWnd::scaleImage(double factor)
{
    QSize newSize;

    scaleFactor *= factor;

    newSize.setWidth((int)((double)imgInput.cols * scaleFactor));
    newSize.setHeight((int)((double)imgInput.rows * scaleFactor));

    qimageZoom = qimage.scaled(newSize);

    pFrame->resize(newSize);
    resize(min(800, newSize.width() + 20), min(600, newSize.height() + 30));
    pFrame->update();

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 32.0);
    zoomOutAct->setEnabled(scaleFactor > 0.1);
}

void CImage2DWnd::adjustScrollBar(QScrollBar* scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value() + ((factor - 1) * scrollBar->pageStep() / 2)));
}

void CImage2DWnd::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Space) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File (PNG)"), QDir::currentPath());
        if(!fileName.isEmpty()) SaveQImage(fileName.toLocal8Bit().constData());
    } else if(event->key() == Qt::Key_O) {
        CDialogParams dlg;

        if(imgInput.type() == CV_32S || imgInput.type() == CV_32F) {
            stringstream ss;
            unsigned int iValue;

            for(iValue = 0; iValue < listValuesColors.size(); iValue++) {
                ss.str(string());
                ss.clear();
                ss << "Value " << iValue;
                dlg.AddRealParam(listValuesColors[iValue].first, ss.str());

                ss.str(string());
                ss.clear();
                ss << "Color " << iValue;
                dlg.AddPoint3iParam(cv::Point3i(listValuesColors[iValue].second), ss.str());
            }

            dlg.AddIntParam(listValuesColors.size(), "Nb colors");
        } else if(imgInput.type() == CV_32SC3 || imgInput.type() == CV_32FC3) {
            dlg.AddPoint3fParam(cv::Point3f(fMin), "Min");
            dlg.AddPoint3fParam(cv::Point3f(fMax), "Max");
        }

        dlg.MakeWidgets();
        dlg.setModal(true);
        dlg.exec();

        if(dlg.result() == QDialog::Accepted) {
            if(imgInput.type() == CV_32S || imgInput.type() == CV_32F) {
                unsigned int iValue, iNbValuesNew;

                for(iValue = 0; iValue < listValuesColors.size(); iValue++) {
                    listValuesColors[iValue].first = dlg.GetRealParam(iValue);
                    listValuesColors[iValue].second = (cv::Point3f)dlg.GetPoint3iParam(iValue);
                }

                iNbValuesNew = (unsigned int)dlg.GetIntParam(0);

                if(iNbValuesNew < listValuesColors.size()) {
                    unsigned int iNbToRemove = listValuesColors.size() - iNbValuesNew;
                    for(iValue = 0; iValue < iNbToRemove; iValue++) listValuesColors.pop_back();
                } else if(iNbValuesNew > listValuesColors.size()) {
                    unsigned int iNbToAdd = iNbValuesNew - listValuesColors.size();
                    pair<float, cv::Vec3f> valueAndColor;

                    valueAndColor = listValuesColors.back();
                    for(iValue = 0; iValue < iNbToAdd; iValue++) listValuesColors.push_back(valueAndColor);
                }
            } else if(imgInput.type() == CV_32SC3 || imgInput.type() == CV_32FC3) {
                // Conversion from cv::Point3f to cv::Vec3f
                fMin = dlg.GetPoint3fParam(0);
                fMax = dlg.GetPoint3fParam(1);
            }
            MakeQImage();
            scaleImage(1.0);
        }
    }
}

void CImage2DWnd::MakeQImage()
{
    int width, height;

    width = imgInput.cols;
    height = imgInput.rows;

    // Reallocate qimage if needed
    if(qimage.width() != width || qimage.height() != height)
        qimage = QImage(QSize(width, height), QImage::Format_RGB888);

    cv::Point p;
    cv::Vec3b b;
    cv::Vec3f f;
    unsigned char* pArray;
    unsigned char* pQimageData;
    float fValue;

    // We just need element-wise multiplication and division on Vec3f here...
    class VecOp
    {
    public:
        static cv::Vec3f mul(const cv::Vec3f& a, const cv::Vec3f& b)
        {
            cv::Vec3f p;
            p.val[0] = a.val[0] * b.val[0];
            p.val[1] = a.val[1] * b.val[1];
            p.val[2] = a.val[2] * b.val[2];
            return p;
        }

        static cv::Vec3f div(const cv::Vec3f& a, const cv::Vec3f& b)
        {
            cv::Vec3f q;
            q.val[0] = a.val[0] / b.val[0];
            q.val[1] = a.val[1] / b.val[1];
            q.val[2] = a.val[2] / b.val[2];
            return q;
        }
    };

    for(p.y = 0; p.y < height; p.y++) {
        pArray = imgInput.ptr(p.y);
        pQimageData = qimage.bits() + p.y * qimage.bytesPerLine();
        for(p.x = 0; p.x < width; p.x++) {
            switch(imgInput.type()) {
                // Scalar-valued images
                case CV_8U:
                    b = cv::Vec3b(*pArray, *pArray, *pArray);
                    break;
                case CV_32S:
                    fValue = (float)(*((int*)pArray));
                    b = GetColorFromLUT(fValue);
                    break;
                case CV_32F:
                    fValue = *((float*)pArray);
                    b = GetColorFromLUT(fValue);
                    break;

                // Vector-valued images
                case CV_8UC3:
                    b = *((cv::Vec3b*)pArray);
                    break;
                case CV_32FC3:
                    f = *((cv::Vec3f*)pArray);
                    b = VecOp::mul(VecOp::div((f - fMin), (fMax - fMin)), cv::Vec3f(255.0, 255.0, 255.0));
                    break;
                default:
                    cout << "CArray2DWnd::UpdateImage(): Unsupported format" << endl;
                    return;
            }

            // OpenCV cv::Mat stores in BGR format, while QImage stores in RGB
            pQimageData[0] = b.val[2];
            pQimageData[1] = b.val[1];
            pQimageData[2] = b.val[0];

            pArray += imgInput.elemSize();
            pQimageData += 3;
        }
    }
}

void CImage2DWnd::SaveQImage(const char* path) const
{
    if(qimage.save(path) == true)
        cout << "Image saved to " << path << endl;
    else
        cout << "Cannot save to " << path << endl;
}

cv::Vec3b CImage2DWnd::GetColorFromLUT(float fValue) const
{
    cv::Vec3b byteColor;
    unsigned int iColor;

    iColor = 0;
    while(iColor < listValuesColors.size() && fValue > listValuesColors[iColor].first) iColor++;

    if(iColor == 0)
        byteColor = listValuesColors[0].second;
    else if(iColor == listValuesColors.size())
        byteColor = listValuesColors[listValuesColors.size() - 1].second;
    else {
        cv::Vec3f fPixelInter;
        float fCoef;

        // Interpolate color
        fCoef = (fValue - listValuesColors[iColor - 1].first) /
                (listValuesColors[iColor].first - listValuesColors[iColor - 1].first);
        fPixelInter = listValuesColors[iColor - 1].second * (1.0f - fCoef) + listValuesColors[iColor].second * fCoef;
        byteColor = fPixelInter;
    }

    return byteColor;
}

/******************************************************************************
                            CListImage2DWnd
******************************************************************************/

unsigned int CListImage2DWnd::GetNbImages() const
{
    return vectImg.size();
}

void CListImage2DWnd::SetCurrentImage(unsigned int idx)
{
    unsigned int l = vectImg.size() - 1;
    idxCurrent = min(max(idx, 0U), l);
    imgInput = vectImg[idxCurrent].first;
    MakeQImage();
    scaleImage(1.0);
    UpdateTextStatusBar();
}

void CListImage2DWnd::ClearImages()
{
    vectImg.clear();
}

void CListImage2DWnd::AddImage(const cv::Mat& img, const char* imgName)
{
    vectImg.push_back(pair<cv::Mat, string>(img.clone(), imgName));
    if(vectImg.size() == 1) {
        idxCurrent = 0;
        imgInput = vectImg[0].first;

        MakeQImage();

        scaleImage(1.0);

        fitToWindowAct->setEnabled(true);
        updateActions();

        if(!fitToWindowAct->isChecked()) pFrame->adjustSize();

        UpdateTextStatusBar();
    }
}

string CListImage2DWnd::GetStatusBarString() const
{
    return vectImg[idxCurrent].second + " " + CImage2DWnd::GetStatusBarString();
}

void CListImage2DWnd::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_A && vectImg.size() > 1) {
        if(idxCurrent > 0) {
            imgInput = vectImg[--idxCurrent].first;
            MakeQImage();
            scaleImage(1.0);
            UpdateTextStatusBar();
        }
    } else if(event->key() == Qt::Key_Z && vectImg.size() > 1) {
        if(idxCurrent < vectImg.size() - 1) {
            imgInput = vectImg[++idxCurrent].first;
            MakeQImage();
            scaleImage(1.0);
            UpdateTextStatusBar();
        }
    } else
        CImage2DWnd::keyPressEvent(event);
}
