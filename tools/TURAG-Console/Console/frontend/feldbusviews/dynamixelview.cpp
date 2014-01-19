#include "dynamixelview.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QPushButton>
#include <QLineEdit>

DynamixelView::DynamixelView(DynamixelDevice* dev, QWidget *parent) :
    QWidget(parent), device(dev)
{
    presentPosition = new QLabel;
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(presentPosition);
    angleLimit = new QLabel;
    layout->addWidget(angleLimit);
    desiredPosition = new QLineEdit;
    layout->addWidget(desiredPosition);
    setPosition = new QPushButton("Position setzen");
    layout->addWidget(setPosition);
    connect(setPosition, SIGNAL(clicked()), this, SLOT(onSetPositionPushed()));

    setLayout(layout);


    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimeout()));
    updateTimer->start(500);
}


void DynamixelView::onUpdateTimeout(void) {
    int position = 0;

    if (device->getCurrentPosition(&position)) {
        presentPosition->setText(QString("%1").arg(position));
    } else {
        presentPosition->setText("ERROR");
    }

    float cwAngleLimit = 0; float ccwAngleLimit = 0; //19.01.2014, Richard R: Variabletyp auf float geändert
    if (device->getCcwAngleLimit(&ccwAngleLimit) && device->getCwAngleLimit(&cwAngleLimit)) {
        angleLimit->setText(QString("%1 - %2").arg(cwAngleLimit).arg(ccwAngleLimit));
    } else {
        angleLimit->setText("Error");
    }
}

void DynamixelView::onSetPositionPushed(void) {
    device->setTorqueEnable(true);
    device->setGoalPosition(desiredPosition->text().toInt());
}
