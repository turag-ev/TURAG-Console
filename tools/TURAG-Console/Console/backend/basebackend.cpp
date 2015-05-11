#include "basebackend.h"
#include <libs/log.h>
#include <QByteArray>
#include <QIODevice>
#include <QDebug>
#include <QFile>

BaseBackend::BaseBackend(QString connectionPrefix, QObject *parent) :
    QObject(parent), connectionPrefix_(connectionPrefix), deviceShouldBeConnectedString(""), deviceRecoveryActive(false)
{
    recoverDeviceTimer.setInterval(500);
    connect(&recoverDeviceTimer, SIGNAL(timeout()), this, SLOT(onRecoverDevice()));

    buffer = new QByteArray;
}


BaseBackend::~BaseBackend(void) {
    closeConnection();
    delete buffer;
}


bool BaseBackend::isReadOnly(void) const {
    if (stream_.get() == nullptr) {
        return true;
    } else {
        return !stream_->isWritable();
    }
}



bool BaseBackend::isOpen(void) const {
    if (stream_.get() == nullptr) {
        return false;
    } else {
        return stream_->isOpen();
    }
}


bool BaseBackend::openConnection(void) {
    if (connectionString_.isEmpty()) {
        return false;
    }

    return openConnection(connectionString_);
}


void BaseBackend::closeConnection(void) {
    if (isOpen() || recoverDeviceTimer.isActive()) {
        emit disconnected(false);
    }

    if (isOpen()) {
        stream_->close();
        stream_->disconnect(this);
    }

    deviceShouldBeConnectedString = "";
    recoverDeviceTimer.stop();
}


void BaseBackend::writeData(QByteArray data) {
    if (isOpen() && !isReadOnly()) {
        int result =  stream_->write(data);

        if (result < data.size()) {
            Log::warning("Es wurden weniger Bytes geschrieben als angefordert");
        } else if (result == -1) {
            Log::critical(QString("Fehler beim Schreiben: %1").arg(stream_->errorString()));
        }

        qDebug() << "writeData:" << data;
    }
}


void BaseBackend::emitDataReady(void) {
    if (stream_.get() && stream_->isReadable()) {
        if (!stream_->isSequential()) {
            stream_->seek(0);
        }
        QByteArray data(stream_->readAll());
        emit dataReady(data);
        buffer->append(data);
    }
}

void BaseBackend::reloadData() {
    emit dataReady(*buffer);
}


QString BaseBackend::getConnectionInfo() {
    return connectionString_;
}

QList<QAction*> BaseBackend::getMenuEntries() {
    return QList<QAction*>();
}

bool BaseBackend::canHandleUrl(const QString& url) const {
    return url.startsWith(connectionPrefix_);
}

void BaseBackend::emitConnected() {
    buffer->clear();

    if (deviceShouldBeConnectedString == connectionString_ && deviceRecoveryActive && recoverDeviceTimer.isActive()) {
        Log::info("Verbindung erfolgreich wiederaufgebaut");
    }

    deviceShouldBeConnectedString = connectionString_;
	emit connected(isReadOnly());

    recoverDeviceTimer.stop();
}

void BaseBackend::connectionWasLost(void) {
    if (isOpen()) {
        stream_->close();
        stream_->disconnect(this);
        emit disconnected(deviceShouldBeConnectedString.size() && deviceRecoveryActive);
    }
    if (deviceShouldBeConnectedString.size() && deviceRecoveryActive) {
        recoverDeviceTimer.start();
    }
}

void BaseBackend::onRecoverDevice(void) {
    Log::info("Versuche wiederzuverbinden...");
    openConnection();
}

void BaseBackend::setDeviceRecovery(bool on) {
    if (!on) {
        recoverDeviceTimer.stop();
    } else if (on && deviceShouldBeConnectedString.size() && !isOpen()) {
        recoverDeviceTimer.start();
    }

    deviceRecoveryActive = on;
}

void BaseBackend::logFilteredErrorMsg(QString msg) {
    if (!recoverDeviceTimer.isActive()) {
        Log::critical(msg);
    }
}

void BaseBackend::logFilteredInfoMessage(QString msg) {
    if (!recoverDeviceTimer.isActive()) {
        Log::info(msg);
    }

}

void BaseBackend::saveBufferToFile(QString fileName) {
    QMetaObject::invokeMethod(this, "saveBufferToFileInternal", Q_ARG(QString, fileName));
}

bool BaseBackend::saveBufferToFileInternal(QString fileName) {
    QFile savefile(std::move(fileName));

    if (buffer->isEmpty()) {
        return false;
    }

    if (!savefile.open(QIODevice::WriteOnly)) {
        Log::critical("Saving output failed: couldn't open file.");
        return false;
    }

    if (!savefile.isWritable()) {
        Log::critical("Saving output failed: file is not writable.");
        return false;
    }

    if (savefile.write(*buffer) == -1) {
        Log::critical("Saving output failed: error while writing.");
        return false;
    }

    Log::info("Ausgabe erfolgreich geschrieben");
    return true;
}

