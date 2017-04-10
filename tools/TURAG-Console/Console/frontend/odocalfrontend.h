#ifndef ODOCALFRONTEND_H
#define ODOCALFRONTEND_H

#include "basefrontend.h"
#include "tina++/units.h"
#include <QListWidgetItem>

class TinaInterface;
class PlainTextFrontend;
class RobotLogFrontend;

class QPushButton;
class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
class QLineEdit;
class QLabel;
class QStateMachine;
class QState;

class OdocalParamsListItem : public QListWidgetItem
{
public:
    OdocalParamsListItem(double rl, double rr, double wd, QListWidget *view_ = Q_NULLPTR) :
        QListWidgetItem(view_, UserType), rl_(rl), rr_(rr), wd_(wd)
    {
        this->setText(QString("[%1, %2, %3]").arg(rl_).arg(rr_).arg(wd_));
    }

    double getRadiusLeft(void) { return rl_; }
    double getRadiusRight(void) { return rr_; }
    double getWheelDistance(void) { return wd_; }
protected:
    double rl_, rr_, wd_;
};

class OdocalFrontend : public BaseFrontend
{
    Q_OBJECT
public:
    explicit OdocalFrontend(QWidget *parent = 0);

public slots:
    void writeData(QByteArray data) override;
    void clear(void) override;

    void onConnected(bool readOnly, QIODevice*) override;
    void onDisconnected(bool reconnecting) override;

signals:
    void cmenuDataAvailable(QByteArray data);

protected:
    TinaInterface *tinaInterface;
    RobotLogFrontend *logview;
    PlainTextFrontend *cmenu;

    // Left column
    QListWidget *parameterHistoryWidget;
    QLabel *paramRadiusLeftText, *paramRadiusRightText, *paramWheelDistanceText;
    QLineEdit *paramRadiusLeft, *paramRadiusRight, *paramWheelDistance;
    QPushButton *addParamBtn;

    // Middle column
    QLineEdit *geometryXa, *geometryXb, *geometryW;
    QLabel *geometryImg;
    QLabel *nextActionText;
    QLineEdit *userInputField;
    QPushButton *execActionBtn;

    // Right column
    QPlainTextEdit *odoLogText;

    // Odocal statemachine
    QStateMachine *odoStateMachine;
    QState *pushToStart1, *measureYBeforeDrive1, *driveRoute1, *measureYAfterDrive1,
        *measureDisplacement1, *pushToStart2, *measureYBeforeDrive2, *driveRoute2,
        *measureYAfterDrive2, *measureDisplacement2;

    // Cmenu control stuff
    QQueue<QByteArray> *cmenuKeystrokes;
    QByteArray *lastCmenuResponse;

    void sendCmenuKeystrokes(QList<QByteArray> keystrokes);
    void sendNextCmenuKeystroke(QByteArray response = 0);

    // Helpers using cmenu
    void setRobotSlow(void);
    void driveRobotForward(void);
    void turnRobotPositive(void);
    void turnRobotNegative(void);
    void resetRobotPose(void);
    void setRobotParams(double rl, double rr, double wd);
    bool getRobotCalibrationMode(void);
    double getRobotLeftWheelRadius(void);
    double getRobotRightWheelRadius(void);
    double getRobotWheelDistance(void);
    double getRobotYPosition(void);

protected slots:
    void addParams(void);
    void fetchParam(OdocalParamsListItem*);
};

#endif // ODOCALFRONTEND_H
