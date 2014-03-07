#ifndef CONNECTIONWIDGETSERIAL_H
#define CONNECTIONWIDGETSERIAL_H

#include "connectionwidget.h"
#include <QStringList>

class QShowEvent;
class QComboBox;
class QPushButton;

class ConnectionWidgetSerial : public ConnectionWidget {
    Q_OBJECT

protected:
    QComboBox* baudrate_;
    QPushButton* serial_button;
    QComboBox* port_name_;

    void showEvent ( QShowEvent * event );

protected slots:
    void connectionChangedInternal(void);

public:
    ConnectionWidgetSerial(QWidget *parent = 0);
};

#endif // CONNECTIONWIDGETSERIAL_H
