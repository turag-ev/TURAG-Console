#ifndef FELDBUSFRONTEND_H
#define FELDBUSFRONTEND_H

#include "basefrontend.h"
#include <tina++/feldbus/host/device.h>
#include <QList>

class QPushButton;
class QListWidget;
class QTextEdit;

using namespace TURAG;

class FeldbusFrontend : public BaseFrontend
{
    Q_OBJECT

protected:
    QPushButton* startInquiry_;
    QListWidget* deviceList_;
    QTextEdit* deviceInfo_;
    QList<Feldbus::DeviceInfo> devices_;

protected slots:
    void onStartInquiry(void);

public:
    FeldbusFrontend(QWidget *parent = 0);

public slots:
    virtual void onConnected(bool readOnly, bool isSequential, QIODevice*);
    virtual void onDisconnected(bool reconnecting);
    virtual void writeData(QByteArray data);
    virtual void clear(void);
    virtual bool saveOutput(void);

};


class FeldbusDeviceFinder : QObject
{
    Q_OBJECT

public slots:
//    data input
};

#endif // FELDBUSFRONTEND_H
