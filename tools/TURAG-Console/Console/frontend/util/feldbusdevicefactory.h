#ifndef FELDBUSDEVICEFACTORY_H
#define FELDBUSDEVICEFACTORY_H

#include <QObject>
#include <tina++/feldbus/host/device.h>
#include <tina++/feldbus/dynamixel/dynamixeldevice.h>

#include <QByteArray>
#include <QString>

using namespace TURAG; // FIXME: evil!

class FeldbusDeviceInfoExt {
public:
    FeldbusDeviceInfoExt() :
        addressLength(Feldbus::Device::AddressLength::byte_),
        address(0),
        device_name(QByteArray("unnamed")) {}

    Feldbus::Device::DeviceInfo device_info;
    Feldbus::Device::AddressLength addressLength;
    int address;
    QByteArray device_name;
    QByteArray versionInfo;

    QString toString() { return QString("%1: %2").arg(address).arg(QString(device_name)); }
};

struct FeldbusDeviceWrapper {
    FeldbusDeviceInfoExt devInfo;
    std::shared_ptr<Feldbus::Device> device;
    QString deviceInfoText;

    FeldbusDeviceWrapper() : device(nullptr) {}

    unsigned oldSlaveAcceptedPackages;
    unsigned oldSlaveOverflow;
    unsigned oldSlaveLostPackages;
    unsigned oldSlaveChecksumError;

};


class FeldbusDeviceFactory  {
public:
    static FeldbusDeviceWrapper* createFeldbusDevice(FeldbusDeviceInfoExt& device_info);
};


class DynamixelDeviceWrapper {
public:
    std::shared_ptr<Feldbus::DynamixelDevice> device_;

protected:
    int modelNumber_;
    int version_;

public:
    DynamixelDeviceWrapper(Feldbus::DynamixelDevice* dev, int modelNumber, int version) :
        device_(dev),
        modelNumber_(modelNumber),
        version_(version) {}

    QString toString(void) {
        return QString("%1: Modell: %2 Version: %3 [dynamixel]").arg(device_.get()->getID()).arg(modelNumber_).arg(version_);
    }

};


#endif // FELDBUSDEVICEFACTORY_H
