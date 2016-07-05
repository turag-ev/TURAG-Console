#include "feldbusfrontend.h"
#include "feldbusviews/feldbusfarbsensorview.h"
#include "feldbusviews/dynamixelview.h"
#include "feldbusviews/feldbusaktorview.h"
#include "feldbusviews/feldbusasebview.h"
#include "feldbusviews/feldbusbootloaderatmegaview.h"
#include "feldbusviews/feldbusbootloaderxmegaview.h"
#include <libs/checkboxext.h>
#include <libs/comboboxext.h>
#include <libs/lineeditext.h>
#include <libs/iconmanager.h>
#include "plaintextfrontend.h"

#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>
#include <tina++/feldbus/host/farbsensor.h>
#include <tina++/feldbus/host/aktor.h>
#include <tina++/feldbus/host/dcmotor.h>
#include <tina++/feldbus/host/servo.h>
#include <tina++/feldbus/dynamixel/dynamixeldevice.h>

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

#include <cmath>
#ifdef Q_OS_WIN32
# include <windows.h>
#endif


using namespace TURAG;

FeldbusFrontend::FeldbusFrontend(QWidget *parent) :
	BaseFrontend("TURAG Feldbus", IconManager::get("logo_feldbus"), parent),
    fromValidator_(nullptr), toValidator_(nullptr), bootloaderFromValidator_(nullptr),
    bootloaderToValidator_(nullptr), selectedDevice_(nullptr), broadcastBootloader(nullptr),
    deviceAddressLength(Feldbus::Device::AddressLength::byte_),
	bootloaderAddressLength(Feldbus::Device::AddressLength::byte_),
	inquiryRunning(false),
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

    QHBoxLayout* layoutAboveTop = new QHBoxLayout;
    checksumCombobox_ = new ComboBoxExt("ChecksumConventionalDevices", 2);
	inquiryWidgetList.append(checksumCombobox_);
    checksumCombobox_->addItem("xor");
    checksumCombobox_->addItem("crc8_icode");
    checksumCombobox_->addItem("Alle");
    checksumCombobox_->selectStandardItem();
    checksumCombobox_->setToolTip("Stellt ein, mit welcher Checksumme nach Geräten gesucht wird. Bei Auswahl von `Alle' dauert die Suche entsprechend länger.");
    QFormLayout* layoutAboveTopFormLayout = new QFormLayout;
	QLabel* checksumLabel = new QLabel("Checksumme:");
	inquiryWidgetList.append(checksumLabel);
	layoutAboveTopFormLayout->addRow(checksumLabel, checksumCombobox_);
    layoutAboveTop->addLayout(layoutAboveTopFormLayout);
    twoByteAddressCheckbox_ = new CheckBoxExt("2-Byte-Adresse", "twoByteAddressConventionalDevicesCheckbox", false);
	inquiryWidgetList.append(twoByteAddressCheckbox_);
	connect(twoByteAddressCheckbox_, SIGNAL(toggled(bool)), this, SLOT(onTwoByteAddressCheckBoxToggled(bool)));
    layoutAboveTop->addWidget(twoByteAddressCheckbox_);

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
    startInquiry_ = new QPushButton("Geräte suchen");
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

    QVBoxLayout* defaultInquiryLayout = new QVBoxLayout;
    defaultInquiryLayout->addLayout(layoutAboveTop);
    defaultInquiryLayout->addLayout(layoutTop);
    defaultInquiryLayout->addLayout(dynamixel_layoutTop);
    defaultInquiryLayout->addStretch();

    /*
     * BMax inquiry Interface
     */
    QHBoxLayout* bootloaderLayoutAboveSetupLayout = new QHBoxLayout;
    bootloaderChecksumCombobox_ = new ComboBoxExt("BootloaderChecksumConventionalDevices", 1);
	bootloaderStartBootloaderWidgetList.append(bootloaderChecksumCombobox_);
	bootloaderInquiryWidgetList.append(bootloaderChecksumCombobox_);
    bootloaderChecksumCombobox_->addItem("xor");
    bootloaderChecksumCombobox_->addItem("crc8_icode");
    bootloaderChecksumCombobox_->selectStandardItem();
    bootloaderChecksumCombobox_->setToolTip("Stellt ein, mit welcher Checksumme nach Geräten gesucht wird.");
    QFormLayout* bootloaderLayoutAboveSetupLayoutFormLayout = new QFormLayout;
	QLabel* bootloaderChecksumLabel = new QLabel("Checksumme:");
	bootloaderStartBootloaderWidgetList.append(bootloaderChecksumLabel);
	bootloaderInquiryWidgetList.append(bootloaderChecksumLabel);
	bootloaderLayoutAboveSetupLayoutFormLayout->addRow(bootloaderChecksumLabel, bootloaderChecksumCombobox_);
    bootloaderLayoutAboveSetupLayout->addLayout(bootloaderLayoutAboveSetupLayoutFormLayout);
    bootloaderTwoByteAddressCheckbox_ = new CheckBoxExt("2-Byte-Adresse", "BootloaderTwoByteAddressConventionalDevicesCheckbox", true);
	bootloaderStartBootloaderWidgetList.append(bootloaderTwoByteAddressCheckbox_);
	bootloaderInquiryWidgetList.append(bootloaderTwoByteAddressCheckbox_);
	connect(bootloaderTwoByteAddressCheckbox_, SIGNAL(toggled(bool)), this, SLOT(onBootloaderTwoByteAddressCheckBoxToggled(bool)));
    bootloaderLayoutAboveSetupLayout->addWidget(bootloaderTwoByteAddressCheckbox_);


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
	QVBoxLayout* bootButtonLayout = new QVBoxLayout;
	startBootloader_ = new QPushButton("Bootloader starten");
	bootloaderInquiryWidgetList.append(startBootloader_);
	bootButtonLayout->addWidget(startBootloader_);
	bootloadertoolsStartInquiry_ = new QPushButton("Geräte suchen");
	bootButtonLayout->addWidget(bootloadertoolsStartInquiry_);
	bootloadertools_layoutTop->addLayout(bootButtonLayout);

    QVBoxLayout* bootloaderLayout = new QVBoxLayout;
    bootloaderLayout->addLayout(bootloaderLayoutAboveSetupLayout);
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
    inquiryTabwidget->addTab(defaultInquiryWidget, "Standard");
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
    deviceInfo_->setMinimumHeight(140);
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
    masterGroupBox_ = new QGroupBox("Master-Statistiken");
    QFormLayout* masterStatisticsLayout = new QFormLayout();
    masterNoErrorPackages_ = new QLabel("0");
    masterStatisticsLayout->addRow("Fehlerfrei", masterNoErrorPackages_);
    masterSendError_ = new QLabel("0");
    masterStatisticsLayout->addRow("Sendefehler", masterSendError_);
    masterMissingData_ = new QLabel("0");
    masterStatisticsLayout->addRow("Fehlende Daten", masterMissingData_);
    masterNoAnswer_ = new QLabel("0");
    masterStatisticsLayout->addRow("Keine Antwort", masterNoAnswer_);
    masterChecksumError_ = new QLabel("0");
    masterStatisticsLayout->addRow("Checksummen-Fehler", masterChecksumError_);
    masterGroupBox_->setLayout(masterStatisticsLayout);

    slaveGroupBox_ = new QGroupBox("Slave-Statistiken");
    QFormLayout* slaveStatisticsLayout = new QFormLayout();
    slaveAcceptedPackages_ = new QLabel("0");
    slaveStatisticsLayout->addRow("Fehlerfrei", slaveAcceptedPackages_);
    slaveOverflow_ = new QLabel("0");
    slaveStatisticsLayout->addRow("Overflow", slaveOverflow_);
    slaveLostPackages_ = new QLabel("0");
    slaveStatisticsLayout->addRow("Verlorene Pakete", slaveLostPackages_);
    slaveChecksumError_ = new QLabel("0");
    slaveStatisticsLayout->addRow("Checksummen-Fehler", slaveChecksumError_);
    slaveUptime_ = new QLabel("0");
    slaveStatisticsLayout->addRow("Uptime", slaveUptime_);
    QVBoxLayout* slaveStatisticsVLayout = new QVBoxLayout;
    slaveStatisticsVLayout->addLayout(slaveStatisticsLayout);
    slaveGroupBox_->setLayout(slaveStatisticsVLayout);
    slaveGroupBox_->setToolTip(
                "Zeigt Paketstatistiken seit der letzten Gerätesuche an.\n"
                "Werte in Klammern sind vor der letzten Gerätesuche entstanden.\n"
                "Es ist zu beachten, dass die Gerätesuche Checksummenfehler im Slave verursachen kann.");

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
	setFeldbusTimeout(10); // [ms]
#endif

    onTwoByteAddressCheckBoxToggled(twoByteAddressCheckbox_->isChecked());
    onBootloaderTwoByteAddressCheckBoxToggled(bootloaderTwoByteAddressCheckbox_->isChecked());

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

    TURAG::Feldbus::Device::AddressLength addressLength;
    int checksumTypeIndex;
    if (!boot) {
		setInquiryWidgetsEnabled(false);
        addressLength = deviceAddressLength;
        checksumTypeIndex = checksumCombobox_->currentIndex();
    } else {
		setBootInquiryWidgetsEnabled(false);
        addressLength = bootloaderAddressLength;
        checksumTypeIndex = bootloaderChecksumCombobox_->currentIndex();
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
			bootloadertoolsStartInquiry_->setText(QStringLiteral("Abbrechen (%1 %)").arg((i - fromAddress) * 100 / (toAddress - fromAddress + 1)));
        } else {
			startInquiry_->setText(QStringLiteral("Abbrechen (%1 %)").arg((i - fromAddress) * 100 / (toAddress - fromAddress + 1)));
        }
        for (int j = 0; j <= static_cast<int>(Feldbus::Device::ChecksumType::crc8_icode); j++) {
            if (j == checksumTypeIndex || checksumTypeIndex > static_cast<int>(Feldbus::Device::ChecksumType::crc8_icode)) {
                Feldbus::Device::ChecksumType chksum_type = static_cast<Feldbus::Device::ChecksumType>(j);

                // the condition for detecting the device is a successful ping-request
                // plus successful receiving of the device info making it possible
                // to double check the assumed checksum type
                Feldbus::Device* dev = new TURAG::Feldbus::Device("", i, this, chksum_type, addressLength, 2, 1);
                if (dev->isAvailable()) {
                    if (dev->getDeviceInfo(&dev_info.device_info)) {
						if (dev_info.device_info.crcType() == chksum_type) {
                            dev_info.address = i;
                            delete dev;

							Feldbus::Device* detectedDev = new TURAG::Feldbus::Device("", dev_info.address, this, static_cast<Feldbus::Device::ChecksumType>(dev_info.device_info.crcType()), addressLength, TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ATTEMPTS, TURAG_FELDBUS_DEVICE_CONFIG_MAX_TRANSMISSION_ERRORS);
							QByteArray name_buffer(dev_info.device_info.nameLength() + 2, '\0');
							if (detectedDev->receiveDeviceRealName(name_buffer.data())) {
                                dev_info.device_name = name_buffer;
                            } else {
                                dev_info.device_name = "???";
                            }

							QByteArray versioninfo_buffer(dev_info.device_info.versionInfoLength() + 2, '\0');
							if (detectedDev->receiveVersionInfo(versioninfo_buffer.data())) {
                                dev_info.versionInfo = versioninfo_buffer;
                            } else {
                                dev_info.versionInfo = "???";
                            }

                            deviceList_->addItem(dev_info.toString());
                            dev_info.addressLength = addressLength;
                            std::shared_ptr<FeldbusDeviceWrapper> sptr;
                            sptr.reset(FeldbusDeviceFactory::createFeldbusDevice(dev_info, this));
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

    for (std::shared_ptr<FeldbusDeviceWrapper> pDevWrapper: devices_) {
        if (pDevWrapper.get()) {
            deviceList_->addItem(pDevWrapper.get()->devInfo.toString());
        }
    }

    for (int i = fromAddress; i <= toAddress; i++) {
		dynamixelStartInquiry_->setText(QStringLiteral("Abbrechen (%1 %)").arg(i * 100 / (toAddress - fromAddress + 1)));

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

            // create Farbsensor view
            Feldbus::Farbsensor* farbsensor = dynamic_cast<Feldbus::Farbsensor*>(selectedDevice_->device.get());
            if (farbsensor) {
                feldbusWidget = new FeldbusFarbsensorView(farbsensor);
                splitter->addWidget(feldbusWidget);
				splitter->setStretchFactor(1,2);
                return;
            }

            // create Aktor view for DC motor
            Feldbus::DCMotor* dcmotor = dynamic_cast<Feldbus::DCMotor*>(selectedDevice_->device.get());
            if (dcmotor) {
                feldbusWidget = new FeldbusAktorView(dcmotor);
                splitter->addWidget(feldbusWidget);
                splitter->setStretchFactor(1,2);
                return;
            }

            // create Aktor view for servo
            Feldbus::Servo* servo = dynamic_cast<Feldbus::Servo*>(selectedDevice_->device.get());
            if (servo) {
                feldbusWidget = new FeldbusAktorView(servo);
                splitter->addWidget(feldbusWidget);
                splitter->setStretchFactor(1,2);
                return;
            }

            // create ASEB view
            Feldbus::Aseb* aseb = dynamic_cast<Feldbus::Aseb*>(selectedDevice_->device.get());
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
		startBootloader_->setText(QStringLiteral("Bootloader starten"));
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
	startBootloader_->setText(QStringLiteral("Abbrechen"));
	QFont font = startBootloader_->font();
	font.setWeight(QFont::Bold);
	startBootloader_->setFont(font);

	for (QWidget* child : bootloaderStartBootloaderWidgetList) {
		child->setEnabled(false);
	}

	if (broadcastBootloader) {
		delete broadcastBootloader;
	}
    broadcastBootloader = new Feldbus::Bootloader("broadcastBootloader", 0, this,
												  static_cast<Feldbus::Device::ChecksumType>(bootloaderChecksumCombobox_->currentIndex()),
												  bootloaderAddressLength);
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
        deviceAddressLength = Feldbus::Device::AddressLength::byte_;
    } else {
        fromValidator_ = new QIntValidator(1, 32767, this);
        toValidator_ = new QIntValidator(1, 32767, this);
        deviceAddressLength = Feldbus::Device::AddressLength::short_;
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
        bootloaderAddressLength = Feldbus::Device::AddressLength::byte_;
    } else {
        bootloaderFromValidator_ = new QIntValidator(1, 32767, this);
        bootloaderToValidator_ = new QIntValidator(1, 32767, this);
        bootloaderAddressLength = Feldbus::Device::AddressLength::short_;
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
		dynamixelStartInquiry_->setEnabled(true);
		startInquiry_->setText("Geräte suchen");
	} else {
		inquiryTabwidget->setTabEnabled(1, false);
		for (QWidget* child : inquiryWidgetList) {
			child->setEnabled(false);
		}
		dynamixelStartInquiry_->setEnabled(false);
	}
}

void FeldbusFrontend::setBootInquiryWidgetsEnabled(bool enabled) {
	if (enabled) {
		inquiryTabwidget->setTabEnabled(0, true);
		bootloadertoolsStartInquiry_->setText("Geräte suchen");

		for (QWidget* child : bootloaderInquiryWidgetList) {
			child->setEnabled(true);
		}
	} else {
		inquiryTabwidget->setTabEnabled(0, false);

		for (QWidget* child : bootloaderInquiryWidgetList) {
			child->setEnabled(false);
		}
	}
}

void FeldbusFrontend::setDynamixelInquiryWidgetsEnabled(bool enabled) {
	if (enabled) {
		inquiryTabwidget->setTabEnabled(1, true);

		for (QWidget* child : inquiryWidgetList) {
			child->setEnabled(true);
		}
		startInquiry_->setEnabled(true);
		dynamixelStartInquiry_->setText("Dynamixel-Servos suchen");
	} else {
		inquiryTabwidget->setTabEnabled(1, false);
		for (QWidget* child : inquiryWidgetList) {
			child->setEnabled(false);
		}
		startInquiry_->setEnabled(false);
	}
}

bool FeldbusFrontend::transceive(const uint8_t *transmit, int *transmit_length, uint8_t *receive, int *receive_length, bool) {
	if (!connected || !turag_rs485_io_device) {
        return false;
    }

	if (transmit_length && transmit) {
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


					if (GetTickCount() - startTick > feldbusTimeout_ms) {
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
    busDataBuffer.clear();
}

void FeldbusFrontend::setFeldbusTimeout(int milliSeconds) {
	feldbusTimeout_ms = milliSeconds;
}

int FeldbusFrontend::getFeldbusTimeout(void) {
	return feldbusTimeout_ms;
}
