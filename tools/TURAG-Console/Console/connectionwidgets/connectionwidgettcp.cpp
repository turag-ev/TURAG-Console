#include "connectionwidgettcp.h"
#include <QMenu>
#include <QAction>


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
    emergencyStopAction->setToolTip("Nur drücken, wenn Skynet geladen hat");
    readWriteAccessAction = new QAction("Schreibrechte annfordern", this);
    readWriteAccessAction->setShortcut(Qt::CTRL + Qt::Key_W);
    startBootloaderAction = new QAction("Bootloader starten", this);
    startBootloaderAction->setShortcut(Qt::CTRL + Qt::Key_B);
    tcpMenu->addAction(readWriteAccessAction);
    tcpMenu->addAction(emergencyStopAction);
    tcpMenu->addAction(startBootloaderAction);

}

void ConnectionWidgetTcp::connectToServer() {
    //QString host = hostEdit->text();

}


QMenu* ConnectionWidgetTcp::getMenu() {
    return tcpMenu;
}
