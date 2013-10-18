#ifndef CONNECTIONWIDGETSERIAL_H
#define CONNECTIONWIDGETSERIAL_H

#include "connectionwidget.h"
#include <QStringList>
#include <QLineEdit>
#include <QPushButton>


class ConnectionWidgetSerial : public ConnectionWidget {
    Q_OBJECT

protected:
    QLineEdit* port_name_edit;
    QLineEdit* bd_edit;
    QPushButton* serial_button;

    static QStringList listDevices(void);

protected slots:
    void connectionChangedInternal(void);

public:
    ConnectionWidgetSerial(QWidget *parent = 0);
};

#endif // CONNECTIONWIDGETSERIAL_H
