#ifndef CONNECTIONWIDGETTCP_H
#define CONNECTIONWIDGETTCP_H

#define DEFAULTHOST "robot.turag.et.tu-dresden.de:30000"

#include "connectionwidget.h"
#include "backend/tcpBackend_protocol.h"

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

typedef struct device{
    QByteArray path;
    QByteArray port;
    QByteArray description;
    QByteArray resetCode;
    QByteArray baudRate;
    bool       onlineStatus;
} device;

class QMenu;
class QAction;
class ConnectionWidget;

class ConnectionWidgetTcp: public ConnectionWidget {
    Q_OBJECT

protected:
    QMenu* tcpMenu;
    QAction* emergencyStopAction;
    QAction* readWriteAccessAction;
    QAction* startBootloaderAction;

    virtual QMenu* getMenu();

protected slots:



public:
    ConnectionWidgetTcp(QWidget *parent = 0);

private:

    //writeAccess steht auf false, falls readonly
    device * selectedDevice;
    bool writeAccess;
    QString recentHost;
    QLineEdit * hostEdit;
    QPushButton * connect_button;
    QTcpSocket * client;
    QVBoxLayout * generalLayout;

    QListWidget * allDevicesWidget;

    QList<QByteArray> puffer;
    QList<device * > allDevices;


    void handleData();
    void receiveData(QByteArray * data);

    //sendet einfach in den ControlChannel
    void send(QByteArray &data);
    void send(QString &string);

    device * findDeviceDescr(QString &descr);

public slots:
    void connectToServer();
    void emergencyStop();
    void readWriteAccess();
    void reset();
    void receive();
    void startDataChannel(QListWidgetItem * item);
};

#endif // CONNECTIONWIDGETTCP_H
