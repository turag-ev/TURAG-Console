#include "connectionwidgettcp.h"
#include <QSettings>
#include <QHostInfo>
#include <QGroupBox>

ConnectionWidgetTcp::ConnectionWidgetTcp (QWidget *parent) :
    ConnectionWidget("Letzte Verbindungen", parent),
    selectedDevice(nullptr),
    associatedBackend(nullptr)
{
    setObjectName("Debug Server");
    setToolTip("Mit Doppelklick oder <Enter> Gerät öffnen");


    QSettings settings;
    settings.beginGroup("ConnectionWidgetTcp");
    recentHost = settings.value("host", QString("%1:%2").arg(DEFAULTHOST).arg(CONTROLSERVER_PORT)).toString();

    //Bezeichner (Label) für den hostedit
    QLabel * hostLabel = new QLabel("Host: ");

    //Bezeichner für channel box
    QLabel * channelBoxLabel = new QLabel("Channel: ");

    //Eingabefeld für host
    hostEdit = new QLineEdit();
    hostEdit->setText(recentHost);

    // create button to connect
    connect_button = new QPushButton("Verbinden");

    QHBoxLayout * editLayout = new QHBoxLayout();
    editLayout->addWidget(hostLabel, 0);
    editLayout->addWidget(hostEdit, 1);
    editLayout->addWidget(connect_button, 0);
    editLayout->addStretch();

    //editLayout in generalLayout einfügen
    layout->addLayout(editLayout);

    //channelBoxLabel in generalLayout einfügen
    layout->addWidget(channelBoxLabel);

    //Signals abfangen
    connect(hostEdit, SIGNAL(returnPressed()), this, SLOT(connectToServer()));
    connect(connect_button, SIGNAL(clicked()), this, SLOT(connectToServer()));

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

    //das QListWidget anlegen
    allDevicesWidget = new QListWidget(this);
    allDevicesWidget->setEnabled(false);
    connect(allDevicesWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(startDataChannel(QListWidgetItem*)));


    layout->addWidget(allDevicesWidget);

    //für usability:
    timeText = new QLabel;
    layout->addWidget(timeText);

    socket = new  QTcpSocket(this);
    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(readyRead()), this, SLOT(receive()));

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

    hostEdit->setEnabled(false);
    allDevicesWidget->setEnabled(true);

    tcpMenu->setEnabled(true);
    emergencyStopAction->setDisabled(true);
    startBootloaderAction->setEnabled(true);

    heartBeatTimer.start();

    saveConnection(hostEdit->text());
    addRecentConnections();
    recentConnectionsContainer->setEnabled(false);
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

    recentConnectionsContainer->setEnabled(true);
}

void ConnectionWidgetTcp::socketError(QAbstractSocket::SocketError error) {
    qDebug() << error << socket->errorString();
    connect_button->setEnabled(true);

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

    // restart heartbeat timer because we received something
    heartBeatTimer.start();
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
            associatedBackend->connectionWasLost();
        }
        heartBeatTimer.stop();
    }
}

void ConnectionWidgetTcp::connectToServer() {
    if (!socket->isOpen()) {
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

        QHostInfo hostinfo = QHostInfo::fromName(hostAddress);

        if (hostinfo.addresses().isEmpty()) {
            socket->connectToHost(QHostAddress::Null, port);
        } else {
            socket->connectToHost(QHostAddress(hostinfo.addresses().at(0)), port);
        }
        connect_button->setDisabled(true);
    } else {
        socket->close();
        if (associatedBackend) {
            associatedBackend->closeConnection();
        }
        heartBeatTimer.stop();
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
    BaseBackend* backend;
    if (associatedBackend) {
        disconnect(associatedBackend, SIGNAL(connected(bool,bool)), this, SLOT(backendConnected()));
    }
    emit connectionChanged(connectionString, nullptr, &backend);
    if (backend) {
        associatedBackend = dynamic_cast<TcpBackend*>(backend);
        connect(associatedBackend, SIGNAL(connected(bool,bool)), this, SLOT(backendConnected()));
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

void ConnectionWidgetTcp::checkData(QString path) {
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
