#ifndef ODOCALFRONTEND_H
#define ODOCALFRONTEND_H

#include "basefrontend.h"
#include "cmath"

#include <QListWidgetItem>
#include <QEvent>
#include <QAbstractTransition>

#include "libs/legacyOdocal/ad.h"
#include "libs/legacyOdocal/pose.h"

class TinaInterface;
class PlainTextFrontend;
class RobotLogFrontend;
class LineEditExt;

class QPushButton;
class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
class QLineEdit;
class QLabel;
class QStateMachine;
class QState;
class QAbstractTransition;
class QTimer;


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
    /// Struct for storing an Odocal parameter set.
    struct OdocalParams {
        double rl, rr, a;
    };

    /// Class for storing OdocalParams in a QListWidgetItem
    class OdocalParamsListItem : public QListWidgetItem
    {
    public:
        OdocalParamsListItem(OdocalParams params, QListWidget *view_ = Q_NULLPTR) :
            QListWidgetItem(view_, UserType), params_(params)
        {
            this->setText(QString("[%1, %2, %3]").arg(params.rl).arg(params.rr).arg(2*params.a));
        }

        OdocalParams params() { return params_; }
        double rl(void) { return params_.rl; }
        double rr(void) { return params_.rr; }
        double cd(void) { return 2*params_.a; }
        double a(void) {return params_.a; }
    protected:
        OdocalParams params_;
    };

    /// Struct for storing a keystroke for CMenu interaction
    struct Keystroke
    {
        /// Byte to send to Cmenu
        char data;
        /// Wheter to keep the response
        bool keepResponse;
    };

    /// Event, fired when there is a Cmenu response available
    struct CmenuResponseEvent : public QEvent
    {
    public:
        static constexpr QEvent::Type TYPE = QEvent::Type(QEvent::User + 1);

        CmenuResponseEvent(QByteArray response) :
            QEvent(TYPE), response_(response)
        {}

        QByteArray getResponse(void) { return response_; }

    protected:
        // Currently saved here for no reason and unused. :(
        QByteArray response_;
    };

    /// Transition for QStatemachine using a CMenuResponseEvent
    class CmenuResponseTransition : public QAbstractTransition
    {
        //Q_OBJECT

    public:
        CmenuResponseTransition(QAbstractState *target_)
        {
            setTargetState(target_);
        }

    protected:
        virtual bool eventTest(QEvent *event) override {
            return event->type() == CmenuResponseEvent::TYPE;
        }

        virtual void onTransition(QEvent *event) override {
            //
        }
    };

    // Standard TinA and CMenu frontend
    TinaInterface *tinaInterface;
    RobotLogFrontend *logview;
    PlainTextFrontend *cmenu;

    // Left column
    QListWidget *parameterHistoryWidget;
    QLabel *paramRadiusLeftText, *paramRadiusRightText, *paramWheelDistanceText;
    QLineEdit *paramRadiusLeft, *paramRadiusRight, *paramWheelDistance;
    QPushButton *addParamBtn;

    // Middle column
    LineEditExt *geometryMx, *geometryMy, *geometryW;
    //QLabel *geometryImg; // TODO: Insert explaining image
    QLabel *nextActionText;
    QLineEdit *geometryYa, *geometryYb;
    QPushButton *execActionBtn;

    // Right column
    QPlainTextEdit *odoLogText;

    // Extra column
    QPushButton *setRobotSlowButton;
    QPushButton *releaseRobotWheelsButton;
    QPushButton *driveRobotForwardButton;
    QPushButton *turnRobotPositiveButton;
    QPushButton *turnRobotNegativeButton;
    QPushButton *resetRobotPoseButton;
    QPushButton *setRobotParamsButton;
    QPushButton *getRobotCalibrationModeButton;
    QPushButton *getRobotLeftWheelRadiusButton;
    QPushButton *getRobotRightWheelRadiusButton;
    QPushButton *getRobotWheelDistanceButton;
    QPushButton *getRobotYPositionButton;

    // Odocal statemachine
    QStateMachine *odoStateMachine;
    QState *waitForUserStart, *pushToStart1, *measureYBeforeDrive1, *driveRoute1, *measureYAfterDrive1, *measureDisplacement1,
        *pushToStart2, *measureYBeforeDrive2, *driveRoute2, *measureYAfterDrive2, *measureDisplacement2;

    // Measured variables
    double yBeforeDrive1, yAfterDrive1, yDisplacement1;
    double yBeforeDrive2, yAfterDrive2, yDisplacement2;

    // Cmenu control stuff
    QQueue<Keystroke> *cmenuKeystrokes;
    bool keepCmenuResponse;
    QByteArray lastCmenuResponse;
    QTimer *cmenuDelayTimer;

    QString charToString(char c);
    void sendCmenuKeystrokes(QList<Keystroke> keystrokes);
    void fetchCmenuResponse(QByteArray response);
    void sendNextCmenuKeystroke(void);

    // Helpers using cmenu
    void setRobotSlow(void);
    void releaseRobotWheels(void);
    void driveRobotForward(void);
    void turnRobotPositive(void);
    void turnRobotNegative(void);
    void resetRobotPose(void);
    void setRobotParams(OdocalParams params);
    void getRobotCalibrationMode(void);
    void getRobotLeftWheelRadius(void);
    void getRobotRightWheelRadius(void);
    void getRobotWheelDistance(void);
    void getRobotYPosition(void);

    // Calculation stuff
    inline static LegacyOdocal::Pose<LegacyOdocal::AD<double, 3>> pose_forward(double L, const OdocalParams& param)
    {
        return LegacyOdocal::Pose<LegacyOdocal::AD<double, 3> >(
            LegacyOdocal::AD<double,3>( L, 3, (double)(.5/param.rl*L), (double)(.5/param.rr*L), (double)(0) ),
            LegacyOdocal::AD<double,3>( 0, 3, (double)(-.5/param.rl/param.a*L), (double)(.5/param.rr/param.a*L), (double)(0) ) );
    }

    inline static LegacyOdocal::Pose<LegacyOdocal::AD<double, 3>> pose_turn(double phi, const OdocalParams& param)
    {
        return LegacyOdocal::Pose<LegacyOdocal::AD<double, 3> >(
            LegacyOdocal::AD<double,3>( 0, 3, (double)(-.5/param.rl*param.a*phi), (double)(.5/param.rr*param.a*phi), (double)(0) ),
            LegacyOdocal::AD<double,3>( phi, 3, (double)(.5/param.rl*phi), (double)(.5/param.rr*phi), (double)(-1.0/param.a*phi) ) );
    }

    inline static double calculateYDisplacement(double y_a, double y_b, double m_x, double m_y, double w)
    {
        return (y_a + sqrt(m_x*m_x + m_y*m_y) * (y_b - y_a) / w);
    }

    OdocalParams calculateNewParameters(OdocalParams param, double length, int count, double yError1, double yError2);

protected slots:
    void addParams(void);
    void fetchParam(OdocalParamsListItem*);
};

#endif // ODOCALFRONTEND_H
