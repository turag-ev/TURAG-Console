#include "feldbusdevicefactory.h"
#include <tina/feldbus/protocol/turag_feldbus_bus_protokoll.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_stellantriebe.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_lokalisierungssensoren.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_aseb.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>
#include <tina++/feldbus/host/dcmotor.h>
#include <tina++/feldbus/host/farbsensor.h>
#include <tina++/feldbus/host/servo.h>
#include <tina++/feldbus/host/aseb.h>
#include <tina++/feldbus/host/bootloader.h>


FeldbusDeviceWrapper FeldbusDeviceFactory::createFeldbusDevice(FeldbusDeviceInfoExt &device_info) {
    QString checksumString;
    QString protocolIdString;
    QString deviceTypeString;
    TURAG::Feldbus::Device* device = nullptr;

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

    case TURAG_FELDBUS_DEVICE_PROTOCOL_ASEB:
        protocolIdString = "ASEB (Application Specific Extension Board)";

        switch (device_info.device_info.deviceTypeId) {
        case TURAG_FELDBUS_ASEB_GENERIC:
            device = new Feldbus::Aseb(device_info.device_name.constData(), device_info.address, (Feldbus::Device::ChecksumType)device_info.device_info.crcType);
            deviceTypeString = "generic ASEB";
            break;

        default:
            deviceTypeString = "unbekannt";
            break;
        }
        break;

    case TURAG_FELDBUS_DEVICE_PROTOCOL_BOOTLOADER:
        protocolIdString = "TinA BMaX-Bootloader";

        switch (device_info.device_info.deviceTypeId) {
        case TURAG_FELDBUS_BOOTLOADER_GENERIC:
            device = new Feldbus::Bootloader(device_info.device_name.constData(), device_info.address, (Feldbus::Device::ChecksumType)device_info.device_info.crcType);
            deviceTypeString = "generic - BMaX";
            break;

        case TURAG_FELDBUS_BOOTLOADER_ATMEGA:
            device = new Feldbus::Bootloader(device_info.device_name.constData(), device_info.address, (Feldbus::Device::ChecksumType)device_info.device_info.crcType);
            deviceTypeString = "AtMega - BMaX";
            break;

        case TURAG_FELDBUS_BOOTLOADER_XMEGA:
            device = new Feldbus::Bootloader(device_info.device_name.constData(), device_info.address, (Feldbus::Device::ChecksumType)device_info.device_info.crcType);
            deviceTypeString = "XMEGA - BMaX";
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

    QString devString;
    if (device_info.address != 0) {
        devString += "<table>\n<tr><td>Name: </td><td>";
        devString += device_info.device_name;
        devString += "</td></tr>\n<tr><td>Adresse: </td><td>";
        devString += QString("%1").arg(device_info.address);
        devString += "</td></tr>\n<tr><td>Protokoll: </td><td>";
        devString += protocolIdString;
        devString += "</td></tr>\n<tr><td>Gerätetyp: </td><td>";
        devString += deviceTypeString;
        devString += "</td></tr>\n<tr><td>Checksumme: </td><td>";
        devString += checksumString;
        devString += "</td></tr>\n<tr><td>Puffer-Größe: </td><td>";
        devString += QString("%1").arg(device_info.device_info.bufferSize);
        devString += "</td></tr>\n</table>";
    }

    FeldbusDeviceWrapper wrapper;
    wrapper.device.reset(device);
    wrapper.devInfo = device_info;
    wrapper.deviceInfoText = devString;

    return wrapper;
}

