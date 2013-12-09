#ifndef CONNECTIONWIDGETSERIAL_H
#define CONNECTIONWIDGETSERIAL_H

#include "connectionwidget.h"
#include <QStringList>
#include <QLineEdit>
#include <QPushButton>

class QShowEvent;
class QCompleter;

class ConnectionWidgetSerial : public ConnectionWidget {
    Q_OBJECT

protected:
    QLineEdit* port_name_edit;
    QLineEdit* bd_edit;
    QPushButton* serial_button;
    QCompleter* completer;

    static QStringList listDevices(void);
    void showEvent ( QShowEvent * event );

protected slots:
    void connectionChangedInternal(void);

public:
    ConnectionWidgetSerial(QWidget *parent = 0);
};

#endif // CONNECTIONWIDGETSERIAL_H
