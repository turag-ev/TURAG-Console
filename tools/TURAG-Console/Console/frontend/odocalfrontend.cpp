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
    geometryMx = new QLineEdit(geometryContainer);
    geometryLayout->addRow("m_x (in mm)", geometryMx);
    geometryMy = new QLineEdit(geometryContainer);
    geometryLayout->addRow("m_y (in mm)", geometryMy);
    geometryW = new QLineEdit(geometryContainer);
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

    odosplitter->restoreState();
    mainsplitter->addWidget(odosplitter);

    mainsplitter->restoreState();
    layout->addWidget(mainsplitter);
    setLayout(layout);
    // === END LAYOUT ===

    // Init CMenu keystroke queue
    cmenuKeystrokes = new QQueue<Keystroke>;

    // Test stuff
    //connect(execActionBtn, &QPushButton::clicked, this, [this](){this->setRobotSlow();} );

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
    pushToStart1->assignProperty(nextActionText, "text", "Placen robot correctly!");
    pushToStart1->assignProperty(enterDisplacementContainer, "enabled", false);
    pushToStart1->assignProperty(execActionBtn, "text", "Start!");
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
    driveRoute2->assignProperty(execActionBtn, "text", "Start!");
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

        calculateNewParameters();
    });
    odoStateMachine->addState(measureDisplacement2);

    odoStateMachine->setInitialState(pushToStart1);

    // For testing purposes
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
    odoLogText->clear();
}

void OdocalFrontend::onConnected(bool readOnly, QIODevice* dev)
{
    logview->onConnected(readOnly, dev);
    cmenu->onConnected(readOnly, dev);
    odoStateMachine->start();
    // TODO: Check if LMC is connected!
    // TODO: Check if LMC is in calibrate mode!
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

/*
 * Do not call this function multiple times in a row without ensuring
 * that the last keystrokes have all finished, since there is only one
 * global keystroke query!
 * TODO: Fix this.
 */
void OdocalFrontend::sendCmenuKeystrokes(QList<Keystroke> keystrokes)
{
    /* For testing purposes
    odoLogText->appendPlainText("=== BEGIN KEYSTROKES ===");
    for (Keystroke keystroke : keystrokes) {
        cmenuKeystrokes->enqueue(keystroke);
    }
    connect(tinaInterface, &TinaInterface::cmenuDataReady, this, &OdocalFrontend::sendNextCmenuKeystroke);
    sendNextCmenuKeystroke();
    */
    lastCmenuResponse = "2.5746";
    odoStateMachine->postEvent(new CmenuResponseEvent(lastCmenuResponse));
}

void OdocalFrontend::sendNextCmenuKeystroke(QByteArray response)
{
    static bool keepResponse;

    if (!cmenuKeystrokes->empty()) {
        Keystroke keystroke = cmenuKeystrokes->dequeue();
        keepResponse = keystroke.keepResponse;

        odoLogText->appendPlainText(QString("Sending Keystroke (%1):").arg(keystroke.keepResponse));
        odoLogText->appendPlainText(keystroke.data);

        emit(dataReady(keystroke.data));
    } else {
        if (keepResponse) {
            lastCmenuResponse = response;

            odoLogText->appendPlainText("=== BEGIN RESPONSE ===");
            odoLogText->appendPlainText(response);
            odoLogText->appendPlainText("=== END RESPONSE ===");
        }
        odoStateMachine->postEvent(new CmenuResponseEvent(lastCmenuResponse));

        keepResponse = false;
        disconnect(tinaInterface, &TinaInterface::cmenuDataReady, this, &OdocalFrontend::sendNextCmenuKeystroke);
    }
}

void OdocalFrontend::setRobotSlow(void)
{
    sendCmenuKeystrokes({{"3", false}, {"V", false}, {"0", false}, {"0", false}, {"\r", false},
                         {"X", false}, {"0", false}, {"0", false}, {"\r", false}, {"\x1b", false}});
}

void OdocalFrontend::releaseRobotWheels(void)
{
    sendCmenuKeystrokes({{"r", false}});
}

void OdocalFrontend::driveRobotForward(void)
{
    sendCmenuKeystrokes({{"5", false}, {"f", false}, {"\x1b", false}});
}

void OdocalFrontend::turnRobotPositive(void)
{
    sendCmenuKeystrokes({{"5", false}, {"x", false}, {"\x1b", false}});
}

void OdocalFrontend::turnRobotNegative(void)
{
    sendCmenuKeystrokes({{"5", false}, {"c", false}, {"\x1b", false}});
}

void OdocalFrontend::resetRobotPose(void)
{
    sendCmenuKeystrokes({{"5", false}, {"z", false}, {"\x1b", false}});
}

void OdocalFrontend::getRobotCalibrationMode(void)
{
    sendCmenuKeystrokes({{"5", false}, {"m", true}, {"\x1b", false}});
}

void OdocalFrontend::setRobotParams(double rl, double rr, double wd)
{
    sendCmenuKeystrokes({{"5", false}, {"S", false}, {"\x1b", false}});
}

void OdocalFrontend::getRobotYPosition(void)
{
    sendCmenuKeystrokes({{"5", false}, {"y", true}, {"\x1b", false}});
}

void OdocalFrontend::getRobotLeftWheelRadius(void)
{
    sendCmenuKeystrokes({{"5", false}, {"l", true}, {"\x1b", false}});
}

void OdocalFrontend::getRobotRightWheelRadius(void)
{
    sendCmenuKeystrokes({{"5", false}, {"r", true}, {"\x1b", false}});
}

void OdocalFrontend::getRobotWheelDistance(void)
{
    sendCmenuKeystrokes({{"5", false}, {"d", true}, {"\x1b", false}});
}

void OdocalFrontend::calculateNewParameters(void)
{
    // Mainly copied from old odocal/src/main.cpp

    OdocalParams param = {
        paramRadiusLeft->text().toDouble(),
        paramRadiusRight->text().toDouble(),
        paramWheelDistance->text().toDouble() / 2.0
    };

    // TODO: Read from QLineEdit
    double L = 1200.0;
    int cnt = 1;

    //path to drive positive
    LegacyOdocal::Pose<LegacyOdocal::AD<double,3> > p;
    for(int i=0;i<2*cnt;++i){
        p+=pose_forward(L,param);
        p+=pose_turn(M_PI,param);
    }
    //path to drive negative
    LegacyOdocal::Pose<LegacyOdocal::AD<double,3> >m;
    for(int i=0;i<2*cnt;++i){
        m+=pose_forward(L,param);
        m+=pose_turn(-M_PI,param);
    }

    double dyp = yDisplacement1 - yBeforeDrive1 - yAfterDrive1;
    double dym = yDisplacement2 - yBeforeDrive2 - yAfterDrive2;

    //solve the system while leaving overall estimated path size constant
    double det_inv=1.0 / ( p.y()[1]*m.y()[2] - p.y()[2]*m.y()[1] - p.y()[0]*m.y()[2] + p.y()[2]*m.y()[0] );
    double dl=det_inv*( -m.y()[2]*dyp + p.y()[2]*dym );
    double dr=det_inv*( m.y()[2]*dyp - p.y()[2]*dym );
    double da=det_inv*( (m.y()[0]-m.y()[1])*dyp + (p.y()[1]-p.y()[0])*dym );

    param.rl+=dl;
    param.rr+=dr;
    param.a+=da;

    odoLogText->appendPlainText(QString("corrected values: rl=%1 (radius) rr=%2 (radius) 2*a=%3 (wheel distance)")
                                .arg(param.rl).arg(param.rr).arg(param.a*2.0));
}
