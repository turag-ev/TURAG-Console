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

    QTimer* updateTimer;

    QLineEdit* desiredPosition;
    QLineEdit* desiredBaudRate;
    QLineEdit* cwAngleLimit;
    QLineEdit* ccwAngleLimit;
    QLineEdit* movingSpeed;
    QLineEdit* torqueLimit;

    QPushButton* setPosition;
    QPushButton* setBaudRate;
    QPushButton* setCwAngleLimit;
    QPushButton* setCcwAngleLimit;
    QPushButton* setMovingSpeed;
    QPushButton* setTorqueLimit;

 protected slots:
    void onUpdateTimeout(void);
    void onSetPositionPushed(void);
    void onSetBaudRate(void);
    void onSetCwAngleLimit(void);
    void onSetCcwAngleLimit(void);
    void onSetMovingSpeed(void);
    void onSetTorqueLimit(void);

    void readCwAngleLimit(void);
    void readCcwAngleLimit(void);
    void readMovingSpeed(void);
    void readTorqueLimit(void);

public:
    explicit DynamixelView(DynamixelDevice* dev, QWidget *parent = 0);

signals:

public slots:

};

#endif // DYNAMIXELVIEW_H
