/*
Copyright (c) 2017 Julien Mille
INSA Centre Val de Loire
Laboratoire d'Informatique Fondamentale et Appliquée de Tours
*/

#include "dialogparams.h"

CDialogParams::CDialogParams(QWidget* parent, Qt::WindowFlags flags) {}

CDialogParams::~CDialogParams() {}

void CDialogParams::AddRealParam(float param, const string& name)
{
    realParams.push_back(pair<float, string>(param, name));
}

void CDialogParams::AddIntParam(int param, const string& name)
{
    intParams.push_back(pair<int, string>(param, name));
}

void CDialogParams::AddBoolParam(bool param, const string& name)
{
    boolParams.push_back(pair<bool, string>(param, name));
}

void CDialogParams::AddPoint3fParam(const cv::Point3f& param, const string& name)
{
    pt3fParams.push_back(pair<cv::Point3f, string>(param, name));
}

void CDialogParams::AddPoint3iParam(const cv::Point3i& param, const string& name)
{
    pt3iParams.push_back(pair<cv::Point3i, string>(param, name));
}

float CDialogParams::GetRealParam(unsigned int idx) const
{
    return realParams[idx].first;
}

int CDialogParams::GetIntParam(unsigned int idx) const
{
    return intParams[idx].first;
}

bool CDialogParams::GetBoolParam(unsigned int idx) const
{
    return boolParams[idx].first;
}

cv::Point3f CDialogParams::GetPoint3fParam(unsigned int idx) const
{
    return pt3fParams[idx].first;
}

cv::Point3i CDialogParams::GetPoint3iParam(unsigned int idx) const
{
    return pt3iParams[idx].first;
}

void CDialogParams::MakeWidgets()
{
    QGridLayout* mainGrid = new QGridLayout;
    QVBoxLayout* topLayout = new QVBoxLayout;
    QHBoxLayout* hboxLayout;

    QLabel* label;
    QPushButton* btn;
    unsigned int iValue;

    for(iValue = 0; iValue < realParams.size(); iValue++) {
        hboxLayout = new QHBoxLayout();
        label = new QLabel(realParams[iValue].second.c_str());

        vectTextEdits.push_back(NULL);
        vectTextEdits.back() = new QLineEdit;
        vectTextEdits.back()->setText(QString::number(realParams[iValue].first, 'f', 4));

        hboxLayout->addWidget(label);
        hboxLayout->addWidget(vectTextEdits.back());
        topLayout->addLayout(hboxLayout);
    }

    for(iValue = 0; iValue < intParams.size(); iValue++) {
        hboxLayout = new QHBoxLayout();
        label = new QLabel(intParams[iValue].second.c_str());

        vectTextEdits.push_back(NULL);
        vectTextEdits.back() = new QLineEdit;
        vectTextEdits.back()->setText(QString::number(intParams[iValue].first));

        hboxLayout->addWidget(label);
        hboxLayout->addWidget(vectTextEdits.back());
        topLayout->addLayout(hboxLayout);
    }

    for(iValue = 0; iValue < boolParams.size(); iValue++) {
        vectCheckBoxes.push_back(NULL);
        vectCheckBoxes.back() = new QCheckBox(boolParams[iValue].second.c_str());
        vectCheckBoxes.back()->setCheckable(true);
        vectCheckBoxes.back()->setChecked(boolParams[iValue].first);

        topLayout->addWidget(vectCheckBoxes.back());
    }

    for(iValue = 0; iValue < pt3fParams.size(); iValue++) {
        hboxLayout = new QHBoxLayout();

        label = new QLabel(pt3fParams[iValue].second.c_str());
        hboxLayout->addWidget(label);

        vectTextEdits.push_back(NULL);
        vectTextEdits.back() = new QLineEdit;
        vectTextEdits.back()->setText(QString::number(pt3fParams[iValue].first.x, 'f', 4));
        hboxLayout->addWidget(vectTextEdits.back());

        vectTextEdits.push_back(NULL);
        vectTextEdits.back() = new QLineEdit;
        vectTextEdits.back()->setText(QString::number(pt3fParams[iValue].first.y, 'f', 4));
        hboxLayout->addWidget(vectTextEdits.back());

        vectTextEdits.push_back(NULL);
        vectTextEdits.back() = new QLineEdit;
        vectTextEdits.back()->setText(QString::number(pt3fParams[iValue].first.z, 'f', 4));
        hboxLayout->addWidget(vectTextEdits.back());

        topLayout->addLayout(hboxLayout);
    }

    for(iValue = 0; iValue < pt3iParams.size(); iValue++) {
        hboxLayout = new QHBoxLayout();

        label = new QLabel(pt3iParams[iValue].second.c_str());
        hboxLayout->addWidget(label);

        vectTextEdits.push_back(NULL);
        vectTextEdits.back() = new QLineEdit;
        vectTextEdits.back()->setText(QString::number(pt3iParams[iValue].first.x));
        hboxLayout->addWidget(vectTextEdits.back());

        vectTextEdits.push_back(NULL);
        vectTextEdits.back() = new QLineEdit;
        vectTextEdits.back()->setText(QString::number(pt3iParams[iValue].first.y));
        hboxLayout->addWidget(vectTextEdits.back());

        vectTextEdits.push_back(NULL);
        vectTextEdits.back() = new QLineEdit;
        vectTextEdits.back()->setText(QString::number(pt3iParams[iValue].first.z));
        hboxLayout->addWidget(vectTextEdits.back());

        topLayout->addLayout(hboxLayout);
    }

    btn = new QPushButton("OK");
    connect(btn, &QPushButton::clicked, this, &CDialogParams::sl_accept);
    topLayout->addWidget(btn);

    btn = new QPushButton("Cancel");
    connect(btn, &QPushButton::clicked, this, &QDialog::reject);

    topLayout->addWidget(btn);

    mainGrid->addLayout(topLayout, 0, 0);
    setLayout(mainGrid);

    QDialog::show();
}

void CDialogParams::sl_accept()
{
    unsigned int iValue;
    QString stringValue;
    int iBase;

    for(iValue = 0; iValue < realParams.size(); iValue++) {
        stringValue = vectTextEdits[iValue]->text();
        realParams[iValue].first = stringValue.toFloat();
    }

    for(iValue = 0; iValue < intParams.size(); iValue++) {
        stringValue = vectTextEdits[iValue + realParams.size()]->text();
        intParams[iValue].first = stringValue.toInt();
    }

    for(iValue = 0; iValue < boolParams.size(); iValue++)
        boolParams[iValue].first = vectCheckBoxes[iValue]->isChecked();

    iBase = realParams.size() + intParams.size();

    for(iValue = 0; iValue < pt3fParams.size(); iValue++) {
        stringValue = vectTextEdits[iBase + 3 * iValue]->text();
        pt3fParams[iValue].first.x = stringValue.toFloat();

        stringValue = vectTextEdits[iBase + 3 * iValue + 1]->text();
        pt3fParams[iValue].first.y = stringValue.toFloat();

        stringValue = vectTextEdits[iBase + 3 * iValue + 2]->text();
        pt3fParams[iValue].first.z = stringValue.toFloat();
    }

    iBase = realParams.size() + intParams.size() + pt3fParams.size() * 3;

    for(iValue = 0; iValue < pt3iParams.size(); iValue++) {
        stringValue = vectTextEdits[iBase + 3 * iValue]->text();
        pt3iParams[iValue].first.x = stringValue.toInt();

        stringValue = vectTextEdits[iBase + 3 * iValue + 1]->text();
        pt3iParams[iValue].first.y = stringValue.toInt();

        stringValue = vectTextEdits[iBase + 3 * iValue + 2]->text();
        pt3iParams[iValue].first.z = stringValue.toInt();
    }

    QDialog::accept();
}
