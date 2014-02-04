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
    QLabel* presentPositionLabel = new QLabel("Present Position (in °)");
    presentPosition = new QLabel;
    presentPositionLayout->addWidget(presentPositionLabel);
    presentPositionLayout->addWidget(presentPosition);
    layout->addLayout(presentPositionLayout);

    //Present Speed
    QHBoxLayout* presentSpeedLayout = new QHBoxLayout;
    QLabel* presentSpeedLabel = new QLabel("Present Speed (in rpm)");
    presentSpeed = new QLabel;
    presentSpeedLayout->addWidget(presentSpeedLabel);
    presentSpeedLayout->addWidget(presentSpeed);
    layout->addLayout(presentSpeedLayout);

    //Present Voltage
    QHBoxLayout* presentVoltageLayout = new QHBoxLayout;
    QLabel* presentVoltageLabel = new QLabel("Present Voltage (in V)");
    presentVoltage = new QLabel;
    presentVoltageLayout->addWidget(presentVoltageLabel);
    presentVoltageLayout->addWidget(presentVoltage);
    layout->addLayout(presentVoltageLayout);

    //Present Load
    QHBoxLayout* presentLoadLayout = new QHBoxLayout;
    QLabel* presentLoadLabel = new QLabel("Present Load (XX% of max Load)");
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

    //Overload
    QHBoxLayout* isOverloadLayout = new QHBoxLayout;
    QLabel* isOverloadLabel = new QLabel("is Overload");
    isOverload = new QLabel;
    isOverloadLayout->addWidget(isOverloadLabel);
    isOverloadLayout->addWidget(isOverload);
    layout->addLayout(isOverloadLayout);

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
    QLabel* desiredPositionLabel = new QLabel("Desired Position (in °)");
    desiredPosition = new QLineEdit;
    setPosition = new QPushButton("Set Position");
    desiredPositionLayout->addWidget(desiredPositionLabel);
    desiredPositionLayout->addWidget(desiredPosition);
    desiredPositionLayout->addWidget(setPosition);
    connect(setPosition, SIGNAL(clicked()), this, SLOT(onSetPositionPushed()));
    layout->addLayout(desiredPositionLayout);

    //Baudrate
    QHBoxLayout* desiredBaudRateLayout = new QHBoxLayout;
    QLabel* desiredBaudRateLabel = new QLabel ("Baudrate");
    desiredBaudRate= new QLineEdit;
    setBaudRate = new QPushButton ("Set Baudrate");
    desiredBaudRateLayout ->addWidget(desiredBaudRateLabel);
    desiredBaudRateLayout ->addWidget(desiredBaudRate);
    desiredBaudRateLayout ->addWidget(setBaudRate);
    connect(setBaudRate, SIGNAL(clicked()), this, SLOT(onSetBaudRate()));
    layout-> addLayout(desiredBaudRateLayout);

    //CW AngleLimit
    QHBoxLayout* cwAngleLimitLayout = new QHBoxLayout;
    QLabel* cwAngleLimitLabel = new QLabel ("CW Angle Limit (in °)");
    cwAngleLimit= new QLineEdit;
    setCwAngleLimit = new QPushButton ("Set CW Angle Limit");
    cwAngleLimitLayout ->addWidget(cwAngleLimitLabel);
    cwAngleLimitLayout ->addWidget(cwAngleLimit);
    cwAngleLimitLayout ->addWidget(setCwAngleLimit);
    connect(setCwAngleLimit, SIGNAL(clicked()), this, SLOT(onSetCwAngleLimit()));
    layout-> addLayout(cwAngleLimitLayout);

    //CcW AngleLimit
    QHBoxLayout* ccwAngleLimitLayout = new QHBoxLayout;
    QLabel* ccwAngleLimitLabel = new QLabel ("CCW Angle Limit (in °)");
    ccwAngleLimit= new QLineEdit;
    setCcwAngleLimit = new QPushButton ("Set CCW Angle Limit");
    ccwAngleLimitLayout ->addWidget(ccwAngleLimitLabel);
    ccwAngleLimitLayout ->addWidget(ccwAngleLimit);
    ccwAngleLimitLayout ->addWidget(setCcwAngleLimit);
    connect(setCcwAngleLimit, SIGNAL(clicked()), this, SLOT(onSetCcwAngleLimit()));
    layout-> addLayout(ccwAngleLimitLayout);

    //MovingSpeed
    QHBoxLayout* movingSpeedLayout = new QHBoxLayout;
    QLabel* movingSpeedLabel = new QLabel ("Moving Speed (0-1023)");
    movingSpeed= new QLineEdit;
    setMovingSpeed = new QPushButton ("Set Moving Speed");
    movingSpeedLayout ->addWidget(movingSpeedLabel);
    movingSpeedLayout ->addWidget(movingSpeed);
    movingSpeedLayout ->addWidget(setMovingSpeed);
    connect(setMovingSpeed, SIGNAL(clicked()), this, SLOT(onSetMovingSpeed()));
    layout-> addLayout(movingSpeedLayout);

    //TorqueLimit
    QHBoxLayout* torqueLimitLayout = new QHBoxLayout;
    QLabel* torqueLimitLabel = new QLabel ("Torque Limit");
    torqueLimit= new QLineEdit;
    setTorqueLimit= new QPushButton ("Set Torque Limit");
    torqueLimitLayout ->addWidget(torqueLimitLabel);
    torqueLimitLayout ->addWidget(torqueLimit);
    torqueLimitLayout ->addWidget(setTorqueLimit);
    connect(setTorqueLimit, SIGNAL(clicked()), this, SLOT(onSetTorqueLimit()));
    layout-> addLayout(torqueLimitLayout);

    //Cw Compl Margin
    QHBoxLayout* cwComplMarginLayout = new QHBoxLayout;
    QLabel* cwComplMarginLabel = new QLabel ("Cw Compl Margin");
    cwComplMargin= new QLineEdit;
    setCwComplMargin= new QPushButton ("Ccw Compl Margin");
    cwComplMarginLayout ->addWidget(cwComplMarginLabel);
    cwComplMarginLayout ->addWidget(cwComplMargin);
    cwComplMarginLayout ->addWidget(setCwComplMargin);
    connect(setCwComplMargin, SIGNAL(clicked()), this, SLOT(onSetCwComplMargin()));
    layout-> addLayout(cwComplMarginLayout);

    //Ccw Compl Margin
    QHBoxLayout* ccwComplMarginLayout = new QHBoxLayout;
    QLabel* ccwComplMarginLabel = new QLabel ("Ccw Compl Margin");
    ccwComplMargin= new QLineEdit;
    setCcwComplMargin= new QPushButton ("Set Ccw Compl Margin");
    ccwComplMarginLayout ->addWidget(ccwComplMarginLabel);
    ccwComplMarginLayout ->addWidget(ccwComplMargin);
    ccwComplMarginLayout ->addWidget(setCcwComplMargin);
    connect(setCcwComplMargin, SIGNAL(clicked()), this, SLOT(onSetCcwComplMargin()));
    layout-> addLayout(ccwComplMarginLayout);

    //Cw Compl Slope
    QHBoxLayout* cwComplSlopeLayout = new QHBoxLayout;
    QLabel* cwComplSlopeLabel = new QLabel ("Cw Compl Slope");
    cwComplSlope= new QLineEdit;
    setCwComplSlope= new QPushButton ("Ccw Compl Slope");
    cwComplSlopeLayout ->addWidget(cwComplSlopeLabel);
    cwComplSlopeLayout ->addWidget(cwComplSlope);
    cwComplSlopeLayout ->addWidget(setCwComplSlope);
    connect(setCwComplSlope, SIGNAL(clicked()), this, SLOT(onSetCwComplSlope()));
    layout-> addLayout(cwComplSlopeLayout);

    //Ccw Compl Slope
    QHBoxLayout* ccwComplSlopeLayout = new QHBoxLayout;
    QLabel* ccwComplSlopeLabel = new QLabel ("Ccw Compl Slope");
    ccwComplSlope= new QLineEdit;
    setCcwComplSlope= new QPushButton ("Set Ccw Compl Slope");
    ccwComplSlopeLayout ->addWidget(ccwComplSlopeLabel);
    ccwComplSlopeLayout ->addWidget(ccwComplSlope);
    ccwComplSlopeLayout ->addWidget(setCcwComplSlope);
    connect(setCcwComplSlope, SIGNAL(clicked()), this, SLOT(onSetCcwComplSlope()));
    layout-> addLayout(ccwComplSlopeLayout);

    //Alarm Shutdown
    QHBoxLayout* alarmShutdownLayout = new QHBoxLayout;
    QLabel* alarmShutdownLabel = new QLabel ("Alarm Shutdown");
    alarmShutdown= new QLineEdit;
    setAlarmShutdown= new QPushButton ("Set AlarmShutdown");
    alarmShutdownLayout ->addWidget(alarmShutdownLabel);
    alarmShutdownLayout ->addWidget(alarmShutdown);
    alarmShutdownLayout ->addWidget(setAlarmShutdown);
    connect(setAlarmShutdown, SIGNAL(clicked()), this, SLOT(onSetAlarmShutdown()));
    layout-> addLayout(alarmShutdownLayout);

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

    //Compliance @init
    readCwComplMargin();
    readCcwComplMargin();
    readCwComplSlope();
    readCcwComplSlope();

    //Alarm Shutdown
    readAlarmShutdown();

    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimeout()));
    updateTimer->start(500);
}

void DynamixelView::readAlarmShutdown(void){
    int currentSetting = 0;
    if (device && device->getAlarmShutdown(&currentSetting)){
        alarmShutdown->setText(QString("%1").arg(currentSetting));
    } else {
        alarmShutdown->setText("Error. Unable to read.");
    }
}

void DynamixelView::readCwComplMargin(void){
    int currentCompliance = 0;
    if (device && device->getCwComplianceMargin(&currentCompliance)){
        cwComplMargin->setText(QString("%1").arg(currentCompliance));
    } else {
        cwComplMargin->setText("Error. Unable to read.");
    }
}

void DynamixelView::readCcwComplMargin(void){
    int currentCompliance = 0;
    if (device && device->getCcwComplianceMargin(&currentCompliance)){
        ccwComplMargin->setText(QString("%1").arg(currentCompliance));
    } else {
        ccwComplMargin->setText("Error. Unable to read.");
    }
}

void DynamixelView::readCwComplSlope(void){
    int currentCompliance = 0;
    if (device && device->getCwComplianceSlope(&currentCompliance)){
        cwComplSlope->setText(QString("%1").arg(currentCompliance));
    } else {
        cwComplSlope->setText("Error. Unable to read.");
    }
}

void DynamixelView::readCcwComplSlope(void){
    int currentCompliance = 0;
    if (device && device->getCcwComplianceSlope(&currentCompliance)){
        ccwComplSlope->setText(QString("%1").arg(currentCompliance));
    } else {
        ccwComplSlope->setText("Error. Unable to read.");
    }
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
    int currentSpeed = 0;
    if (device && device->getMovingSpeed(&currentSpeed)){
        movingSpeed->setText(QString("%1").arg(currentSpeed));
    } else{
        movingSpeed->setText("Error: unable to read movingSpeed");
    }
}

void DynamixelView::onUpdateTimeout(void) {
    if (!device) return;

    float position = 0;
    float u = 0;
    int load = 0;
    int direction = 0;
    float speed = 0;
    int speedDirection = 0;
    bool movement;
    QString directionString= "";
    QString speedDirectionString="";

    if (device && device->isOverload()){
            isOverload->setText("OVERLOAD");
        } else {
        isOverload->setText("-");
    }

    if (device->isMoving(&movement)){
        if (movement){
            isMoving->setText("true");
        } else {
            isMoving->setText("false");
}
    } else {
        isMoving->setText("Error: Unable to detect moving");
    }

    if (device->getCurrentPosition(&position)) {
        presentPosition->setText(QString("%1").arg(position));
    } else {
        presentPosition->setText("ERROR");
    }

    if (device->getPresentVoltage(&u)) {
        presentVoltage->setText(QString("%1").arg(u));
    } else {
        presentVoltage->setText("ERROR");
    }

    if (device->getPresentSpeed(&speed, &speedDirection)) {
        if (speedDirection==1){
            speedDirectionString=" CW";
        }
        if (speedDirection==0){
            speedDirectionString=" CCW";
        }
        presentSpeed->setText(QString("%1").arg(speed)+speedDirectionString);
    } else {
        presentSpeed->setText("ERROR");
    }

    if (device->getPresentLoad(&load, &direction)) {
        if (direction==1){
            directionString=" CW";
        }
        if (direction==0){
            directionString=" CCW";
        }
        presentLoad->setText(QString("%1").arg(load)+directionString);
    } else {
        presentLoad->setText("ERROR");
    }

/*    float cwAngleLimit = 0; float ccwAngleLimit = 0; //19.01.2014, Richard R: Variabletyp auf float geändert
    if (device->getCcwAngleLimit(&ccwAngleLimit) && device->getCwAngleLimit(&cwAngleLimit)) {
        angleLimit->setText(QString("%1 - %2").arg(cwAngleLimit).arg(ccwAngleLimit));
    } else {
        angleLimit->setText("Error");
    }*/
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

void DynamixelView::onSetCwComplMargin(void){
    if (!device) return;
    device->setCwComplianceMargin(cwComplMargin->text().toInt());
    readCwComplMargin();
}

void DynamixelView::onSetCcwComplMargin(void){
    if (!device) return;
    device->setCcwComplianceMargin(ccwComplMargin->text().toInt());
    readCcwComplMargin();
}
void DynamixelView::onSetCwComplSlope(void){
    if (!device) return;
    device->setCwComplianceSlope(cwComplSlope->text().toInt());
    readCwComplSlope();
}
void DynamixelView::onSetCcwComplSlope(void){
    if (!device) return;
    device->setCcwComplianceSlope(ccwComplSlope->text().toInt());
    readCcwComplSlope();
}

void DynamixelView::onSetAlarmShutdown(void){
    if (!device) return;
    device->setAlarmShutdown(alarmShutdown->text().toInt());
    readAlarmShutdown();
}
