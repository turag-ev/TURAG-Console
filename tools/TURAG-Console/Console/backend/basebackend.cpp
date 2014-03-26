#include "basebackend.h"
#include <QByteArray>
#include <QIODevice>

BaseBackend::BaseBackend(QString connectionPrefix, QObject *parent) :
    QObject(parent), connectionPrefix_(connectionPrefix), deviceShouldBeConnected(false), deviceRecoveryActive(false)
{
    recoverDeviceTimer.setInterval(500);
    connect(&recoverDeviceTimer, SIGNAL(timeout()), this, SLOT(onRecoverDevice()));
}


BaseBackend::~BaseBackend(void) {
    closeConnection();
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


bool BaseBackend::isSequential(void) const {
    if (stream_.get() == nullptr) {
        return false;
    } else {
        return stream_->isSequential();
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
    deviceShouldBeConnected = false;
    recoverDeviceTimer.stop();
}


void BaseBackend::writeData(QByteArray data) {
    if (isOpen() && !isReadOnly()) {
        int result =  stream_->write(data);

        if (result < data.size()) {
            emit errorOccured("Es wurden weniger Bytes geschrieben als angefordert");
        } else if (result == -1) {
            emit errorOccured(QString("Fehler beim Schreiben: %1").arg(stream_->errorString()));
        }
    }
}


void BaseBackend::checkData(void) {
    if (stream_.get() && stream_->isReadable()) {
        if (!stream_->isSequential()) {
            stream_->seek(0);
        }
        emit dataReady(stream_->readAll());
    }
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
    if (deviceShouldBeConnected && deviceRecoveryActive && recoverDeviceTimer.isActive()) {
        emit infoMessage("Verbindung erflogreich wiederaufgebaut");
    }

    deviceShouldBeConnected = true;
    emit connected(isReadOnly(), stream_->isSequential());

    recoverDeviceTimer.stop();
}

void BaseBackend::connectionWasLost(void) {
    if (isOpen()) {
        stream_->close();
        stream_->disconnect(this);
        emit disconnected(deviceShouldBeConnected && deviceRecoveryActive);
    }
    if (deviceShouldBeConnected && deviceRecoveryActive) {
        recoverDeviceTimer.start();
    }
}

void BaseBackend::onRecoverDevice(void) {
    emit infoMessage("Versuche wiederzuverbinden...");
    openConnection();
}

void BaseBackend::setDeviceRecovery(bool on) {
    if (!on) {
        recoverDeviceTimer.stop();
    } else if (on && deviceShouldBeConnected && !isOpen()) {
        recoverDeviceTimer.start();
    }

    deviceRecoveryActive = on;
}

void BaseBackend::emitErrorOccured(QString msg) {
    if (!recoverDeviceTimer.isActive()) {
        emit errorOccured(msg);
    }
}

void BaseBackend::emitInfoMessage(QString msg) {
    if (!recoverDeviceTimer.isActive()) {
        emit infoMessage(msg);
    }

}

