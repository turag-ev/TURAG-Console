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
    QTimer* updateTimer;
    QLineEdit* desiredPosition;
    QPushButton* setPosition;

 protected slots:
    void onUpdateTimeout(void);
    void onSetPositionPushed(void);

public:
    explicit DynamixelView(DynamixelDevice* dev, QWidget *parent = 0);

signals:

public slots:

};

#endif // DYNAMIXELVIEW_H
