#ifndef CONNECTIONWIDGETSERIAL_H
#define CONNECTIONWIDGETSERIAL_H

#include "connectionwidget.h"
#include <QStringList>
#include <QTimer>

class QShowEvent;
class QComboBox;
class QPushButton;


class ConnectionWidgetSerial : public ConnectionWidget {
    Q_OBJECT

public:
    ConnectionWidgetSerial(QWidget *parent = 0);

protected:
    QComboBox* baudrate_;
    QPushButton* serial_button;
    QComboBox* port_name_;

    virtual void showEvent ( QShowEvent * event );
    virtual void hideEvent ( QHideEvent * event );

protected slots:
    void connectionChangedInternal(void);

private:
    QTimer deviceUpdateTimer;

private slots:
    void onDeviceUpdate(void);

};

#endif // CONNECTIONWIDGETSERIAL_H
