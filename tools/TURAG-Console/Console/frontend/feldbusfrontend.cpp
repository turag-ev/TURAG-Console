#include "feldbusfrontend.h"
#include "feldbusviews/dynamixelview.h"
#include "feldbusviews/feldbusaktorview.h"
#include "feldbusviews/feldbusasebview.h"
#include "feldbusviews/feldbusbootloaderatmegaview.h"
#include "feldbusviews/feldbusbootloaderxmegaview.h"
#include "feldbusviews/feldbusbootloaderstm32v2view.h"
#include "feldbusviews/feldbusmuxerview.h"
#include <libs/checkboxext.h>
#include <libs/comboboxext.h>
#include <libs/lineeditext.h>
#include <libs/iconmanager.h>
#include "plaintextfrontend.h"

#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>
#include <tina++/feldbus/host/feldbus_devicelocator.h>
#include <tina++/feldbus/host/feldbus_binaryaddresssearcher.h>
#include <tina++/feldbus/host/muxer_64_32.h>
#include <tina++/feldbus/dynamixel/dynamixeldevice.h>
#include <tina++/feldbus/dynamixel/dynamixel.h>
#include <tina++/feldbus/host/legacystellantriebedevice.h>
#include <tina++/time.h>

#include <QCoreApplication>
#include <QDebug>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QSerialPort>
#include <QThread>

#include <cmath>
#ifdef Q_OS_WIN32
# include <windows.h>
#endif


using namespace TURAG;

FeldbusFrontend::FeldbusFrontend(QWidget *parent) :
	BaseFrontend("TURAG Feldbus", IconManager::get("logo_feldbus"), parent),
	FeldbusAbstraction("Bus", false),
    fromValidator_(nullptr), toValidator_(nullptr), bootloaderFromValidator_(nullptr),
    bootloaderToValidator_(nullptr), selectedDevice_(nullptr), broadcastBootloader(nullptr),
	inquiryRunning(false),
    enumerationRunning(false),
    bootloaderActivationRunning(false),
	connected(false),
	turag_rs485_io_device(nullptr)
{
    /*
     * Default device inquiry interface
     */
    QHBoxLayout* layout = new QHBoxLayout;
    splitter = new QSplitter;

    QWidget* left_layout_widget = new QWidget;
    QVBoxLayout* left_layout = new QVBoxLayout;


    QHBoxLayout* layoutEnumerate = new QHBoxLayout;
    QLabel* enumerateLabel = new QLabel("Enumerate devices:");
    layoutEnumerate->addWidget(enumerateLabel);
    enumerateSequentialButton = new QPushButton("Sequential");
    layoutEnumerate->addWidget(enumerateSequentialButton);
    connect(enumerateSequentialButton, SIGNAL(clicked()), this, SLOT(onReenumerateDevicesSequential()));
    inquiryWidgetList.append(enumerateSequentialButton);
    enumerateBinaryButton = new QPushButton("Binary");
    layoutEnumerate->addWidget(enumerateBinaryButton);
    connect(enumerateBinaryButton, SIGNAL(clicked()), this, SLOT(onReenumerateDevicesBinary()));
    inquiryWidgetList.append(enumerateBinaryButton);
    enumerateBothButton = new QPushButton("Both");
    layoutEnumerate->addWidget(enumerateBothButton);
    connect(enumerateBothButton, SIGNAL(clicked()), this, SLOT(onReenumerateDevicesBoth()));
    inquiryWidgetList.append(enumerateBothButton);
    stopEnumerateButton = new QPushButton("Cancel");
    stopEnumerateButton->setEnabled(false);
    layoutEnumerate->addWidget(stopEnumerateButton);
    connect(stopEnumerateButton, SIGNAL(clicked()), this, SLOT(onStopEnumerate()));
    inquiryWidgetList.append(stopEnumerateButton);
    //layoutEnumerate->addStretch();


    QHBoxLayout* layoutTop = new QHBoxLayout;
    QLabel* fromLabel = new QLabel("Von:");
	inquiryWidgetList.append(fromLabel);
	layoutTop->addWidget(fromLabel);
    fromEdit_ = new LineEditExt(objectName() + "fromAddress", "1");
	inquiryWidgetList.append(fromEdit_);
	layoutTop->addWidget(fromEdit_);
    QLabel* topLabel = new QLabel("bis:");
	inquiryWidgetList.append(topLabel);
	layoutTop->addWidget(topLabel);
    toEdit_ = new LineEditExt(objectName() + "toAddress", "25");
	inquiryWidgetList.append(toEdit_);
	layoutTop->addWidget(toEdit_);
    startInquiry_ = new QPushButton("Search Devices");
    inquiryWidgetList.append(startInquiry_);
    layoutTop->addWidget(startInquiry_);

    dynamixelFromValidator_ = new QIntValidator(0, 252, this);
    dynamixelToValidator_ = new QIntValidator(1, 253, this);

    QHBoxLayout* dynamixel_layoutTop = new QHBoxLayout;
    QLabel* dynamixel_fromLabel = new QLabel("Von:");
	inquiryWidgetList.append(dynamixel_fromLabel);
	dynamixel_layoutTop->addWidget(dynamixel_fromLabel);
    dynamixelFromEdit_ = new LineEditExt(objectName() + "dynamixelFromAddress", "1");
	inquiryWidgetList.append(dynamixelFromEdit_);
	dynamixelFromEdit_->setValidator(dynamixelFromValidator_);
	dynamixel_layoutTop->addWidget(dynamixelFromEdit_);
    QLabel* dynamixel_topLabel = new QLabel("bis:");
	inquiryWidgetList.append(dynamixel_topLabel);
	dynamixel_layoutTop->addWidget(dynamixel_topLabel);
    dynamixelToEdit_ = new LineEditExt(objectName() + "dynamixelToAddress", "25");
	inquiryWidgetList.append(dynamixelToEdit_);
	dynamixelToEdit_->setValidator(dynamixelToValidator_);
    dynamixel_layoutTop->addWidget(dynamixelToEdit_);
    dynamixelStartInquiry_ = new QPushButton("Dynamixel-Servos suchen");
    dynamixel_layoutTop->addWidget(dynamixelStartInquiry_);
    inquiryWidgetList.append(dynamixelStartInquiry_);

    QVBoxLayout* defaultInquiryLayout = new QVBoxLayout;
    //defaultInquiryLayout->addLayout(layoutAboveTop);
    defaultInquiryLayout->addLayout(layoutEnumerate);
    defaultInquiryLayout->addLayout(layoutTop);
    defaultInquiryLayout->addLayout(dynamixel_layoutTop);
    defaultInquiryLayout->addStretch();

    /*
     * BMax inquiry Interface
     */

    QHBoxLayout* bootloaderLayoutAboveSetupLayout = new QHBoxLayout;
    startBootloader_ = new QPushButton("Send Start-Bootloader packets");
    bootloaderInquiryWidgetList.append(startBootloader_);
    bootloaderLayoutAboveSetupLayout->addStretch();
    bootloaderLayoutAboveSetupLayout->addWidget(startBootloader_);


    QHBoxLayout* bootloadertools_layoutTop = new QHBoxLayout;
	QLabel* fromLabel_Boot = new QLabel("Von:");
	bootloaderInquiryWidgetList.append(fromLabel_Boot);
	bootloadertools_layoutTop->addWidget(fromLabel_Boot);
    bootFromEdit_ = new LineEditExt(objectName() + "bootFromAddress", "1");
	bootloaderInquiryWidgetList.append(bootFromEdit_);
	bootloadertools_layoutTop->addWidget(bootFromEdit_);
    QLabel* topLabel_Boot = new QLabel("bis:");
	bootloaderInquiryWidgetList.append(topLabel_Boot);
	bootloadertools_layoutTop->addWidget(topLabel_Boot);
    bootToEdit_ = new LineEditExt(objectName() + "bootToAddress", "25");
	bootloaderInquiryWidgetList.append(bootToEdit_);
	bootloadertools_layoutTop->addWidget(bootToEdit_);
    bootloadertoolsStartInquiry_ = new QPushButton("Search Devices");
    bootloaderInquiryWidgetList.append(bootloadertoolsStartInquiry_);
    bootloadertools_layoutTop->addWidget(bootloadertoolsStartInquiry_);

    QHBoxLayout* bootloaderLayoutEnumerate = new QHBoxLayout;
    QLabel* bootloaderEnumerateLabel = new QLabel("Enumerate devices:");
    bootloaderLayoutEnumerate->addWidget(bootloaderEnumerateLabel);
    bootloaderEnumerateBinaryButton = new QPushButton("Binary");
    bootloaderLayoutEnumerate->addWidget(bootloaderEnumerateBinaryButton);
    connect(bootloaderEnumerateBinaryButton, SIGNAL(clicked()), this, SLOT(onBootloaderReenumerateDevicesBinary()));
    bootloaderInquiryWidgetList.append(bootloaderEnumerateBinaryButton);
    bootloaderStopEnumerateButton = new QPushButton("Cancel");
    bootloaderLayoutEnumerate->addWidget(bootloaderStopEnumerateButton);
    bootloaderStopEnumerateButton->setEnabled(false);
    connect(bootloaderStopEnumerateButton, SIGNAL(clicked()), this, SLOT(onStopEnumerate()));
    bootloaderInquiryWidgetList.append(bootloaderStopEnumerateButton);
    //bootloaderLayoutEnumerate->addStretch();


    QVBoxLayout* bootloaderLayout = new QVBoxLayout;
    bootloaderLayout->addLayout(bootloaderLayoutAboveSetupLayout);
    bootloaderLayout->addLayout(bootloaderLayoutEnumerate);
    bootloaderLayout->addLayout(bootloadertools_layoutTop);
    bootloaderLayout->addStretch();

    /*
     * tabwidget for default & BMax inquiries
     */
    QWidget* defaultInquiryWidget = new QWidget;
    defaultInquiryWidget->setLayout(defaultInquiryLayout);
    QWidget* bmaxInquiryWidget = new QWidget;
    bmaxInquiryWidget->setLayout(bootloaderLayout);

    inquiryTabwidget = new QTabWidget;
    inquiryTabwidget->addTab(defaultInquiryWidget, "Default");
    inquiryTabwidget->addTab(bmaxInquiryWidget, "BMax");


    /*
     * layouts for detected devices
     */
    QVBoxLayout* deviceLayout = new QVBoxLayout;
    deviceList_ = new QListWidget;
    connect(deviceList_, SIGNAL(currentRowChanged(int)), this, SLOT(onDeviceSelected(int)));
    deviceList_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::QSizePolicy::MinimumExpanding);
    deviceInfo_ = new QTextEdit;
    deviceInfo_->setReadOnly(true);
    deviceInfo_->setMinimumHeight(205);
    deviceInfo_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    deviceLayout->addWidget(deviceList_);
    deviceLayout->addWidget(deviceInfo_);
    deviceLayout->setStretch(0,2);

    QWidget* leftDeviceWidget = new QWidget;
    leftDeviceWidget->setLayout(deviceLayout);
    busLog_ = new PlainTextFrontend;

    QTabWidget* tabwidget = new QTabWidget;
    tabwidget->addTab(leftDeviceWidget, "Geräte");
    tabwidget->addTab(busLog_, "Log");

    /*
     * Statistics labels
     */
    masterGroupBox_ = new QGroupBox("Host Statistics");
    QFormLayout* masterStatisticsLayout = new QFormLayout();
    masterNoErrorPackages_ = new QLabel("0");
    masterStatisticsLayout->addRow("Successful", masterNoErrorPackages_);
    masterSendError_ = new QLabel("0");
    masterStatisticsLayout->addRow("Transmit Error", masterSendError_);
    masterMissingData_ = new QLabel("0");
    masterStatisticsLayout->addRow("Missing data", masterMissingData_);
    masterNoAnswer_ = new QLabel("0");
    masterStatisticsLayout->addRow("No Answer", masterNoAnswer_);
    masterChecksumError_ = new QLabel("0");
    masterStatisticsLayout->addRow("Checksum Error", masterChecksumError_);
    masterGroupBox_->setLayout(masterStatisticsLayout);

    slaveGroupBox_ = new QGroupBox("Device Statistics");
    QFormLayout* slaveStatisticsLayout = new QFormLayout();
    slaveAcceptedPackages_ = new QLabel("0");
    slaveStatisticsLayout->addRow("Successful", slaveAcceptedPackages_);
    slaveOverflow_ = new QLabel("0");
    slaveStatisticsLayout->addRow("Overflow", slaveOverflow_);
    slaveLostPackages_ = new QLabel("0");
    slaveStatisticsLayout->addRow("Lost Packets", slaveLostPackages_);
    slaveChecksumError_ = new QLabel("0");
    slaveStatisticsLayout->addRow("Checksum Error", slaveChecksumError_);
    slaveUptime_ = new QLabel("0");
    slaveStatisticsLayout->addRow("Uptime", slaveUptime_);
    QVBoxLayout* slaveStatisticsVLayout = new QVBoxLayout;
    slaveStatisticsVLayout->addLayout(slaveStatisticsLayout);
    slaveGroupBox_->setLayout(slaveStatisticsVLayout);
    slaveGroupBox_->setToolTip(
                "Shows packet statistics since the last device search.\n"
                "Values in brackets originated before the last device search.");

    QHBoxLayout* statisticsLayout = new QHBoxLayout;
    statisticsLayout->addWidget(masterGroupBox_);
    statisticsLayout->addWidget(slaveGroupBox_);

    QHBoxLayout* statisticsUpdateLayout = new QHBoxLayout;
    updateStatisticsButton_ = new QPushButton("Refresh");
    connect(updateStatisticsButton_, SIGNAL(clicked()), this, SLOT(onUpdateStatisticsSlave()));
    updateStatisticsAuto_ = new CheckBoxExt("Auto", "updateFeldbusStatisticsAutomatically");
    connect(updateStatisticsAuto_, SIGNAL(toggled(bool)), updateStatisticsButton_, SLOT(setDisabled(bool)));
    updateStatisticsButton_->setDisabled(updateStatisticsAuto_->isChecked());
    statisticsUpdateLayout->addWidget(updateStatisticsAuto_);
    statisticsUpdateLayout->addWidget(updateStatisticsButton_);
    slaveStatisticsVLayout->addLayout(statisticsUpdateLayout);

    disableStatistics();

    left_layout->addWidget(inquiryTabwidget);
    left_layout->addWidget(tabwidget);
    left_layout->setStretch(1, 2);
    left_layout->addLayout(statisticsLayout);
    left_layout_widget->setLayout(left_layout);

    feldbusWidget = new QWidget;
    layout->addWidget(splitter);
    splitter->addWidget(left_layout_widget);
    splitter->addWidget(feldbusWidget);
	splitter->setStretchFactor(1,2);
    setLayout(layout);

    connect(startInquiry_, SIGNAL(clicked()), this, SLOT(onStartInquiry()));
    connect(bootloadertoolsStartInquiry_, SIGNAL(clicked()), this, SLOT(onStartBootInquiry()));
    connect(dynamixelStartInquiry_, SIGNAL(clicked()), SLOT(onStartDynamixelInquiry()));
    connect(startBootloader_, SIGNAL(clicked()), this, SLOT(onStartBoot()));
    setEnabled(false);

    connect(&availabilityChecker_, SIGNAL(timeout()), this, SLOT(onCheckDeviceAvailability()));
    connect(&sendBroadcastTimer_, SIGNAL(timeout()), this, SLOT(requestStartBootBroad()));
    connect(&updateStatisticsTimer_, SIGNAL(timeout()), this, SLOT(onUpdateStatistics()));

#ifdef Q_OS_WIN32
	// windows is a bit slower :D
	setFeldbusTimeout(50); // [ms]
#else
    setFeldbusTimeout(20); // [ms]
#endif

    onTwoByteAddressCheckBoxToggled(false);
    onBootloaderTwoByteAddressCheckBoxToggled(false);

//#warning please remove me
//    feldbusWidget->hide();
//    splitter->addWidget(new FeldbusAsebView(0));

}


FeldbusFrontend::~FeldbusFrontend() {
    validateAdressFields();
    dynamixelValidateAdressFields();

    if (broadcastBootloader) {
        delete broadcastBootloader;
    }
}

void FeldbusFrontend::onConnected(bool readOnly, QIODevice* dev_) {
    connected = true;
	turag_rs485_io_device = dev_;
    if (!readOnly) {
        setEnabled(true);
    }
    resetStatistics();
}

void FeldbusFrontend::onDisconnected(bool reconnecting) {
    (void)reconnecting;
    connected = false;
	turag_rs485_io_device = nullptr;
    setEnabled(false);
    updateStatisticsTimer_.stop();
}

void FeldbusFrontend::writeData(QByteArray data_) {
	busDataBuffer.append(data_);
}

void FeldbusFrontend::clear(void) {
    deviceInfo_->clear();
    deviceList_->clear();
    dynamixelDevices_.clear();
    devices_.clear();
    busLog_->clear();
}

void FeldbusFrontend::validateAdressFields() {
    if (!fromEdit_->hasAcceptableInput()) {
        fromEdit_->setText("1");
    }
    if (!bootFromEdit_->hasAcceptableInput()) {
        bootFromEdit_->setText("1");
    }
    if (!toEdit_->hasAcceptableInput()) {
		toEdit_->setText(QStringLiteral("%1").arg(fromEdit_->text().toInt()+1));
    }
    if (!bootToEdit_->hasAcceptableInput()) {
		bootToEdit_->setText(QStringLiteral("%1").arg(bootFromEdit_->text().toInt()+1));
    }
    int fromAddress = fromEdit_->text().toInt();
    int toAddress = toEdit_->text().toInt();
    int bootFromAddress = bootFromEdit_->text().toInt();
    int bootToAddress = bootToEdit_->text().toInt();

    if (fromAddress > toAddress) {
		fromEdit_->setText(QStringLiteral("%1").arg(toAddress - 1));
    }
    if (bootFromAddress > bootToAddress) {
		bootFromEdit_->setText(QStringLiteral("%1").arg(bootToAddress - 1));
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
		dynamixelFromEdit_->setText(QStringLiteral("%1").arg(toAddress - 1));
    }
}

void FeldbusFrontend::onInquiry(bool boot) {
	inquiryRunning = true;
	bootloaderActivationRunning = false;

    int checksumTypeIndex;
    checksumTypeIndex = static_cast<int>(Feldbus::ChecksumType::crc8);

    if (!boot) {
		setInquiryWidgetsEnabled(false);
        //checksumTypeIndex = checksumCombobox_->currentIndex();
    } else {
		setBootInquiryWidgetsEnabled(false);
        //checksumTypeIndex = bootloaderChecksumCombobox_->currentIndex();
    }

	availabilityChecker_.stop();

    validateAdressFields();

    int fromAddress = fromEdit_->text().toInt();
    int toAddress = toEdit_->text().toInt();

    if (boot) {
        fromAddress = bootFromEdit_->text().toInt();
        toAddress = bootToEdit_->text().toInt();
    }

	deviceList_->clearSelection();
    deviceList_->clear();
    devices_.clear();
    deviceInfo_->clear();
    busDataBuffer.clear();

    FeldbusDeviceInfoExt dev_info;
    dev_info.address = 0;

    for (int i = fromAddress; i <= toAddress; i++) {
        if (boot) {
            bootloadertoolsStartInquiry_->setText(QStringLiteral("Cancel (%1 %)").arg((i - fromAddress) * 100 / (toAddress - fromAddress + 1)));
        } else {
            startInquiry_->setText(QStringLiteral("Cancel (%1 %)").arg((i - fromAddress) * 100 / (toAddress - fromAddress + 1)));
        }
        for (int j = 0; j <= static_cast<int>(Feldbus::ChecksumType::crc8); j++) {
            if (j == checksumTypeIndex || checksumTypeIndex > static_cast<int>(Feldbus::ChecksumType::crc8)) {
				Feldbus::ChecksumType chksum_type = static_cast<Feldbus::ChecksumType>(j);

                // the condition for detecting the device is a successful ping-request
                // plus successful receiving of the device info making it possible
                // to double check the assumed checksum type
                Feldbus::Device* dev = new TURAG::Feldbus::Device("", i, *this, chksum_type, 2, 1);
                if (dev->isAvailable()) {
                    if (dev->getDeviceInfo(&dev_info.device_info) && dev->getExtendedDeviceInfo(&dev_info.extended_device_info)) {
						if (dev_info.device_info.crcType() == chksum_type) {
                            dev_info.address = i;
                            delete dev;

                            Feldbus::Device* detectedDev = new TURAG::Feldbus::Device("", dev_info.address, *this, static_cast<Feldbus::ChecksumType>(dev_info.device_info.crcType()), TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ATTEMPTS, TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS);
                            QByteArray name_buffer(dev_info.extended_device_info.nameLength() + 2, '\0');
							if (detectedDev->receiveDeviceRealName(name_buffer.data())) {
                                dev_info.device_name = name_buffer;
                            } else {
                                dev_info.device_name = "???";
                            }

                            QByteArray versioninfo_buffer(dev_info.extended_device_info.versionInfoLength() + 2, '\0');
							if (detectedDev->receiveVersionInfo(versioninfo_buffer.data())) {
                                dev_info.versionInfo = versioninfo_buffer;
                            } else {
                                dev_info.versionInfo = "???";
                            }

                            deviceList_->addItem(dev_info.toString());
                            std::shared_ptr<FeldbusDeviceWrapper> sptr;
							sptr.reset(FeldbusDeviceFactory::createFeldbusDevice(dev_info, *this));
                            devices_.append(sptr);

                            if (sptr->device.get()) {
                                // try to save old pakage statistics so we can show them
                                // in brackets
                                sptr->oldSlaveAcceptedPackages = 0;
                                sptr->oldSlaveOverflow = 0;
                                sptr->oldSlaveLostPackages = 0;
                                sptr->oldSlaveChecksumError = 0;
								if (dev_info.device_info.packageStatisticsAvailable()) {
                                    uint32_t count[4];
                                    if (sptr->device->receiveAllSlaveErrorCount(count)) {
                                        // if the call succeeded, we have one accepted package more
                                        sptr->oldSlaveAcceptedPackages = count[0] + 1;
                                        sptr->oldSlaveOverflow = count[1];
                                        sptr->oldSlaveLostPackages = count[2];
                                        sptr->oldSlaveChecksumError = count[3];
                                    }
                                }
                            }

							delete detectedDev;

                            break;
                        }
                    } else {
                        // some device answered, but getting the device-info failed.
                        // This could be a checksum failure but most likely it's a device with
                        // an outdated firmware. We indicate this fact by showing question marks.
                        deviceList_->addItem(QString("%1: ???").arg(i));

                        // deviceList_ and devices_ need to be in sync
                        FeldbusDeviceWrapper* emptyWrapper = new FeldbusDeviceWrapper;
                        emptyWrapper->deviceInfoText =
                                QString("A valid response was received on this address using checksum type %1, but getting the "
                                "device-info failed. This could be a checksum failure but most "
                                "likely it's a device with an outdated firmware which is no longer supported.").arg(j);
                        std::shared_ptr<FeldbusDeviceWrapper> sptr;
                        sptr.reset(emptyWrapper);
                        devices_.append(sptr);
                    }
                }
                delete dev;
            }
        }
        QCoreApplication::processEvents();

		if (!inquiryRunning) {
			break;
		}
    }

	inquiryRunning = false;

    for (DynamixelDeviceWrapper dev_wrapper : dynamixelDevices_) {
        deviceList_->addItem(dev_wrapper.toString());
    }

	if (!boot) {
		setInquiryWidgetsEnabled(true);
	} else {
		setBootInquiryWidgetsEnabled(true);
	}

    availabilityChecker_.start(250);
}

void FeldbusFrontend::onStartInquiry(void) {
	if (!inquiryRunning) {
		// start an inquiry
		onInquiry(false);
	} else {
		// if inquiryRunning is set, then the user wants to cancel
		// which is signalled by setting inquiryRunning to false.
		inquiryRunning = false;
	}
}

void FeldbusFrontend::onStartBootInquiry(void) {
	if (!inquiryRunning) {
		// start an inquiry
		onInquiry(true);
	} else {
		// if inquiryRunning is set, then the user wants to cancel
		// which is signalled by setting inquiryRunning to false.
		inquiryRunning = false;
	}
}

void FeldbusFrontend::onStartDynamixelInquiry(void) {
	if (inquiryRunning) {
		inquiryRunning = false;
		return;
	}

	inquiryRunning = true;

	setDynamixelInquiryWidgetsEnabled(false);

	availabilityChecker_.stop();

    dynamixelValidateAdressFields();

    int fromAddress = dynamixelFromEdit_->text().toInt();
    int toAddress = dynamixelToEdit_->text().toInt();

    deviceList_->clearSelection();
    deviceList_->clear();
    busDataBuffer.clear();

    dynamixelDevices_.clear();
    deviceInfo_->clear();

    turag_dxl_initialize(static_cast<FeldbusAbstraction*>(this));

    for (std::shared_ptr<FeldbusDeviceWrapper> pDevWrapper: devices_) {
        if (pDevWrapper.get()) {
            deviceList_->addItem(pDevWrapper.get()->devInfo.toString());
        }
    }

    for (int i = fromAddress; i <= toAddress; i++) {
        dynamixelStartInquiry_->setText(QStringLiteral("Cancel (%1 %)").arg(i * 100 / (toAddress - fromAddress + 1)));

        Feldbus::DynamixelDevice* dev = new Feldbus::DynamixelDevice("", i, 2, 1);
        int modelNumber = 0;
        int version = 0;
        if (dev->isAvailable() && dev->getModelNumber(&modelNumber) && dev->getFirmwareVersion(&version)) {
            delete dev;
            dev = new Feldbus::DynamixelDevice("", i, 5, 35);
            DynamixelDeviceWrapper dev_wrapper(dev, modelNumber, version);
            dynamixelDevices_.append(dev_wrapper);
            deviceList_->addItem(dev_wrapper.toString());
        }
        QCoreApplication::processEvents();

		if (!inquiryRunning) {
			break;
		}
	}

	inquiryRunning = false;

    availabilityChecker_.start(250);

	setDynamixelInquiryWidgetsEnabled(true);
}


void FeldbusFrontend::enumerate(bool useSequentialSearch, bool useBinarySearch, bool boot)
{
    if (enumerationRunning) {
        return;
    }

    enumerationRunning = true;

    setEnumerateWidgetsEnabled(false, boot);
    availabilityChecker_.stop();

    deviceList_->clearSelection();
    deviceList_->clear();
    devices_.clear();
    deviceInfo_->clear();
    busDataBuffer.clear();


    int checksumTypeIndex;
    checksumTypeIndex = static_cast<int>(Feldbus::ChecksumType::crc8);


    Feldbus::ChecksumType checksumType;
    /*
    if (!boot) {
        checksumTypeIndex = checksumCombobox_->currentIndex();
    } else {
        checksumTypeIndex = bootloaderChecksumCombobox_->currentIndex();
    }
    */

    // use crc8_icode even if "both" is selected, because nobody ever uses anything else
    // but the default crc.
    if (checksumTypeIndex > static_cast<int>(Feldbus::ChecksumType::crc8)) {
        checksumType = Feldbus::ChecksumType::crc8;
    } else {
        checksumType = static_cast<Feldbus::ChecksumType>(checksumTypeIndex);
    }

    Feldbus::DeviceLocator locator(*this, checksumType);

    QList<uint32_t> foundUuids;
    std::tie(foundUuids, std::ignore) =
            enumerateBusNodes(locator, useSequentialSearch, useBinarySearch, &enumerationRunning, [&](int count) {
        QString progress = QStringLiteral("Cancel (%1)").arg(count);
        if (boot) {
            bootloaderStopEnumerateButton->setText(progress);
        } else {
            stopEnumerateButton->setText(progress);
        }
        QCoreApplication::processEvents();
    });

    if (boot) {
        bootloaderStopEnumerateButton->setText("Cancel");
    } else {
        stopEnumerateButton->setText("Cancel");
    }
    setEnumerateWidgetsEnabled(true, boot);

    enumerationRunning = false;
    availabilityChecker_.start(250);


    if (foundUuids.length() > 0) {
        if (boot) {
            bootFromEdit_->setText(QString::number(1));
            bootToEdit_->setText(QString::number(foundUuids.length()));
        } else {
            fromEdit_->setText(QString::number(1));
            toEdit_->setText(QString::number(foundUuids.length()));
        }

        onInquiry(boot);
    }

}

void FeldbusFrontend::onStopEnumerate()
{
    enumerationRunning = false;
}

void FeldbusFrontend::onReenumerateDevicesSequential() {
    enumerate(true, false, false);
}

void FeldbusFrontend::onReenumerateDevicesBinary()
{
    enumerate(false, true, false);
}

void FeldbusFrontend::onReenumerateDevicesBoth()
{
    enumerate(true, true, false);
}

void FeldbusFrontend::onBootloaderReenumerateDevicesBinary()
{
    enumerate(false, true, true);
}

void FeldbusFrontend::onDeviceSelected(int row) {
    feldbusWidget->hide();
	delete feldbusWidget;
    selectedDevice_ = nullptr;
    disableStatistics();
    updateStatisticsTimer_.stop();

    for (DynamixelDeviceWrapper dev_wrapper : dynamixelDevices_) {
        dev_wrapper.device_.get()->setLed(false);
    }


    if (row < devices_.size() && row != -1) {
        selectedDevice_ = devices_.at(row).get();
        deviceInfo_->setText(selectedDevice_->deviceInfoText);

        if (selectedDevice_->device.get()) {
            updateStatisticsTimer_.start(750);
            enableStatistics();

            // create Aktor view for generic Aktor
            Feldbus::LegacyStellantriebeDevice* aktor = dynamic_cast<Feldbus::LegacyStellantriebeDevice*>(selectedDevice_->device.get());
            if (aktor) {
                feldbusWidget = new FeldbusAktorView(aktor);
                splitter->addWidget(feldbusWidget);
                splitter->setStretchFactor(1,2);
                return;
            }

            // create ASEB view
            Feldbus::ASEBBase* aseb = dynamic_cast<Feldbus::ASEBBase*>(selectedDevice_->device.get());
            if (aseb) {
                feldbusWidget = new FeldbusAsebView(aseb);
                splitter->addWidget(feldbusWidget);
                splitter->setStretchFactor(1,2);
                return;
            }

			// create Bootloader Atmega view
			Feldbus::BootloaderAtmega* atmegaBoot = dynamic_cast<Feldbus::BootloaderAtmega*>(selectedDevice_->device.get());
			if (atmegaBoot) {
                feldbusWidget = new FeldbusBootloaderAtmegaView(atmegaBoot, this);
				splitter->addWidget(feldbusWidget);
				splitter->setStretchFactor(1,2);
				return;
			}

			// create Bootloader Xmega view
			Feldbus::BootloaderXmega* xmegaBoot = dynamic_cast<Feldbus::BootloaderXmega*>(selectedDevice_->device.get());
			if (xmegaBoot) {
                feldbusWidget = new FeldbusBootloaderXmegaView(xmegaBoot, this);
				splitter->addWidget(feldbusWidget);
				splitter->setStretchFactor(1,2);
				return;
			}

            // create Bootloader STM32v2 view
            Feldbus::BootloaderStm32v2* stm32v2Boot = dynamic_cast<Feldbus::BootloaderStm32v2*>(selectedDevice_->device.get());
            if (stm32v2Boot) {
                feldbusWidget = new FeldbusBootloaderStm32v2View(stm32v2Boot, this);
                splitter->addWidget(feldbusWidget);
                splitter->setStretchFactor(1,2);
                return;
            }

            // create generic Bootloader view
            Feldbus::BootloaderAvrBase* genericBoot = dynamic_cast<Feldbus::BootloaderAvrBase*>(selectedDevice_->device.get());
            if (genericBoot) {
                feldbusWidget = new FeldbusBootloaderBaseView(genericBoot, this);
                splitter->addWidget(feldbusWidget);
                splitter->setStretchFactor(1,2);
                return;
            }

            // create Muxer view
            Feldbus::Muxer_64_32* muxer = dynamic_cast<Feldbus::Muxer_64_32*>(selectedDevice_->device.get());
            if (muxer) {
                feldbusWidget = new FeldbusMuxerView(muxer, this);
                splitter->addWidget(feldbusWidget);
                splitter->setStretchFactor(1,2);
                return;
        }



        }
    } else if (row >= devices_.size() && row < devices_.size() + dynamixelDevices_.size()) {
        Feldbus::DynamixelDevice* dev = dynamixelDevices_.at(row - devices_.size()).device_.get();
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
    busLog_->writeData(QString("%1\n").arg(msg).toLatin1());
}

void FeldbusFrontend::onCheckDeviceAvailability(void) {
    int i = 0;

    for (std::shared_ptr<FeldbusDeviceWrapper> pDevWrapper : devices_) {
        //for (FeldbusDeviceWrapper dev_wrapper_ : devices_) {
        if (pDevWrapper.get() && pDevWrapper.get()->device.get() && !pDevWrapper.get()->device.get()->isAvailable()) {
            deviceList_->item(i)->setText(pDevWrapper.get()->devInfo.toString() + " OFFLINE");
        }
        ++i;
    }

    i = 0;
    for (DynamixelDeviceWrapper dev_wrapper : dynamixelDevices_) {
        if (dev_wrapper.device_.get()->hasReachedTransmissionErrorLimit()) {
            deviceList_->item(i + devices_.size())->setText(dev_wrapper.toString() + " OFFLINE");
        }
        ++i;
    }
}


void FeldbusFrontend::requestStartBootBroad(void) {
	static int animationCounter = 0;
	static int animationState = 0;

	if (bootloaderActivationRunning) {
		broadcastBootloader->sendEnterBootloaderBroadcast();

		++animationCounter;
		if (animationCounter > 35) {
			animationCounter = 0;
			++animationState;

			switch(animationState) {
			case  1: {
				QFont font = startBootloader_->font();
				font.setWeight(QFont::Normal);
				startBootloader_->setFont(font);
				break;
			}

			default:  {
				QFont font = startBootloader_->font();
				font.setWeight(QFont::Bold);
				startBootloader_->setFont(font);
				animationState = 0;
				break;
			}
			}
		}
	} else {
        startBootloader_->setText(QStringLiteral("Send Start-Bootloader packets"));
		QFont font = startBootloader_->font();
		font.setWeight(QFont::Normal);
		startBootloader_->setFont(font);
		sendBroadcastTimer_.stop();
		animationCounter = 0;
		animationState = 0;

		if (!inquiryRunning) {
			for (QWidget* child : bootloaderStartBootloaderWidgetList) {
				child->setEnabled(true);
			}
		}
	}
}


void FeldbusFrontend::onStartBoot(void) {
	if (bootloaderActivationRunning) {
		bootloaderActivationRunning = false;
		return;
	}

	bootloaderActivationRunning = true;
	sendBroadcastTimer_.start(10);
    startBootloader_->setText(QStringLiteral("Cancel"));
	QFont font = startBootloader_->font();
	font.setWeight(QFont::Bold);
	startBootloader_->setFont(font);

	for (QWidget* child : bootloaderStartBootloaderWidgetList) {
		child->setEnabled(false);
	}

	if (broadcastBootloader) {
		delete broadcastBootloader;
	}
    /*broadcastBootloader = new Feldbus::Bootloader(
                "broadcastBootloader", 0, *this, static_cast<Feldbus::ChecksumType>(bootloaderChecksumCombobox_->currentIndex()));*/
    broadcastBootloader = new Feldbus::Bootloader(
                "broadcastBootloader", 0, *this, Feldbus::ChecksumType::crc8);
}

void FeldbusFrontend::onUpdateStatistics(void) {
    if (selectedDevice_->device.get()) {
		int successfulTransmissions = selectedDevice_->device.get()->getTotalTransmissions()
				- selectedDevice_->device.get()->getNoAnswerErrors()
				- selectedDevice_->device.get()->getMissingDataErrors()
				- selectedDevice_->device.get()->getChecksumErrors()
				- selectedDevice_->device.get()->getTransmitErrors();

		masterNoErrorPackages_->setText(QString("%1 (%2 %)").arg(successfulTransmissions)
										.arg(100 * successfulTransmissions / selectedDevice_->device.get()->getTotalTransmissions()));
        masterNoAnswer_->setText(QString("%1").arg(selectedDevice_->device.get()->getNoAnswerErrors()));
        masterMissingData_->setText(QString("%1").arg(selectedDevice_->device.get()->getMissingDataErrors()));
        masterChecksumError_->setText(QString("%1").arg(selectedDevice_->device.get()->getChecksumErrors()));
        masterSendError_->setText(QString("%1").arg(selectedDevice_->device.get()->getTransmitErrors()));

        if (updateStatisticsAuto_->isChecked()) {
            onUpdateStatisticsSlave();
        }
    }
}

void FeldbusFrontend::onUpdateStatisticsSlave(void) {
    if (selectedDevice_->device.get()) {
        float uptime = NAN;

        if (selectedDevice_->device->receiveUptime(&uptime)) {
            if (std::isnan(uptime)) {
                slaveUptime_->setText("n/a");
            } else {
                slaveUptime_->setText(QString("%1").arg(uptime, 0, 'f', 2));
            }
        } else {
            slaveUptime_->setText("error");
        }

        Feldbus::Device::DeviceInfo devInfo;
        selectedDevice_->device->getDeviceInfo(&devInfo);

		if (devInfo.packageStatisticsAvailable()) {
            uint32_t count[4];
            if (selectedDevice_->device->receiveAllSlaveErrorCount(count)) {
                slaveAcceptedPackages_->setText(QString("%1 (%2)").arg(count[0] - selectedDevice_->oldSlaveAcceptedPackages).arg(selectedDevice_->oldSlaveAcceptedPackages));
                slaveOverflow_->setText(QString("%1 (%2)").arg(count[1] - selectedDevice_->oldSlaveOverflow).arg(selectedDevice_->oldSlaveOverflow));
                slaveLostPackages_->setText(QString("%1 (%2)").arg(count[2] - selectedDevice_->oldSlaveLostPackages).arg(selectedDevice_->oldSlaveLostPackages));
                slaveChecksumError_->setText(QString("%1 (%2)").arg(count[3] - selectedDevice_->oldSlaveChecksumError).arg(selectedDevice_->oldSlaveChecksumError));
            } else {
                slaveAcceptedPackages_->setText("error");
                slaveOverflow_->setText("error");
                slaveLostPackages_->setText("error");
                slaveChecksumError_->setText("error");
            }
        } else {
            slaveAcceptedPackages_->setText("n/a");
            slaveOverflow_->setText("n/a");
            slaveLostPackages_->setText("n/a");
            slaveChecksumError_->setText("n/a");
        }
    }
}

void FeldbusFrontend::disableStatistics(void) {
    masterGroupBox_->setEnabled(false);
    slaveGroupBox_->setEnabled(false);
}

void FeldbusFrontend::enableStatistics(void) {
    masterGroupBox_->setEnabled(true);
    slaveGroupBox_->setEnabled(true);
}

void FeldbusFrontend::resetStatistics(void) {
    masterNoErrorPackages_->setText("0");
    masterNoAnswer_->setText("0");
    masterMissingData_->setText("0");
    masterChecksumError_->setText("0");
    masterSendError_->setText("0");
    slaveOverflow_->setText("0");
    slaveChecksumError_->setText("0");
    slaveLostPackages_->setText("0");
    slaveAcceptedPackages_->setText("0");
    slaveUptime_->setText("0");
}


std::tuple<QList<uint32_t>, bool>
FeldbusFrontend::enumerateBusNodes(Feldbus::DeviceLocator &locator,
                                   bool useSequentialSearch,
                                   bool useBinarySearch, bool *keepRunning,
                                   std::function<void(int)> onDeviceCountChanged) {
    QList<uint32_t> foundUuids;
    unsigned binarySearchDelayTimeMs = 5;

    // in case the user does not supply means to cancel.
    bool keepRunningDummy = true;
    if (keepRunning == nullptr) {
        keepRunning = &keepRunningDummy;
    }

    if (!useSequentialSearch && !useBinarySearch) {
        return std::make_tuple(foundUuids, false);
    }

    // first reset all bus addresses
    if (!locator.resetAllBusAddresses()) {
        return {};
    }

    // directly return result of a binary only search function
    if (useSequentialSearch == false) {
        auto searcher = BinaryAddressSearcher(locator, binarySearchDelayTimeMs);

        while (*keepRunning && !searcher.searchFinished()) {
            bool foundDevice = false;
            uint32_t address = 0;
            if (!searcher.TryFindNextDevice(&foundDevice, &address)) {
                return std::make_tuple(foundUuids, false);
            }

            if (foundDevice) {
                if (!locator.setBusAddress(address, foundUuids.length() + 1)) {
                    return std::make_tuple(foundUuids, false);
                }

                foundUuids.push_back(address);
                if (onDeviceCountChanged) {
                    onDeviceCountChanged(foundUuids.length());
                }
            }
        }
        return std::make_tuple(foundUuids, false);
    }

    // Thus we can assume here that useSequentialSearch == true
    // and useBinarySearch is true or false
    bool deviceOrderKnown = true;

    if (!locator.disableBusNeighbors()) {
        return std::make_tuple(foundUuids, false);
    }

    while (*keepRunning) {
        uint32_t address = 0;
        if (!locator.sendBroadcastPing(&address)) {
            // if the broadcast ping fails it can mean that either there are no more
            // devices or more than one device tried to respond. If enabled, we fall
            // back to binary search.
            if (useBinarySearch) {
                int deviceCountBeforeBinarySearch = foundUuids.length();

                // enumerate devices which do not have valid bus address yet
                auto searcher = BinaryAddressSearcher(locator, binarySearchDelayTimeMs, true);

                while (*keepRunning && !searcher.searchFinished()) {
                    bool foundDevice = false;
                    if (!searcher.TryFindNextDevice(&foundDevice, &address)) {
                        return std::make_tuple(foundUuids, deviceOrderKnown);
                    }

                    if (foundDevice) {
                        if (!locator.setBusAddress(address, foundUuids.length() + 1)) {
                            return std::make_tuple(foundUuids, deviceOrderKnown);
                        }
                        deviceOrderKnown = false;
                        foundUuids.push_back(address);
                        if (onDeviceCountChanged) {
                            onDeviceCountChanged(foundUuids.length());
                        }
                    }
                }

                // return on cancel or if the binary search did not yield any more devices.
                if (!*keepRunning || deviceCountBeforeBinarySearch == foundUuids.length()) {
                    return std::make_tuple(foundUuids, deviceOrderKnown);
                }
            } else {
                // if the broadcast ping failed but we cannot do binary search,
                // just return the current result
                return std::make_tuple(foundUuids, deviceOrderKnown);
            }
        } else {
            // we found a device while doing sequential search
            if (!locator.setBusAddress(address, foundUuids.length() + 1)) {
                return std::make_tuple(foundUuids, deviceOrderKnown);
            }
            foundUuids.push_back(address);
            if (onDeviceCountChanged) {
                onDeviceCountChanged(foundUuids.length());
            }
        }

        // enable the next bus node
        if (!locator.enableBusNeighbors()) {
            return std::make_tuple(foundUuids, deviceOrderKnown);
        }
    }

    // the loop was cancelled
    return std::make_tuple(foundUuids, deviceOrderKnown);
}

void FeldbusFrontend::onTwoByteAddressCheckBoxToggled(bool state) {
    fromEdit_->setValidator(0);
    toEdit_->setValidator(0);

    if (fromValidator_) {
        fromValidator_->deleteLater();
    }
    if (toValidator_) {
        toValidator_->deleteLater();
    }

    if (!state) {
        fromValidator_ = new QIntValidator(1, 127, this);
        toValidator_ = new QIntValidator(1, 127, this);
    } else {
        fromValidator_ = new QIntValidator(1, 32767, this);
        toValidator_ = new QIntValidator(1, 32767, this);
    }

    fromEdit_->setValidator(fromValidator_);
    toEdit_->setValidator(toValidator_);
    validateAdressFields();
}


void FeldbusFrontend::onBootloaderTwoByteAddressCheckBoxToggled(bool state) {
    bootFromEdit_->setValidator(0);
    bootToEdit_->setValidator(0);

    if (bootloaderFromValidator_) {
        delete bootloaderFromValidator_;
    }
    if (bootloaderToValidator_) {
        delete bootloaderToValidator_;
    }

    if (!state) {
        bootloaderFromValidator_ = new QIntValidator(1, 127, this);
        bootloaderToValidator_ = new QIntValidator(1, 127, this);
    } else {
        bootloaderFromValidator_ = new QIntValidator(1, 32767, this);
        bootloaderToValidator_ = new QIntValidator(1, 32767, this);
    }

    bootFromEdit_->setValidator(bootloaderFromValidator_);
    bootToEdit_->setValidator(bootloaderToValidator_);
    validateAdressFields();
}

void FeldbusFrontend::setInquiryWidgetsEnabled(bool enabled) {
	if (enabled) {
		inquiryTabwidget->setTabEnabled(1, true);

		for (QWidget* child : inquiryWidgetList) {
			child->setEnabled(true);
		}
        startInquiry_->setText("Search Devices");
	} else {
		inquiryTabwidget->setTabEnabled(1, false);
		for (QWidget* child : inquiryWidgetList) {
			child->setEnabled(false);
		}
        startInquiry_->setEnabled(true);
	}
    bootloaderStopEnumerateButton->setEnabled(false);
    stopEnumerateButton->setEnabled(false);
}

void FeldbusFrontend::setBootInquiryWidgetsEnabled(bool enabled) {
	if (enabled) {
		inquiryTabwidget->setTabEnabled(0, true);
        bootloadertoolsStartInquiry_->setText("Search Devices");

		for (QWidget* child : bootloaderInquiryWidgetList) {
			child->setEnabled(true);
		}
	} else {
		inquiryTabwidget->setTabEnabled(0, false);

		for (QWidget* child : bootloaderInquiryWidgetList) {
			child->setEnabled(false);
		}
        bootloadertoolsStartInquiry_->setEnabled(true);
	}
    bootloaderStopEnumerateButton->setEnabled(false);
    stopEnumerateButton->setEnabled(false);
}

void FeldbusFrontend::setDynamixelInquiryWidgetsEnabled(bool enabled) {
	if (enabled) {
		inquiryTabwidget->setTabEnabled(1, true);

		for (QWidget* child : inquiryWidgetList) {
			child->setEnabled(true);
		}
		dynamixelStartInquiry_->setText("Dynamixel-Servos suchen");
	} else {
		inquiryTabwidget->setTabEnabled(1, false);
		for (QWidget* child : inquiryWidgetList) {
			child->setEnabled(false);
		}
        dynamixelStartInquiry_->setEnabled(true);
	}
    bootloaderStopEnumerateButton->setEnabled(false);
    stopEnumerateButton->setEnabled(false);
}

void FeldbusFrontend::setEnumerateWidgetsEnabled(bool enabled, bool boot)
{
    if (enabled) {
        if (boot) {
            for (QWidget* child : bootloaderInquiryWidgetList) {
                child->setEnabled(true);
            }
            bootloaderStopEnumerateButton->setEnabled(false);
            inquiryTabwidget->setTabEnabled(0, true);
        } else {
            for (QWidget* child : inquiryWidgetList) {
                child->setEnabled(true);
            }
            stopEnumerateButton->setEnabled(false);
            inquiryTabwidget->setTabEnabled(1, true);
        }
    } else {
        if (boot) {
            for (QWidget* child : bootloaderInquiryWidgetList) {
                child->setEnabled(false);
            }
            bootloaderStopEnumerateButton->setEnabled(true);
            inquiryTabwidget->setTabEnabled(0, false);
        } else {
            for (QWidget* child : inquiryWidgetList) {
                child->setEnabled(false);
            }
            stopEnumerateButton->setEnabled(true);
            inquiryTabwidget->setTabEnabled(1, false);
        }
    }
}

bool FeldbusFrontend::doTransceive(const uint8_t *transmit, int *transmit_length, uint8_t *receive, int *receive_length, bool delayTransmission) {
    QSerialPort* serialPort = dynamic_cast<QSerialPort*>(turag_rs485_io_device);
    static SystemTime lastTransmission;

    // TODO: move at a better place.
    SystemTime requiredProcessingTime = SystemTime::fromMsec(10);

    if (!connected || serialPort == nullptr) {
        return false;
    }

	if (transmit_length && transmit) {
        if (delayTransmission) {
            unsigned requiredFrameDelayUs = static_cast<unsigned>(15.0 /serialPort->baudRate() * 1e6);
            while (lastTransmission + SystemTime::fromUsec(requiredFrameDelayUs) +
                   requiredProcessingTime > SystemTime::now()) {
                QThread::msleep(1);
            }

            clearBuffer();
        }

		int transmit_length_copy = *transmit_length;

		if (transmit_length_copy > 0) {
			QString transmitmsg = QString("_Write: %1 {").arg(transmit_length_copy);
			for (int i = 0; i < transmit_length_copy; ++i) {
				transmitmsg += QString("%1 ").arg(transmit[i]);
			}
			transmitmsg += "}:";

			*transmit_length = turag_rs485_io_device->write(reinterpret_cast<const char*>(transmit), transmit_length_copy);

			if (transmit_length_copy != *transmit_length) {
				onRs485DebugMsg(transmitmsg + " failed");
				if (receive_length) {
					*receive_length = 0;
				}
				return false;
			}
			if (!turag_rs485_io_device->waitForBytesWritten(feldbusTimeout_ms)) {
				onRs485DebugMsg(transmitmsg + " timeout");
				if (receive_length) {
					*receive_length = 0;
				}
				return false;
			}
            lastTransmission = SystemTime::now();
			onRs485DebugMsg(transmitmsg + " ok");
		}
	}

	if (receive_length && receive) {
		int receive_length_copy = *receive_length;

		if (receive_length_copy > 0) {
			QString outmsg = QString("_Read required: %1").arg(receive_length_copy);

			#ifdef Q_OS_WIN32
				unsigned long startTick = GetTickCount();
				int lastSize = busDataBuffer.size();

				while (busDataBuffer.size() < receive_length_copy) {
					turag_rs485_io_device->waitForReadyRead(0);
					if (lastSize < busDataBuffer.size()) {
						lastSize = busDataBuffer.size();
						startTick = GetTickCount();
					}


                    if (GetTickCount() - startTick > static_cast<unsigned long>(feldbusTimeout_ms)) {
						onRs485DebugMsg(outmsg + QString(" Timeout (%1)").arg(busDataBuffer.size()));
						*receive_length = busDataBuffer.size();
						clearBuffer();
						return false;
					}
				}
			#else
				while (busDataBuffer.size() < receive_length_copy) {
					if (!turag_rs485_io_device->waitForReadyRead(feldbusTimeout_ms)) {
						onRs485DebugMsg(outmsg + QString(" Timeout (%1)").arg(busDataBuffer.size()));
						*receive_length = busDataBuffer.size();
						clearBuffer();
						return false;
					}
				}
			#endif

			onRs485DebugMsg(outmsg + QString(" available: %1/%2").arg(busDataBuffer.size()).arg(receive_length_copy));
			outmsg = "data: {";
			for (int i = 0; i < receive_length_copy; ++i) {
				receive[i] = busDataBuffer.at(i);
				outmsg += QString("%1 ").arg(receive[i]);
			}
			onRs485DebugMsg(outmsg + "}");
			busDataBuffer.remove(0, receive_length_copy);
		}
	}

    return true;
}

void FeldbusFrontend::clearBuffer(void) {
    if (busDataBuffer.length() > 0) {
        onRs485DebugMsg(QString("cleared %1 byte").arg(busDataBuffer.length()));
    }
    busDataBuffer.clear();
}

void FeldbusFrontend::setFeldbusTimeout(int milliSeconds) {
	feldbusTimeout_ms = milliSeconds;
}

int FeldbusFrontend::getFeldbusTimeout(void) {
	return feldbusTimeout_ms;
}
