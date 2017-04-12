#include "odocalfrontend.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QFormLayout>
#include <QStateMachine>
#include <QQueue>
#include <QTimer>

#include "libs/iconmanager.h"
#include "libs/splitterext.h"
#include "libs/lineeditext.h"

#include "util/tinainterface.h"

#include "logview.h"
#include "plaintextfrontend.h"

OdocalFrontend::OdocalFrontend(QWidget *parent) :
    BaseFrontend("Odometrie-Kalibrierung", IconManager::get("turag-tina"), parent)
{
    tinaInterface = new TinaInterface(this);
    logview = new RobotLogFrontend(tinaInterface, this);
    cmenu = new PlainTextFrontend(this);

    connect(tinaInterface, SIGNAL(cmenuDataReady(QByteArray)), cmenu, SLOT(writeData(QByteArray)));
    connect(logview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(cmenu, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));

    // === START LAYOUT ===
    QVBoxLayout* layout = new QVBoxLayout;
    SplitterExt* mainsplitter = new SplitterExt(Qt::Vertical, "odocal_vsplit", this);

    // Top like TinA Robot view, use even the same name
    SplitterExt* topsplitter = new SplitterExt(Qt::Horizontal, "robotFrontend_splitter", mainsplitter);
    topsplitter->addWidget(logview);
    topsplitter->addWidget(cmenu);
    topsplitter->restoreState();
    mainsplitter->addWidget(topsplitter);

    // New Odocal Interface
    SplitterExt* odosplitter = new SplitterExt(Qt::Horizontal, "odocal_odosplit", mainsplitter);

    // left column
    QWidget* odoleftcontainer = new QWidget(odosplitter);
    QVBoxLayout* odoleftlayout = new QVBoxLayout;

    parameterHistoryWidget = new QListWidget(odoleftcontainer);
    connect(parameterHistoryWidget, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem *item) { this->fetchParam(reinterpret_cast<OdocalParamsListItem*>(item)); });
    odoleftlayout->addWidget(parameterHistoryWidget);

    QWidget* odoParamContainer = new QWidget(odoleftcontainer);
    QGridLayout* odoParamLayout = new QGridLayout;

    paramRadiusLeftText = new QLabel("Wheel radius left (mm)", odoParamContainer);
    odoParamLayout->addWidget(paramRadiusLeftText, 0, 0);
    paramRadiusLeft = new QLineEdit(odoParamContainer);
    odoParamLayout->addWidget(paramRadiusLeft, 1, 0);

    paramRadiusLeftText = new QLabel("Wheel radius right (mm)", odoParamContainer);
    odoParamLayout->addWidget(paramRadiusLeftText, 0, 1);
    paramRadiusRight = new QLineEdit(odoParamContainer);
    odoParamLayout->addWidget(paramRadiusRight, 1, 1);

    paramRadiusLeftText = new QLabel("Wheel distance (mm)", odoParamContainer);
    odoParamLayout->addWidget(paramRadiusLeftText, 0, 2);
    paramWheelDistance = new QLineEdit(odoParamContainer);
    odoParamLayout->addWidget(paramWheelDistance, 1, 2);

    addParamBtn = new QPushButton("Add", odoParamContainer);
    connect(addParamBtn, &QPushButton::clicked, this, &OdocalFrontend::addParams);
    odoParamLayout->addWidget(addParamBtn, 1, 4);

    odoParamContainer->setLayout(odoParamLayout);
    odoleftlayout->addWidget(odoParamContainer);

    odoleftcontainer->setLayout(odoleftlayout);
    odosplitter->addWidget(odoleftcontainer);

    // middle column
    QWidget* odomiddlecontainer = new QWidget(odosplitter);
    QVBoxLayout* odomiddlelayout = new QVBoxLayout;

    QWidget* geometryContainer = new QWidget(odomiddlecontainer);
    QFormLayout* geometryLayout = new QFormLayout;
    geometryMx = new LineEditExt("odocal_geometryMx", "0", true, geometryContainer);
    geometryLayout->addRow("m_x (in mm)", geometryMx);
    geometryMy = new LineEditExt("odocal_geometryMy", "0", true, geometryContainer);
    geometryLayout->addRow("m_y (in mm)", geometryMy);
    geometryW = new LineEditExt("odocal_geometryW", "0", true, geometryContainer);
    geometryLayout->addRow("w (in mm)", geometryW);
    geometryContainer->setLayout(geometryLayout);
    odomiddlelayout->addWidget(geometryContainer);

    nextActionText = new QLabel("", odomiddlecontainer);
    odomiddlelayout->addWidget(nextActionText);

    QWidget *enterDisplacementContainer = new QWidget(odomiddlecontainer);
    QFormLayout *enterDisplacementLayout = new QFormLayout;
    geometryYa = new QLineEdit(odomiddlecontainer);
    enterDisplacementLayout->addRow("y_a (in mm)", geometryYa);
    geometryYb = new QLineEdit(odomiddlecontainer);
    enterDisplacementLayout->addRow("y_b (in mm)", geometryYb);
    enterDisplacementContainer->setLayout(enterDisplacementLayout);
    odomiddlelayout->addWidget(enterDisplacementContainer);
    enterDisplacementContainer->setEnabled(false);

    execActionBtn = new QPushButton("Connect Robot first!", odomiddlecontainer);
    execActionBtn->setEnabled(false);
    odomiddlelayout->addWidget(execActionBtn);

    odomiddlecontainer->setLayout(odomiddlelayout);
    odosplitter->addWidget(odomiddlecontainer);

    // right column
    QWidget* odorightcontainer = new QWidget(odosplitter);
    QVBoxLayout* odorightlayout = new QVBoxLayout;

    odoLogText = new QPlainTextEdit(odorightcontainer);
    odoLogText->setReadOnly(true);
    odorightlayout->addWidget(odoLogText);

    odorightcontainer->setLayout(odorightlayout);
    odosplitter->addWidget(odorightcontainer);

    // extra column
    QWidget *odoExtraContainer = new QWidget(odosplitter);
    QVBoxLayout *odoExtraLayout = new QVBoxLayout;

    setRobotSlowButton = new QPushButton("Set robot slow", odoExtraContainer);
    connect(setRobotSlowButton, &QPushButton::clicked, this, [this](){ setRobotSlow(); });
    odoExtraLayout->addWidget(setRobotSlowButton);
    releaseRobotWheelsButton = new QPushButton("Release robot wheels", odoExtraContainer);
    connect(releaseRobotWheelsButton, &QPushButton::clicked, this, [this](){ releaseRobotWheels(); });
    odoExtraLayout->addWidget(releaseRobotWheelsButton);
    driveRobotForwardButton = new QPushButton("Drive robot Forward", odoExtraContainer);
    connect(driveRobotForwardButton, &QPushButton::clicked, this, [this](){ driveRobotForward(); });
    odoExtraLayout->addWidget(driveRobotForwardButton);
    turnRobotPositiveButton = new QPushButton("Turn robot +PI", odoExtraContainer);
    connect(turnRobotPositiveButton, &QPushButton::clicked, this, [this](){ turnRobotPositive(); });
    odoExtraLayout->addWidget(turnRobotPositiveButton);
    turnRobotNegativeButton = new QPushButton("Turn robot -PI", odoExtraContainer);
    connect(turnRobotNegativeButton, &QPushButton::clicked, this, [this](){ turnRobotNegative(); });
    odoExtraLayout->addWidget(turnRobotNegativeButton);
    resetRobotPoseButton = new QPushButton("Reset robot pose", odoExtraContainer);
    connect(resetRobotPoseButton, &QPushButton::clicked, this, [this](){ resetRobotPose(); });
    odoExtraLayout->addWidget(resetRobotPoseButton);
    setRobotParamsButton = new QPushButton("Set robot params", odoExtraContainer);
    connect(setRobotParamsButton, &QPushButton::clicked, this, [this](){ setRobotParams(0.0, 0.0, 0.0); });
    odoExtraLayout->addWidget(setRobotParamsButton);
    getRobotCalibrationModeButton = new QPushButton("Get calibration mode", odoExtraContainer);
    connect(getRobotCalibrationModeButton, &QPushButton::clicked, this, [this](){ getRobotCalibrationMode(); });
    odoExtraLayout->addWidget(getRobotCalibrationModeButton);
    getRobotLeftWheelRadiusButton = new QPushButton("Get left wheel radius", odoExtraContainer);
    connect(getRobotLeftWheelRadiusButton, &QPushButton::clicked, this, [this](){ getRobotLeftWheelRadius(); });
    odoExtraLayout->addWidget(getRobotLeftWheelRadiusButton);
    getRobotRightWheelRadiusButton = new QPushButton("Get right wheel radius", odoExtraContainer);
    connect(getRobotRightWheelRadiusButton, &QPushButton::clicked, this, [this](){ getRobotRightWheelRadius(); });
    odoExtraLayout->addWidget(getRobotRightWheelRadiusButton);
    getRobotWheelDistanceButton = new QPushButton("Get wheel distance", odoExtraContainer);
    connect(getRobotWheelDistanceButton, &QPushButton::clicked, this, [this](){ getRobotWheelDistance(); });
    odoExtraLayout->addWidget(getRobotWheelDistanceButton);
    getRobotYPositionButton = new QPushButton("Get y position", odoExtraContainer);
    connect(getRobotYPositionButton, &QPushButton::clicked, this, [this](){ getRobotYPosition(); });
    odoExtraLayout->addWidget(getRobotYPositionButton);

    odoExtraContainer->setLayout(odoExtraLayout);
    odosplitter->addWidget(odoExtraContainer);

    odosplitter->restoreState();
    mainsplitter->addWidget(odosplitter);

    mainsplitter->restoreState();
    layout->addWidget(mainsplitter);
    setLayout(layout);
    // === END LAYOUT ===


    // Init CMenu keystroke queue
    cmenuKeystrokes = new QQueue<Keystroke>;
    // Cmenu delay until next keystroke
    cmenuDelayTimer = new QTimer(this);
    cmenuDelayTimer->setSingleShot(true);
    cmenuDelayTimer->setInterval(50);
    connect(cmenuDelayTimer, &QTimer::timeout, this, &OdocalFrontend::sendNextCmenuKeystroke);


    // Init statemachine!
    odoStateMachine = new QStateMachine;
    waitForUserStart = new QState;
    pushToStart1 = new QState;
    measureYBeforeDrive1 = new QState;
    driveRoute1 = new QState;
    measureYAfterDrive1 = new QState;
    measureDisplacement1 = new QState;
    pushToStart2 = new QState;
    measureYBeforeDrive2 = new QState;
    driveRoute2 = new QState;
    measureYAfterDrive2 = new QState;
    measureDisplacement2 = new QState;

    // Wait for user to initiate start
    waitForUserStart->assignProperty(nextActionText, "text", "Press Start to begin!");
    waitForUserStart->assignProperty(enterDisplacementContainer, "enabled", false);
    waitForUserStart->assignProperty(execActionBtn, "text", "Start!");
    waitForUserStart->assignProperty(execActionBtn, "enabled", true);
    waitForUserStart->addTransition(execActionBtn, &QPushButton::clicked, pushToStart1);
    odoStateMachine->addState(waitForUserStart);

    // Push to start
    pushToStart1->assignProperty(nextActionText, "text", "Placen robot correctly!");
    pushToStart1->assignProperty(enterDisplacementContainer, "enabled", false);
    pushToStart1->assignProperty(execActionBtn, "text", "Done!");
    pushToStart1->assignProperty(execActionBtn, "enabled", true);
    pushToStart1->addTransition(execActionBtn, &QPushButton::clicked, measureYBeforeDrive1);
    connect(pushToStart1, &QState::entered, this, &OdocalFrontend::releaseRobotWheels);
    odoStateMachine->addState(pushToStart1);

    // Measure y before drive
    measureYBeforeDrive1->assignProperty(nextActionText, "text", "Measuring Y from Bot ...");
    measureYBeforeDrive1->assignProperty(enterDisplacementContainer, "enabled", false);
    measureYBeforeDrive1->assignProperty(execActionBtn, "text", "Waiting for Cmenu response.");
    measureYBeforeDrive1->assignProperty(execActionBtn, "enabled", false);
    measureYBeforeDrive1->addTransition(new CmenuResponseTransition(driveRoute1));
    connect(measureYBeforeDrive1, &QState::entered, this, &OdocalFrontend::getRobotYPosition);
    connect(measureYBeforeDrive1, &QState::exited, this, [this](){
        yBeforeDrive1 = QString(lastCmenuResponse).toDouble();
        odoLogText->appendPlainText(QString("Robot thinks he's at y=%1mm.").arg(yBeforeDrive1));
    });
    odoStateMachine->addState(measureYBeforeDrive1);

    // Drive route
    driveRoute1->assignProperty(nextActionText, "text", "Please drive the Robot via CMenu control!\nAutomated driving is not implemented yet.");
    driveRoute1->assignProperty(enterDisplacementContainer, "enabled", false);
    driveRoute1->assignProperty(execActionBtn, "text", "Done!");
    driveRoute1->assignProperty(execActionBtn, "enabled", true);
    driveRoute1->addTransition(execActionBtn, &QPushButton::clicked, measureYAfterDrive1);
    odoStateMachine->addState(driveRoute1);

    // TODO: Insert States while Driving

    // Measure y after drive
    measureYAfterDrive1->assignProperty(nextActionText, "text", "Measuring Y from Bot ...");
    measureYAfterDrive1->assignProperty(enterDisplacementContainer, "enabled", false);
    measureYAfterDrive1->assignProperty(execActionBtn, "text", "Waiting for CMenu response.");
    measureYAfterDrive1->assignProperty(execActionBtn, "enabled", false);
    measureYAfterDrive1->addTransition(new CmenuResponseTransition(measureDisplacement1));
    connect(measureYAfterDrive1, &QState::entered, this, &OdocalFrontend::getRobotYPosition);
    connect(measureYAfterDrive1, &QState::exited, this, [this](){
        yAfterDrive1 = QString(lastCmenuResponse).toDouble();
        odoLogText->appendPlainText(QString("Robot thinks he's at y=%1mm.").arg(yAfterDrive1));
    });
    odoStateMachine->addState(measureYAfterDrive1);

    // Measure displacement
    measureDisplacement1->assignProperty(nextActionText, "text", "Measure the y offset from the bot to the start position!\nThen enter the value in the field below!");
    measureDisplacement1->assignProperty(enterDisplacementContainer, "enabled", true);
    measureDisplacement1->assignProperty(execActionBtn, "text", "Save value");
    measureDisplacement1->assignProperty(execActionBtn, "enabled", true);
    measureDisplacement1->addTransition(execActionBtn, &QPushButton::clicked, pushToStart2);
    connect(measureDisplacement1, &QState::exited, this, [this](){
        double m_x = geometryMx->text().toDouble();
        double m_y = geometryMy->text().toDouble();
        double w = geometryW->text().toDouble();
        double y_a = geometryYa->text().toDouble();
        double y_b = geometryYb->text().toDouble();
        yDisplacement1 = calculateYDisplacement(y_a, y_b, m_x, m_y, w);
        odoLogText->appendPlainText(QString("But in fact, he is at y=%1mm.").arg(yDisplacement1));
    });
    odoStateMachine->addState(measureDisplacement1);

    // Push to start
    pushToStart2->assignProperty(nextActionText, "text", "Placen robot correctly!");
    pushToStart2->assignProperty(enterDisplacementContainer, "enabled", false);
    pushToStart2->assignProperty(execActionBtn, "text", "Done!");
    pushToStart2->assignProperty(execActionBtn, "enabled", true);
    pushToStart2->addTransition(execActionBtn, &QPushButton::clicked, measureYBeforeDrive2);
    connect(pushToStart2, &QState::entered, this, &OdocalFrontend::releaseRobotWheels);
    odoStateMachine->addState(pushToStart2);

    // Measure y before drive
    measureYBeforeDrive2->assignProperty(nextActionText, "text", "Measuring Y from Bot ...");
    measureYBeforeDrive2->assignProperty(enterDisplacementContainer, "enabled", false);
    measureYBeforeDrive2->assignProperty(execActionBtn, "text", "Done!");
    measureYBeforeDrive2->assignProperty(execActionBtn, "enabled", true);
    measureYBeforeDrive2->addTransition(new CmenuResponseTransition(driveRoute2));
    connect(measureYBeforeDrive2, &QState::entered, this, &OdocalFrontend::getRobotYPosition);
    connect(measureYBeforeDrive2, &QState::exited, this, [this](){
        yBeforeDrive2 = QString(lastCmenuResponse).toDouble();
        odoLogText->appendPlainText(QString("Robot thinks he's at y=%1mm.").arg(yBeforeDrive2));
    });
    odoStateMachine->addState(measureYBeforeDrive2);

    // Drive route
    driveRoute2->assignProperty(nextActionText, "text", "Please drive the Robot via CMenu control!\nAutomated driving is not implemented yet.");
    driveRoute2->assignProperty(enterDisplacementContainer, "enabled", false);
    driveRoute2->assignProperty(execActionBtn, "text", "Done!");
    driveRoute2->assignProperty(execActionBtn, "enabled", true);
    driveRoute2->addTransition(execActionBtn, &QPushButton::clicked, measureYAfterDrive2);
    odoStateMachine->addState(driveRoute2);

    // TODO: Insert States while Driving

    // Measure y after drive
    measureYAfterDrive2->assignProperty(nextActionText, "text", "Measuring Y from Bot ...");
    measureYAfterDrive2->assignProperty(enterDisplacementContainer, "enabled", false);
    measureYAfterDrive2->assignProperty(execActionBtn, "text", "Done!");
    measureYAfterDrive2->assignProperty(execActionBtn, "enabled", true);
    measureYAfterDrive2->addTransition(new CmenuResponseTransition(measureDisplacement2));
    connect(measureYAfterDrive2, &QState::entered, this, &OdocalFrontend::getRobotYPosition);
    connect(measureYAfterDrive2, &QState::exited, this, [this](){
        yAfterDrive2 = QString(lastCmenuResponse).toDouble();
        odoLogText->appendPlainText(QString("Robot thinks he's at y=%1mm.").arg(yAfterDrive2));
    });
    odoStateMachine->addState(measureYAfterDrive2);

    // Measure displacement
    measureDisplacement2->assignProperty(nextActionText, "text", "Measure the y offset from the bot to the start position!\nThen enter the value in the field below!");
    measureDisplacement2->assignProperty(enterDisplacementContainer, "enabled", true);
    measureDisplacement2->assignProperty(execActionBtn, "text", "Save value");
    measureDisplacement2->assignProperty(execActionBtn, "enabled", true);
    measureDisplacement2->addTransition(execActionBtn, &QPushButton::clicked, pushToStart1);
    connect(measureDisplacement2, &QState::exited, this, [this](){
        double m_x = geometryMx->text().toDouble();
        double m_y = geometryMy->text().toDouble();
        double w = geometryW->text().toDouble();
        double y_a = geometryYa->text().toDouble();
        double y_b = geometryYb->text().toDouble();
        yDisplacement2 = calculateYDisplacement(y_a, y_b, m_x, m_y, w);
        odoLogText->appendPlainText(QString("But in fact, he is at y=%1mm.").arg(yDisplacement2));

        // Feed calculator with information
        OdocalParams params = {
            paramRadiusLeft->text().toDouble(),
            paramRadiusRight->text().toDouble(),
            paramWheelDistance->text().toDouble() / 2.0
        };
        double yError1 = yDisplacement1 - yBeforeDrive1 - yAfterDrive1;
        double yError2 = yDisplacement2 - yBeforeDrive2 - yAfterDrive2;
        double length = 1200.0;
        int count = 1;
        OdocalParams newParams = calculateNewParameters(params, length, count, yError1, yError2);
        odoLogText->appendPlainText(QString("corrected values: rl=%1 (radius) rr=%2 (radius) 2*a=%3 (wheel distance)")
                                .arg(newParams.rl).arg(newParams.rr).arg(newParams.a*2.0));

        // Add new parameter to list and input fields
        parameterHistoryWidget->addItem(new OdocalParamsListItem(newParams, parameterHistoryWidget));
        paramRadiusLeft->setText(QString::number(params.rl));
        paramRadiusRight->setText(QString::number(params.rr));
        paramWheelDistance->setText(QString::number(params.a * 2.0));
    });
    odoStateMachine->addState(measureDisplacement2);

    odoStateMachine->setInitialState(waitForUserStart);
}

void OdocalFrontend::writeData(QByteArray data_)
{
    tinaInterface->dataInput(data_);
}

void OdocalFrontend::clear(void)
{
    logview->clear();
    cmenu->clear();
    tinaInterface->clear();
    odoLogText->clear();
}

void OdocalFrontend::onConnected(bool readOnly, QIODevice* dev)
{
    logview->onConnected(readOnly, dev);
    cmenu->onConnected(readOnly, dev);
    odoStateMachine->start();
    odoLogText->appendPlainText("Connected. Let's hope we have connected to an LMC in calibrate mode.");
}

void OdocalFrontend::onDisconnected(bool reconnecting)
{
    logview->onDisconnected(reconnecting);
    cmenu->onDisconnected(reconnecting);
    odoStateMachine->stop();
    odoLogText->appendPlainText("Disconnected.");
}

void OdocalFrontend::addParams(void)
{
    double rl = paramRadiusLeft->text().toDouble();
    double rr = paramRadiusRight->text().toDouble();
    double wd = paramWheelDistance->text().toDouble();
    OdocalParamsListItem *newParams = new OdocalParamsListItem({rl, rr, wd/2}, parameterHistoryWidget);
    parameterHistoryWidget->addItem(newParams);
}

void OdocalFrontend::fetchParam(OdocalParamsListItem *item)
{
    paramRadiusLeft->setText(QString::number(item->rl()));
    paramRadiusRight->setText(QString::number(item->rr()));
    paramWheelDistance->setText(QString::number(item->cd()));
}

QString OdocalFrontend::charToString(char c)
{
    QString res = "0x" + QString::number(c, 16);
    if (QChar(c).isPrint()) {
        res += " (" + QString(QChar(c)) + ")";
    }
    return res;
}

/*
 * Do not call this function multiple times in a row without ensuring
 * that the last keystrokes have all finished, since there is only one
 * global keystroke query!
 * TODO: Fix this.
 */
void OdocalFrontend::sendCmenuKeystrokes(QList<Keystroke> keystrokes)
{
    if (cmenuKeystrokes->empty()) {
        QString printList = "Appending keystrokes: ";
        for (Keystroke keystroke : keystrokes) {
            printList += charToString(keystroke.data) + ", ";
            cmenuKeystrokes->enqueue(keystroke);
        }
        //odoLogText->appendPlainText(printList);
        connect(tinaInterface, &TinaInterface::cmenuDataReady, this, &OdocalFrontend::fetchCmenuResponse);
        sendNextCmenuKeystroke();
    }
}

void OdocalFrontend::fetchCmenuResponse(QByteArray response)
{
    if (keepCmenuResponse) {
        lastCmenuResponse = response;
        /*
        odoLogText->appendPlainText("=== BEGIN RESPONSE ===");
        odoLogText->appendPlainText(response);
        odoLogText->appendPlainText("=== END RESPONSE ===");
        */
    }

    cmenuDelayTimer->start();
}

void OdocalFrontend::sendNextCmenuKeystroke(void)
{

    if (!cmenuKeystrokes->empty()) {
        Keystroke keystroke = cmenuKeystrokes->dequeue();
        keepCmenuResponse = keystroke.keepResponse;

        //QString keepStr = keepCmenuResponse ? " (keep)" : "";
        //odoLogText->appendPlainText("Sending keystroke " + charToString(keystroke.data) + keepStr);

        emit(dataReady(QByteArray().append(keystroke.data)));
    } else {
        odoStateMachine->postEvent(new CmenuResponseEvent(lastCmenuResponse));

        keepCmenuResponse = false;
        disconnect(tinaInterface, &TinaInterface::cmenuDataReady, this, &OdocalFrontend::fetchCmenuResponse);
    }
}

void OdocalFrontend::setRobotSlow(void)
{
    sendCmenuKeystrokes({{'3', false}, {'V', false}, {'0', false}, {'\r', false},
                         {'X', false}, {'0', false}, {'\r', false}, {'\x1b', false}});
}

void OdocalFrontend::releaseRobotWheels(void)
{
    sendCmenuKeystrokes({{'3', false}, {'r', false}, {'\x1b', false}});
}

void OdocalFrontend::driveRobotForward(void)
{
    sendCmenuKeystrokes({{'5', false}, {'f', false}, {'\x1b', false}});
}

void OdocalFrontend::turnRobotPositive(void)
{
    sendCmenuKeystrokes({{'5', false}, {'x', false}, {'\x1b', false}});
}

void OdocalFrontend::turnRobotNegative(void)
{
    sendCmenuKeystrokes({{'5', false}, {'c', false}, {'\x1b', false}});
}

void OdocalFrontend::resetRobotPose(void)
{
    sendCmenuKeystrokes({{'5', false}, {'z', false}, {'\x1b', false}});
}

void OdocalFrontend::getRobotCalibrationMode(void)
{
    sendCmenuKeystrokes({{'5', false}, {'m', true}, {'\x1b', false}});
}

void OdocalFrontend::setRobotParams(double rl, double rr, double wd)
{
    sendCmenuKeystrokes({{'5', false}, {'S', false}, {'\x1b', false}});
}

void OdocalFrontend::getRobotYPosition(void)
{
    sendCmenuKeystrokes({{'5', false}, {'y', true}, {'\x1b', false}});
}

void OdocalFrontend::getRobotLeftWheelRadius(void)
{
    sendCmenuKeystrokes({{'5', false}, {'l', true}, {'\x1b', false}});
}

void OdocalFrontend::getRobotRightWheelRadius(void)
{
    sendCmenuKeystrokes({{'5', false}, {'r', true}, {'\x1b', false}});
}

void OdocalFrontend::getRobotWheelDistance(void)
{
    sendCmenuKeystrokes({{'5', false}, {'d', true}, {'\x1b', false}});
}

OdocalFrontend::OdocalParams OdocalFrontend::calculateNewParameters(
        OdocalParams param, double length, int count, double yError1, double yError2)
{
    // Mainly copied from old odocal/src/main.cpp

    //path to drive positive
    LegacyOdocal::Pose<LegacyOdocal::AD<double,3> > p;
    for(int i=0; i < 2*count; ++i) {
        p+=pose_forward(length,param);
        p+=pose_turn(M_PI,param);
    }
    //path to drive negative
    LegacyOdocal::Pose<LegacyOdocal::AD<double,3> >m;
    for(int i=0; i < 2*count; ++i) {
        m+=pose_forward(length,param);
        m+=pose_turn(-M_PI,param);
    }

    //solve the system while leaving overall estimated path size constant
    double det_inv=1.0 / ( p.y()[1]*m.y()[2] - p.y()[2]*m.y()[1] - p.y()[0]*m.y()[2] + p.y()[2]*m.y()[0] );
    double dl=det_inv*( -m.y()[2]*yError1 + p.y()[2]*yError2 );
    double dr=det_inv*( m.y()[2]*yError1 - p.y()[2]*yError2 );
    double da=det_inv*( (m.y()[0]-m.y()[1])*yError1 + (p.y()[1]-p.y()[0])*yError2 );

    param.rl+=dl;
    param.rr+=dr;
    param.a+=da;

    return param;
}
