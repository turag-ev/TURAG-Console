#ifndef FELDBUSDEVICEFACTORY_H
#define FELDBUSDEVICEFACTORY_H

#include <QObject>
#include <tina++/feldbus/host/device.h>

#include <QByteArray>
#include <QString>

using namespace TURAG;

class FeldbusDeviceInfoExt {
public:
    FeldbusDeviceInfoExt() :
        address(0),
        device_name(QByteArray("unnamed")) {}

    Feldbus::DeviceInfo device_info;
    int address;
    QByteArray device_name;

    QString toString() { return QString("%1: %2").arg(address).arg(QString(device_name)); }
};



class FeldbusDeviceFactory : public QObject {
    Q_OBJECT
protected:
    Feldbus::Device* device;
    QString checksumString;
    QString protocolIdString;
    QString deviceTypeString;
    FeldbusDeviceInfoExt device_info_;


public:
    FeldbusDeviceFactory(QObject* parent = nullptr) :
        QObject(parent),
        device(nullptr),
        checksumString("unbekannt"),
        protocolIdString("unbekannt"),
        deviceTypeString("unbekannt")   {}

    ~FeldbusDeviceFactory() {
        if (device) {
            delete device;
        }
    }

    bool createFeldbusDevice(FeldbusDeviceInfoExt& device_info);
    Feldbus::Device* getDevice() const { return device; }
    QString getDeviceInfoText() const;
};




#endif // FELDBUSDEVICEFACTORY_H
