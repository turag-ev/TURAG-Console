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
#include <QTabWidget>
#include <tina++/feldbus/host/farbsensor.h>
#include <tina++/feldbus/host/aktor.h>
#include <tina++/feldbus/host/dcmotor.h>
#include <tina++/feldbus/host/servo.h>
#include <tina++/feldbus/dynamixel/dynamixeldevice.h>
#include "feldbusviews/feldbusfarbsensorview.h"
#include "feldbusviews/dynamixelview.h"
#include "feldbusviews/feldbusaktorview.h"
#include "feldbusviews/feldbusasebview.h"
#include <debugprintclass.h>
#include "plaintextfrontend.h"


using namespace TURAG;

FeldbusFrontend::FeldbusFrontend(QWidget *parent) :
    BaseFrontend("TURAG Feldbus", parent)
{
    fromValidator_ = new QIntValidator(1, 126, this);
    toValidator_ = new QIntValidator(2, 127, this);

    QHBoxLayout* layout = new QHBoxLayout;
    splitter = new QSplitter;

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

    dynamixelFromValidator_ = new QIntValidator(0, 252, this);
    dynamixelToValidator_ = new QIntValidator(1, 253, this);

    QHBoxLayout* dynamixel_layoutTop = new QHBoxLayout;
    QLabel* dynamixel_fromLabel = new QLabel("Von:");
    dynamixel_layoutTop->addWidget(dynamixel_fromLabel);
    dynamixelFromEdit_ = new QLineEdit;
    dynamixelFromEdit_->setValidator(dynamixelFromValidator_);
    dynamixel_layoutTop->addWidget(dynamixelFromEdit_);
    QLabel* dynamixel_topLabel = new QLabel("bis:");
    dynamixel_layoutTop->addWidget(dynamixel_topLabel);
    dynamixelToEdit_ = new QLineEdit;
    dynamixelToEdit_->setValidator(dynamixelToValidator_);
    dynamixel_layoutTop->addWidget(dynamixelToEdit_);
    dynamixelStartInquiry_ = new QPushButton("Dynamixel-Servos suchen");
    dynamixel_layoutTop->addWidget(dynamixelStartInquiry_);

    QVBoxLayout* deviceLayout = new QVBoxLayout;
    deviceList_ = new QListWidget;
    connect(deviceList_, SIGNAL(currentRowChanged(int)), this, SLOT(onDeviceSelected(int)));
    deviceLayout->addWidget(deviceList_);
    deviceInfo_ = new QTextEdit;
    deviceInfo_->setReadOnly(true);
    deviceLayout->addWidget(deviceInfo_);
    QWidget* leftDeviceWidget = new QWidget;
    leftDeviceWidget->setLayout(deviceLayout);
    busLog_ = new PlainTextFrontend;

    QTabWidget* tabwidget = new QTabWidget;
    tabwidget->addTab(leftDeviceWidget, "Geräte");
    tabwidget->addTab(busLog_, "Log");
    connect(&rs485Debug, SIGNAL(debugMsg(QString)), this, SLOT(onRs485DebugMsg(QString)));


    left_layout->addLayout(layoutTop);
    left_layout->addLayout(dynamixel_layoutTop);
    left_layout->addWidget(tabwidget);
    left_layout_widget->setLayout(left_layout);

    feldbusWidget = new QWidget;
    layout->addWidget(splitter);
    splitter->addWidget(left_layout_widget);
    splitter->addWidget(feldbusWidget);
    splitter->setStretchFactor(1,2);
    setLayout(layout);

    connect(startInquiry_, SIGNAL(clicked()), this, SLOT(onStartInquiry()));
    connect(dynamixelStartInquiry_, SIGNAL(clicked()), SLOT(onStartDynamixelInquiry()));
    setEnabled(false);

#ifdef Q_OS_WIN32
    // windows is a bit slower :D
    turag_rs485_init(0, turag_ms_to_ticks(50));
#else
     turag_rs485_init(0, turag_ms_to_ticks(10));
#endif

    QSettings settings;
    settings.beginGroup(objectName());
    fromEdit_->setText(settings.value("fromAddress", "1").toString());
    toEdit_->setText(settings.value("toAddress", "127").toString());
    dynamixelFromEdit_->setText(settings.value("dynamixelFromAddress", "1").toString());
    dynamixelToEdit_->setText(settings.value("dynamixelToAddress", "253").toString());

    deviceFactory = new FeldbusDeviceFactory(this);

//#warning please remove me
//    feldbusWidget->hide();
//    splitter->addWidget(new FeldbusAsebView(0));

}


FeldbusFrontend::~FeldbusFrontend() {
    for (Feldbus::DynamixelDevice* dev : dynamixelDevices_) {
        delete dev;
    }
    validateAdressFields();
    dynamixelValidateAdressFields();

    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("fromAddress", fromEdit_->text());
    settings.setValue("toAddress", toEdit_->text());
    settings.setValue("dynamixelFromAddress", dynamixelFromEdit_->text());
    settings.setValue("dynamixelToAddress", dynamixelToEdit_->text());
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
    busLog_->clear();
}

bool FeldbusFrontend::saveOutput(void) {
    return false;
}

void FeldbusFrontend::validateAdressFields() {
    if (!fromEdit_->hasAcceptableInput()) {
        fromEdit_->setText("0");
    }
    if (!toEdit_->hasAcceptableInput()) {
        toEdit_->setText(QString("%1").arg(fromEdit_->text().toInt()+1));
    }
    int fromAddress = fromEdit_->text().toInt();
    int toAddress = toEdit_->text().toInt();

    if (fromAddress > toAddress) {
        fromEdit_->setText(QString("%1").arg(toAddress - 1));
    }
}

void FeldbusFrontend::dynamixelValidateAdressFields() {
    if (!dynamixelFromEdit_->hasAcceptableInput()) {
        dynamixelFromEdit_->setText("0");
    }
    if (!dynamixelToEdit_->hasAcceptableInput()) {
        dynamixelToEdit_->setText("127");
    }
    int fromAddress = dynamixelFromEdit_->text().toInt();
    int toAddress = dynamixelToEdit_->text().toInt();

    if (fromAddress > toAddress) {
        dynamixelFromEdit_->setText(QString("%1").arg(toAddress - 1));
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
    turag_rs485_data_buffer.clear();

    FeldbusDeviceInfoExt dev_info;
    dev_info.address = 0;

    for (int i = fromAddress; i <= toAddress; i++) {
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

            Feldbus::Device* dev = new TURAG::Feldbus::Device("", i, chksum_type, 2, 1);
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

    for (Feldbus::DynamixelDevice* dev : dynamixelDevices_) {
        int modelNumber = 0;
        int version = 0;

        dev->getModelNumber(&modelNumber);
        dev->getFirmwareVersion(&version);

        deviceList_->addItem(QString("%1: Modell: %2 Version: %3 [dynamixel]").arg(dev->getID()).arg(modelNumber).arg(version));
    }

    startInquiry_->setEnabled(true);
    startInquiry_->setText("Geräte suchen");
}

void FeldbusFrontend::onStartDynamixelInquiry(void) {
    dynamixelStartInquiry_->setEnabled(false);

    dynamixelValidateAdressFields();

    int fromAddress = dynamixelFromEdit_->text().toInt();
    int toAddress = dynamixelToEdit_->text().toInt();

    deviceList_->clearSelection();
    deviceList_->clear();
    turag_rs485_data_buffer.clear();

    for (Feldbus::DynamixelDevice* dev : dynamixelDevices_) {
        delete dev;
    }
    dynamixelDevices_.clear();
    deviceInfo_->clear();

    for (FeldbusDeviceInfoExt dev_info : devices_) {
        deviceList_->addItem(dev_info.toString());
    }

    for (int i = fromAddress; i <= toAddress; i++) {
        dynamixelStartInquiry_->setText(QString("Dynamixel-Servos suchen: %1 %").arg(i * 100 / (toAddress - fromAddress + 1)));

        Feldbus::DynamixelDevice* dev = new Feldbus::DynamixelDevice("", i, 2, 1);
        int modelNumber = 0;
        int version = 0;
        if (dev->isAvailable() && dev->getModelNumber(&modelNumber) && dev->getFirmwareVersion(&version)) {
            dev->setMaxTransmissionAttempts(TURAG_DYNAMIXEL_DEVICE_CONFIG_MAX_TRANSMISSION_ATTEMPTS);
            dynamixelDevices_.append(dev);
            deviceList_->addItem(QString("%1: Modell: %2 Version: %3 [dynamixel]").arg(dev->getID()).arg(modelNumber).arg(version));
        }
        QCoreApplication::processEvents();
    }

    dynamixelStartInquiry_->setEnabled(true);
    dynamixelStartInquiry_->setText("Dynamixel-Servos suchen");
}

void FeldbusFrontend::onDeviceSelected( int row) {
    feldbusWidget->hide();
    delete feldbusWidget;

    for (Feldbus::DynamixelDevice* dev : dynamixelDevices_) {
        dev->setLed(false);
    }


    if (row < devices_.size() && row != -1) {
        FeldbusDeviceInfoExt device_info = devices_.at(row);

        deviceFactory->createFeldbusDevice(device_info);
        deviceInfo_->setText(deviceFactory->getDeviceInfoText());


        // create Farbsensor view
        Feldbus::Farbsensor* farbsensor = dynamic_cast<Feldbus::Farbsensor*>(deviceFactory->getDevice());
        if (farbsensor) {
            feldbusWidget = new FeldbusFarbsensorView(farbsensor);
            splitter->addWidget(feldbusWidget);
            splitter->setStretchFactor(1,2);
            return;
        }

        // create Aktor view for DC motor
        Feldbus::DCMotor* dcmotor = dynamic_cast<Feldbus::DCMotor*>(deviceFactory->getDevice());
        if (dcmotor) {
            feldbusWidget = new FeldbusAktorView(dcmotor);
            splitter->addWidget(feldbusWidget);
            splitter->setStretchFactor(1,2);
            return;
        }

        // create Aktor view for servo
        Feldbus::Servo* servo = dynamic_cast<Feldbus::Servo*>(deviceFactory->getDevice());
        if (servo) {
            feldbusWidget = new FeldbusAktorView(servo);
            splitter->addWidget(feldbusWidget);
            splitter->setStretchFactor(1,2);
            return;
        }

        // create ASEB view
        Feldbus::Aseb* aseb = dynamic_cast<Feldbus::Aseb*>(deviceFactory->getDevice());
        if (aseb) {
            feldbusWidget = new FeldbusAsebView(aseb);
            splitter->addWidget(feldbusWidget);
            splitter->setStretchFactor(1,2);
            return;
        }

        // TODO: create more views


    } else if (row >= devices_.size() && row < devices_.size() + dynamixelDevices_.size()) {
        Feldbus::DynamixelDevice* dev = dynamixelDevices_.at(row - devices_.size());
        dev->setLed(true);

        feldbusWidget = new DynamixelView(dev);
        splitter->addWidget(feldbusWidget);
        splitter->setStretchFactor(1,2);
        return;
    }

    // create default QWidget if there is no suitable option
    feldbusWidget = new QWidget;
    splitter->addWidget(feldbusWidget);
    splitter->setStretchFactor(1,2);
}


void FeldbusFrontend::onRs485DebugMsg(QString msg) {
    busLog_->writeData(msg.toLatin1());
}
