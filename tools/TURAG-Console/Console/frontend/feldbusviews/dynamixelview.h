#ifndef DYNAMIXELVIEW_H
#define DYNAMIXELVIEW_H

#include <QWidget>
#include <tina++/feldbus/dynamixel/dynamixeldevice.h>

class QLabel;
class QTimer;
class LineEditExt;
class QPushButton;
class QCheckBox;

using namespace TURAG::Feldbus; // FIXME: evil!


class DynamixelView : public QWidget
{
    Q_OBJECT

public:
    explicit DynamixelView(DynamixelDevice* dev, QWidget *parent = 0);

 protected slots:
    void onUpdateTimeout(void);
    void onSetPositionPushed(void);
    void onSetBaudRate(void);
    void onSetID(void);
    void onSetCwAngleLimit(void);
    void onSetCcwAngleLimit(void);
    void onSetMovingSpeed(void);
    void onSetTorqueLimit(void);
    void onSetCwComplMargin(void);
    void onSetCcwComplMargin(void);
    void onSetCwComplSlope(void);
    void onSetCcwComplSlope(void);
    void onSetAlarmShutdown(void);
    void onSetReturnDelayTime(void);


    void readCwAngleLimit(void);
    void readCcwAngleLimit(void);
    void readMovingSpeed(void);
    void readTorqueLimit(void);
    void readCwComplMargin(void);
    void readCcwComplMargin(void);
    void readCwComplSlope(void);
    void readCcwComplSlope(void);
    void readAlarmShutdown(void);
    void readReturnDelayTime(void);
    void readGoalPosition(void);
    void readTorqueEnable(void);

    void onTorqueEnable(bool state);

protected:
    DynamixelDevice* device;

    QCheckBox* torqueEnable;

    QLabel* presentPosition;
    QLabel* angleLimit;
    QLabel* presentBaudRate;
    QLabel* presentLoad;
    QLabel* presentVoltage;
    QLabel* presentSpeed;
    QLabel* isMoving;
    QLabel* isOverload;
    QLabel* temperature;

    QTimer* updateTimer;

    LineEditExt* desiredPosition;
    LineEditExt* desiredBaudRate;
    LineEditExt* setIdText;
    LineEditExt* cwAngleLimit;
    LineEditExt* ccwAngleLimit;
    LineEditExt* movingSpeed;
    LineEditExt* torqueLimit;
    LineEditExt* cwComplMargin;
    LineEditExt* ccwComplMargin;
    LineEditExt* cwComplSlope;
    LineEditExt* ccwComplSlope;
    LineEditExt* alarmShutdown;
    LineEditExt* returnDelayTime;


    QPushButton* setPosition;
    QPushButton* setBaudRate;
    QPushButton* setIdButton;
    QPushButton* setCwAngleLimit;
    QPushButton* setCcwAngleLimit;
    QPushButton* setMovingSpeed;
    QPushButton* setTorqueLimit;
    QPushButton* setCwComplMargin;
    QPushButton* setCcwComplMargin;
    QPushButton* setCwComplSlope;
    QPushButton* setCcwComplSlope;
    QPushButton* setAlarmShutdown;
    QPushButton* setReturnDelayTime;

};

#endif // DYNAMIXELVIEW_H
