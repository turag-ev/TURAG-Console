#include "controller.h"

#include "backend/filebackend.h"
#include "backend/serialbackend.h"

#include "frontend/plaintextfrontend.h"
#include "frontend/scfrontend.h"
#include "frontend/oscilloscope.h"
#include "frontend/feldbusfrontend.h"

#include "connectionwidgets/connectionwidgetfile.h"
#include "connectionwidgets/connectionwidgetserial.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QTimer>
#include <QMenu>


Controller::Controller(QWidget *parent) :
    QStackedWidget(parent), currentBackend(nullptr), currentFrontendIndex(0),
    autoReconnect(false), connectionShouldBeOpen(false), connectionMenu(nullptr)
{
    // add all available Backends to list with parent this
    availableBackends.append(new FileBackend(this));
    availableBackends.append(new SerialBackend(this));

    // add all available Frontends to list without a parent
    availableFrontends.append(new PlainTextFrontend);
    availableFrontends.append(new SCFrontend);
    availableFrontends.append(new Oscilloscope);
    availableFrontends.append(new FeldbusFrontend);

    // add all available connectionWidgets to list without a parent
    availableConnectionWidgets.append(new ConnectionWidgetSerial);
    availableConnectionWidgets.append(new ConnectionWidgetFile);


    // ---------------------------------------------------------------
    // ---------------------------------------------------------------
    // ---------------------------------------------------------------
    // ---------------------------------------------------------------


    // receive all error and infomessages and connection signals from all available backends
    for (BaseBackend* iter : availableBackends) {
        connect(iter, SIGNAL(errorOccured(QString)), this, SLOT(onErrorOccured(QString)));
        connect(iter, SIGNAL(infoMessage(QString)), this, SLOT(onInfoMessage(QString)));
        connect(iter,SIGNAL(connected(bool,bool)),this,SLOT(onConnected(bool,bool)), Qt::QueuedConnection);
        connect(iter,SIGNAL(disconnected()),this,SLOT(onDisconnected()), Qt::QueuedConnection);
    }


    // build welcome screen
    QVBoxLayout* layout = new QVBoxLayout();
    QWidget* welcome_screen = new QWidget();

    for (auto iter : availableConnectionWidgets) {
        layout->addWidget(iter);
        connect(iter,SIGNAL(connectionChanged(QString, bool*)),this,SLOT(openConnection(QString, bool*)));

        QFrame* line = new QFrame();
        line->setFrameShape(static_cast<QFrame::Shape>(QFrame::HLine));
        line->setFrameShadow(QFrame::Sunken);
        layout->addWidget(line);
    }

    cancelButton = new QPushButton("Abbrechen");
    layout->addSpacing(15);
    layout->addWidget(cancelButton, 0, Qt::AlignLeft);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(onCancelNewConnection()));
    cancelButton->hide();

    layout->addStretch(100);
    welcome_screen->setLayout(layout);


    // fill stackedWidget base class with frontends and connectionWidgets (which are contained in the welcomeScreen)
    for (auto iter : availableFrontends) {
        addWidget(iter);
    }
    addWidget(welcome_screen);

    // set up reconnection timer
    reconnectTimer = new QTimer(this);
    reconnectTimer->setInterval(500);
    connect(reconnectTimer, SIGNAL(timeout()), this, SLOT(onReconnectTimeout()));

    openNewConnection();
}


QList<QString> Controller::getAvailableFrontends(void) const {
    QList<QString> list;

    for (auto iter : availableFrontends) {
        list.append(iter->objectName());
    }

    return list;
}



void Controller::setFrontend(int newFrontendIndex) {
    setFrontend(newFrontendIndex, true);
}

void Controller::setFrontend(int newFrontendIndex, bool calledManually) {
    // only evaluate this function if :
    // - the desired frontend index is valid and
    // - either the chosen frontend stays the same
    // but it is not shown just now or we want to set a new
    // frontend
    if (newFrontendIndex >= availableFrontends.size() ||
            (newFrontendIndex == currentFrontendIndex && currentFrontendIndex == currentIndex())) {
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

        if (currentBackend->isOpen()) {
            connect(currentBackend, SIGNAL(dataReady(QByteArray)), newFrontend, SLOT(writeData(QByteArray)));
            connect(newFrontend, SIGNAL(dataReady(QByteArray)), currentBackend, SLOT(writeData(QByteArray)));
            connect(newFrontend, SIGNAL(requestData()), currentBackend, SLOT(checkData()), Qt::QueuedConnection);
            connect(this,SIGNAL(connected(bool,bool,QIODevice*)),newFrontend,SLOT(onConnected(bool,bool,QIODevice*)));
            connect(this,SIGNAL(disconnected(bool)),newFrontend,SLOT(onDisconnected(bool)));
            if (calledManually) newFrontend->onConnected(currentBackend->isReadOnly(), currentBackend->isSequential(), currentBackend->getDevice());

            // ensure that new frontends receive all data if the backend is
            // a random-access-device
            if (!currentBackend->isSequential()) {
                currentBackend->checkData();
            }
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
        availableFrontends.at(currentFrontendIndex)->clear();

        // build signal-slot connection before opening stream
        BaseFrontend* currentFrontend = availableFrontends.at(currentFrontendIndex);
        connect(currentBackend, SIGNAL(dataReady(QByteArray)), currentFrontend, SLOT(writeData(QByteArray)));
        connect(currentFrontend, SIGNAL(dataReady(QByteArray)), currentBackend, SLOT(writeData(QByteArray)));
        connect(currentFrontend, SIGNAL(requestData()), currentBackend, SLOT(checkData()), Qt::QueuedConnection);

        if (currentBackend->openConnection()) {
            cancelButton->show();
            connectionShouldBeOpen = true;

            if (!currentBackend->isSequential()) {
                currentBackend->checkData();
            }

        } else {
            // destroy signal-slot connection in case of failure
            currentBackend->disconnect(currentFrontend);
            currentFrontend->disconnect(currentBackend);
        }
    }
}


void Controller::openConnection(QString connectionString, bool *success) {
    closeConnection();

    BaseFrontend* currentFrontend = availableFrontends.at(currentFrontendIndex);
    currentFrontend->clear();

    auto iter = std::find_if(availableBackends.begin(),
                             availableBackends.end(),
                             [&](const BaseBackend* b) -> bool { return b->canHandleUrl(connectionString); });

    if (iter != availableBackends.end()) {
        // we got a backend
        BaseBackend* backend = *iter;

        // build signal-slot connection before opening stream
        connect(backend, SIGNAL(dataReady(QByteArray)), currentFrontend, SLOT(writeData(QByteArray)));
        connect(currentFrontend, SIGNAL(dataReady(QByteArray)), backend, SLOT(writeData(QByteArray)));
        connect(currentFrontend, SIGNAL(requestData()), backend, SLOT(checkData()), Qt::QueuedConnection);

        if (backend->openConnection(connectionString)) {
            currentBackend = backend;
            cancelButton->show();
            setFrontend(currentFrontendIndex, false);
            cancelButton->hide();
            *success = true;
            connectionShouldBeOpen = true;
            return;
        }

        // destroy signal-slot connection if it was not the right backend
        backend->disconnect(currentFrontend);
        currentFrontend->disconnect(backend);
    }

    *success = false;
}


void Controller::closeConnection(void) {
    if (currentBackend == nullptr) {
        return;
    } else {
        currentBackend->closeConnection();
        connectionShouldBeOpen = false;
        if (autoReconnect && connectionShouldBeOpen && reconnectTimer->isActive()) {
            reconnectTimer->stop();
            emit disconnected(false);
        }

        BaseFrontend* currentFrontend = availableFrontends.at(currentFrontendIndex);
        currentBackend->disconnect(currentFrontend);
        currentFrontend->disconnect(currentBackend);
    }
}


void Controller::saveOutput(void) {
    if (availableFrontends.at(currentFrontendIndex)->saveOutput()) {
        emit infoMessage("Ausgabe erfolgreich geschrieben");
    } else {
        emit errorOccured("Ausgabe schreiben fehlgeschlagen");
    }
}


void Controller::openNewConnection(void) {
    setCurrentIndex(availableFrontends.size());
}

void Controller::onCancelNewConnection() {
    setCurrentIndex(currentFrontendIndex);
}


void Controller::setExternalContextActions(QList<QAction*> actions) {
    for (auto iter : availableFrontends) {
        iter->setExternalContextActions(actions);
    }
}


void Controller::setAutoReconnect(bool on) {
    autoReconnect = on;
    if (autoReconnect && connectionShouldBeOpen && currentBackend && !currentBackend->isOpen()) {
        reconnectTimer->start();
    } else if (!autoReconnect && reconnectTimer->isActive()) {
        connectionShouldBeOpen = false;
        reconnectTimer->stop();
        emit disconnected(false);
    }
}


void Controller::onConnected(bool readOnly, bool isSequential) {
    if (autoReconnect && connectionShouldBeOpen && reconnectTimer->isActive()) {
        reconnectTimer->stop();
        emit infoMessage("Verbindung erfolgreich wiederaufgebaut");
    }

    if (connectionMenu) {
        connectionMenu->addActions(currentBackend->getMenuEntries());
    }

    emit connected(readOnly, isSequential, currentBackend->getDevice());
}

void Controller::onDisconnected() {
    if (autoReconnect && connectionShouldBeOpen) {
        reconnectTimer->start();
        emit disconnected(true);
    } else {
        emit disconnected(false);
    }

    if (connectionMenu) {
        for (QAction* action : currentBackend->getMenuEntries()) {
            connectionMenu->removeAction(action);
        }
    }
}

void Controller::onErrorOccured(QString msg) {
    // only forward error messages if we are not reconnecting
    if (!(autoReconnect && connectionShouldBeOpen && currentBackend && !currentBackend->isOpen())) {
        emit errorOccured(msg);
    }
}

void Controller::onInfoMessage(QString msg) {
    // only forward info messages if we are not reconnecting
    if (!(autoReconnect && connectionShouldBeOpen && currentBackend && !currentBackend->isOpen())) {
        emit infoMessage(msg);
    }
}


void Controller::onReconnectTimeout() {
    if (autoReconnect && connectionShouldBeOpen && currentBackend && !currentBackend->isOpen()) {
        emit infoMessage("Versuche wiederzuverbinden...");
        openConnection();
    } else {
        reconnectTimer->stop();
    }
}
