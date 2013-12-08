#include "feldbusfrontend.h"
#include <tina/rs485.h>
#include <QSplitter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QDebug>
#include <QCoreApplication>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>



FeldbusFrontend::FeldbusFrontend(QWidget *parent) :
    BaseFrontend("TURAG Feldbus", parent)
{
    fromValidator_ = new QIntValidator(1, 126, this);
    toValidator_ = new QIntValidator(2, 127, this);

    QHBoxLayout* layout = new QHBoxLayout;
    QSplitter* splitter = new QSplitter;

    QWidget* left_layout_widget = new QWidget;
    QVBoxLayout* left_layout = new QVBoxLayout;

    QHBoxLayout* layoutTop = new QHBoxLayout;
    QLabel* fromLabel = new QLabel("Von:");
    layoutTop->addWidget(fromLabel);
    fromEdit_ = new QLineEdit;
    fromEdit_->setValidator(fromValidator_);
    layoutTop->addWidget(fromEdit_);
    QLabel* topLabel = new QLabel("bis:");
    layoutTop->addWidget(topLabel);
    toEdit_ = new QLineEdit;
    toEdit_->setValidator(toValidator_);
    layoutTop->addWidget(toEdit_);
    startInquiry_ = new QPushButton("Geräte suchen");
    layoutTop->addWidget(startInquiry_);

    QWidget* topWidget = new QWidget;
    topWidget->setLayout(layoutTop);

    left_layout->addWidget(topWidget);
    deviceList_ = new QListWidget;
    connect(deviceList_, SIGNAL(currentRowChanged(int)), this, SLOT(onDeviceSelected(int)));
    left_layout->addWidget(deviceList_);
    deviceInfo_ = new QTextEdit;
    deviceInfo_->setReadOnly(true);
    left_layout->addWidget(deviceInfo_);
    left_layout_widget->setLayout(left_layout);

    layout->addWidget(splitter);
    splitter->addWidget(left_layout_widget);
    setLayout(layout);

    connect(startInquiry_, SIGNAL(clicked()), this, SLOT(onStartInquiry()));
    setEnabled(false);

#ifdef Q_OS_WIN32
    // windows is a bit slower :D
    turag_rs485_init(0, turag_ms_to_ticks(25));
#else
     turag_rs485_init(0, turag_ms_to_ticks(10));
#endif

    QSettings settings;
    settings.beginGroup(objectName());
    fromEdit_->setText(settings.value("fromAddress", "1").toString());
    toEdit_->setText(settings.value("toAddress", "127").toString());

    deviceFactory = new FeldbusDeviceFactory(this);

}


FeldbusFrontend::~FeldbusFrontend() {
    validateAdressFields();

    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("fromAddress", fromEdit_->text());
    settings.setValue("toAddress", toEdit_->text());
}

void FeldbusFrontend::onConnected(bool readOnly, bool isSequential, QIODevice* dev) {
    (void)isSequential;
    turag_rs485_io_device = dev;
    if (!readOnly) {
        setEnabled(true);
    }
}

void FeldbusFrontend::onDisconnected(bool reconnecting) {
    (void)reconnecting;
    turag_rs485_io_device = nullptr;
    setEnabled(false);
}

void FeldbusFrontend::writeData(QByteArray data) {
    turag_rs485_data_buffer.append(data);
}

void FeldbusFrontend::clear(void) {
    deviceInfo_->clear();
    deviceList_->clear();
}

bool FeldbusFrontend::saveOutput(void) {
    return false;
}

void FeldbusFrontend::validateAdressFields() {
    if (!fromEdit_->hasAcceptableInput()) {
        fromEdit_->setText("0");
    }
    if (!toEdit_->hasAcceptableInput()) {
        toEdit_->setText("127");
    }
    int fromAddress = fromEdit_->text().toInt();
    int toAddress = toEdit_->text().toInt();

    if (fromAddress > toAddress) {
        fromEdit_->setText(QString("%1").arg(toAddress - 1));
    }
}

void FeldbusFrontend::onStartInquiry(void) {
    startInquiry_->setEnabled(false);

    validateAdressFields();

    int fromAddress = fromEdit_->text().toInt();
    int toAddress = toEdit_->text().toInt();

    deviceList_->clearSelection();
    deviceList_->clear();
    devices_.clear();
    deviceInfo_->clear();

    FeldbusDeviceInfoExt dev_info;
    dev_info.address = 0;

    for (int i = fromAddress; i < toAddress; i++) {
        startInquiry_->setText(QString("Geräte suchen: %1 %").arg(i * 100 / (toAddress - fromAddress + 1)));
        for (int j = 0; j < 2; j++) {
            Feldbus::Device::ChecksumType chksum_type = Feldbus::Device::ChecksumType::xor_based;

            switch (j) {
            case 0:
                chksum_type = Feldbus::Device::ChecksumType::xor_based;
                break;
            case 1:
                chksum_type = Feldbus::Device::ChecksumType::crc8_icode;
                break;
            }

            Feldbus::Device* dev = new TURAG::Feldbus::Device("", i, chksum_type, 1, 1);
            if (dev->isAvailable()) {
                if (dev->getDeviceInfo(&dev_info.device_info)) {
                    if (dev_info.device_info.crcType == (uint8_t)chksum_type) {
                        dev_info.address = i;
                        delete dev;
                        break;
                    }
                }
            }
            delete dev;
        }
        if (dev_info.address == i) {
            QByteArray name_buffer(dev_info.device_info.nameLength + 2, '0');
            Feldbus::Device* dev = new TURAG::Feldbus::Device("", dev_info.address, (Feldbus::Device::ChecksumType)dev_info.device_info.crcType, 5, 1);
            if (dev->receiveDeviceRealName(name_buffer.data())) {
                dev_info.device_name = name_buffer;
                devices_.append(dev_info);
                deviceList_->addItem(dev_info.toString());
            }
            delete dev;
        }

        QCoreApplication::processEvents();
    }

    startInquiry_->setEnabled(true);
    startInquiry_->setText("Geräte suchen");
}


void FeldbusFrontend::onDeviceSelected( int row) {
    if (row < devices_.size() && row != -1) {
        FeldbusDeviceInfoExt device_info = devices_.at(row);

        deviceFactory->createFeldbusDevice(device_info);

        deviceInfo_->setText(deviceFactory->getDeviceInfoText());
    }
}



