#include "connectionwidgettcp.h"

/*TODO List
 *[angefangen]    Oberfäche für das Wählen des device fertig machen
 *[]    festestellen, welches device gerade ausgewählt ist
 *[]    beim reset den mount path des ausgewählten device senden
 *
 *
 *
 */


ConnectionWidgetTcp::ConnectionWidgetTcp (QWidget *parent) :
    ConnectionWidget("Letzte Verbindungen", parent) {
    QSettings settings;
    settings.beginGroup("RecentTcpConnections");

    recentHost =
    settings.value("recentHost", QString(DEFAULTHOST)).toString();

    //Bezeichner (Label) für den hostedit
    QLabel * hostLabel = new QLabel("Host: ");

    //Bezeichner für channel box
    QLabel * channelBoxLabel = new QLabel("Channel: ");

    //Eingabefeld für host
    hostEdit = new QLineEdit();
    hostEdit->setText(recentHost);

    // create button to connect
    connect_button = new QPushButton("Verbinden");
    connect(connect_button, SIGNAL(clicked()), this, SLOT(connectToServer()));

    //GesamtLayout ist ein QVBoxLayout
    QVBoxLayout * generalLayout = new QVBoxLayout();

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
    //emergencyStopAction->setToolTip("Nur drücken, wenn Skynet geladen hat");
    connect(emergencyStopAction, SIGNAL(triggered()), this, SLOT(emergencyStop()));

    readWriteAccessAction = new QAction("Schreibrechte anfordern", this);
    readWriteAccessAction->setShortcut(Qt::CTRL + Qt::Key_W);
    connect(readWriteAccessAction, SIGNAL(triggered()), this, SLOT(readWriteAccess()));


    //frage: wie teilen wir das dem datachannel mit?
    startBootloaderAction = new QAction("&Bootloader starten", this);
    startBootloaderAction->setShortcut(Qt::CTRL + Qt::Key_B);
    connect(startBootloaderAction, SIGNAL(triggered()), this, SLOT(reset()));

    tcpMenu->addAction(readWriteAccessAction);
    tcpMenu->addAction(emergencyStopAction);
    tcpMenu->addAction(startBootloaderAction);

    //das QListWidget anlegen
    allDevicesWidget = new QListWidget(this);

    //jetzt um den Socket kümmern
    //listen leeren
    puffer.clear();
    allDevices.clear();

    client = new  QTcpSocket(this);
    //connect(client, SIGNAL(connected()), this, SLOT(connected()));
    //connect(client, SIGNAL(hostFound()), this, SLOT(hostFound()));
    //connect(client, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error()));
    connect(client, SIGNAL(readyRead()), this, SLOT(receive()));

}

void ConnectionWidgetTcp::handleData() {

    if (puffer.at(0) == WADENIED) {
        //hier machen, was passieren soll, wenn man Schreibrechte erhalten hat
        writeAccess = false;
        readWriteAccessAction->setText("Schreibrechte anfordern");

        //anschließend puffer leeren
        puffer.clear();
    }
    else if (puffer.at(0) == WAGRANTED) {
        writeAccess = true;
        readWriteAccessAction->setText("Schreibrechte aufgeben");

        puffer.clear();
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
            qDebug() << "mache ein device" << endl;
            device * newDevice = new device;
            newDevice->path = puffer.at(2 + i * 5);
            newDevice->port = puffer.at(3 + i * 5);
            newDevice->description = puffer.at(4 + i * 5);
            newDevice->resetCode = puffer.at(5 + i * 5);
            newDevice->baudRate = puffer.at(6 + i * 5);
            allDevices.append(newDevice);
        }
        for (i = 0; i < amount; i++) {
            //amount liegt auf puffer.at(1), dann kommen die devices, 5 * amount, + 1 in den ersten onlinestatus und je nach i inkrementieren
            allDevices.at(i)->onlineStatus = puffer.at( 1 + amount * 5 + 1 + i).toInt();
        }
        puffer.clear();

        //und jetzt für jedes das QListWidget füllen:
        for (i = 0; i < allDevices.size(); i++) {
            device * currentDevice = allDevices.at(i);
            QString descr(currentDevice->description);
            if (currentDevice->onlineStatus) {
                descr.append(" <online>");
            }
            else {
                descr.append(" <offline>");
            }
            allDevicesWidget->addItem(descr);
        }

    }
    else {
        //dont know what to do
        qDebug() << "Unknown data package type, received: " << endl;
        int i;
        for (i = 0; i < puffer.size(); i++) {
            qDebug() << puffer.at(i) << endl;
        }
        puffer.clear();
    }
}

void ConnectionWidgetTcp::receiveData(QByteArray * data) {
    *data = client->readLine(150);
    data->chop(1);
}

void ConnectionWidgetTcp::send(QByteArray &data) {
    data.append("\n");
    client->write(data);
}

void ConnectionWidgetTcp::send(QString &string) {
    QByteArray data = string.toAscii();
    send(data);
}

QMenu* ConnectionWidgetTcp::getMenu() {
    return tcpMenu;
}

/*
 *
 *SLOTS
 */
void ConnectionWidgetTcp::connectToServer() {
    qint16 port;
    QString portString;
    QString hostAddress;
    QString host = hostEdit->text();
    int index = host.indexOf(":");
    int i;
    for(i = 0; i < index; i++) {
        hostAddress.append(host.at(i));
    }
    for(i = index; i < hostAddress.size(); i++) {
        portString.append(hostAddress.at(i));
    }
    port = portString.toInt();

    QHostAddress serverAddress(hostAddress);

    client->connectToHost(serverAddress, port);

}

void ConnectionWidgetTcp::emergencyStop() {
    //steht noch aus hat geringste Priorität

}

void ConnectionWidgetTcp::readWriteAccess() {
    if (writeAccess == true) {
        QString readonly(READONLY);
        send(readonly);
    }
    else {
        QString sWriteAccess(WRITEACCESS);
        send(sWriteAccess);
    }
}

void ConnectionWidgetTcp::reset() {
    //steht noch aus evtl doch über control cahnnel senden?
}

void ConnectionWidgetTcp::receive() {
    while (client->canReadLine()) {
        QByteArray data;
        receiveData(&data);
        if (data == TERMINATE) {
            handleData();
        }
        else {
            puffer.append(data);
        }

    }

}





