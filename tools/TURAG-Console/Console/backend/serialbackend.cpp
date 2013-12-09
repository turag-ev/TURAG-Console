#include "serialbackend.h"
#include <QtSerialPort/QSerialPort>
#include <QFileInfo>

const QString SerialBackend::connectionPrefix = "serial://";

SerialBackend::SerialBackend(QObject *parent) :
    BaseBackend(SerialBackend::connectionPrefix, parent)
{
}


bool SerialBackend::openConnection(QString connectionString) {
    if (!canHandleUrl(connectionString)) {
        return false;
    }

    int colonIndex = connectionString.lastIndexOf(":");
    if (colonIndex == -1) {
        return false;
    }

    // close connection in case we had one open
    closeConnection();

    // extract arguments
    QString device = connectionString.mid(connectionPrefix_.length(),
                                  colonIndex - connectionPrefix_.length());
    QString baudrate = connectionString.right(connectionString.size() - colonIndex - 1);

    // open stream
    std::unique_ptr<QSerialPort> port(new QSerialPort(device));

    bool success = port->open(QIODevice::ReadWrite);
    if (!success) {
        emit errorOccured(QString("Fehler beim Öffnen der seriellen Konsole: %1").arg(port->errorString()));
        return false;
    }

    success = port->setBaudRate(baudrate.toInt());
    if (!success) {
        emit errorOccured(QString("Fehler beim Setzen der Baudrate: %1").arg(port->errorString()));
        return false;
    }

    connectionString_ = connectionString;

    stream_ = std::move(port);
    connect(stream_.get(),SIGNAL(readyRead()),this,SLOT(checkData()));
    connect(stream_.get(),SIGNAL(error(QSerialPort::SerialPortError)),this,SLOT(onError(QSerialPort::SerialPortError)));


    emit connected(!stream_->isWritable(), stream_->isSequential());

    return true;
}

QString SerialBackend::getConnectionInfo() {
    if (connectionString_.isEmpty()) {
        return "";
    } else {
        int colonIndex = connectionString_.lastIndexOf(":");
        QString device = connectionString_.mid(connectionPrefix_.length(),
                                      colonIndex - connectionPrefix_.length());
        QString baudrate = connectionString_.right(connectionString_.size() - colonIndex - 1);

        return QFileInfo(device).fileName() + ":" + baudrate;
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
        emit errorOccured("Fehler: " + errormsg);
        break;
    case QSerialPort::ReadError:
        errormsg = "An I/O error occurred while reading the data.";
        emit errorOccured("Fehler: " + errormsg);
        break;
    case QSerialPort::ResourceError:
        errormsg = "An I/O error occurred when a resource becomes unavailable, e.g. when the device is unexpectedly removed from the system.";
        emit errorOccured("Fehler: " + errormsg);
        closeConnection();
        break;
    case QSerialPort::UnsupportedOperationError:
        errormsg = "The requested device operation is not supported or prohibited by the running operating system.";
        emit errorOccured("Fehler: " + errormsg);
        break;
    case QSerialPort::UnknownError:
        errormsg = "An unidentified error occurred."; break;
    }

}

