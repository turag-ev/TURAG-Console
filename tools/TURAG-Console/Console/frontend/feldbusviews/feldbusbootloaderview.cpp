#include "feldbusbootloaderview.h"
#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <tina/rs485.h>


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
    connect(buttonGetFilePath_, SIGNAL(clicked()), this, SLOT(onOpenFile()));

    QHBoxLayout* firmwareUpload = new QHBoxLayout;
    button_transferToMC_ = new QPushButton("Firmware auf Gerät überspielen");
    firmwareUpload->addStretch();
    firmwareUpload->addWidget(button_transferToMC_);
    firmwareUpload->addStretch();
    connect(button_transferToMC_, SIGNAL(clicked()), this, SLOT(onTransferFirmware()));

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
                    page_size = 64;
                    break;

                case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA88:
                    deviceName = "AtMega88";
                    page_size = 64;
                    break;

                case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA48:
                    deviceName = "AtMega48";
                    page_size = 64;
                    break;

                case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA168:
                    deviceName = "AtMega168";
                    page_size = 128;
                    break;

                case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA32:
                    deviceName = "AtMega32";
                    page_size = 128;
                    break;

                case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA644:
                    deviceName = "AtMega644";
                    page_size = 256;
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

void FeldbusBootloaderView::onOpenFile(void) {
    QFileDialog dialog(this, "Firmwaredatei auswählen", "");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Intel Hex-file (*.hex)"));
    if (dialog.exec() == QDialog::Accepted) {
      QStringList files = dialog.selectedFiles();
      textFilePath->setText(QString("%1").arg(files[0]));

      //Test!!!
      //onCreateBinary();
    }
}

void FeldbusBootloaderView::onCreateBinary(void){
    // modified Arguments for hex2bin_convert; was to lazy to modify hex2bin_convert itself
    int argc_CB = 2;
    char *argv_CB[30];

    input_filename = (char*)textFilePath->text().toStdString().c_str();

    argv_CB[1] = input_filename;

    hex2bin_convert(argc_CB,argv_CB);
}

bool FeldbusBootloaderView::onReadBinary(void){

    char *input_filename_bin = input_filename;

    // Replace ".hex" with ".bin"
    input_filename_bin[strlen(input_filename_bin)-1] = 'n';
    input_filename_bin[strlen(input_filename_bin)-2] = 'i';
    input_filename_bin[strlen(input_filename_bin)-3] = 'b';

    //button_transferToMC_->setText(QString("%1").arg(input_filename_bin));


    QFile file (input_filename_bin);
    if (file.open(QIODevice::ReadOnly)){
        fsize = file.size();
        memblock = file.readAll();
        file.close();

        return true;

    }else{
        //Error
        return false;
    }
}

void FeldbusBootloaderView::onTransferFirmware(void){
    onCreateBinary();
    onReadBinary();

    // Calculate pages count
    pages_max = (int)fsize / page_size;
    if( ((int)fsize % page_size) != 0) pages_max++;

    if (dev) {
        TURAG::Feldbus::Request<uint8_t> request;
        request.data = TURAG_FELDBUS_BOOTLOADER_COMMAND_TEST;

        TURAG::Feldbus::Response<uint8_t> response;

        if (dev->transceive(request, &response)) {

            switch(response.data){
                case TURAG_FELDBUS_BOOTLOADER_COMMAND_TEST:
                    button_transferToMC_->setText(QString("Test erfolgreich"));
                    break;
                default:
                    button_transferToMC_->setText(QString("Erste Kommunikation fehlgeschlagen!"));
            }
        }

        int page_cur;
        int output_length;
        uint8_t buffer_Command[512];
        uint8_t output_data;
        uint8_t *output = (uint8_t*)(&output_data);

        for(page_cur = 0; page_cur < pages_max; page_cur++){ //ÜBERPRÜFEN, ANZAHL != Page_size

            int percent = 100 * (page_cur+1) / pages_max;
            button_transferToMC_->setText(QString("Gesendet: %1 % ").arg(percent));

            int page_addr = page_cur * page_size;

            buffer_Command[0] = TURAG_FELDBUS_BOOTLOADER_COMMAND_PAGE_WRITE;

            // Page-Number
            buffer_Command[1] = ((page_addr >> 8) & 0xFF);
            buffer_Command[2] = (page_addr & 0xFF);

            // Data
            for(int i = 0; i < page_size; i++){
                buffer_Command[i+3] = memblock[i];
            }
/**
            if (dev->transceive(buffer_Command, page_size + 4, output, output_length)) {

                switch(response.data){
                    case TURAG_FELDBUS_BOOTLOADER_RESPONSE_PAGE_CORRECT_SIZE:
                        button_transferToMC_->setText(QString("Größe korrekt"));
                        break;
                    case TURAG_FELDBUS_BOOTLOADER_RESPONSE_PAGE_CORRECT_CONTENT:
                        button_transferToMC_->setText(QString("Inhalt korrekt"));
                        break;
                }
            }
**/
            memblock += page_size;
        }

        //button_transferToMC_->setText(QString("Firmware erfolgreich geschrieben!"));

    }
}


/*

        if (dev->transceive(request, &response)) {

            switch(response.data){
                case TURAG_FELDBUS_BOOTLOADER_COMMAND_TEST:
                    button_transferToMC_->setText(QString("Test erfolgreich"));
                    break;
                default:
                    button_transferToMC_->setText(QString("Erste Kommunikation fehlgeschlagen!"));
            }
        }

switch (buf[1]){
            case TURAG_FELDBUS_BOOTLOADER_RESPONSE_PAGE_CORRECT_SIZE:
                printf("Correct size\n\n");
                return true;

            case TURAG_FELDBUS_BOOTLOADER_RESPONSE_PAGE_CORRECT_CONTENT:
                printf("Correct content in Flash\n\n");
                return true;

            case TURAG_FELDBUS_BOOTLOADER_RESPONSE_PAGE_FAIL_CONTENT:
                printf("Wrong content in Flash!\n");
                return false;

            case TURAG_FELDBUS_BOOTLOADER_RESPONSE_PAGE_FAIL_SIZE:
                printf("Wrong page size!\n");
                return false;

            case TURAG_FELDBUS_BOOTLOADER_RESPONSE_PAGE_FAIL_END:
                printf("Wrong End. Run trough!\n");
                return false;

            case TURAG_FELDBUS_BOOTLOADER_COMMAND_TEST:
                printf("Yeah - Communication test successful\n\n");
                return true;
}



    QFile file (input_filename_bin);
    if (file.open(QIODevice::ReadOnly | QIODevice::ReadOnly)){
        fsize = file.size();
        memblock = new char [fsize];
        //file.seekg (0, ios::beg);
        file.read (memblock, fsize);
        file.close();

        delete[] memblock;

*/


