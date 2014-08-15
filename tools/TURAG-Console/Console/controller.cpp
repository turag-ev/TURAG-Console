#include "controller.h"

#include "backend/filebackend.h"
#include "backend/serialbackend.h"
#include "backend/tcpbackend.h"

#include "frontend/plaintextfrontend.h"
#include "frontend/scfrontend.h"
#include "frontend/oscilloscope.h"
#include "frontend/feldbusfrontend.h"
#include "frontend/mcfrontend.h"
#include "frontend/stmcamfrontend.h"

#include "connectionwidgets/connectionwidgetfile.h"
#include "connectionwidgets/connectionwidgetserial.h"
#include "connectionwidgets/connectionwidgettcp.h"

#include <QVBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QMenu>
#include <QTabWidget>
#include <QLabel>
#include <QSettings>
#include <QMenuBar>
#include <QDateTime>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QFrame>
#include <QScrollArea>


Controller::Controller(QWidget *parent) :
    QStackedWidget(parent), currentBackend(nullptr), currentFrontendIndex(0),
    menuBar_(nullptr), widgetMenu_(nullptr), autoSaveOn(false)
{
    // add all available Backends to list with parent this
    availableBackends.append(new FileBackend(this));
    availableBackends.append(new SerialBackend(this));
    TcpBackend* tcpbackend = new TcpBackend(this);
    availableBackends.append(tcpbackend);

    // add all available Frontends to list without a parent
    availableFrontends.append(new PlainTextFrontend);
    //availableFrontends.append(new SCFrontend);
    availableFrontends.append(new MCFrontend);
    availableFrontends.append(new Oscilloscope);
    availableFrontends.append(new FeldbusFrontend);
    availableFrontends.append(new STMCamFrontend);

    // add all available connectionWidgets to list without a parent
    availableConnectionWidgets.append(new ConnectionWidgetSerial);
    availableConnectionWidgets.append(new ConnectionWidgetFile);
    ConnectionWidgetTcp* connectionwidgettcp = new ConnectionWidgetTcp;
    availableConnectionWidgets.append(connectionwidgettcp);


    // ---------------------------------------------------------------
    // ---------------------------------------------------------------
    // ---------------------------------------------------------------
    // ---------------------------------------------------------------


    // receive all error and infomessages and connection signals from all available backends
    for (BaseBackend* iter : availableBackends) {
        connect(iter,SIGNAL(connected(bool,bool)),this,SLOT(onConnected(bool,bool)), Qt::QueuedConnection);
        connect(iter,SIGNAL(disconnected(bool)),this,SLOT(onDisconnected()), Qt::QueuedConnection);
        connect(iter,SIGNAL(disconnected(bool)),this,SIGNAL(disconnected(bool)), Qt::QueuedConnection);
    }


    // special feature for tcp connections
    connect(tcpbackend, SIGNAL(requestData(QString)), connectionwidgettcp, SLOT(requestData(QString)));

    // build welcome screen
    QVBoxLayout* layout = new QVBoxLayout();
    QWidget* welcome_screen = new QWidget();

    tabwidget = new QTabWidget;

    for (ConnectionWidget* iter : availableConnectionWidgets) {
        QScrollArea* scrollarea = new QScrollArea;
        scrollarea->setWidgetResizable(true);
        scrollarea->setWidget(iter);
        scrollarea->setFrameShape(QFrame::NoFrame);

        tabwidget->addTab(scrollarea, iter->objectName());
        connect(iter,SIGNAL(connectionChanged(QString, bool*,BaseBackend**)),this,SLOT(openConnection(QString, bool*,BaseBackend**)));
    }
    layout->addWidget(tabwidget);
    connect(tabwidget, SIGNAL(currentChanged(int)), this, SLOT(onToolboxChangedCurrent(int)));

    if (tabwidget->count() > 0) {
        QSettings settings;
        settings.beginGroup("Controller");
        tabwidget->setCurrentIndex(settings.value("currentIndex", 0).toInt());
    }

    cancelButton = new QPushButton("Abbrechen");
    layout->addSpacing(5);
    layout->addWidget(cancelButton, 0, Qt::AlignLeft);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelNewConnection()));

    welcome_screen->setLayout(layout);


    // fill stackedWidget base class with frontends and connectionWidgets (which are contained in the welcomeScreen)
    for (auto iter : availableFrontends) {
        addWidget(iter);
    }
    addWidget(welcome_screen);

    openNewConnection();
}


Controller::~Controller() {
}


void Controller::onToolboxChangedCurrent(int index) {
    QSettings settings;
    settings.beginGroup("Controller");
    settings.setValue("currentIndex", index);
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

        connect(currentBackend, SIGNAL(dataReady(QByteArray)), newFrontend, SLOT(writeData(QByteArray)));
        connect(newFrontend, SIGNAL(dataReady(QByteArray)), currentBackend, SLOT(writeData(QByteArray)));
        connect(newFrontend, SIGNAL(requestData()), this, SLOT(refresh()), Qt::QueuedConnection);
        connect(this,SIGNAL(connected(bool,bool,QIODevice*)),newFrontend,SLOT(onConnected(bool,bool,QIODevice*)));
        connect(this,SIGNAL(disconnected(bool)),newFrontend,SLOT(onDisconnected(bool)));
        if (currentBackend->isOpen()) {
            if (calledManually) newFrontend->onConnected(currentBackend->isReadOnly(), currentBackend->isBuffered(), currentBackend->getDevice());
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
    emit newConnectionDialogStateChanged(false);
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


void Controller::openConnection(QString connectionString, bool *success, BaseBackend** openedBackend) {
    // show widgets menu
    if (menuBar_) {
        if (widgetMenu_ && menuBar_->actions().contains(widgetMenu_->menuAction())) {
            menuBar_->removeAction(widgetMenu_->menuAction());
        }
        ConnectionWidget * connectionWidget = static_cast<ConnectionWidget * >(sender());
        if (connectionWidget) {
            widgetMenu_ = connectionWidget->getMenu();
            if (widgetMenu_) {
                menuBar_->addMenu(widgetMenu_);
            }
        }
    }


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
        connect(currentFrontend, SIGNAL(requestData()), this, SLOT(refresh()), Qt::QueuedConnection);

        if (backend->openConnection(connectionString)) {
            currentBackend = backend;
            setFrontend(currentFrontendIndex, false);
            if (success) *success = true;
            if (openedBackend) *openedBackend = currentBackend;
            return;
        }

        // destroy signal-slot connection if connecting failed
        backend->disconnect(currentFrontend);
        currentFrontend->disconnect(backend);
    }

    if (success) *success = false;
    if (openedBackend) *openedBackend = currentBackend;
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

void Controller::openNewConnection(void) {
    setCurrentIndex(availableFrontends.size());
    emit newConnectionDialogStateChanged(true);
}

void Controller::cancelNewConnection() {
    setCurrentIndex(currentFrontendIndex);
    emit newConnectionDialogStateChanged(false);
}


void Controller::setExternalContextActions(QList<QAction*> actions) {
    for (auto iter : availableFrontends) {
        iter->setExternalContextActions(actions);
    }
}


void Controller::setAutoReconnect(bool on) {
    for (BaseBackend* backend : availableBackends) {
        backend->setDeviceRecovery(on);
    }
}

void Controller::onConnected(bool readOnly, bool isBuffered) {
    if (connectionMenu) {
        connectionMenu->addActions(currentBackend->getMenuEntries());
    }

    refresh();

    emit connected(readOnly, isBuffered, currentBackend->getDevice());
}

void Controller::onDisconnected() {
    if (autoSaveOn) {
        QString file = QDir::toNativeSeparators(QDir::homePath() + "/turag-" + QDateTime::currentDateTime().toString(Qt::ISODate) + ".turag");
        currentBackend->saveBufferToFile(file);
    }
    if (connectionMenu) {
        for (QAction* action : currentBackend->getMenuEntries()) {
            connectionMenu->removeAction(action);
        }
    }
}

void Controller::refresh(void) {
    availableFrontends.at(currentFrontendIndex)->clear();
    if (currentBackend) {
        currentBackend->reloadData();
    }
}
