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

#include "libs/iconmanager.h"
#include "libs/splitterext.h"

#include "util/tinainterface.h"

#include "logview.h"
#include "plaintextfrontend.h"

OdocalFrontend::OdocalFrontend(QWidget *parent) :
    BaseFrontend("Odometrie-Kalibrierung", IconManager::get("turag-tina"), parent)
{
    /*
     * Add Odocal Menu Buttons:
     * - Last Parameters (List)
     * - Input current parameters manually
     * - After each iteration, copy new parameters to
     *   current parameter
     * - Insert parameter from list?
     * - Warn if no calibrate mode
     * - Input fields for bot geometry
     * - image for bot geometry?
     * - input field for current offset
     *
     * Current LMC features:
     * Set acceleration and velocity
     * Get Pose
     * Get Inc
     * Reset Pose
     * Clean Wheels
     * Set new params
     * Drive 2m
     * Turn (-)pi
     * Wizard
     *
     * Needed LMC features:
     * Get current params
     * Get y pose
     */

    tinaInterface = new TinaInterface(this);
    logview = new RobotLogFrontend(tinaInterface, this);
    cmenu = new PlainTextFrontend(this);

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

    connect(tinaInterface, SIGNAL(cmenuDataReady(QByteArray)), cmenu, SLOT(writeData(QByteArray)));

    // connect outputs of logview and cmenu to own dataReadySignal
    connect(logview, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(cmenu, SIGNAL(dataReady(QByteArray)), this, SIGNAL(dataReady(QByteArray)));
    connect(logview, SIGNAL(activatedMessage(char,QString)), this, SLOT(activatedMessage(char,QString)));
}

void OdocalFrontend::writeData(QByteArray data_) {
    tinaInterface->dataInput(data_);
}

void OdocalFrontend::clear(void) {
    logview->clear();
    cmenu->clear();
    tinaInterface->clear();
}

void OdocalFrontend::onConnected(bool readOnly, QIODevice* dev) {
    logview->onConnected(readOnly, dev);
    cmenu->onConnected(readOnly, dev);
    // TODO: Check if LMC is connected!
    odoLogText->appendPlainText("Connected.");
}

void OdocalFrontend::onDisconnected(bool reconnecting) {
    logview->onDisconnected(reconnecting);
    cmenu->onDisconnected(reconnecting);
    odoLogText->appendPlainText("Disconnected.");
}


void OdocalFrontend::activatedMessage(char,QString message)
{
    /*
    if (message.startsWith(QStringLiteral("Graph")) && message.size() > 6)
    {
        bool ok = false;
        int i = message.mid(6, message.indexOf(':') - 6).toInt(&ok);
        if (ok) {
            tabs->setCurrentIndex(1);
            graphView->activateGraph(i);
        }
    }
    */
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
