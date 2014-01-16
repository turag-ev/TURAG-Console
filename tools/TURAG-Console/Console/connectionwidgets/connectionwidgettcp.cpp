#include "connectionwidgettcp.h"
#include <QSettings>
#include <QHostInfo>

ConnectionWidgetTcp::ConnectionWidgetTcp (QWidget *parent) :
    ConnectionWidget("Letzte Verbindungen", parent),
    selectedDevice(nullptr),
    writeAccess(false),
    associatedBackend(nullptr)
{

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

    //GesamtLayout ist ein QVBoxLayout
    generalLayout = new QVBoxLayout();

    QHBoxLayout * editLayout = new QHBoxLayout();
    editLayout->addWidget(hostLabel, 0);
    editLayout->addWidget(hostEdit, 1);
    editLayout->addWidget(connect_button, 0);
    editLayout->addStretch();

    //editLayout in generalLayout einfügen
    generalLayout->addLayout(editLayout);

    //channelBoxLabel in generalLayout einfügen
    generalLayout->addWidget(channelBoxLabel);

    setLayout(generalLayout);

    //Signals abfangen
    connect(hostEdit, SIGNAL(returnPressed()), this, SLOT(connectToServer()));
    connect(connect_button, SIGNAL(clicked()), this, SLOT(connectToServer()));

    setObjectName("Debug Server");

    //tcp menu erstellen
    tcpMenu = new QMenu("Debug-Server", this);
    emergencyStopAction = new QAction("Notaus", this);
    emergencyStopAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    emergencyStopAction->setDisabled(true);
    connect(emergencyStopAction, SIGNAL(triggered()), this, SLOT(emergencyStop()));

    requestWriteAccessAction = new QAction("Schreibrechte anfordern", this);
    requestWriteAccessAction->setShortcut(Qt::CTRL + Qt::Key_W);
    requestWriteAccessAction->setDisabled(true);
    connect(requestWriteAccessAction, SIGNAL(triggered()), this, SLOT(onRequestWriteAccess()));


    //frage: wie teilen wir das dem datachannel mit?
    startBootloaderAction = new QAction("&Bootloader starten", this);
    startBootloaderAction->setShortcut(Qt::CTRL + Qt::Key_B);
    startBootloaderAction->setDisabled(true);
    connect(startBootloaderAction, SIGNAL(triggered()), this, SLOT(reset()));

    //QAction, die Schreibrechte mit Gewalt einfordert
    requestWriteAccessActionForce = new QAction("Schreibrechte erzwingen", this);
    requestWriteAccessActionForce->setDisabled(true);
    requestWriteAccessActionForce->setToolTip("Meister Yoda sagt: Die Macht dir helfen wird zu erlangen die Schreibrechte");
    connect(requestWriteAccessActionForce, SIGNAL(triggered()), this, SLOT(forceWriteAccess()));

    //submenu um Schreibrechte mit Gewalt an sich zu reißen
    QMenu * extended = new QMenu("Erweitert", this);
    extended->addAction(requestWriteAccessActionForce);

    tcpMenu->addAction(requestWriteAccessAction);
    tcpMenu->addAction(emergencyStopAction);
    tcpMenu->addAction(startBootloaderAction);
    tcpMenu->addMenu(extended);

    //das QListWidget anlegen
    allDevicesWidget = new QListWidget(this);
    connect(allDevicesWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(startDataChannel(QListWidgetItem*)));


    generalLayout->addWidget(allDevicesWidget);

    //für usability:
    bottomInfoText = new QLabel("Doppelklick öffnet den Datachannel zum gewählten Gerät", this);
    generalLayout->addWidget(bottomInfoText);

    socket = new  QTcpSocket(this);
    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(readyRead()), this, SLOT(receive()));
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

    tcpMenu->setEnabled(true);
    requestWriteAccessAction->setEnabled(true);
    emergencyStopAction->setDisabled(true);
    startBootloaderAction->setDisabled(true);

    onRequestWriteAccess();
}

void ConnectionWidgetTcp::socketDisconnected(void) {
    connect_button->setText("Verbinden");
    connect_button->setEnabled(true);

    tcpMenu->setDisabled(true);
    requestWriteAccessAction->setDisabled(true);
    requestWriteAccessActionForce->setDisabled(true);

    writeAccess = false;

    for (device* dev : allDevices) {
        delete dev;
    }
    allDevices.clear();
    allDevicesWidget->clear();
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

    if (puffer.at(0) == WADENIED) {
        //hier machen, was passieren soll, wenn man Schreibrechte erhalten hat
        writeAccess = false;
        requestWriteAccessAction->setText("Schreibrechte anfordern");
        requestWriteAccessActionForce->setEnabled(true);
        if (associatedBackend) {
            associatedBackend->setWriteAccess(false);
            startBootloaderAction->setEnabled(false);
        }
        emit errorOccured("Schreibrechte nicht gewährt/verloren");
    }
    else if (puffer.at(0) == WAGRANTED) {
        writeAccess = true;
        requestWriteAccessAction->setText("Schreibrechte aufgeben");
        requestWriteAccessActionForce->setDisabled(true);
        if (associatedBackend) {
            associatedBackend->setWriteAccess(true);
            startBootloaderAction->setEnabled(true);
        }
    }
    else if (puffer.at(0) == DEVICE) {
        /*
         *Das Übermitteln der Devices läuft wie folgt ab:
         *I.Ankündigen, dass man devices übermittelt
         *II.Anzahl der Devices übermitteln
         *III.Die Eigenschaften der einzelnen Devices übertragen
         *  1)path
         *  2)port
         *  3)description
         *  4)reset code
         *  5)baudrate
         */
        int amount = puffer.at(1).toInt();
        int i;
        //int countDevice = 0; //ich muss mir merken, in welchem device ich bin
        for (i = 0; i < amount; i++) {
            device * newDevice = new device;
            newDevice->path = QString(puffer.at(2 + i * 5));
            newDevice->port = QString(puffer.at(3 + i * 5));
            newDevice->description = QString(puffer.at(4 + i * 5));
            newDevice->resetCode = QString(puffer.at(5 + i * 5));
            newDevice->baudRate = QString(puffer.at(6 + i * 5));
            allDevices.append(newDevice);
        }
        QString online_status_line;
        //amount liegt auf puffer.at(1), dann kommen die devices, 5 * amount
        int offset = 1 + amount * 5 + 1;

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
    else if (puffer.at(0) == ONLINESTATUS) {
        bool online;
        QString path;
        QString line(puffer.at(1));

        if (line.endsWith(ONLINESTATUS_ONLINE)) {
            path = line.left(line.size() - ONLINESTATUS_ONLINE.size() - 1);
            online = true;
        } else if (line.endsWith(ONLINESTATUS_OFFLINE)) {
            path = line.left(line.size() - ONLINESTATUS_OFFLINE.size() - 1);
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

void ConnectionWidgetTcp::receiveData(QByteArray * data) {
    *data = socket->readLine(150);
    data->chop(1);
}

void ConnectionWidgetTcp::send(QByteArray data) {
    data.append("\n");
    socket->write(data);
}

void ConnectionWidgetTcp::send(QString string) {
    send(string.toLatin1());
}


QMenu* ConnectionWidgetTcp::getMenu() {
    return tcpMenu;
}

/*
 *
 *SLOTS
 */
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
    }
}

void ConnectionWidgetTcp::emergencyStop() {
    //steht noch aus hat geringste Priorität

}

void ConnectionWidgetTcp::onRequestWriteAccess() {
    if (writeAccess == true) {
        send(READONLY);
    }
    else {
        send(WRITEACCESS);
    }
}

void ConnectionWidgetTcp::reset() {
    send(RESET_DEVICE);
    send(selectedDevice->path);
}

void ConnectionWidgetTcp::receive() {
    while (socket->canReadLine()) {
        QByteArray data;
        receiveData(&data);
        if (data == TERMINATE) {
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
    connectionString.append(selectedDevice->description);

    //Signal emitten mit dem connectionstring;
    //save connectionString hat hier keine Bedeutung
    BaseBackend* backend;
    emit connectionChanged(connectionString, nullptr, &backend);

    associatedBackend = static_cast<TcpBackend*>(backend);

    if (associatedBackend) {
        associatedBackend->setWriteAccess(writeAccess);
        startBootloaderAction->setEnabled(writeAccess);
    }
}

void ConnectionWidgetTcp::forceWriteAccess() {
    if (!writeAccess) {
        send(FORCEWRITEACCESS);
    }
}





