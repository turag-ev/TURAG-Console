#include "connectionwidgettcp.h"
#include <QSettings>
#include <QHostInfo>
#include <QGroupBox>
#include <QSplitter>

ConnectionWidgetTcp::ConnectionWidgetTcp (QWidget *parent) :
    ConnectionWidget("Letzte Verbindungen", parent),
    selectedDevice(nullptr),
    associatedBackend(nullptr)
{
    setObjectName("Debug Server");

    socket = new  QTcpSocket(this);
    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(readyRead()), this, SLOT(receive()));


    QSettings settings;
    settings.beginGroup("ConnectionWidgetTcp");
    recentHost = settings.value("host", QString("%1:%2").arg(DEFAULTHOST).arg(CONTROLSERVER_PORT)).toString();

    // Eingabemaske für Host
    // --------------------------------------------------
    QLabel * hostLabel = new QLabel("Host: ");

    //Eingabefeld für host
    hostEdit = new QLineEdit();
    hostEdit->setText(recentHost);
    connect(hostEdit, SIGNAL(returnPressed()), this, SLOT(connectToServer()));

    // create button to connect
    connect_button = new QPushButton("Verbinden");
    connect(connect_button, SIGNAL(clicked()), this, SLOT(connectToServer()));

    connect_cancel_button = new QPushButton("Abbrechen");
    connect_cancel_button->setVisible(false);
    connect(connect_cancel_button, SIGNAL(clicked()), this, SLOT(cancel_connecting()));

    QHBoxLayout * editLayout = new QHBoxLayout();
    editLayout->addWidget(hostLabel, 0);
    editLayout->addWidget(hostEdit, 1);
    editLayout->addWidget(connect_button, 0);
    editLayout->addWidget(connect_cancel_button, 0);
    editLayout->addStretch();



    // Channel Box
    // ----------------------------------------
    QLabel * channelBoxLabel = new QLabel("Channel: ");

    //das QListWidget anlegen
    allDevicesWidget = new QListWidget(this);
    allDevicesWidget->setEnabled(false);
    allDevicesWidget->setToolTip("Mit Doppelklick oder <Enter> Gerät öffnen");
    connect(allDevicesWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(startDataChannel(QListWidgetItem*)));

    //für usability:
    timeText = new QLabel;

    QVBoxLayout* channelBoxLayout = new QVBoxLayout;
    channelBoxLayout->addWidget(channelBoxLabel);
    channelBoxLayout->addWidget(allDevicesWidget);
    channelBoxLayout->addWidget(timeText);


    // Server log
    // ----------------------------------------
    QLabel * serverLogLabel = new QLabel("Serverlog:");
    serverLog = new QListWidget;
    serverLog->setEnabled(false);
    serverLog->setSelectionMode(QAbstractItemView::NoSelection);

    QVBoxLayout* serverLogLayout = new QVBoxLayout;
    serverLogLayout->addWidget(serverLogLabel);
    serverLogLayout->addWidget(serverLog);


    // add stuff to layout
    // ----------------------------------------
    QWidget* leftWidget = new QWidget;
    leftWidget->setLayout(channelBoxLayout);
    QWidget* rightWidget = new QWidget;
    rightWidget->setLayout(serverLogLayout);

    QSplitter* splitter = new QSplitter;
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(1, 2);

    layout->addLayout(editLayout);
    layout->addWidget(splitter);


    //tcp menu erstellen
    tcpMenu = new QMenu("Debug-Server", this);
    emergencyStopAction = new QAction("Notaus", this);
    emergencyStopAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    emergencyStopAction->setDisabled(true);
    connect(emergencyStopAction, SIGNAL(triggered()), this, SLOT(emergencyStop()));

    startBootloaderAction = new QAction("&Bootloader starten", this);
    startBootloaderAction->setShortcut(Qt::CTRL + Qt::Key_B);
    startBootloaderAction->setDisabled(true);
    connect(startBootloaderAction, SIGNAL(triggered()), this, SLOT(reset()));

    tcpMenu->addAction(emergencyStopAction);
    tcpMenu->addAction(startBootloaderAction);


    setContextMenuPolicy(Qt::CustomContextMenu);
    contextMenu = new QMenu(this);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showContextMenu(QPoint)));
    bootloaderContextAction = new QAction(this);
    connect(bootloaderContextAction, SIGNAL(triggered()), this, SLOT(resetFromContextMenu()));
    contextMenu->addAction(bootloaderContextAction);

    heartBeatTimer.setInterval(3 * HEARTBEAT_SEND_PERIOD);
    connect(&heartBeatTimer, SIGNAL(timeout()), this, SLOT(heartBeatTimerOccured()));

    layout->addSpacing(10);
    addRecentConnections();
}

ConnectionWidgetTcp::~ConnectionWidgetTcp() {
    QSettings settings;
    settings.beginGroup("ConnectionWidgetTcp");

    settings.setValue("host", hostEdit->text());

    for (device* dev : allDevices) {
        delete dev;
    }
}

void ConnectionWidgetTcp::socketConnected(void) {
    connect_button->setText("Trennen");
    connect_button->setEnabled(true);
    connect_button->setVisible(true);
    connect_cancel_button->setVisible(false);

    allDevicesWidget->setEnabled(true);

    tcpMenu->setEnabled(true);
    emergencyStopAction->setDisabled(true);
    startBootloaderAction->setEnabled(true);

    heartBeatTimer.start();
    timeText->setText("Serverzeit: ");

    saveConnection(hostEdit->text());
    addRecentConnections();
    if (recentConnectionsContainer) recentConnectionsContainer->setEnabled(false);

    serverLog->clear();
    serverLog->setEnabled(true);
}

void ConnectionWidgetTcp::socketDisconnected(void) {
    connect_button->setText("Verbinden");
    connect_button->setEnabled(true);

    timeText->setText("");
    hostEdit->setEnabled(true);
    allDevicesWidget->setEnabled(false);

    tcpMenu->setDisabled(true);
    startBootloaderAction->setEnabled(false);

    selectedDevice = nullptr;
    for (device* dev : allDevices) {
        delete dev;
    }
    allDevices.clear();
    allDevicesWidget->clear();

    if (recentConnectionsContainer) recentConnectionsContainer->setEnabled(true);
    serverLog->setEnabled(false);
}

void ConnectionWidgetTcp::socketError(QAbstractSocket::SocketError error) {
    qDebug() << error << socket->errorString();
    connect_button->setEnabled(true);
    connect_button->setVisible(true);
    connect_cancel_button->setVisible(false);
    if (recentConnectionsContainer) recentConnectionsContainer->setEnabled(true);
    hostEdit->setEnabled(true);

    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        if (socket->isOpen()) socket->close();
        emit errorOccured("Connection refused");
        break;
    case QAbstractSocket::HostNotFoundError:
        if (socket->isOpen()) socket->close();
        emit errorOccured("Host not found");
        break;
    case QAbstractSocket::RemoteHostClosedError:
        if (socket->isOpen()) socket->close();
        emit errorOccured("Remote host closed");
        break;
    case QAbstractSocket::NetworkError:
        if (socket->isOpen()) socket->close();
        emit errorOccured("Host unreachebale");
        break;
    default:
        break;
    }

}

void ConnectionWidgetTcp::handleData() {
    if (puffer.size() > 0) {
        if (puffer.at(0) == QByteArray(DEVICE)) {
            /*
         *Das Übermitteln der Devices läuft wie folgt ab:
         *I.Ankündigen, dass man devices übermittelt
         *II.Anzahl der Devices übermitteln
         *III.Die Eigenschaften der einzelnen Devices übertragen
         *  1)path
         *  2)port
         *  3)description
         */
            if (puffer.size() > 1) {
                bool ok;
                int amount = puffer.at(1).toInt(&ok);

                if (ok && puffer.size() > 1 + amount * 4) {
                    int i;
                    //int countDevice = 0; //ich muss mir merken, in welchem device ich bin
                    for (i = 0; i < amount; i++) {
                        device * newDevice = new device;
                        newDevice->path = QString(puffer.at(2 + i * 3));
                        newDevice->port = QString(puffer.at(3 + i * 3));
                        newDevice->description = QString(puffer.at(4 + i * 3));
                        allDevices.append(newDevice);
                    }
                    QString online_status_line;
                    //amount liegt auf puffer.at(1), dann kommen die devices, 3 * amount
                    int offset = 1 + amount * 3 + 1;

                    for (i = 0; i < amount; i++) {
                        online_status_line = puffer.at(i + offset);

                        if (online_status_line.endsWith("online")) {
                            allDevices.at(i)->onlineStatus = true;
                        } else {
                            allDevices.at(i)->onlineStatus = false;
                        }
                    }
                    fillDeviceList();
                }
            }
        }
        else if (puffer.at(0) == QByteArray(ONLINESTATUS) && puffer.size() > 1) {
            bool online;
            QString path;
            QString line(puffer.at(1));

            if (line.endsWith(QString(ONLINESTATUS_ONLINE))) {
                path = line.left(line.size() - QString(ONLINESTATUS_ONLINE).size() - 1);
                online = true;
            } else if (line.endsWith(QString(ONLINESTATUS_OFFLINE))) {
                path = line.left(line.size() - QString(ONLINESTATUS_OFFLINE).size() - 1);
                online = false;
            } else {
                return;
            }

            for (device* dev : allDevices) {
                if (dev->path == path) {
                    dev->onlineStatus = online;
                    break;
                }
            }
            fillDeviceList();
        }
        else if (puffer.at(0) == QByteArray(CONTROL_TIME) && puffer.size() > 1) {
            timeText->setText(QString("Serverzeit: ").append(puffer.at(1)));
        }
        else if (puffer.at(0) == QByteArray(STATUS_MESSAGE) && puffer.size() > 1) {
            for (int i = 1; i < puffer.size(); ++i) {
                QListWidgetItem * item = new QListWidgetItem();
                item->setText(puffer.at(i));
                serverLog->addItem(item);
            }
        }
        else {
            //dont know what to do
            qDebug() << "Unknown data package type, received: " << endl;
            int i;
            for (i = 0; i < puffer.size(); i++) {
                qDebug() << puffer.at(i) << endl;
            }
        }
        puffer.clear();
    }
}

void ConnectionWidgetTcp::fillDeviceList(void) {
    allDevicesWidget->clear();

    for (device* currentDevice : allDevices) {
        QString descr(currentDevice->description);
        QListWidgetItem * item = new QListWidgetItem();

        if (currentDevice->onlineStatus) {
            descr.append(" <online>");
        }
        else {
            descr.append(" <offline>");
            item->setTextColor(Qt::red);
        }

        if (currentDevice == selectedDevice) {
            descr.append(" <aktiv>");
        }

        item->setText(descr);

        allDevicesWidget->addItem(item);
    }
    int size = 0;
    for (int i = 0; i < allDevicesWidget->count(); ++i) {
        size += allDevicesWidget->sizeHintForRow(i);
    }
    allDevicesWidget->setMinimumHeight(size + allDevicesWidget->sizeHintForRow(0));
}

void ConnectionWidgetTcp::send(QByteArray data) {
    data.append("\n");
    socket->write(data);
}

void ConnectionWidgetTcp::send(QString string) {
    send(string.toLatin1());
}


/*
 *
 *SLOTS
 */
void ConnectionWidgetTcp::heartBeatTimerOccured(void) {
    if (socket->isOpen()) {
        socket->close();
        if (associatedBackend) {
            // if we would use connectionWasLost() here, we could make use of the
            // autoReconnect-feature. The trouble with this is, that most likely the server is offline
            // which results in connections attempts that time out before failing.
            // It then becomes nearly impossible to
            // open a new connection. That's why we just close the connection here.
            associatedBackend->closeConnection();
        }
        heartBeatTimer.stop();
    }
}

void ConnectionWidgetTcp::connectToServer() {
    if (!socket->isOpen()) {
        QSettings settings;
        settings.beginGroup("ConnectionWidgetTcp");
        settings.setValue("host", hostEdit->text());

        qint16 port;
        QString hostAddress;
        QString host = hostEdit->text().toCaseFolded();

        int index = host.indexOf(":");

        if (index == -1) {
            port = CONTROLSERVER_PORT;
            hostAddress = host;
            if (hostAddress.isEmpty()) {
                hostAddress = "127.0.0.1";
            }
        } else {
            hostAddress = host.left(index);
            if (hostAddress.isEmpty()) {
                hostAddress = "127.0.0.1";
            }

            if (host.endsWith(":")) {
                port = CONTROLSERVER_PORT;
            } else {
                port = host.right(host.size() - index - 1).toInt();
            }
        }

        connect_button->setEnabled(false);
        connect_button->setVisible(false);
        connect_cancel_button->setVisible(true);
        hostEdit->setEnabled(false);
        if (recentConnectionsContainer) recentConnectionsContainer->setEnabled(false);

        socket->connectToHost(hostAddress, port);
    } else {
        socket->close();
        if (associatedBackend) {
            associatedBackend->closeConnection();
        }
        heartBeatTimer.stop();
    }
}

void ConnectionWidgetTcp::cancel_connecting(void) {
    if (socket) {
        socket->abort();
        connect_button->setEnabled(true);
        connect_button->setVisible(true);
        connect_cancel_button->setVisible(false);
        hostEdit->setEnabled(true);
        if (recentConnectionsContainer) recentConnectionsContainer->setEnabled(true);

    }
}

void ConnectionWidgetTcp::emergencyStop() {
    //steht noch aus hat geringste Priorität

}


void ConnectionWidgetTcp::reset() {
    QByteArray data(RESET_DEVICE);
    data.append(" ");
    data.append(selectedDevice->path);
    send(data);
}

void ConnectionWidgetTcp::resetFromContextMenu() {
    if (allDevicesWidget->currentRow() >= 0 && allDevicesWidget->currentRow() < allDevicesWidget->count()) {
        device * newSelectedDevice = allDevices.at(allDevicesWidget->currentRow());

        if (newSelectedDevice->onlineStatus == false) {
            QByteArray data(LEAVE_BOOTLOADER);
            data.append(" ");
            data.append(newSelectedDevice->path);
            send(data);

        } else {
            QByteArray data(RESET_DEVICE);
            data.append(" ");
            data.append(newSelectedDevice->path);
            send(data);
        }
    }
}

void ConnectionWidgetTcp::receive() {
    // restart heartbeat timer because we received something
    heartBeatTimer.start();

    while (socket->canReadLine()) {
        QByteArray data(socket->readLine(150));
        data.chop(1);
        if (data == QByteArray(TERMINATE)) {
            handleData();
        } else {
            puffer.append(data);
        }
    }
}

void ConnectionWidgetTcp::startDataChannel(QListWidgetItem * item) {
    device * newSelectedDevice = allDevices.at(allDevicesWidget->row(item));

    if (newSelectedDevice->onlineStatus == false) {
        return;
    }
    selectedDevice = newSelectedDevice;
    fillDeviceList();

    //den connection String zusammenbasteln
    QString connectionString("tcp://");
    connectionString.append(socket->peerAddress().toString());
    connectionString.append(":");

    connectionString.append(selectedDevice->port);
    connectionString.append("/");
    connectionString.append(selectedDevice->path);
    connectionString.append(":");
    connectionString.append(selectedDevice->description);

    //Signal emitten mit dem connectionstring;
    //save connectionString hat hier keine Bedeutung
    if (associatedBackend) {
        disconnect(associatedBackend, SIGNAL(connected(bool,bool)), this, SLOT(backendConnected()));
        disconnect(associatedBackend, SIGNAL(dataReady(QByteArray)), &heartBeatTimer, SLOT(start()));

        // tcp backend might be in auto reconnect mode - in that case we couldn't do a manual reconnect
        // so we have to do a manual disconnect
        associatedBackend->closeConnection();
    }
    BaseBackend* backend;
    emit connectionChanged(connectionString, nullptr, &backend);
    if (backend) {
        associatedBackend = dynamic_cast<TcpBackend*>(backend);
        connect(associatedBackend, SIGNAL(connected(bool,bool)), this, SLOT(backendConnected()));
        connect(associatedBackend, SIGNAL(dataReady(QByteArray)), &heartBeatTimer, SLOT(start()), Qt::UniqueConnection);
    }
}


void ConnectionWidgetTcp::showContextMenu(const QPoint & pos) {
    if (allDevicesWidget->currentRow() >= 0 && allDevicesWidget->currentRow() < allDevicesWidget->count()) {
        device * newSelectedDevice = allDevices.at(allDevicesWidget->currentRow());

        if (newSelectedDevice->onlineStatus == false) {
            bootloaderContextAction->setText("Bootloader verlassen/Programm starten");
        } else {
            bootloaderContextAction->setText("Bootloader starten/Programm verlassen");
        }
        QPoint globalPos = mapToGlobal(pos);
        contextMenu->popup(globalPos);
    }
}

void ConnectionWidgetTcp::requestData(QString path) {
    if (!selectedDevice && associatedBackend) {
        QString path = associatedBackend->getDevicePath();

        for (device* dev : allDevices) {
            if (dev->path == path) {
                selectedDevice = dev;
                break;
            }
        }
        fillDeviceList();
    }

    if (selectedDevice && path == selectedDevice->path && socket->isOpen()) {
        QByteArray data(PRINT_BUFFER);
        data.append(" ");
        data.append(selectedDevice->path);
        data.append(" ");
        data.append(socket->localAddress().toString());
        send(data);

    }
}


void ConnectionWidgetTcp::onOpenRecentConnection(int index) {
    if (!socket->isOpen()) {
        hostEdit->setText(recent_connections.at(index));
        connectToServer();
    }
}

// this slot is important for the case that the datachannel is connected
// before the control channel
void ConnectionWidgetTcp::backendConnected() {
    if (!socket->isOpen()) {
        connectToServer();
    }
}
