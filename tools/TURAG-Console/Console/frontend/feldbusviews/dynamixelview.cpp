#include "dynamixelview.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QPushButton>
#include <QLineEdit>

DynamixelView::DynamixelView(DynamixelDevice* dev, QWidget *parent) :
    QWidget(parent), device(dev)
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    QHBoxLayout* presentPositionLayout = new QHBoxLayout;
    QLabel* presentPositionLabel = new QLabel("Present Position");
    presentPosition = new QLabel;
    presentPositionLayout->addWidget(presentPositionLabel);
    presentPositionLayout->addWidget(presentPosition);
    layout->addLayout(presentPositionLayout);

    //Present Voltage
    QHBoxLayout* presentVoltageLayout = new QHBoxLayout;
    QLabel* presentVoltageLabel = new QLabel("Present Voltage");
    presentVoltage = new QLabel;
    presentVoltageLayout->addWidget(presentVoltageLabel);
    presentVoltageLayout->addWidget(presentVoltage);
    layout->addLayout(presentVoltageLayout);

    //Present Load
    QHBoxLayout* presentLoadLayout = new QHBoxLayout;
    QLabel* presentLoadLabel = new QLabel("Present Position");
    presentLoad = new QLabel;
    presentLoadLayout->addWidget(presentLoadLabel);
    presentLoadLayout->addWidget(presentLoad);
    layout->addLayout(presentLoadLayout);

    /*QHBoxLayout* presentBaudRateLayout = new QHBoxLayout;
    QLabel* presentBaudRateLabel = new QLabel ("Present Baud Rate");
    presentBaudRate = new QLabel;
    presentBaudRateLayout ->addWidget(presentBaudRateLabel);
    presentBaudRateLayout ->addWidget(presentBaudRate);
    layout->addLayout(presentBaudRateLayout);*/

    angleLimit = new QLabel;
    layout->addWidget(angleLimit);


    QHBoxLayout* desiredPositionLayout = new QHBoxLayout;
    QLabel* desiredPositionLabel = new QLabel("Desired Position");
    desiredPosition = new QLineEdit;
    setPosition = new QPushButton("Position setzen");
    desiredPositionLayout->addWidget(desiredPositionLabel);
    desiredPositionLayout->addWidget(desiredPosition);
    desiredPositionLayout->addWidget(setPosition);
    connect(setPosition, SIGNAL(clicked()), this, SLOT(onSetPositionPushed()));
    layout->addLayout(desiredPositionLayout);

    QHBoxLayout* desiredBaudRateLayout = new QHBoxLayout;
    QLabel* desiredBaudRateLabel = new QLabel ("Baudrate");
    desiredBaudRate= new QLineEdit;
    setBaudRate = new QPushButton ("Baudrate setzen");
    desiredBaudRateLayout ->addWidget(desiredBaudRateLabel);
    desiredBaudRateLayout ->addWidget(desiredBaudRate);
    desiredBaudRateLayout ->addWidget(setBaudRate);
    connect(setBaudRate, SIGNAL(clicked()), this, SLOT(onSetBaudRate()));
    layout-> addLayout(desiredBaudRateLayout);

    float currentBaudRate = 0;
    if (device->getBaudRate(&currentBaudRate)){
      desiredBaudRate->setText(QString("%1").arg(currentBaudRate));
    } else{
        desiredBaudRate->setText("Error. Unable to read BaudRate");
    }




    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimeout()));
    updateTimer->start(500);
}


void DynamixelView::onUpdateTimeout(void) {
    if (!device) return;
    float position = 0;

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

void DynamixelView::onSetBaudRate(void){
    device->setBaudRate(desiredBaudRate->text().toFloat());
    float currentBaudRate = 0;
    if (device->getBaudRate(&currentBaudRate)){
      desiredBaudRate->setText(QString("%1").arg(currentBaudRate));
    } else{
        desiredBaudRate->setText("Error. Unable to read BaudRate");
    }

}
