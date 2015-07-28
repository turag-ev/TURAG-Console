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
#include <QPushButton>
#include <QFormLayout>
#include <QUrl>
#include <QUrlQuery>
#include <qt/expander-widget/expanderwidget.h>
#include <libs/keyboardenabledcombobox.h>
#include <libs/log.h>

#include <QTextEdit>


ConnectionWidgetSerial::ConnectionWidgetSerial(QWidget *parent) :
    ConnectionWidget("Letzte Verbindungen", parent)
{
    setObjectName("Serielle Schnittstelle");

    // create input for serial device
    port_name_ = new KeyboardEnabledComboBox;
    port_name_->setMaximumWidth(350);
    port_name_->setEditable(true);
    port_name_->setToolTip("Choose device from list or type in custom device name.");
    connect(port_name_, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(onKeyPressed(QKeyEvent*)));

    // create input for baud rate
    baudrate_ = new KeyboardEnabledComboBox();
    baudrate_->setToolTip("Choose baudrate from list or type in custom baudrate value.");
    baudrate_->setMaximumWidth(100);
    baudrate_->setEditable(true);
    connect(baudrate_, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(onKeyPressed(QKeyEvent*)));

    baudrate_->clear();
    int default_index = 0;
    for (const qint32 baudrate : QSerialPortInfo::standardBaudRates()) {
        baudrate_->addItem(QString("%1").arg(baudrate));
		if (baudrate == 500000) {
            default_index = baudrate_->count()-1;
        }
    }
    baudrate_->setCurrentIndex(default_index);

	// number of data bits
	dataBits_ = new QComboBox;
	dataBits_->addItems(QStringList({"5", "6", "7", "8"}));
	dataBits_->setMaximumWidth(200);
	dataBits_->setCurrentIndex(3);

	// number of stop bits
	numberOfStopBits_ = new QComboBox;
	numberOfStopBits_->addItems(QStringList({"1", "1.5", "2"}));
	numberOfStopBits_->setMaximumWidth(200);
	numberOfStopBits_->setCurrentIndex(0);

	// parity type
	parityType_ = new QComboBox;
	parityType_->addItems(QStringList({"none", "even", "odd", "space", "mark"}));
	parityType_->setMaximumWidth(200);
	parityType_->setCurrentIndex(0);

	QFormLayout* extendedFormLayout = new QFormLayout;
	extendedFormLayout->addRow("Bit-Zahl", dataBits_);
	extendedFormLayout->addRow("Stop-Bits", numberOfStopBits_);
	extendedFormLayout->addRow("Parität", parityType_);

	QWidget* extendedSettingsWidget = new QWidget;
	extendedSettingsWidget->setLayout(extendedFormLayout);
	expander = new ExpanderWidget;
	expander->setExpanderTitle("weitere Einstellungen");
	expander->setExpanded(false);
	expander->setWidget(extendedSettingsWidget);


	// create button to connect
    serial_button = new QPushButton("Verbinden");
	serial_button->setMaximumWidth(200);
    connect(serial_button, SIGNAL(clicked()), this, SLOT(connectionChangedInternal()));


	// horizontal layout for input fields
	QFormLayout* port_layout = new QFormLayout();
	port_layout->addRow("Port", port_name_);
	port_layout->addRow("Baudrate", baudrate_);

	layout->addLayout(port_layout);
	layout->addWidget(expander);
	layout->addSpacing(10);
	layout->addWidget(serial_button);

    addRecentConnections();

    connect(&deviceUpdateTimer, SIGNAL(timeout()), this, SLOT(onDeviceUpdate()));
}

void ConnectionWidgetSerial::connectionChangedInternal() {
	QUrl url;
	url.setHost(""); // fixes small display bug of QUrl
	url.setScheme(SerialBackend::protocolScheme);
	url.setPath(port_name_->currentText());
	url.setFragment(baudrate_->currentText());

	if (expander->isExpanded()) {
		QUrlQuery query;
		query.addQueryItem("databits", dataBits_->currentText());
		query.addQueryItem("stopbits", numberOfStopBits_->currentText());
		query.addQueryItem("parity", parityType_->currentText());
		url.setQuery(query);
	}

	bool save = false;
	emit connectionChanged(url, &save, nullptr);
    if (save) {
		saveConnection(url.toDisplayString());
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


void ConnectionWidgetSerial::onKeyPressed(QKeyEvent* e) {
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        connectionChangedInternal();
    }
}
