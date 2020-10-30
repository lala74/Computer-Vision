/*
Copyright (c) 2017 Julien Mille
INSA Centre Val de Loire
Laboratoire d'Informatique Fondamentale et Appliquée de Tours
*/

#ifndef DIALOGPARAMS_H
#define DIALOGPARAMS_H

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>
#include <opencv2/core.hpp>  // For cv::Point3i and cv::Point3f
#include <string>
#include <vector>

using namespace std;

/*  Make a dialog box with a modulable list of user-defined parameters
    Parameters can be of 5 types: real, integer, boolean, OpenCV's cv::Point3f, OpenCV's cv::Point3i
    First, initial values and names of parameters should be set just after constructing the CDialogParams
    object, using Add...Param(...) member functions
    Second, generate widgets using MakeWidgets() member function
    Then, display the dialog box as you would do with a regular QDialog
    Finally, get new values of parameters by calling Get...Params(...)
    Indices passed to Get...Param(...) should be relevant w.r.t the number of times
    the corresponding Add...Param(...) method was called before

    Example:

    CDialogParams dlg;
    int a=2, b=-4;
    float c=3.0;

    dlg.AddIntParam(a, "First integer");
    dlg.AddRealParam(c, "First real");
    dlg.AddIntParam(b, "Second integer");

    dlg.MakeWidgets()
    dlg.setModal(true);
    dlg.exec();

    // Now, there are two integers and one real stored in the dialog box
    // so you should get the new values using something like
    if (dlg.result()==QDialog::Accepted)
    {
        c = dlg.GetRealParam(0);
        a = dlg.GetIntParam(0);
        b = dlg.GetIntParam(1);
    } */
class CDialogParams : public QDialog
{
    // Member variables
protected:
    vector<QLineEdit*> vectTextEdits;
    vector<QCheckBox*> vectCheckBoxes;

    vector<pair<float, string>> realParams;
    vector<pair<int, string>> intParams;
    vector<pair<bool, string>> boolParams;
    vector<pair<cv::Point3f, string>> pt3fParams;
    vector<pair<cv::Point3i, string>> pt3iParams;

    // Member functions
public:
    // Default constructor
    CDialogParams(QWidget* parent = NULL, Qt::WindowFlags f = 0);

    // Destructor
    ~CDialogParams();

    void AddRealParam(float, const string&);
    void AddIntParam(int, const string&);
    void AddBoolParam(bool, const string&);
    void AddPoint3fParam(const cv::Point3f&, const string&);
    void AddPoint3iParam(const cv::Point3i&, const string&);

    // Get parameter by index
    // The index should be relevant with the calling sequence of Add...Param(...)
    float GetRealParam(unsigned int) const;
    int GetIntParam(unsigned int) const;
    bool GetBoolParam(unsigned int) const;
    cv::Point3f GetPoint3fParam(unsigned int) const;
    cv::Point3i GetPoint3iParam(unsigned int) const;

    // Make widgets according to parameters previously added
    void MakeWidgets();

protected:
    // Called when user presses OK
    void sl_accept();
};

#endif
