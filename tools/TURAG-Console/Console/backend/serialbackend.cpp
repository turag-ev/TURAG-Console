#include "serialbackend.h"
#include <QtSerialPort/QSerialPort>
#include <QFileInfo>
#include <QDebug>
#include <QUrlQuery>
#include <QUrl>

const QString SerialBackend::protocolScheme = "serial";


SerialBackend::SerialBackend(QObject *parent) :
	BaseBackend({SerialBackend::protocolScheme}, parent)
{
	// it usually doesn't make sense and it must be disabled
	// for feldbusfrontend
	setDataEmissionChunkSize(0);
}

SerialBackend::~SerialBackend() {
}


bool SerialBackend::doConnectionPreconditionChecking(const QUrl& url) {
	if (url.path().isEmpty()) {
		logFilteredErrorMsg("no path specified");
		return false;
	}
	if (!url.hasFragment()) {
		logFilteredErrorMsg("no baudrate specified");
		return false;
	}
	return true;
}

BaseBackend::ConnectionStatus SerialBackend::doOpenConnection(const QUrl &url) {
	QSerialPort* port = new QSerialPort(url.path());
	stream_.reset(port);

	bool success = port->open(QIODevice::ReadWrite);
    if (!success) {
		logFilteredErrorMsg(QString("Fehler beim Öffnen der seriellen Konsole: %1").arg(port->errorString()));
		return ConnectionStatus::failed;
    }

	success = port->setBaudRate(url.fragment().toInt());
    if (!success) {
		logFilteredErrorMsg(QString("Fehler beim Setzen der Baudrate: %1").arg(port->errorString()));
		return ConnectionStatus::failed;
    }

	QUrlQuery query(url);

	// set data bit length
	if (query.hasQueryItem("databits")) {
		if (query.queryItemValue("databits") == "5") {
			success = port->setDataBits(QSerialPort::Data5);
		} else if (query.queryItemValue("databits") == "6") {
			success = port->setDataBits(QSerialPort::Data6);
		} else if (query.queryItemValue("databits") == "7") {
			success = port->setDataBits(QSerialPort::Data7);
		} else if (query.queryItemValue("databits") == "8") {
			success = port->setDataBits(QSerialPort::Data8);
		} else {
			success = false;
		}
	} else {
		success = port->setDataBits(QSerialPort::Data8);
	}
    if (!success) {
		logFilteredErrorMsg(QString("Fehler beim Setzen der Framelänge: %1").arg(port->errorString()));
		return ConnectionStatus::failed;
    }

	// set stop bits length
	if (query.hasQueryItem("stopbits")) {
		if (query.queryItemValue("stopbits") == "1") {
			success = port->setStopBits(QSerialPort::OneStop);
		} else if (query.queryItemValue("stopbits") == "1.5") {
			success = port->setStopBits(QSerialPort::OneAndHalfStop);
		} else if (query.queryItemValue("stopbits") == "2") {
			success = port->setStopBits(QSerialPort::TwoStop);
		} else {
			success = false;
		}
	} else {
		success = port->setStopBits(QSerialPort::OneStop);
	}
    if (!success) {
		logFilteredErrorMsg(QString("Fehler beim Setzen der Stopp-Bits: %1").arg(port->errorString()));
		return ConnectionStatus::failed;
    }

	// set parity type
	if (query.hasQueryItem("parity")) {
		if (query.queryItemValue("parity") == "none") {
			success = port->setParity(QSerialPort::NoParity);
		} else if (query.queryItemValue("parity") == "even") {
			success = port->setParity(QSerialPort::EvenParity);
		} else if (query.queryItemValue("parity") == "odd") {
			success = port->setParity(QSerialPort::OddParity);
		} else if (query.queryItemValue("parity") == "mark") {
			success = port->setParity(QSerialPort::MarkParity);
		} else if (query.queryItemValue("parity") == "space") {
			success = port->setParity(QSerialPort::SpaceParity);
		} else {
			success = false;
		}
	} else {
		success = port->setParity(QSerialPort::NoParity);
	}
	if (!success) {
		logFilteredErrorMsg(QString("Fehler beim Setzen der Parität: %1").arg(port->errorString()));
		return ConnectionStatus::failed;
	}

	// set flow control
	success = port->setFlowControl(QSerialPort::NoFlowControl);
    if (!success) {
		logFilteredErrorMsg(QString("Fehler beim Setzen der Flow-Control: %1").arg(port->errorString()));
		return ConnectionStatus::failed;
    }

	connect(stream_.data(),SIGNAL(readyRead()),this,SLOT(emitDataReady()));
	connect(stream_.data(),SIGNAL(error(QSerialPort::SerialPortError)),this,SLOT(onError(QSerialPort::SerialPortError)));

	return ConnectionStatus::successful;
}

QString SerialBackend::getConnectionInfo() const {
	if (connectionUrl_.isEmpty()) {
        return "";
    } else {
		return connectionUrl_.path() + ":" + connectionUrl_.fragment();
    }
}


void SerialBackend::onError(QSerialPort::SerialPortError error) {
    QString errormsg;

	switch (error) {
    case QSerialPort::NoError:
        errormsg = "Kein Fehler, Glück gehabt ;-)."; break;
    case QSerialPort::DeviceNotFoundError:
        errormsg = "An error occurred while attempting to open an non-existing device."; break;
    case QSerialPort::PermissionError:
        errormsg = "An error occurred while attempting to open an already opened device by another process or a user not having enough permission and credentials to open."; break;
    case QSerialPort::OpenError:
        errormsg = "An error occurred while attempting to open an already opened device in this object."; break;
    case QSerialPort::ParityError:
        errormsg = "Parity error detected by the hardware while reading data."; break;
    case QSerialPort::FramingError:
        errormsg = "Framing error detected by the hardware while reading data."; break;
    case QSerialPort::BreakConditionError:
        errormsg = "Break condition detected by the hardware on the input line."; break;
    case QSerialPort::WriteError:
        errormsg = "An I/O error occurred while writing the data.";
        logFilteredErrorMsg("Fehler: " + errormsg);
        break;
    case QSerialPort::ReadError:
		// This error used to make a call to connectionWasLost(). But I have
		// a funny adapter which raises this error two times after opening
		// and works totally fine afterwards, so I removed the call.
        errormsg = "An I/O error occurred while reading the data.";
        break;
    case QSerialPort::ResourceError:
		// This error occurs when a serial USB adapter is removed and thus the call
		// to connectionWasLost() is required.
        errormsg = "An I/O error occurred when a resource becomes unavailable, e.g. when the device is unexpectedly removed from the system.";
        logFilteredErrorMsg("Fehler: " + errormsg);
        connectionWasLost();
        break;
    case QSerialPort::UnsupportedOperationError:
        errormsg = "The requested device operation is not supported or prohibited by the running operating system.";
        logFilteredErrorMsg("Fehler: " + errormsg);
        break;
    case QSerialPort::UnknownError:
        errormsg = "An unidentified error occurred."; break;

    default: break;
    }

	qWarning() << errormsg;
}

