#ifndef CONNECTIONWIDGETTCP_H
#define CONNECTIONWIDGETTCP_H

#define DEFAULTHOST "robot.turag.et.tu-dresden.de:30000"

#include "connectionwidget.h"

#include <QSettings>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QtNetwork/QHostAddress>

class ConnectionWidgetTcp: public ConnectionWidget {
    Q_OBJECT

protected:


protected slots:



public:
    ConnectionWidgetTcp(QWidget *parent = 0);

private:
    QString recentHost;
    QLineEdit * hostEdit;
    QPushButton * connect_button;

public slots:
    void connectToServer();
};

#endif // CONNECTIONWIDGETTCP_H
