#include "basebackend.h"
#include <libs/log.h>
#include <QByteArray>
#include <QIODevice>
#include <QDebug>
#include <QFile>
#include <initializer_list>

BaseBackend::BaseBackend(std::initializer_list<QString> protocolScheme, QObject *parent) :
	QObject(parent), connectionInProgress(false), deviceRecoveryActive(false), dataEmissionChunkSize(10240)
{
	for (const QString& scheme : protocolScheme) {
		supportedProtocolSchemes_.push_back(scheme);
	}

    recoverDeviceTimer.setInterval(500);
    connect(&recoverDeviceTimer, SIGNAL(timeout()), this, SLOT(onRecoverDevice()));

	readTimer.setInterval(0);
	connect(&readTimer, SIGNAL(timeout()), this, SLOT(onReadData()));
}


BaseBackend::~BaseBackend() {
	closeConnectionInternal();
}


bool BaseBackend::isReadOnly(void) const {
	if (stream_.data() == nullptr) {
        return true;
    } else {
        return !stream_->isWritable();
    }
}



bool BaseBackend::isOpen(void) const {
	if (stream_.data() == nullptr) {
        return false;
    } else {
        return stream_->isOpen();
    }
}

bool BaseBackend::isConnectionInProgress(void) const {
	return connectionInProgress;
}

bool BaseBackend::openConnection(const QUrl &url) {
	// check preconditions
	if (!url.isValid()) {
		return false;
	}

	if (!canHandleUrl(url)) {
		return false;
	}

	if (!doConnectionPreconditionChecking(url)) {
		return false;
	}

	// is there some asynchronous connection process already
	// running? If so, cancel.
	if (connectionInProgress) {
		return false;
	}
	connectionInProgress = true;

	// close connection in case we had one open
	if (isOpen()) closeConnection();

	connectionUrl_ = url;

	// Eventually open the connection.
	ConnectionStatus result = doOpenConnection(url);

	if (result == ConnectionStatus::failed) {
		if (connectionInProgress) {
			connectingFailed();
		}
		return false;
	} else if (result == ConnectionStatus::successful) {
		if (connectionInProgress) {
			connectingSuccessful();
		}
		return true;
	} else {
		return true;
	}
}

bool BaseBackend::doConnectionPreconditionChecking(const QUrl&) {
	return true;
}

bool BaseBackend::openConnection(void) {
	if (connectionUrl_.isEmpty()) {
        return false;
    }

	return openConnection(connectionUrl_);
}

void BaseBackend::closeConnection(void) {
	// call sub-class implementation
	doCleanUpConnection();

	closeConnectionInternal();
}

void BaseBackend::doCleanUpConnection(void) {

}

void BaseBackend::closeConnectionInternal(void) {
	if (isOpen() || recoverDeviceTimer.isActive()) {
		emit disconnected(false);
	}

	if (isOpen()) {
		stream_->close();
		stream_->disconnect(this);
	}
	stream_.reset();

	connectionInProgress = false;

	deviceShouldBeConnectedUrl.clear();
	recoverDeviceTimer.stop();
	readTimer.stop();
}


void BaseBackend::writeData(QByteArray data) {
    if (isOpen() && !isReadOnly()) {
        int result =  stream_->write(data);

        if (result < data.size()) {
			logWarning("Es wurden weniger Bytes geschrieben als angefordert");
        } else if (result == -1) {
			logCritical(QString("Fehler beim Schreiben: %1").arg(stream_->errorString()));
        }

        qDebug() << "writeData:" << data;
    }
}


void BaseBackend::emitDataReady(void) {
	if (stream_.data() && stream_->isReadable()) {
        if (!stream_->isSequential()) {
            stream_->seek(0);
        }

		QByteArray data(stream_->readAll());

		if (dataEmissionChunkSize != 0) {
			if (!readTimer.isActive()) {
				readIndex = buffer.size();
				readTimer.start();
			}
		} else {
			emit dataReady(data);
		}

		// TODO implement gzip support

		// check whether buffer->size() == 0 --> beginning of stream
		// only applicalble if stream is not sequential

		if (buffer.size() + data.size() > buffer.capacity()) {
			buffer.reserve(buffer.size() + 1024 * 1024);
		}
		buffer.append(data);
    }
}

void BaseBackend::reloadData() {
	if (dataEmissionChunkSize != 0) {
		readIndex = 0;
		readTimer.start();
	} else {
		emit dataReady(buffer);
	}
}

// this slot is called by a zero-timer. We always output small chunks of data.
// This keeps the GUI responsive.
void BaseBackend::onReadData() {
	int readBytes = std::min(buffer.size() - readIndex, dataEmissionChunkSize);

	emit dataReady(buffer.mid(readIndex, readBytes));
	readIndex += readBytes;

	if (readIndex >= buffer.size()) {
		readTimer.stop();
	}
}


QString BaseBackend::getConnectionInfo() const {
	return connectionUrl_.toDisplayString();
}

bool BaseBackend::canHandleUrl(const QUrl &url) const {
	for (const QString& scheme : supportedProtocolSchemes_) {
		if (url.scheme() == scheme) {
			return true;
		}
	}
	return false;
}

void BaseBackend::connectingSuccessful() {
	connectionInProgress = false;

	buffer.clear();

	if (deviceShouldBeConnectedUrl == connectionUrl_ && deviceRecoveryActive && recoverDeviceTimer.isActive()) {
		logInfo("Verbindung erfolgreich wiederaufgebaut");
    }

	deviceShouldBeConnectedUrl = connectionUrl_;
	emit connected(isReadOnly());

    recoverDeviceTimer.stop();
}

void BaseBackend::connectingFailed(void) {
	connectionInProgress = false;

	if (stream_.data()) {
		if (isOpen()) {
			stream_->close();
		}
		stream_->disconnect(this);
	}
	stream_.reset();
}

void BaseBackend::connectionWasLost(void) {
    if (isOpen()) {
        stream_->close();
		emit disconnected(!deviceShouldBeConnectedUrl.isEmpty() && deviceRecoveryActive);
    }
	if (!deviceShouldBeConnectedUrl.isEmpty() && deviceRecoveryActive) {
        recoverDeviceTimer.start();
    }
	stream_.reset();
}

void BaseBackend::onRecoverDevice(void) {
	logInfo("Versuche wiederzuverbinden...");
    openConnection();
}

void BaseBackend::setDeviceRecovery(bool on) {
    if (!on) {
        recoverDeviceTimer.stop();
	} else if (on && !deviceShouldBeConnectedUrl.isEmpty() && !isOpen()) {
        recoverDeviceTimer.start();
    }

    deviceRecoveryActive = on;
}

void BaseBackend::logFilteredErrorMsg(QString msg) {
    if (!recoverDeviceTimer.isActive()) {
		logCritical(msg);
    }
}

void BaseBackend::logFilteredInfoMessage(QString msg) {
    if (!recoverDeviceTimer.isActive()) {
		logInfo(msg);
    }

}

void BaseBackend::saveBufferToFile(QString fileName) {
    QMetaObject::invokeMethod(this, "saveBufferToFileInternal", Q_ARG(QString, fileName));
}

bool BaseBackend::saveBufferToFileInternal(QString fileName) {
    QFile savefile(std::move(fileName));

	if (buffer.isEmpty()) {
        return false;
    }

    if (!savefile.open(QIODevice::WriteOnly)) {
		logCritical("Saving output failed: couldn't open file.");
        return false;
    }

    if (!savefile.isWritable()) {
		logCritical("Saving output failed: file is not writable.");
        return false;
    }

	if (savefile.write(buffer) == -1) {
		logCritical("Saving output failed: error while writing.");
        return false;
    }

	logInfo("Ausgabe erfolgreich geschrieben");
    return true;
}

