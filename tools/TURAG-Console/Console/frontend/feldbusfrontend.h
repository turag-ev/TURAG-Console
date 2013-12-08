#ifndef FELDBUSFRONTEND_H
#define FELDBUSFRONTEND_H

#include "basefrontend.h"
#include <tina++/feldbus/host/device.h>
#include <QList>
#include "util/feldbusdevicefactory.h"


class QPushButton;
class QListWidget;
class QTextEdit;
class QLineEdit;
class QIntValidator;


using namespace TURAG;


class FeldbusFrontend : public BaseFrontend
{
    Q_OBJECT

protected:

    QPushButton* startInquiry_;
    QListWidget* deviceList_;
    QTextEdit* deviceInfo_;
    QList<FeldbusDeviceInfoExt> devices_;
    QLineEdit* fromEdit_;
    QLineEdit*toEdit_;
    QIntValidator* fromValidator_;
    QIntValidator* toValidator_;
    FeldbusDeviceFactory* deviceFactory;

protected slots:
    void onStartInquiry(void);
    void onDeviceSelected(int row);

public:
    FeldbusFrontend(QWidget *parent = 0);
    ~FeldbusFrontend();

public slots:
    virtual void onConnected(bool readOnly, bool isSequential, QIODevice*);
    virtual void onDisconnected(bool reconnecting);
    virtual void writeData(QByteArray data);
    virtual void clear(void);
    virtual bool saveOutput(void);

private:
    void validateAdressFields();

};



#endif // FELDBUSFRONTEND_H
