#include "feldbusbootloaderview.h"
#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

FeldbusBootloaderView::FeldbusBootloaderView(TURAG::Feldbus::Device *dev_, QWidget *parent) :
    QWidget(parent), dev(dev_)
{

    QHBoxLayout* deviceType = new QHBoxLayout;
    QLabel* labelDeviceType = new QLabel("Gerätetyp:");
    QLineEdit* textDeviceType = new QLineEdit;
    textDeviceType->setReadOnly(true);
    deviceType->addWidget(labelDeviceType);
    deviceType->addWidget(textDeviceType);

    QHBoxLayout* firmwareOptions = new QHBoxLayout;
    QLabel* labelFilePath = new QLabel("Firmwaredatei:");

    QLineEdit* textFilePath = new QLineEdit;
    QPushButton *button_getFilePath = new QPushButton("Dateipfad öffnen");
    firmwareOptions->addWidget(labelFilePath);
    firmwareOptions->addWidget(textFilePath);
    firmwareOptions->addWidget(button_getFilePath);

    QHBoxLayout* firmwareUpload = new QHBoxLayout;
    QPushButton *button_transferToMC = new QPushButton("Firmware auf Gerät überspielen");
    firmwareUpload->addStretch();
    firmwareUpload->addWidget(button_transferToMC);
    firmwareUpload->addStretch();

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addStretch();
    layout->addLayout(deviceType);
    layout->addLayout(firmwareOptions);
    layout->addSpacing(40);
    layout->addLayout(firmwareUpload);
    layout->addStretch();
    setLayout(layout);


    if (dev) {
        TURAG::Feldbus::Request<uint8_t> request;
        request.data = TURAG_FELDBUS_BOOTLOADER_COMMAND_GET_MCUID;

        TURAG::Feldbus::Response<uint8_t> response;

        if (dev->transceive(request, &response)) {

            switch(response.data){

                case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA8:
                    deviceName = "AtMega8";
                    break;

                case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA88:
                    deviceName = "AtMega88";
                    break;

                case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA48:
                    deviceName = "AtMega48";
                    break;

                case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA168:
                    deviceName = "AtMega168";
                    break;

                case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA32:
                    deviceName = "AtMega32";
                    break;

                case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA644:
                    deviceName = "AtMega644";
                    break;

                default:
                    deviceName = QString("%1").arg(response.data);
                    break;
            }

            textDeviceType->setText(deviceName);
        } else {
            textDeviceType->setText("ERROR");
        }
    }

    //void FeldbusBootloaderView::
}
