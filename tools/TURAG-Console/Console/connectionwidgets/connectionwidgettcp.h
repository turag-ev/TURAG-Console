#ifndef CONNECTIONWIDGETTCP_H
#define CONNECTIONWIDGETTCP_H

#include "connectionwidget.h"
#include <Debug_Server/debugserver_protocol.h>
#include "../backend/tcpbackend.h"

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
#include <QTimer>

class QMenu;
class QAction;
class ConnectionWidget;
class FastListWidget;

class ConnectionWidgetTcp: public ConnectionWidget {
    Q_OBJECT

public:
	static constexpr const char* DEFAULTHOST = "robot.turag.et.tu-dresden.de";

    struct device {
        QString path;
        QString port;
        QString description;
		bool    onlineStatus;
    };

    ConnectionWidgetTcp(QWidget *parent = 0);
    ~ConnectionWidgetTcp();


protected slots:
    virtual void onOpenRecentConnection(int index);

private slots:
    void connectToServer();
    void cancel_connecting(void);
    void resetFromContextMenu();
    void receive();
    void startDataChannel(QListWidgetItem * item);

    void socketConnected(void);
    void socketDisconnected(void);
    void socketError(QAbstractSocket::SocketError error);

    void showContextMenu(const QPoint & pos);
    void heartBeatTimerOccured(void);

    void backendConnected();

private:
    void fillDeviceList(void);

    void handleData();

    //sendet einfach in den ControlChannel
    void send(QByteArray data);
    void send(QString string);

    QTimer heartBeatTimer;

    device * selectedDevice;
    TcpBackend* associatedBackend;

    QLineEdit * hostEdit;
    QPushButton * connect_button;
    QPushButton * connect_cancel_button;
    QTcpSocket * socket;

    QListWidget * allDevicesWidget;

    FastListWidget* serverLog;

    //Infotext, der unter devices Box angezeigt wird
    QLabel * timeText;

    QList<QByteArray> puffer;
    QList<device * > allDevices;

    QMenu* contextMenu;
	QAction* bootloaderContextAction;

};

#endif // CONNECTIONWIDGETTCP_H
