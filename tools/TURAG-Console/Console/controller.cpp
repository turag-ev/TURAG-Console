#include "controller.h"

#include "backend/filebackend.h"
#include "backend/serialbackend.h"
#include "backend/tcpbackend.h"
#include "backend/webdavbackend.h"

#include "frontend/plaintextfrontend.h"
#include "frontend/oscilloscope.h"
#include "frontend/feldbusfrontend.h"
#include "frontend/robotfrontend.h"
#include "frontend/stmcamfrontend.h"
#include "frontend/rawfrontend.h"
#include "frontend/scfrontend.h"

#include <libs/log.h>

#include <QLabel>
#include <QSettings>
#include <QDateTime>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QByteArray>


Controller::Controller(QWidget *parent) :
	QStackedWidget(parent), currentBackend(nullptr), currentFrontendIndex(0),
	autoSaveOn(false)
{
    // add all available Backends to list with parent this
    availableBackends.append(new FileBackend(this));
    availableBackends.append(new SerialBackend(this));
	availableBackends.append(new TcpBackend(this));
	availableBackends.append(new WebDAVBackend(true, this));

    // add all available Frontends to list without a parent
	availableFrontends.append(new PlainTextFrontend);
    //availableFrontends.append(new RobotFrontend);
    availableFrontends.append(new Oscilloscope);
    availableFrontends.append(new FeldbusFrontend);
    //availableFrontends.append(new STMCamFrontend);
	availableFrontends.append(new RawFrontend);
    //availableFrontends.append(new SCFrontend);


    // ---------------------------------------------------------------
    // ---------------------------------------------------------------
    // ---------------------------------------------------------------
    // ---------------------------------------------------------------


    // receive all error and infomessages and connection signals from all available backends
    for (BaseBackend* iter : availableBackends) {
		connect(iter,SIGNAL(connected(bool)),this,SLOT(onConnected(bool)), Qt::QueuedConnection);
        connect(iter,SIGNAL(disconnected(bool)),this,SLOT(onDisconnected()), Qt::QueuedConnection);
        connect(iter,SIGNAL(disconnected(bool)),this,SIGNAL(disconnected(bool)), Qt::QueuedConnection);
    }


	// fill stackedWidget base class with frontends
	for (auto iter : availableFrontends) {
		addWidget(iter);
    }
}


Controller::~Controller() {
}


QList<BaseFrontend *> Controller::getAvailableFrontends(void) const {
	return availableFrontends;
}



void Controller::setFrontend(int newFrontendIndex) {
    setFrontend(newFrontendIndex, true);
}

void Controller::setFrontend(int newFrontendIndex, bool calledManually) {
    // only evaluate this function if :
    // - the desired frontend index is valid and
    // - either the chosen frontend stays the same
	//   but it is not shown just now or we want to set a new
	//   frontend
    if (newFrontendIndex >= availableFrontends.size() ||
			(newFrontendIndex == currentFrontendIndex && calledManually)) {
        return;
    }

    if (currentBackend != nullptr) {
        BaseFrontend* currentFrontend = availableFrontends.at(currentFrontendIndex);
        BaseFrontend* newFrontend = availableFrontends.at(newFrontendIndex);

        currentBackend->disconnect(currentFrontend);
        currentFrontend->disconnect(currentBackend);
        this->disconnect(currentFrontend);
        if (calledManually) currentFrontend->onDisconnected(false);
        newFrontend->clear();

        connect(currentBackend, SIGNAL(dataReady(QByteArray)), newFrontend, SLOT(writeData(QByteArray)));
        connect(newFrontend, SIGNAL(dataReady(QByteArray)), currentBackend, SLOT(writeData(QByteArray)));
        connect(newFrontend, SIGNAL(requestData()), this, SLOT(refresh()), Qt::QueuedConnection);
		connect(this,SIGNAL(connected(bool,QIODevice*)),newFrontend,SLOT(onConnected(bool,QIODevice*)));
        connect(this,SIGNAL(disconnected(bool)),newFrontend,SLOT(onDisconnected(bool)));
        if (currentBackend->isOpen()) {
			if (calledManually) newFrontend->onConnected(currentBackend->isReadOnly(), currentBackend->getDevice());
        } else {
            if (calledManually) newFrontend->onDisconnected(false);
        }
        if (newFrontendIndex != currentFrontendIndex) {
            refresh();
        }

        currentFrontendIndex = newFrontendIndex;
    } else {
        currentFrontendIndex = newFrontendIndex;
    }

    setCurrentIndex(currentFrontendIndex);
}


bool Controller::isConnected(void) const {
    if (currentBackend == nullptr) {
        return false;
    } else {
        return currentBackend->isOpen();
    }
}

QString Controller::getConnectionInfo() {
    if (currentBackend == nullptr) {
        return "no Backend";
    } else {
        return currentBackend->getConnectionInfo();
    }
}


void Controller::openConnection(void) {
    if (currentBackend != nullptr && !currentBackend->isOpen()) {
        BaseFrontend* currentFrontend = availableFrontends.at(currentFrontendIndex);
        currentFrontend->clear();

        closeConnection();

        // build signal-slot connection before opening stream
        connect(currentBackend, SIGNAL(dataReady(QByteArray)), currentFrontend, SLOT(writeData(QByteArray)));
        connect(currentFrontend, SIGNAL(dataReady(QByteArray)), currentBackend, SLOT(writeData(QByteArray)));
        connect(currentFrontend, SIGNAL(requestData()), this, SLOT(refresh()), Qt::QueuedConnection);

        if (!currentBackend->openConnection()) {
            // destroy signal-slot connection in case of failure
            currentBackend->disconnect(currentFrontend);
            currentFrontend->disconnect(currentBackend);
        }
    }
}


void Controller::openConnection(const QUrl &connectionUrl, bool *success) {

    closeConnection();

    BaseFrontend* currentFrontend = availableFrontends.at(currentFrontendIndex);
    currentFrontend->clear();

    auto iter = std::find_if(availableBackends.begin(),
                             availableBackends.end(),
							 [&](const BaseBackend* b) -> bool { return b->canHandleUrl(connectionUrl); });

    if (iter != availableBackends.end()) {
        // we got a backend
        BaseBackend* backend = *iter;

        // build signal-slot connection before opening stream
        connect(backend, SIGNAL(dataReady(QByteArray)), currentFrontend, SLOT(writeData(QByteArray)));
        connect(currentFrontend, SIGNAL(dataReady(QByteArray)), backend, SLOT(writeData(QByteArray)));
        connect(currentFrontend, SIGNAL(requestData()), this, SLOT(refresh()), Qt::QueuedConnection);

		if (backend->openConnection(connectionUrl)) {
            currentBackend = backend;
            setFrontend(currentFrontendIndex, false);
            if (success) *success = true;
            return;
        }

        // destroy signal-slot connection if connecting failed
        backend->disconnect(currentFrontend);
        currentFrontend->disconnect(backend);
		logCritical("connection failed");
	} else {
		logWarning("no suitable backend available");
	}

    if (success) *success = false;
}


void Controller::closeConnection(void) {
    if (currentBackend == nullptr) {
        return;
    } else {
        currentBackend->closeConnection();

        BaseFrontend* currentFrontend = availableFrontends.at(currentFrontendIndex);
        currentBackend->disconnect(currentFrontend);
        currentFrontend->disconnect(currentBackend);
    }
}


void Controller::saveOutput(void) {
    if (currentBackend) {
        QString filename = QFileDialog::getSaveFileName(this);

        if (filename.isEmpty()) return;

        currentBackend->saveBufferToFile(filename);
    }
}


void Controller::setAutoReconnect(bool on) {
    for (BaseBackend* backend : availableBackends) {
        backend->setDeviceRecovery(on);
    }
}

void Controller::onConnected(bool readOnly) {
    refresh();

	emit connected(readOnly, currentBackend->getDevice());
}

void Controller::onDisconnected() {
    if (autoSaveOn) {
        QString file = QDir::toNativeSeparators(QDir::homePath() + "/turag-" + QDateTime::currentDateTime().toString(Qt::ISODate) + ".turag");
        currentBackend->saveBufferToFile(file);
    }
}

void Controller::refresh(void) {
    availableFrontends.at(currentFrontendIndex)->clear();
    if (currentBackend) {
        currentBackend->reloadData();
    }
}
