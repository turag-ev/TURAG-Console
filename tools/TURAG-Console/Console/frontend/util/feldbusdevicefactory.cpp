#include "feldbusdevicefactory.h"
#include <tina/feldbus/protocol/turag_feldbus_bus_protokoll.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_stellantriebe.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_lokalisierungssensoren.h>
#include <tina++/feldbus/host/dcmotor.h>
#include <tina++/feldbus/host/farbsensor.h>
#include <tina++/feldbus/host/servo.h>


bool FeldbusDeviceFactory::createFeldbusDevice(FeldbusDeviceInfoExt &device_info) {
    if (device) delete device;
    device = nullptr;

    device_info_ = device_info;

    switch (device_info.device_info.crcType) {
    case TURAG_FELDBUS_CHECKSUM_XOR:
        checksumString = "xor-Checksumme";
        break;

    case TURAG_FELDBUS_CHECKSUM_CRC8_ICODE:
        checksumString = "CRC-8 I-CODE";
        break;

    default:
        checksumString = "unbekannt";
        break;
    }

    switch (device_info.device_info.deviceProtocolId) {
    case TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE:
        protocolIdString = "Stellantrieb";

        switch (device_info.device_info.deviceTypeId) {
        case TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_DC:
            device = new Feldbus::DCMotor(device_info.device_name.constData(), device_info.address, (Feldbus::Device::ChecksumType)device_info.device_info.crcType);
            deviceTypeString = "DC-Motor";
            break;

        case TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_SERVO:
            device = new Feldbus::Servo(device_info.device_name.constData(), device_info.address, (Feldbus::Device::ChecksumType)device_info.device_info.crcType);
            deviceTypeString = "Servo-Motor";
            break;

        default:
            deviceTypeString = "unbekannt";
            break;
        }
        break;

    case TURAG_FELDBUS_DEVICE_PROTOCOL_LOKALISIERUNGSSENSOREN:
        protocolIdString = "Lokalisierungs-Sensor";

        switch (device_info.device_info.deviceTypeId) {
        case TURAG_FELDBUS_LOKALISIERUNGSSENSOREN_DEVICE_TYPE_COLORSENSOR:
            device = new Feldbus::Farbsensor(device_info.device_name.constData(), device_info.address, (Feldbus::Device::ChecksumType)device_info.device_info.crcType);
            deviceTypeString = "Farbsensor";
            break;

        case TURAG_FELDBUS_LOKALISIERUNGSSENSOREN_DEVICE_TYPE_ENCODER:
            deviceTypeString = "Encoder";
            break;

        case TURAG_FELDBUS_LOKALISIERUNGSSENSOREN_DEVICE_TYPE_GEGI:
            deviceTypeString = "GeGi";
            break;

        case TURAG_FELDBUS_LOKALISIERUNGSSENSOREN_DEVICE_TYPE_IMU_TESTBOARD:
            deviceTypeString = "IMU-Testboard";
            break;


        default:
            deviceTypeString = "unbekannt";
            break;
        }
        break;

    default:
        protocolIdString = "unbekannt";
        deviceTypeString = "unbekannt";
        break;
    }

    return false;
}


QString FeldbusDeviceFactory::getDeviceInfoText() const {
    QString output("");

    if (device_info_.address == 0) {
        return output;
    }

    output += "<table>\n<tr><td>Name: </td><td>";
    output += device_info_.device_name;
    output += "</td></tr>\n<tr><td>Adresse: </td><td>";
    output += QString("%1").arg(device_info_.address);
    output += "</td></tr>\n<tr><td>Protokoll: </td><td>";
    output += protocolIdString;
    output += "</td></tr>\n<tr><td>Gerätetyp: </td><td>";
    output += deviceTypeString;
    output += "</td></tr>\n<tr><td>Checksumme: </td><td>";
    output += checksumString;
    output += "</td></tr>\n<tr><td>Puffer-Größe: </td><td>";
    output += QString("%1").arg(device_info_.device_info.bufferSize);
    output += "</td></tr>\n</table>";

    return output;
}


