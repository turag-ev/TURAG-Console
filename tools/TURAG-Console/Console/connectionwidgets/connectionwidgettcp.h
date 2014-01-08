#ifndef CONNECTIONWIDGETTCP_H
#define CONNECTIONWIDGETTCP_H

#define DEFAULTHOST "robot.turag.et.tu-dresden.de"

#include "connectionwidget.h"
#include "../../Debug-Server/Debug_Server/debugserver_protocol.h"
#include "../backend/tcpbackend.h"

#include <QSettings>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>
#include <QList>
#include <QByteArray>
#include <QMenu>
#include <QAction>
#include <QListWidget>

class QMenu;
class QAction;
class ConnectionWidget;

class ConnectionWidgetTcp: public ConnectionWidget {
    Q_OBJECT

public:
    struct device {
        QString path;
        QString port;
        QString description;
        QString resetCode;
        QString baudRate;
        bool       onlineStatus;
    };

protected:
    QMenu* tcpMenu;
    QAction* emergencyStopAction;
    QAction* requestWriteAccessAction;
    QAction* startBootloaderAction;
    QAction* requestWriteAccessActionForce;

    virtual QMenu* getMenu();


public:
    ConnectionWidgetTcp(QWidget *parent = 0);
    ~ConnectionWidgetTcp();


private:

    //writeAccess steht auf false, falls readonly
    device * selectedDevice;
    bool writeAccess;
    TcpBackend* associatedBackend;

    QString recentHost;
    QLineEdit * hostEdit;
    QPushButton * connect_button;
    QTcpSocket * socket;
    QVBoxLayout * generalLayout;

    QListWidget * allDevicesWidget;

    //Infotext, der unter devices Box angezeigt wird
    QLabel * bottomInfoText;

    QList<QByteArray> puffer;
    QList<device * > allDevices;

    void fillDeviceList(void);

    void handleData();
    void receiveData(QByteArray * data);

    //sendet einfach in den ControlChannel
    void send(QByteArray data);
    void send(QString string);

protected slots:
    void connectToServer();
    void emergencyStop();
    void onRequestWriteAccess();
    void reset();
    void receive();
    void startDataChannel(QListWidgetItem * item);
    void forceWriteAccess();

    void socketConnected(void);
    void socketDisconnected(void);
    void socketError(QAbstractSocket::SocketError error);
};

#endif // CONNECTIONWIDGETTCP_H
