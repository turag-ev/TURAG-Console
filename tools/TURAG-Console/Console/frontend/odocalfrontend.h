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
class QPlainTextEdit;
class QLineEdit;
class QLabel;
class QStateMachine;
class QListWidgetItem;

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

protected:
    TinaInterface *tinaInterface;
    RobotLogFrontend *logview;
    PlainTextFrontend *cmenu;

    QListWidget *parameterHistoryWidget;
    QLabel *paramRadiusLeftText, *paramRadiusRightText, *paramWheelDistanceText;
    QLineEdit *paramRadiusLeft, *paramRadiusRight, *paramWheelDistance;
    QPushButton *addParamBtn;

    QLineEdit *geometryXa, *geometryXb, *geometryW;
    QLabel *geometryImg;
    QLabel *nextActionText;
    QLineEdit *userInputField;
    QPushButton *execActionBtn;

    QPlainTextEdit *odoLogText;

    QStateMachine *odoStateMachine;

protected slots:
    void addParams(void);
    void fetchParam(OdocalParamsListItem*);

private slots:
    void activatedMessage(char, QString message);

};

#endif // ODOCALFRONTEND_H
