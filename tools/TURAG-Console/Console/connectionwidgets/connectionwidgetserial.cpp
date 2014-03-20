#include "connectionwidgetserial.h"
#include "backend/serialbackend.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDir>
#include <QLabel>
#include <QHBoxLayout>
#include <QCompleter>
#include <QStringList>
#include <QShowEvent>
#include <QSerialPortInfo>
#include <QDebug>
#include <QComboBox>
#include <QPushButton>


ConnectionWidgetSerial::ConnectionWidgetSerial(QWidget *parent) :
    ConnectionWidget("Letzte Verbindungen", parent)
{
    setObjectName("Serielle Schnittstelle");

    // create input for serial device
    port_name_ = new QComboBox;
    port_name_->setMaximumWidth(350);
    port_name_->setEditable(true);
    port_name_->setToolTip("Choose device from list or type in custom device name.");

    // create input for baud rate
    baudrate_ = new QComboBox();
    baudrate_->setToolTip("Choose baudrate from list or type in custom baudrate value.");
    baudrate_->setMaximumWidth(100);
    baudrate_->setEditable(true);

    baudrate_->clear();
    for (const qint32 baudrate : QSerialPortInfo::standardBaudRates()) {
        baudrate_->addItem(QString("%1").arg(baudrate));
    }

    // create button to connect
    serial_button = new QPushButton("Verbinden");
    // connect signals directly to base signal
    connect(serial_button, SIGNAL(clicked()), this, SLOT(connectionChangedInternal()));


    // horizontal layout for input text edits
    QHBoxLayout* port_layout = new QHBoxLayout();
    port_layout->addWidget(port_name_, 1);
    port_layout->addWidget(baudrate_, 1);
    port_layout->addWidget(serial_button, 0);
    port_layout->addStretch();

    // vertical layout including the button and a label
    layout = new QVBoxLayout();
    layout->addLayout(port_layout);

    layout->addSpacing(10);
    addRecentConnections();

    setLayout(layout);

    connect(&deviceUpdateTimer, SIGNAL(timeout()), this, SLOT(onDeviceUpdate()));
}



void ConnectionWidgetSerial::connectionChangedInternal() {
    QString connectionString = SerialBackend::connectionPrefix + port_name_->currentText() + ":" + baudrate_->currentText();

    bool save = false;
    emit connectionChanged(connectionString, &save, nullptr);
    if (save) {
        saveConnection(connectionString);
        addRecentConnections();
    }

}



void ConnectionWidgetSerial::onDeviceUpdate(void) {
    if (QSerialPortInfo::availablePorts().count() != port_name_->count()) {
        port_name_->clear();
        for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts()) {
            port_name_->addItem(info.systemLocation());
        }
    }
}


void ConnectionWidgetSerial::showEvent ( QShowEvent * event ) {
    port_name_->clear();
    for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts()) {
        port_name_->addItem(info.systemLocation());
    }

    deviceUpdateTimer.start(500);

    QWidget::showEvent(event);
}
void ConnectionWidgetSerial::hideEvent ( QHideEvent * event ) {
    deviceUpdateTimer.stop();

    QWidget::hideEvent(event);
}
