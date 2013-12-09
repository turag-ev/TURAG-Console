#include "connectionwidgetserial.h"
#include "backend/serialbackend.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDir>
#include <QLabel>
#include <QHBoxLayout>
#include <QCompleter>
#include <QStringList>


ConnectionWidgetSerial::ConnectionWidgetSerial(QWidget *parent) :
    ConnectionWidget("Letzte Verbindungen", parent)
{
    // create input for serial device
    port_name_edit = new QLineEdit();
    port_name_edit->setPlaceholderText("Gerätename");
    port_name_edit->setMaximumWidth(350);
    QCompleter* completer = new QCompleter(ConnectionWidgetSerial::listDevices(), this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    port_name_edit->setCompleter(completer);

    // create input for baud rate
    bd_edit = new QLineEdit();
    bd_edit->setPlaceholderText("Baudrate");
    bd_edit->setMaximumWidth(100);

    // create button to connect
    serial_button = new QPushButton("Verbinden");

    // horizontal layout for input text edits
    QHBoxLayout* port_layout = new QHBoxLayout();
    port_layout->addWidget(port_name_edit, 1);
    port_layout->addWidget(bd_edit, 1);
    port_layout->addWidget(serial_button, 0);
    port_layout->addStretch();

    // vertical layout including the button and a label
    layout = new QVBoxLayout();
    layout->addLayout(port_layout);

    layout->addSpacing(10);
    addRecentConnections();

    setLayout(layout);

    // connect signals directly to base signal
    connect(bd_edit, SIGNAL(returnPressed()), this, SLOT(connectionChangedInternal()));
    connect(serial_button, SIGNAL(clicked()), this, SLOT(connectionChangedInternal()));

    setObjectName("Serielle Schnittstelle");
}


QStringList ConnectionWidgetSerial::listDevices(void) {
  QStringList result;
  QDir dev_dir("/dev");
  dev_dir.setFilter(QDir::System | QDir::Readable | QDir::NoDotAndDotDot);

  QStringList entries = dev_dir.entryList();
  foreach (QString dev, entries) {
    result.push_back(QString("/dev/") + dev);
  }

  return result;
}


void ConnectionWidgetSerial::connectionChangedInternal() {
    QString connectionString = SerialBackend::connectionPrefix + port_name_edit->text() + ":" + bd_edit->text();

    bool save = false;
    emit connectionChanged(connectionString, &save);
    if (save) {
        saveConnection(connectionString);
        addRecentConnections();
    }

}

