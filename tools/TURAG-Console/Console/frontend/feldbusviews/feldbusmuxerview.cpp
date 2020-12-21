#include "feldbusmuxerview.h"


#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>



using namespace TURAG::Feldbus;

void FeldbusMuxerView::setReadOnly(QLineEdit* lineEedit) {
    QPalette pal = lineEedit->palette();
    QColor clr = this->palette().color(QPalette::Window);
    pal.setColor(QPalette::Active, QPalette::Base, clr);
    pal.setColor(QPalette::Inactive, QPalette::Base, clr);
    lineEedit->setPalette(pal);
}


FeldbusMuxerView::FeldbusMuxerView(Muxer_64_32 *muxer_, QWidget *parent) :
    QWidget(parent), muxer(muxer_)
{
    if (!muxer->intitialize()) {
        QVBoxLayout* layout = new QVBoxLayout;
        QLabel* title = new QLabel("<h1>64-32-Muxer: initialization failed</h1>");
        layout->addWidget(title);
        setLayout(layout);
        return;
    }

    QVBoxLayout* layout = new QVBoxLayout;
    QLabel* title = new QLabel("<h1>64-32-Muxer</h1>");
    layout->addWidget(title);

    QGridLayout* grid = new QGridLayout;
    outputEnabled = new QPushButton("output OFF");
    outputEnabled->setCheckable(true);
    connect(outputEnabled, &QPushButton::clicked, this, &FeldbusMuxerView::setOutput);
    grid->addWidget(outputEnabled, 0, 0, 1, 2);
    QLabel* label = new QLabel("trigger mode");
    grid->addWidget(label, 1, 0);
    triggerModeEdit = new QLineEdit;
    setReadOnly(triggerModeEdit);
    grid->addWidget(triggerModeEdit, 1, 1);
    triggerModeComboBox = new QComboBox;
    triggerModeComboBox->addItem("NoTrigger");
    triggerModeComboBox->addItem("FallingEdge");
    triggerModeComboBox->addItem("RisingEdge");
    triggerModeComboBox->addItem("BothEdges");
    grid->addWidget(triggerModeComboBox, 2, 0);
    QPushButton* button = new QPushButton("Set trigger mode");
    connect(button, &QPushButton::pressed, this, &FeldbusMuxerView::setTriggerMode);
    grid->addWidget(button, 2, 1);
    QGroupBox* groupbox = new QGroupBox("global output and trigger configuration");
    groupbox->setLayout(grid);
    layout->addWidget(groupbox);

    grid = new QGridLayout;
    label = new QLabel("cycle index");
    grid->addWidget(label, 0, 0);
    cycleIndexEdit = new QLineEdit;
    setReadOnly(cycleIndexEdit);
    grid->addWidget(cycleIndexEdit, 0, 1);
    label = new QLabel("cycle length");
    grid->addWidget(label, 1, 0);
    cycleLengthEdit = new QLineEdit;
    setReadOnly(cycleLengthEdit);
    grid->addWidget(cycleLengthEdit, 1, 1);
    label = new QLabel("max cycle length");
    grid->addWidget(label, 2, 0);
    maxCycleLengthEdit = new QLineEdit;
    setReadOnly(maxCycleLengthEdit);
    grid->addWidget(maxCycleLengthEdit, 2, 1);
    changeCycleIndexEdit = new QLineEdit;
    grid->addWidget(changeCycleIndexEdit, 3, 0);
    button = new QPushButton("Set cycle index");
    connect(button, &QPushButton::pressed, this, &FeldbusMuxerView::SetCycleIndex);
    grid->addWidget(button, 3, 1);
    changeCycleLengthEdit = new QLineEdit;
    grid->addWidget(changeCycleLengthEdit, 4, 0);
    button = new QPushButton("Set cycle length");
    connect(button, &QPushButton::pressed, this, &FeldbusMuxerView::SetCycleLength);
    grid->addWidget(button, 4, 1);
    groupbox = new QGroupBox("cycle index/length");
    groupbox->setLayout(grid);
    layout->addWidget(groupbox);


    grid = new QGridLayout;
    label = new QLabel("transmitter index");
    grid->addWidget(label, 0, 0);
    setTransmitterEdit = new QLineEdit;
    grid->addWidget(setTransmitterEdit, 0, 1);
    label = new QLabel("receiver index");
    grid->addWidget(label, 1, 0);
    setReceiverEdit = new QLineEdit;
    grid->addWidget(setReceiverEdit, 1, 1);
    button = new QPushButton("set manual output");
    connect(button, &QPushButton::pressed, this, &FeldbusMuxerView::setManualOutput);
    grid->addWidget(button, 2, 1);
    groupbox = new QGroupBox("manual output control");
    groupbox->setLayout(grid);
    layout->addWidget(groupbox);


    grid = new QGridLayout;
    label = new QLabel("cycle index");
    grid->addWidget(label, 0, 0);
    configCycleEdit = new QLineEdit;
    grid->addWidget(configCycleEdit, 0, 1);
    label = new QLabel("transmitter index");
    grid->addWidget(label, 1, 0);
    configTransmitterEdit = new QLineEdit;
    grid->addWidget(configTransmitterEdit, 1, 1);
    label = new QLabel("receiver index");
    grid->addWidget(label, 2, 0);
    configReceiverEdit = new QLineEdit;
    grid->addWidget(configReceiverEdit, 2, 1);
    button = new QPushButton("set cycle config");
    connect(button, &QPushButton::pressed, this, &FeldbusMuxerView::setCycleConfig);
    grid->addWidget(button, 3, 1);
    button = new QPushButton("clear cycle config");
    connect(button, &QPushButton::pressed, this, &FeldbusMuxerView::clearCycleConfig);
    grid->addWidget(button, 4, 1);
    groupbox = new QGroupBox("cycle schedule setup");
    groupbox->setLayout(grid);
    layout->addWidget(groupbox);

    setLayout(layout);


    updateTimer.setInterval(500);
    connect(&updateTimer, &QTimer::timeout, this, &FeldbusMuxerView::update);
    updateTimer.start();
}

FeldbusMuxerView::~FeldbusMuxerView()
{

}

void FeldbusMuxerView::SetCycleIndex()
{
    bool success = false;
    unsigned cycleIndex = changeCycleIndexEdit->text().toUInt(&success);

    if (success) {
        muxer->setCycleIndex(cycleIndex);
    }
}

void FeldbusMuxerView::SetCycleLength()
{
    bool success = false;
    unsigned cycleLength = changeCycleLengthEdit->text().toUInt(&success);

    if (success) {
        muxer->setCycleLength(cycleLength);
    }
}

void FeldbusMuxerView::update()
{
    unsigned cycleIndex;
    if (muxer->getCycleIndex(&cycleIndex)) {
        cycleIndexEdit->setText(QString("%1").arg(cycleIndex));
    } else {
        cycleIndexEdit->setText("ERROR");
    }

    cycleLengthEdit->setText(QString("%1").arg(muxer->getCycleLength()));
    maxCycleLengthEdit->setText(QString("%1").arg(muxer->getMaxCycleLength()));

    switch (muxer->getTriggerMode()) {
    case Muxer_64_32::TriggerMode::BothEdges:
        triggerModeEdit->setText("BothEdges");
        break;

    case Muxer_64_32::TriggerMode::FallingEdge:
        triggerModeEdit->setText("FallingEdge");
        break;

    case Muxer_64_32::TriggerMode::RisingEdge:
        triggerModeEdit->setText("RisingEdge");
        break;

    case Muxer_64_32::TriggerMode::NoTrigger:
        triggerModeEdit->setText("NoTrigger");
        break;

    case Muxer_64_32::TriggerMode::Undefined:
        triggerModeEdit->setText("Undefined");
        break;
    }
}

void FeldbusMuxerView::setManualOutput()
{
    bool success = false;
    unsigned trm = setTransmitterEdit->text().toUInt(&success);
    if (!success || trm > 255) return;

    unsigned rec = setReceiverEdit->text().toUInt(&success);
    if (!success || rec > 255) return;

    muxer->setManualOutput(static_cast<uint8_t>(trm), static_cast<uint8_t>(rec));
}

void FeldbusMuxerView::setOutput()
{
    if (outputEnabled->isChecked()) {
        if (muxer->enableOutput()) {
            outputEnabled->setText("output ON");
        } else {
            outputEnabled->setChecked(muxer->getOutputEnabled());
        }
    } else {
        if (muxer->disableOutput()) {
            outputEnabled->setText("output OFF");
        } else {
            outputEnabled->setChecked(muxer->getOutputEnabled());
        }
    }
}

void FeldbusMuxerView::setCycleConfig()
{
    bool success = false;
    unsigned cycleIndex = configCycleEdit->text().toUInt(&success);
    if (!success || cycleIndex > 65535) return;

    unsigned trm = configTransmitterEdit->text().toUInt(&success);
    if (!success || trm > 255) return;

    unsigned rec = configReceiverEdit->text().toUInt(&success);
    if (!success || rec > 255) return;

    muxer->setConfig(static_cast<uint16_t>(cycleIndex), static_cast<uint8_t>(trm), static_cast<uint8_t>(rec));
}

void FeldbusMuxerView::clearCycleConfig()
{
    muxer->clearConfig();
}

void FeldbusMuxerView::setTriggerMode()
{
    int triggerMode = triggerModeComboBox->currentIndex();
    if (triggerMode == -1) return;

    muxer->setTriggerMode(static_cast<Muxer_64_32::TriggerMode>(triggerMode));
}
