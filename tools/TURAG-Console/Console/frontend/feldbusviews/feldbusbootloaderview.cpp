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

    //QWidget *window = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout;
    QHBoxLayout* hlayout = new QHBoxLayout;
    QHBoxLayout* hlayout2 = new QHBoxLayout;

    QLabel* labelDeviceType = new QLabel("Gerätetyp");
    QLabel* labelFilePath = new QLabel("Firmwaredatei");
    QLineEdit* textDeviceType = new QLineEdit;
    QLineEdit* textFilePath = new QLineEdit;
    QPushButton *button_transferToMC = new QPushButton("Firmware auf Gerät überspielen");


    textDeviceType->setReadOnly(true);

    hlayout->addWidget(labelDeviceType);
    hlayout->addWidget(textDeviceType);
    layout->addLayout(hlayout);

    hlayout2->addWidget(labelFilePath);
    hlayout2->addWidget(textFilePath);
    layout->addLayout(hlayout2);

    hlayout2->addWidget(button_transferToMC);
    layout->addLayout(hlayout2);

    setLayout(layout);


    if (dev) {
        TURAG::Feldbus::Device::Request<uint8_t> request;
        request.data = TURAG_FELDBUS_BOOTLOADER_COMMAND_GET_MCUID;

        TURAG::Feldbus::Device::Response<uint8_t> response;

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
