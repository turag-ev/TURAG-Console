#include "feldbusbootloaderview.h"
#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>

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
    labelFilePath = new QLabel("Firmwaredatei:");
    textFilePath = new QLineEdit;
    buttonGetFilePath_ = new QPushButton("Dateipfad öffnen");
    firmwareOptions->addWidget(labelFilePath);
    firmwareOptions->addWidget(textFilePath);
    firmwareOptions->addWidget(buttonGetFilePath_);

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


    connect(buttonGetFilePath_, SIGNAL(clicked()), this, SLOT(onOpenFile()));


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

}

void FeldbusBootloaderView::onOpenFile() {
    QFileDialog dialog(this, "Firmwaredatei auswählen", "");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Intel Hex-file (*.hex)"));
    if (dialog.exec() == QDialog::Accepted) {
      QStringList files = dialog.selectedFiles();

      textFilePath->setText(QString("%1").arg(files[0]));
      //textFilePath->setText(files);

    }

}
