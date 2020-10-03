#ifndef TURAG_CONSOLE_CONSOLE_CONNECTIONWIDGETS_CONNECTIONWIDGETSERIAL_H
#define TURAG_CONSOLE_CONSOLE_CONNECTIONWIDGETS_CONNECTIONWIDGETSERIAL_H

#include "connectionwidget.h"
#include <QStringList>
#include <QTimer>

class QShowEvent;
class KeyboardEnabledComboBox;
class QPushButton;
class QComboBox;
class ExpanderWidget;



class ConnectionWidgetSerial : public ConnectionWidget {
    Q_OBJECT

public:
    ConnectionWidgetSerial(QWidget *parent = 0);

protected:
    KeyboardEnabledComboBox* baudrate_;
    QPushButton* serial_button;
    KeyboardEnabledComboBox* port_name_;
	ExpanderWidget* expander;
	QComboBox* dataBits_;
	QComboBox* numberOfStopBits_;
	QComboBox* parityType_;


    virtual void showEvent ( QShowEvent * event );
    virtual void hideEvent ( QHideEvent * event );

protected slots:
    void connectionChangedInternal(void);

private:
    QTimer deviceUpdateTimer;

private slots:
    void onDeviceUpdate(void);
    void onKeyPressed(QKeyEvent* e);

};

#endif // TURAG_CONSOLE_CONSOLE_CONNECTIONWIDGETS_CONNECTIONWIDGETSERIAL_H
