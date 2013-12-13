#ifndef FELDBUSFRONTEND_H
#define FELDBUSFRONTEND_H

#include "basefrontend.h"
#include <tina++/feldbus/host/device.h>
#include <QList>
#include "util/feldbusdevicefactory.h"
#include <tina++/feldbus/dynamixel/dynamixeldevice.h>



class QPushButton;
class QListWidget;
class QTextEdit;
class QLineEdit;
class QIntValidator;
class QWidget;
class QSplitter;
class PlainTextFrontend;


using namespace TURAG;


class FeldbusFrontend : public BaseFrontend
{
    Q_OBJECT

protected:
    QWidget* feldbusWidget;
    QListWidget* deviceList_;
    QTextEdit* deviceInfo_;
    QList<FeldbusDeviceInfoExt> devices_;
    QPushButton* startInquiry_;
    QLineEdit* fromEdit_;
    QLineEdit* toEdit_;
    QIntValidator* fromValidator_;
    QIntValidator* toValidator_;
    FeldbusDeviceFactory* deviceFactory;

    QPushButton* dynamixelStartInquiry_;
    QLineEdit* dynamixelFromEdit_;
    QLineEdit* dynamixelToEdit_;
    QIntValidator* dynamixelFromValidator_;
    QIntValidator* dynamixelToValidator_;
    QList<TURAG::Feldbus::DynamixelDevice*> dynamixelDevices_;

    QSplitter* splitter;
    PlainTextFrontend* busLog_;

protected slots:
    void onStartInquiry(void);
    void onDeviceSelected(int row);

    void onStartDynamixelInquiry(void);
    void onRs485DebugMsg(QString msg);

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
    void dynamixelValidateAdressFields();

};



#endif // FELDBUSFRONTEND_H
