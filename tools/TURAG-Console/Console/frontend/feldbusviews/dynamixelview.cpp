#include "dynamixelview.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QPushButton>
#include <libs/qlineedit2.h>
#include <QCheckBox>
#include <QDebug>

DynamixelView::DynamixelView(DynamixelDevice* dev, QWidget *parent) :
    QWidget(parent), device(dev)
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    torqueEnable = new QCheckBox("Motor aktiviert");
    layout->addWidget(torqueEnable);
    connect(torqueEnable, SIGNAL(toggled(bool)), this, SLOT(onTorqueEnable(bool)));

    QGridLayout* read_layout = new QGridLayout;
    layout->addLayout(read_layout);

    //Present Position
    QLabel* presentPositionLabel = new QLabel("Present Position [°]");
    presentPosition = new QLabel;
    read_layout->addWidget(presentPositionLabel, 0 , 0);
    read_layout->addWidget(presentPosition, 0 , 1);

    //Present Speed
    QLabel* presentSpeedLabel = new QLabel("Present Speed [rpm]");
    presentSpeed = new QLabel;
    read_layout->addWidget(presentSpeedLabel, 1, 0);
    read_layout->addWidget(presentSpeed, 1, 1);

    //Present Voltage
    QLabel* presentVoltageLabel = new QLabel("Present Voltage [V]");
    presentVoltage = new QLabel;
    read_layout->addWidget(presentVoltageLabel, 2, 0);
    read_layout->addWidget(presentVoltage, 2, 1);

    //Present Load
    QLabel* presentLoadLabel = new QLabel("Present Load [% of max]");
    presentLoad = new QLabel;
    read_layout->addWidget(presentLoadLabel, 3, 0);
    read_layout->addWidget(presentLoad, 3, 1);

    //Moving
    QLabel* isMovingLabel = new QLabel("is Moving");
    isMoving = new QLabel;
    read_layout->addWidget(isMovingLabel, 4, 0);
    read_layout->addWidget(isMoving, 4, 1);

    //Overload
    QLabel* isOverloadLabel = new QLabel("is Overload");
    isOverload = new QLabel;
    read_layout->addWidget(isOverloadLabel, 5, 0);
    read_layout->addWidget(isOverload, 5, 1);

    // Temperature
    QLabel* temperatureLabel = new QLabel("Temperature [° C]");
    temperature = new QLabel;
    read_layout->addWidget(temperatureLabel, 6, 0);
    read_layout->addWidget(temperature, 6, 1);



    QGridLayout* gridLayout = new QGridLayout;
    layout->addLayout(gridLayout);

    //Desired Position
    QLabel* desiredPositionLabel = new QLabel("Desired Position [°]");
    desiredPosition = new QLineEdit2;
    setPosition = new QPushButton("Set");
    gridLayout-> addWidget(desiredPositionLabel, 0, 0);
    gridLayout->addWidget(desiredPosition, 0, 1);
    gridLayout->addWidget(setPosition, 0 , 2);
    connect(setPosition, SIGNAL(clicked()), this, SLOT(onSetPositionPushed()));
    connect(desiredPosition, SIGNAL(returnPressed()), this, SLOT(onSetPositionPushed()));

    //Baudrate
    QLabel* desiredBaudRateLabel = new QLabel ("Baudrate");
    desiredBaudRate= new QLineEdit2;
    setBaudRate = new QPushButton ("Set");
    gridLayout ->addWidget(desiredBaudRateLabel, 1, 0);
    gridLayout ->addWidget(desiredBaudRate, 1, 1);
    gridLayout ->addWidget(setBaudRate, 1, 2);
    connect(setBaudRate, SIGNAL(clicked()), this, SLOT(onSetBaudRate()));
    connect(desiredBaudRate, SIGNAL(returnPressed()), this, SLOT(onSetBaudRate()));

    //CW AngleLimit
    QLabel* cwAngleLimitLabel = new QLabel ("CW Angle Limit [°]");
    cwAngleLimit= new QLineEdit2;
    setCwAngleLimit = new QPushButton ("Set");
    gridLayout ->addWidget(cwAngleLimitLabel, 2, 0);
    gridLayout ->addWidget(cwAngleLimit, 2, 1);
    gridLayout ->addWidget(setCwAngleLimit, 2, 2);
    connect(setCwAngleLimit, SIGNAL(clicked()), this, SLOT(onSetCwAngleLimit()));
    connect(cwAngleLimit, SIGNAL(returnPressed()), this, SLOT(onSetCwAngleLimit()));

    //CcW AngleLimit
    QLabel* ccwAngleLimitLabel = new QLabel ("CCW Angle Limit [°]");
    ccwAngleLimit= new QLineEdit2;
    setCcwAngleLimit = new QPushButton ("Set");
    gridLayout ->addWidget(ccwAngleLimitLabel, 3, 0);
    gridLayout ->addWidget(ccwAngleLimit, 3, 1);
    gridLayout ->addWidget(setCcwAngleLimit, 3, 2);
    connect(setCcwAngleLimit, SIGNAL(clicked()), this, SLOT(onSetCcwAngleLimit()));
    connect(ccwAngleLimit, SIGNAL(returnPressed()), this, SLOT(onSetCcwAngleLimit()));

    //MovingSpeed
    QLabel* movingSpeedLabel = new QLabel ("Moving Speed (0-1023)");
    movingSpeed= new QLineEdit2;
    setMovingSpeed = new QPushButton ("Set");
    gridLayout ->addWidget(movingSpeedLabel, 4, 0);
    gridLayout ->addWidget(movingSpeed, 4, 1);
    gridLayout ->addWidget(setMovingSpeed, 4, 2);
    connect(setMovingSpeed, SIGNAL(clicked()), this, SLOT(onSetMovingSpeed()));
    connect(movingSpeed, SIGNAL(returnPressed()), this, SLOT(onSetMovingSpeed()));

    //TorqueLimit
    QLabel* torqueLimitLabel = new QLabel ("Torque Limit [%]");
    torqueLimit= new QLineEdit2;
    setTorqueLimit= new QPushButton ("Set");
    gridLayout ->addWidget(torqueLimitLabel, 5, 0);
    gridLayout ->addWidget(torqueLimit, 5, 1);
    gridLayout ->addWidget(setTorqueLimit, 5, 2);
    connect(setTorqueLimit, SIGNAL(clicked()), this, SLOT(onSetTorqueLimit()));
    connect(torqueLimit, SIGNAL(returnPressed()), this, SLOT(onSetTorqueLimit()));

    //Cw Compl Margin
    QLabel* cwComplMarginLabel = new QLabel ("Cw Compl Margin");
    cwComplMargin= new QLineEdit2;
    setCwComplMargin= new QPushButton ("Set");
    gridLayout ->addWidget(cwComplMarginLabel, 6, 0);
    gridLayout ->addWidget(cwComplMargin, 6, 1);
    gridLayout ->addWidget(setCwComplMargin, 6, 2);
    connect(setCwComplMargin, SIGNAL(clicked()), this, SLOT(onSetCwComplMargin()));
    connect(cwComplMargin, SIGNAL(returnPressed()), this, SLOT(onSetCwComplMargin()));

    //Ccw Compl Margin
    QLabel* ccwComplMarginLabel = new QLabel ("Ccw Compl Margin");
    ccwComplMargin= new QLineEdit2;
    setCcwComplMargin= new QPushButton ("Set");
    gridLayout ->addWidget(ccwComplMarginLabel, 7, 0);
    gridLayout ->addWidget(ccwComplMargin, 7, 1);
    gridLayout ->addWidget(setCcwComplMargin, 7, 2);
    connect(setCcwComplMargin, SIGNAL(clicked()), this, SLOT(onSetCcwComplMargin()));
    connect(ccwComplMargin, SIGNAL(returnPressed()), this, SLOT(onSetCcwComplMargin()));

    //Cw Compl Slope
    QLabel* cwComplSlopeLabel = new QLabel ("Cw Compl Slope");
    cwComplSlope= new QLineEdit2;
    setCwComplSlope= new QPushButton ("Set");
    gridLayout ->addWidget(cwComplSlopeLabel, 8, 0);
    gridLayout ->addWidget(cwComplSlope, 8, 1);
    gridLayout ->addWidget(setCwComplSlope, 8, 2);
    connect(setCwComplSlope, SIGNAL(clicked()), this, SLOT(onSetCwComplSlope()));
    connect(cwComplSlope, SIGNAL(returnPressed()), this, SLOT(onSetCwComplSlope()));

    //Ccw Compl Slope
    QLabel* ccwComplSlopeLabel = new QLabel ("Ccw Compl Slope");
    ccwComplSlope= new QLineEdit2;
    setCcwComplSlope= new QPushButton ("Set");
    gridLayout ->addWidget(ccwComplSlopeLabel, 9, 0);
    gridLayout ->addWidget(ccwComplSlope, 9, 1);
    gridLayout ->addWidget(setCcwComplSlope, 9, 2);
    connect(setCcwComplSlope, SIGNAL(clicked()), this, SLOT(onSetCcwComplSlope()));
    connect(ccwComplSlope, SIGNAL(returnPressed()), this, SLOT(onSetCcwComplSlope()));

    //Alarm Shutdown
    QLabel* alarmShutdownLabel = new QLabel ("Alarm Shutdown");
    alarmShutdown= new QLineEdit2;
    setAlarmShutdown= new QPushButton ("Set");
    gridLayout ->addWidget(alarmShutdownLabel, 10, 0);
    gridLayout ->addWidget(alarmShutdown, 10, 1);
    gridLayout ->addWidget(setAlarmShutdown, 10, 2);
    connect(setAlarmShutdown, SIGNAL(clicked()), this, SLOT(onSetAlarmShutdown()));
    connect(alarmShutdown, SIGNAL(returnPressed()), this, SLOT(onSetAlarmShutdown()));

    //Return delay time
    QLabel* returnDelayLabel = new QLabel ("Return Delay Time (0 - 508) [us]");
    returnDelayTime= new QLineEdit2;
    setReturnDelayTime= new QPushButton ("Set");
    gridLayout ->addWidget(returnDelayLabel, 11, 0);
    gridLayout ->addWidget(returnDelayTime, 11, 1);
    gridLayout ->addWidget(setReturnDelayTime, 11, 2);
    connect(setReturnDelayTime, SIGNAL(clicked()), this, SLOT(onSetReturnDelayTime()));
    connect(returnDelayTime, SIGNAL(returnPressed()), this, SLOT(onSetReturnDelayTime()));

    //Baudrate@init
    float currentBaudRate = 0;
    if (device && device->getBaudRate(&currentBaudRate)){
      desiredBaudRate->setText(QString("%1").arg(currentBaudRate));
    } else{
        desiredBaudRate->setText("Error. Unable to read BaudRate");
    }

    readGoalPosition();

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

    readReturnDelayTime();
    readTorqueEnable();

    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimeout()));
//    updateTimer->start(500);
}

void DynamixelView::readAlarmShutdown(void){
    int currentSetting = 0;
    if (device && device->getAlarmShutdown(&currentSetting)){
        alarmShutdown->setText(QString("%1").arg(currentSetting));
    } else {
        alarmShutdown->setText("Error. Unable to read.");
    }
}

void DynamixelView::readReturnDelayTime(void) {
    int currentSetting = 0;
    if (device && device->getReturnDelayTime(&currentSetting)){
        returnDelayTime->setText(QString("%1").arg(currentSetting));
    } else {
        returnDelayTime->setText("Error. Unable to read.");
    }
}

void DynamixelView::readGoalPosition(void) {
    float currentSetting = 0;
    if (device && device->getGoalPosition(&currentSetting)){
        desiredPosition->setText(QString("%1").arg(currentSetting));
    } else {
        desiredPosition->setText("Error. Unable to read.");
    }
}

void DynamixelView::readTorqueEnable(void) {
    bool currentSetting = false;
    if (device && device->getTorqueEnable(&currentSetting)){
        torqueEnable->setChecked(currentSetting);
    } else {
        torqueEnable->setChecked(false);
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

    int temperature_value = 0;
    if (device->getTemperature(&temperature_value)) {
        temperature->setText(QString("%1").arg(temperature_value));
    } else {
        temperature->setText("ERROR");
    }
}
void DynamixelView::onSetTorqueLimit(void) {
    if (!device) return;
    device->setTorqueLimit(torqueLimit->text().toInt());
    readTorqueLimit();
}

void DynamixelView::onSetPositionPushed(void) {
    if (!device) return;

    device->setGoalPosition(desiredPosition->text().toFloat());
    readGoalPosition();
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

void DynamixelView::onSetReturnDelayTime(void) {
    if (!device) return;
    device->setReturnDelayTime(returnDelayTime->text().toInt());
    readReturnDelayTime();
}

void DynamixelView::onTorqueEnable(bool state) {
    if (!device) return;
    device->setTorqueEnable(state);
    readTorqueEnable();
}
