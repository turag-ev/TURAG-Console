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

#include "libs/iconmanager.h"
#include "libs/splitterext.h"

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

    paramRadiusLeftText = new QLabel("Wheel radius left (in mm)", odoParamContainer);
    odoParamLayout->addWidget(paramRadiusLeftText, 0, 0);
    paramRadiusLeft = new QLineEdit(odoParamContainer);
    odoParamLayout->addWidget(paramRadiusLeft, 1, 0);

    paramRadiusLeftText = new QLabel("Wheel radius right (in mm)", odoParamContainer);
    odoParamLayout->addWidget(paramRadiusLeftText, 0, 1);
    paramRadiusRight = new QLineEdit(odoParamContainer);
    odoParamLayout->addWidget(paramRadiusRight, 1, 1);

    paramRadiusLeftText = new QLabel("Wheel distance (in mm)", odoParamContainer);
    odoParamLayout->addWidget(paramRadiusLeftText, 0, 2);
    paramWheelDistance = new QLineEdit(odoParamContainer);
    odoParamLayout->addWidget(paramWheelDistance, 1, 2);

    addParamBtn = new QPushButton("Param. hinzuf.", odoParamContainer);
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
    geometryXa = new QLineEdit(geometryContainer);
    geometryLayout->addRow("x_a (in mm)", geometryXa);
    geometryXb = new QLineEdit(geometryContainer);
    geometryLayout->addRow("x_b (in mm)", geometryXb);
    geometryW = new QLineEdit(geometryContainer);
    geometryLayout->addRow("w (in mm)", geometryW);
    geometryContainer->setLayout(geometryLayout);
    odomiddlelayout->addWidget(geometryContainer);

    nextActionText = new QLabel("Bitte Roboter verbinden!", odomiddlecontainer);
    odomiddlelayout->addWidget(nextActionText);

    userInputField = new QLineEdit(odomiddlecontainer);
    userInputField->setEnabled(false);
    odomiddlelayout->addWidget(userInputField);

    execActionBtn = new QPushButton("Execute Action!", odomiddlecontainer);
    //execActionBtn->setEnabled(true);
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

    odosplitter->restoreState();
    mainsplitter->addWidget(odosplitter);

    mainsplitter->restoreState();
    layout->addWidget(mainsplitter);
    setLayout(layout);

    // Init CMenu keystroke queue
    cmenuKeystrokes = new QQueue<QByteArray>;

    // Test stuff
    connect(execActionBtn, &QPushButton::clicked, this, [this](){this->setRobotSlow();} );

    // Init statemachine!
    odoStateMachine = new QStateMachine;
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

    // Push to start
    // TODO: Release wheels on enter!
    pushToStart1->assignProperty(nextActionText, "text", "Placen robot correctly!");
    pushToStart1->assignProperty(userInputField, "enabled", false);
    pushToStart1->assignProperty(execActionBtn, "text", "Start!");
    pushToStart1->assignProperty(execActionBtn, "enabled", true);
    pushToStart1->addTransition(execActionBtn, &QPushButton::clicked, measureYBeforeDrive1);
    connect(pushToStart1, &QState::entered, this, &OdocalFrontend::releaseRobotWheels);
    odoStateMachine->addState(pushToStart1);

    // Measure y before drive
    measureYBeforeDrive1->assignProperty(nextActionText, "text", "Measuring Y from Bot ...");
    measureYBeforeDrive1->assignProperty(userInputField, "enabled", false);
    measureYBeforeDrive1->assignProperty(execActionBtn, "text", "Waiting for Cmenu response.");
    measureYBeforeDrive1->assignProperty(execActionBtn, "enabled", false);
    //measureYBeforeDrive1->addTransition(execActionBtn, &QPushButton::clicked, driveRoute1);
    measureYBeforeDrive1->addTransition(new CmenuResponseTransition(driveRoute1));
    connect(measureYBeforeDrive1, &QState::entered, this, &OdocalFrontend::getRobotYPosition);
    connect(measureYBeforeDrive1, &QState::exited, this, [this](){
        yBeforeDrive1 = QString(lastCmenuResponse).toDouble();
        odoLogText->appendPlainText(QString("Robot thinks he's at y=%1mm.").arg(yBeforeDrive1));
    });
    odoStateMachine->addState(measureYBeforeDrive1);

    // Drive route
    driveRoute1->assignProperty(nextActionText, "text", "Bot will drive a few miles.");
    driveRoute1->assignProperty(userInputField, "enabled", false);
    driveRoute1->assignProperty(execActionBtn, "text", "Start!");
    driveRoute1->assignProperty(execActionBtn, "enabled", true);
    driveRoute1->addTransition(execActionBtn, &QPushButton::clicked, measureYAfterDrive1);
    odoStateMachine->addState(driveRoute1);

    // TODO: Insert States while Driving

    // Measure y after drive
    measureYAfterDrive1->assignProperty(nextActionText, "text", "Measuring Y from Bot ...");
    measureYAfterDrive1->assignProperty(userInputField, "enabled", false);
    measureYAfterDrive1->assignProperty(execActionBtn, "text", "Waiting for Cmenu response.");
    measureYAfterDrive1->assignProperty(execActionBtn, "enabled", false);
    //measureYAfterDrive1->addTransition(execActionBtn, &QPushButton::clicked, measureDisplacement1);
    measureYAfterDrive1->addTransition(new CmenuResponseTransition(measureDisplacement1));
    connect(measureYAfterDrive1, &QState::entered, this, &OdocalFrontend::getRobotYPosition);
    connect(measureYAfterDrive1, &QState::exited, this, [this](){
        yAfterDrive1 = QString(lastCmenuResponse).toDouble();
        odoLogText->appendPlainText(QString("Robot thinks he's at y=%1mm.").arg(yAfterDrive1));
    });
    odoStateMachine->addState(measureYAfterDrive1);

    // Measure displacement
    measureDisplacement1->assignProperty(nextActionText, "text", "Measure the y offset from the bot to the start position!\nThen enter the value in the field below!");
    measureDisplacement1->assignProperty(userInputField, "enabled", true);
    measureDisplacement1->assignProperty(execActionBtn, "text", "Save value");
    measureDisplacement1->assignProperty(execActionBtn, "enabled", true);
    measureDisplacement1->addTransition(execActionBtn, &QPushButton::clicked, pushToStart2);
    // enter function: set focus
    // leave function: save value to variable!
    odoStateMachine->addState(measureDisplacement1);

    // Push to start
    // TODO: Release wheels on enter!
    pushToStart2->assignProperty(nextActionText, "text", "Placen robot correctly!");
    pushToStart2->assignProperty(userInputField, "enabled", false);
    pushToStart2->assignProperty(execActionBtn, "text", "Start!");
    pushToStart2->assignProperty(execActionBtn, "enabled", true);
    pushToStart2->addTransition(execActionBtn, &QPushButton::clicked, measureYBeforeDrive2);
    odoStateMachine->addState(pushToStart2);

    // Measure y before drive
    measureYBeforeDrive2->assignProperty(nextActionText, "text", "Measuring Y from Bot ...");
    measureYBeforeDrive2->assignProperty(userInputField, "enabled", false);
    measureYBeforeDrive2->assignProperty(execActionBtn, "text", "Done!");
    measureYBeforeDrive2->assignProperty(execActionBtn, "enabled", true);
    // TODO: Use cmenu answer event and disable button!
    measureYBeforeDrive2->addTransition(execActionBtn, &QPushButton::clicked, driveRoute2);
    odoStateMachine->addState(measureYBeforeDrive2);

    // Drive route
    driveRoute2->assignProperty(nextActionText, "text", "Bot will drive a few miles.");
    driveRoute2->assignProperty(userInputField, "enabled", false);
    driveRoute2->assignProperty(execActionBtn, "text", "Start!");
    driveRoute2->assignProperty(execActionBtn, "enabled", true);
    driveRoute2->addTransition(execActionBtn, &QPushButton::clicked, measureYAfterDrive2);
    odoStateMachine->addState(driveRoute2);

    // TODO: Insert States while Driving

    // Measure y after drive
    measureYAfterDrive2->assignProperty(nextActionText, "text", "Measuring Y from Bot ...");
    measureYAfterDrive2->assignProperty(userInputField, "enabled", false);
    measureYAfterDrive2->assignProperty(execActionBtn, "text", "Done!");
    measureYAfterDrive2->assignProperty(execActionBtn, "enabled", true);
    // TODO: Use cmenu answer event and disable button!
    measureYAfterDrive2->addTransition(execActionBtn, &QPushButton::clicked, measureDisplacement2);
    odoStateMachine->addState(measureYAfterDrive2);

    // Measure displacement
    measureDisplacement2->assignProperty(nextActionText, "text", "Measure the y offset from the bot to the start position!\nThen enter the value in the field below!");
    measureDisplacement2->assignProperty(userInputField, "enabled", true);
    measureDisplacement2->assignProperty(execActionBtn, "text", "Save value");
    measureDisplacement2->assignProperty(execActionBtn, "enabled", true);
    measureDisplacement2->addTransition(execActionBtn, &QPushButton::clicked, pushToStart1);
    // Leave function: Save value to variable!
    odoStateMachine->addState(measureDisplacement2);

    odoStateMachine->setInitialState(pushToStart1);
    odoStateMachine->start();
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
}

void OdocalFrontend::onConnected(bool readOnly, QIODevice* dev)
{
    logview->onConnected(readOnly, dev);
    cmenu->onConnected(readOnly, dev);
    // TODO: Check if LMC is connected!
    // TODO: Check if LMC is in calibrate mode!
    odoLogText->appendPlainText("Connected.");
}

void OdocalFrontend::onDisconnected(bool reconnecting)
{
    logview->onDisconnected(reconnecting);
    cmenu->onDisconnected(reconnecting);
    odoLogText->appendPlainText("Disconnected.");
}

void OdocalFrontend::addParams(void)
{
    double rl = paramRadiusLeft->text().toDouble();
    double rr = paramRadiusRight->text().toDouble();
    double wd = paramWheelDistance->text().toDouble();
    OdocalParamsListItem *newParams = new OdocalParamsListItem(rl, rr, wd, parameterHistoryWidget);
    parameterHistoryWidget->addItem(newParams);
}

void OdocalFrontend::fetchParam(OdocalParamsListItem *item)
{
    paramRadiusLeft->setText(QString::number(item->getRadiusLeft()));
    paramRadiusRight->setText(QString::number(item->getRadiusRight()));
    paramWheelDistance->setText(QString::number(item->getWheelDistance()));
}

/*
 * Do not call this function multiple times in a row without ensuring
 * that the last keystrokes have all finished, since there is only one
 * global keystroke query!
 * TODO: Fix this.
 */
void OdocalFrontend::sendCmenuKeystrokes(QList<QByteArray> keystrokes)
{
    for (QByteArray keystroke : keystrokes) {
        cmenuKeystrokes->enqueue(keystroke);
    }
    connect(tinaInterface, &TinaInterface::cmenuDataReady, this, &OdocalFrontend::sendNextCmenuKeystroke);
    sendNextCmenuKeystroke();
}

void OdocalFrontend::sendNextCmenuKeystroke(QByteArray response)
{
    if (!cmenuKeystrokes->empty()) {
        QByteArray keystroke = cmenuKeystrokes->dequeue();
        emit(dataReady(keystroke));
    } else {
        lastCmenuResponse = &response;
        odoStateMachine->postEvent(new CmenuResponseEvent(response));
        disconnect(tinaInterface, &TinaInterface::cmenuDataReady, this, &OdocalFrontend::sendNextCmenuKeystroke);
    }
}

void OdocalFrontend::setRobotSlow(void)
{
    sendCmenuKeystrokes({"3", "V", "0", "0", "\r", "X", "0", "0", "\r", "\x1b"});
}

void OdocalFrontend::releaseRobotWheels(void)
{
    sendCmenuKeystrokes({"r"});
}

void OdocalFrontend::driveRobotForward(void)
{
    sendCmenuKeystrokes({"5", "f", "\x1b"});
}

void OdocalFrontend::turnRobotPositive(void)
{
    sendCmenuKeystrokes({"5", "x", "\x1b"});
}

void OdocalFrontend::turnRobotNegative(void)
{
    sendCmenuKeystrokes({"5", "c", "\x1b"});
}

void OdocalFrontend::resetRobotPose(void)
{
    sendCmenuKeystrokes({"5", "z", "\x1b"});
}

bool OdocalFrontend::getRobotCalibrationMode(void)
{
    sendCmenuKeystrokes({"5", "m", "\x1b"});
    return false;
}

void OdocalFrontend::setRobotParams(double rl, double rr, double wd)
{
    sendCmenuKeystrokes({"5", "S", "\x1b"});
}

double OdocalFrontend::getRobotYPosition(void)
{
    sendCmenuKeystrokes({"5", "y", "\x1b"});
    return 0.0;
}

double OdocalFrontend::getRobotLeftWheelRadius(void)
{
    sendCmenuKeystrokes({"5", "l", "\x1b"});
    return 0.0;
}

double OdocalFrontend::getRobotRightWheelRadius(void)
{
    sendCmenuKeystrokes({"5", "r", "\x1b"});
    return 0.0;
}

double OdocalFrontend::getRobotWheelDistance(void)
{
    sendCmenuKeystrokes({"5", "d", "\x1b"});
    return 0.0;
}
