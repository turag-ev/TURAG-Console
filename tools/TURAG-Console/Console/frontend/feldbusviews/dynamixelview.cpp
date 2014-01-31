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

    //Present Position
    QHBoxLayout* presentPositionLayout = new QHBoxLayout;
    QLabel* presentPositionLabel = new QLabel("Present Position");
    presentPosition = new QLabel;
    presentPositionLayout->addWidget(presentPositionLabel);
    presentPositionLayout->addWidget(presentPosition);
    layout->addLayout(presentPositionLayout);

    //Present Speed
    QHBoxLayout* presentSpeedLayout = new QHBoxLayout;
    QLabel* presentSpeedLabel = new QLabel("Present Speed");
    presentSpeed = new QLabel;
    presentSpeedLayout->addWidget(presentSpeedLabel);
    presentSpeedLayout->addWidget(presentSpeed);
    layout->addLayout(presentSpeedLayout);

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

    //Moving
    QHBoxLayout* isMovingLayout = new QHBoxLayout;
    QLabel* isMovingLabel = new QLabel("is Moving");
    isMoving = new QLabel;
    isMovingLayout->addWidget(isMovingLabel);
    isMovingLayout->addWidget(isMoving);
    layout->addLayout(isMovingLayout);

    /*QHBoxLayout* presentBaudRateLayout = new QHBoxLayout;
    QLabel* presentBaudRateLabel = new QLabel ("Present Baud Rate");
    presentBaudRate = new QLabel;
    presentBaudRateLayout ->addWidget(presentBaudRateLabel);
    presentBaudRateLayout ->addWidget(presentBaudRate);
    layout->addLayout(presentBaudRateLayout);*/

    angleLimit = new QLabel;
    layout->addWidget(angleLimit);


    //Desired Position
    QHBoxLayout* desiredPositionLayout = new QHBoxLayout;
    QLabel* desiredPositionLabel = new QLabel("Desired Position");
    desiredPosition = new QLineEdit;
    setPosition = new QPushButton("Position setzen");
    desiredPositionLayout->addWidget(desiredPositionLabel);
    desiredPositionLayout->addWidget(desiredPosition);
    desiredPositionLayout->addWidget(setPosition);
    connect(setPosition, SIGNAL(clicked()), this, SLOT(onSetPositionPushed()));
    layout->addLayout(desiredPositionLayout);

    //Baudrate
    QHBoxLayout* desiredBaudRateLayout = new QHBoxLayout;
    QLabel* desiredBaudRateLabel = new QLabel ("Baudrate");
    desiredBaudRate= new QLineEdit;
    setBaudRate = new QPushButton ("Baudrate setzen");
    desiredBaudRateLayout ->addWidget(desiredBaudRateLabel);
    desiredBaudRateLayout ->addWidget(desiredBaudRate);
    desiredBaudRateLayout ->addWidget(setBaudRate);
    connect(setBaudRate, SIGNAL(clicked()), this, SLOT(onSetBaudRate()));
    layout-> addLayout(desiredBaudRateLayout);

    //CW AngleLimit
    QHBoxLayout* cwAngleLimitLayout = new QHBoxLayout;
    QLabel* cwAngleLimitLabel = new QLabel ("CW Angle Limit");
    cwAngleLimit= new QLineEdit;
    setCwAngleLimit = new QPushButton ("CW Angle Limit setzten");
    cwAngleLimitLayout ->addWidget(cwAngleLimitLabel);
    cwAngleLimitLayout ->addWidget(cwAngleLimit);
    cwAngleLimitLayout ->addWidget(setCwAngleLimit);
    connect(setCwAngleLimit, SIGNAL(clicked()), this, SLOT(onSetCwAngleLimit()));
    layout-> addLayout(cwAngleLimitLayout);

    //CcW AngleLimit
    QHBoxLayout* ccwAngleLimitLayout = new QHBoxLayout;
    QLabel* ccwAngleLimitLabel = new QLabel ("CCW Angle Limit");
    ccwAngleLimit= new QLineEdit;
    setCcwAngleLimit = new QPushButton ("CCW Angle Limit setzten");
    ccwAngleLimitLayout ->addWidget(ccwAngleLimitLabel);
    ccwAngleLimitLayout ->addWidget(ccwAngleLimit);
    ccwAngleLimitLayout ->addWidget(setCcwAngleLimit);
    connect(setCcwAngleLimit, SIGNAL(clicked()), this, SLOT(onSetCcwAngleLimit()));
    layout-> addLayout(ccwAngleLimitLayout);

    //MovingSpeed
    QHBoxLayout* movingSpeedLayout = new QHBoxLayout;
    QLabel* movingSpeedLabel = new QLabel ("Moving Speed");
    movingSpeed= new QLineEdit;
    setMovingSpeed = new QPushButton ("Moving Speed setzen");
    movingSpeedLayout ->addWidget(movingSpeedLabel);
    movingSpeedLayout ->addWidget(movingSpeed);
    movingSpeedLayout ->addWidget(setMovingSpeed);
    connect(setMovingSpeed, SIGNAL(clicked()), this, SLOT(onSetMovingSpeed()));
    layout-> addLayout(movingSpeedLayout);

    //TorqueLimit
    QHBoxLayout* torqueLimitLayout = new QHBoxLayout;
    QLabel* torqueLimitLabel = new QLabel ("Torque Limit");
    torqueLimit= new QLineEdit;
    setTorqueLimit= new QPushButton ("Torque Limit setzen");
    torqueLimitLayout ->addWidget(torqueLimitLabel);
    torqueLimitLayout ->addWidget(torqueLimit);
    torqueLimitLayout ->addWidget(setTorqueLimit);
    connect(setTorqueLimit, SIGNAL(clicked()), this, SLOT(onSetTorqueLimit()));
    layout-> addLayout(torqueLimitLayout);

    //Baudrate@init
    float currentBaudRate = 0;
    if (device && device->getBaudRate(&currentBaudRate)){
      desiredBaudRate->setText(QString("%1").arg(currentBaudRate));
    } else{
        desiredBaudRate->setText("Error. Unable to read BaudRate");
    }

    //Angle Limit@init
    readCcwAngleLimit();
    readCwAngleLimit();

    //Moving Speed@init
    readMovingSpeed();

    //TorqueLimit @init
    readTorqueLimit();

    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimeout()));
    updateTimer->start(500);
}

void DynamixelView::readTorqueLimit(void){
    int currentTorqueLimit = 0;
    if (device && device->getTorqueLimit(&currentTorqueLimit)){
        torqueLimit->setText(QString("%1").arg(currentTorqueLimit));
    } else {
        torqueLimit->setText("Error. Unable to read TorqueLimit");
}
}

void DynamixelView::readCwAngleLimit(void) {
    float currentCwAngleLimit = 0;
    if (device && device->getCwAngleLimit(&currentCwAngleLimit)){
        cwAngleLimit->setText(QString("%1").arg(currentCwAngleLimit));
    } else
        cwAngleLimit->setText("Error. Unable to read CwAngleLimit");
}

void DynamixelView::readCcwAngleLimit(void) {
    float currentCcwAngleLimit = 0;
    if (device && device->getCcwAngleLimit(&currentCcwAngleLimit)){
        ccwAngleLimit->setText(QString("%1").arg(currentCcwAngleLimit));
    } else
        ccwAngleLimit->setText("Error. Unable to read CcwAngleLimit");
}

void DynamixelView::readMovingSpeed(void){
    float currentSpeed = 0;
    if (device && device->getMovingSpeed(&currentSpeed)){
        movingSpeed->setText(QString("%1").arg(currentSpeed));
    } else{
        movingSpeed->setText("Error: unable to read movingSpeed");
    }
}

void DynamixelView::onUpdateTimeout(void) {
    if (!device) return;

    float position = 0;
    float voltage = 0;
    int load = 0;
    int direction = 0;
    float speed = 0;
    int speedDirection = 0;
    bool movement;
    QString directionString= "";
    QString speedDirectionString="";

    if (device->isMoving(&movement)){
        if (movement){
            isMoving->setText("true");
        }
    } else {
        isMoving->setText("Error: Unable to detect moving");
    }

    if (device->getCurrentPosition(&position)) {
        presentPosition->setText(QString("%1").arg(position));
    } else {
        presentPosition->setText("ERROR");
    }

    if (device->getPresentVoltage(&voltage)) {
        presentVoltage->setText(QString("%1").arg(voltage));
    } else {
        presentVoltage->setText("ERROR");
    }

    if (device->getPresentSpeed(&speed, &speedDirection)) {
        if (speedDirection==1){
            speedDirectionString="CW";
        }
        if (speedDirection==0){
            speedDirectionString="CCW";
        }
        presentLoad->setText(QString("%1").arg(speed)+speedDirectionString);
    } else {
        presentLoad->setText("ERROR");
    }

    if (device->getPresentLoad(&load, &direction)) {
        if (direction==1){
            directionString="CW";
        }
        if (direction==0){
            directionString="CCW";
        }
        presentLoad->setText(QString("%1").arg(load)+directionString);
    } else {
        presentLoad->setText("ERROR");
    }

    float cwAngleLimit = 0; float ccwAngleLimit = 0; //19.01.2014, Richard R: Variabletyp auf float geändert
    if (device->getCcwAngleLimit(&ccwAngleLimit) && device->getCwAngleLimit(&cwAngleLimit)) {
        angleLimit->setText(QString("%1 - %2").arg(cwAngleLimit).arg(ccwAngleLimit));
    } else {
        angleLimit->setText("Error");
    }
}
void DynamixelView::onSetTorqueLimit(void) {
    if (!device) return;
    device->setTorqueLimit(torqueLimit->text().toInt());
    readTorqueLimit();
}

void DynamixelView::onSetPositionPushed(void) {
    if (!device) return;
    device->setTorqueEnable(true);
    device->setGoalPosition(desiredPosition->text().toInt());
}

void DynamixelView::onSetBaudRate(void){
    if (!device) return;
    device->setBaudRate(desiredBaudRate->text().toFloat());
    float currentBaudRate = 0;
    if (device->getBaudRate(&currentBaudRate)){
      desiredBaudRate->setText(QString("%1").arg(currentBaudRate));
    } else{
        desiredBaudRate->setText("Error. Unable to read BaudRate");
    }

}

void DynamixelView::onSetCwAngleLimit(void) {
    if (!device) return;
    device->setCwAngleLimit(cwAngleLimit->text().toFloat());
    readCwAngleLimit();
}

void DynamixelView::onSetCcwAngleLimit(void) {
    if (!device) return;
    device->setCcwAngleLimit(ccwAngleLimit->text().toFloat());
    readCcwAngleLimit();
}

void DynamixelView::onSetMovingSpeed(void){
    if (!device) return;
    device->setMovingSpeed(movingSpeed->text().toFloat());
    readMovingSpeed();
}
