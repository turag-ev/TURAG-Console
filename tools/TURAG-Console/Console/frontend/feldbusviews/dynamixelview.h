#ifndef DYNAMIXELVIEW_H
#define DYNAMIXELVIEW_H

#include <QWidget>
#include <tina++/feldbus/dynamixel/dynamixeldevice.h>

class QLabel;
class QTimer;
class QLineEdit;
class QPushButton;

using namespace TURAG::Feldbus;


class DynamixelView : public QWidget
{
    Q_OBJECT

protected:
    DynamixelDevice* device;

    QLabel* presentPosition;
    QLabel* angleLimit;
    QLabel* presentBaudRate;
    QLabel* presentLoad;
    QLabel* presentVoltage;
    QLabel* presentSpeed;
    QLabel* isMoving;
    QLabel* isOverload;

    QTimer* updateTimer;

    QLineEdit* desiredPosition;
    QLineEdit* desiredBaudRate;
    QLineEdit* cwAngleLimit;
    QLineEdit* ccwAngleLimit;
    QLineEdit* movingSpeed;
    QLineEdit* torqueLimit;
    QLineEdit* cwComplMargin;
    QLineEdit* ccwComplMargin;
    QLineEdit* cwComplSlope;
    QLineEdit* ccwComplSlope;
    QLineEdit* alarmShutdown;


    QPushButton* setPosition;
    QPushButton* setBaudRate;
    QPushButton* setCwAngleLimit;
    QPushButton* setCcwAngleLimit;
    QPushButton* setMovingSpeed;
    QPushButton* setTorqueLimit;
    QPushButton* setCwComplMargin;
    QPushButton* setCcwComplMargin;
    QPushButton* setCwComplSlope;
    QPushButton* setCcwComplSlope;
    QPushButton* setAlarmShutdown;

 protected slots:
    void onUpdateTimeout(void);
    void onSetPositionPushed(void);
    void onSetBaudRate(void);
    void onSetCwAngleLimit(void);
    void onSetCcwAngleLimit(void);
    void onSetMovingSpeed(void);
    void onSetTorqueLimit(void);
    void onSetCwComplMargin(void);
    void onSetCcwComplMargin(void);
    void onSetCwComplSlope(void);
    void onSetCcwComplSlope(void);
    void onSetAlarmShutdown(void);


    void readCwAngleLimit(void);
    void readCcwAngleLimit(void);
    void readMovingSpeed(void);
    void readTorqueLimit(void);
    void readCwComplMargin(void);
    void readCcwComplMargin(void);
    void readCwComplSlope(void);
    void readCcwComplSlope(void);
    void readAlarmShutdown(void);

public:
    explicit DynamixelView(DynamixelDevice* dev, QWidget *parent = 0);

signals:

public slots:

};

#endif // DYNAMIXELVIEW_H
