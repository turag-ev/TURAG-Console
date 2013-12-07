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


FeldbusFrontend::FeldbusFrontend(QWidget *parent) :
    BaseFrontend("TURAG Feldbus", parent)
{
    QHBoxLayout* layout = new QHBoxLayout;
    QSplitter* splitter = new QSplitter;

    QWidget* left_layout_widget = new QWidget;
    QVBoxLayout* left_layout = new QVBoxLayout;
    startInquiry_ = new QPushButton("Geräte suchen");
    left_layout->addWidget(startInquiry_);
    deviceList_ = new QListWidget;
    left_layout->addWidget(deviceList_);
    deviceInfo_ = new QTextEdit;
    left_layout->addWidget(deviceInfo_);
    left_layout_widget->setLayout(left_layout);

    layout->addWidget(splitter);
    splitter->addWidget(left_layout_widget);
    setLayout(layout);

    connect(startInquiry_, SIGNAL(clicked()), this, SLOT(onStartInquiry()));
    setEnabled(false);

    turag_rs485_init(0, turag_ms_to_ticks(2000));
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


void FeldbusFrontend::onStartInquiry(void) {
    startInquiry_->setEnabled(false);

    for (int i = 1; i < 128; i++) {
//        qDebug() << "Gerät: " << i;
        startInquiry_->setText(QString("Geräte suchen: %1 %").arg(i*100/128));
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

            Feldbus::Device* dev = new TURAG::Feldbus::Device("", i, chksum_type, 1,1);
            if (dev->isAvailable()) {
                Feldbus::DeviceInfo dev_info;
                if (dev->getDeviceInfo(&dev_info)) {
                    if (dev_info.crcType == (uint8_t)chksum_type) {
                        devices_.append(dev_info);
                        deviceList_->addItem(QString("Gerät: %1").arg(i));
                        delete dev;
                        break;
                    }
                }
            }
            delete dev;
        }
//        QCoreApplication::processEvents();
    }

    startInquiry_->setEnabled(true);
    startInquiry_->setText("Geräte suchen");
}
