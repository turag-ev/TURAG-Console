#include "feldbusdevicefactory.h"
#include <tina/feldbus/protocol/turag_feldbus_bus_protokoll.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_stellantriebe.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_lokalisierungssensoren.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_aseb.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>
#include <tina++/feldbus/host/legacystellantriebedevice.h>
#include <tina++/feldbus/host/aseb.h>
#include <tina++/feldbus/host/muxer_64_32.h>
#include <tina++/feldbus/host/bootloader.h>
#include <tina++/feldbus/host/feldbusabstraction.h>


FeldbusDeviceWrapper* FeldbusDeviceFactory::createFeldbusDevice(FeldbusDeviceInfoExt &device_info, TURAG::Feldbus::FeldbusAbstraction& bus) {
    QString checksumString;
    QString protocolIdString;
    QString deviceTypeString;
    TURAG::Feldbus::Device* device = nullptr;

	switch (device_info.device_info.crcType()) {
    case TURAG::Feldbus::ChecksumType::xor_based:
        checksumString = "xor-Checksumme";
        break;

    case TURAG::Feldbus::ChecksumType::crc8:
        checksumString = "CRC-8";
        break;

    default:
        checksumString = "unbekannt";
        break;
    }

	switch (device_info.device_info.deviceProtocolId()) {
    case TURAG_FELDBUS_DEVICE_PROTOCOL_STELLANTRIEBE:
        protocolIdString = "Stellantrieb";
        device = new Feldbus::LegacyStellantriebeDevice(
                    device_info.device_name.constData(),
                    device_info.address,
                    bus,
                        device_info.device_info.crcType());

		switch (device_info.device_info.deviceTypeId()) {
        case TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_DC:
            deviceTypeString = "DC-Motor";
            break;

        case TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_SERVO:
            deviceTypeString = "Servo-Motor";
            break;

        case TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_STEPPER:
            deviceTypeString = "Schrittmotor";
            break;

        case TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_POWERPLANE:
            deviceTypeString = "Powerplane";
            break;

        case TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_ESCON:
            deviceTypeString = "ESCON-Motherboard";
            break;

        case TURAG_FELDBUS_STELLANTRIEBE_DEVICE_TYPE_FARBSENSOR:
            deviceTypeString = "Farbsensor";
            break;

        default:
            deviceTypeString = "unbekannt";
            break;
        }
        break;

    case TURAG_FELDBUS_DEVICE_PROTOCOL_LOKALISIERUNGSSENSOREN:
        protocolIdString = "Lokalisierungs-Sensor";

		switch (device_info.device_info.deviceTypeId()) {
        case TURAG_FELDBUS_LOKALISIERUNGSSENSOREN_DEVICE_TYPE_COLORSENSOR:
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

        device = new Feldbus::ASEBBase(
                    device_info.device_name.constData(),
                    device_info.address,
                    bus,
                    device_info.device_info.crcType());

		switch (device_info.device_info.deviceTypeId()) {
        case TURAG_FELDBUS_ASEB_GENERIC:
            deviceTypeString = "ASEB";
            break;

        default:
            deviceTypeString = "unbekannt";
            break;
        }
        break;

    case TURAG_FELDBUS_DEVICE_PROTOCOL_BOOTLOADER:
        protocolIdString = "TinA BMaX-Bootloader";

		switch (device_info.device_info.deviceTypeId()) {
        case TURAG_FELDBUS_BOOTLOADER_GENERIC:
        case TURAG_FELDBUS_BOOTLOADER_STM32:
            device = new Feldbus::BootloaderAvrBase(
                        device_info.device_name.constData(),
                        device_info.address,
                        bus,
                        device_info.device_info.crcType());
            deviceTypeString = "BMaX - generic";
            break;
        case TURAG_FELDBUS_BOOTLOADER_ATMEGA:
            device = new Feldbus::BootloaderAtmega(
                        device_info.device_name.constData(),
                        device_info.address,
						bus,
                        device_info.device_info.crcType());
            deviceTypeString = "BMaX - ATmega";
            break;

        case TURAG_FELDBUS_BOOTLOADER_XMEGA:
            device = new Feldbus::BootloaderXmega(
                        device_info.device_name.constData(),
                        device_info.address,
						bus,
                        device_info.device_info.crcType());
            deviceTypeString = "BMaX - ATxmega";
            break;

        case TURAG_FELDBUS_BOOTLOADER_STM32V2:
            device = new Feldbus::BootloaderStm32v2(
                        device_info.device_name.constData(),
                        device_info.address,
                        bus,
                        device_info.device_info.crcType());
            deviceTypeString = "STM32 (bootloader v2)";
            break;

        default:
            deviceTypeString = "unbekannt";
            break;
        }
        break;


    default:
        protocolIdString = "unbekannt";

        switch (device_info.device_info.deviceTypeId()) {
        case 0x01:
            device = new Feldbus::Muxer_64_32(
                        device_info.device_name.constData(),
                        device_info.address,
                        bus,
                        device_info.device_info.crcType());
            deviceTypeString = "64-32-Muxer";
            break;

        default:
        deviceTypeString = "unbekannt";
        device = new Feldbus::Device(
                    device_info.device_name.constData(),
                    device_info.address,
					bus,
                        device_info.device_info.crcType());
        break;
    }
        break;
    }

    QString devString;
    if (device_info.address != 0) {
        devString += "<table>\n<tr><td>Name: </td><td>";
        devString += device_info.device_name;
        devString += "</td></tr>\n<tr><td>Adresse: </td><td>";
        devString += QString("%1").arg(device_info.address);
        devString += "</td></tr>\n<tr><td>UUID: </td><td>";
        devString += QString("%1").arg(device_info.uuidString());
        devString += "</td></tr>\n<tr><td>Protokoll: </td><td>";
        devString += protocolIdString;
        devString += "</td></tr>\n<tr><td>Gerätetyp: </td><td>";
        devString += deviceTypeString;
        devString += "</td></tr>\n<tr><td>Checksumme: </td><td>";
        devString += checksumString;
        devString += "</td></tr>\n<tr><td>Puffer-Größe: </td><td>";
        devString += QString("%1").arg(device_info.extended_device_info.bufferSize());
        devString += "</td></tr>\n<tr><td>Uptime-Frequ.: </td><td>";
		devString += device_info.device_info.uptimeFrequency() ? QString("%1 Hz").arg(device_info.device_info.uptimeFrequency()) : "n/a";
        devString += "</td></tr>\n<tr><td>Versions-Info: </td><td>";
        devString += device_info.versionInfo;
        devString += "</td></tr>\n</table>";
    }

    FeldbusDeviceWrapper* wrapper = new FeldbusDeviceWrapper;
    wrapper->device.reset(device);
    wrapper->devInfo = device_info;
    wrapper->deviceInfoText = devString;

    return wrapper;
}

