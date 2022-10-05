#ifndef TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSFRONTEND_H
#define TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSFRONTEND_H

#include "basefrontend.h"
#include "util/feldbusdevicefactory.h"

#include <tina++/feldbus/host/feldbusabstraction.h>
#include <tina++/feldbus/host/device.h>
#include <tina++/feldbus/host/bootloader.h>
#include <tina++/feldbus/dynamixel/dynamixeldevice.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>

#include <QList>
#include <QTimer>
#include <QCheckBox>

#include <tuple>
#include <functional>



class QPushButton;
class QListWidget;
class QTextEdit;
class QLineEdit;
class QIntValidator;
class QWidget;
class QSplitter;
class PlainTextFrontend;
class QLabel;
class CheckBoxExt;
class ComboBoxExt;
class LineEditExt;
class QGroupBox;
class QTabWidget;

namespace TURAG::Feldbus {
    class DeviceLocator;
}


using namespace TURAG; // FIXME: evil!


class FeldbusFrontend : public BaseFrontend, public Feldbus::FeldbusAbstraction
{
    Q_OBJECT

public:
	FeldbusFrontend(QWidget *parent = 0);
	virtual ~FeldbusFrontend();

    virtual bool doTransceive(const uint8_t *transmit, int *transmit_length, uint8_t *receive, int *receive_length, bool delayTransmission);
    virtual void clearBuffer(void);
	void setFeldbusTimeout(int milliSeconds);
	int getFeldbusTimeout(void);


public slots:
	virtual void onConnected(bool readOnly, QIODevice*);
	virtual void onDisconnected(bool reconnecting);
	virtual void writeData(QByteArray data);
	virtual void clear(void);

protected slots:
	void onInquiry(bool boot);

	void onStartInquiry(void);
	void onDeviceSelected(int row);

    void onReenumerateDevicesSequential(void);
    void onReenumerateDevicesBinary(void);
    void onReenumerateDevicesBoth(void);
    void onBootloaderReenumerateDevicesSequential(void);
    void onBootloaderReenumerateDevicesBinary(void);
    void onBootloaderReenumerateDevicesBoth(void);
    void onStopEnumerate(void);

	void onStartBootInquiry(void);

	void onStartDynamixelInquiry(void);
	void onRs485DebugMsg(QString msg);

	void onCheckDeviceAvailability(void);

	void onStartBoot(void);
	void requestStartBootBroad(void);

	void onUpdateStatistics(void);
	void onUpdateStatisticsSlave(void);

    void onTwoByteAddressCheckBoxToggled(bool state);
    void onBootloaderTwoByteAddressCheckBoxToggled(bool state);

protected:
    QWidget* feldbusWidget;
    QListWidget* deviceList_;
    QTextEdit* deviceInfo_;
    QList<std::shared_ptr<FeldbusDeviceWrapper>> devices_;
    QTabWidget* inquiryTabwidget;

    //ComboBoxExt* checksumCombobox_;
    QPushButton* startInquiry_;
    LineEditExt* fromEdit_;
    LineEditExt* toEdit_;
    QIntValidator* fromValidator_;
    QIntValidator* toValidator_;
	QList<QWidget*> inquiryWidgetList;
    QPushButton* enumerateSequentialButton;
    QPushButton* enumerateBinaryButton;
    QPushButton* enumerateBothButton;
    QPushButton* stopEnumerateButton;

    //ComboBoxExt* bootloaderChecksumCombobox_;
    QPushButton* startBootloader_;
    QPushButton* bootloadertoolsStartInquiry_;
    LineEditExt* bootFromEdit_;
    LineEditExt* bootToEdit_;
    QIntValidator* bootloaderFromValidator_;
    QIntValidator* bootloaderToValidator_;
	QList<QWidget*> bootloaderInquiryWidgetList;
	QList<QWidget*> bootloaderStartBootloaderWidgetList;
    QPushButton* bootloaderEnumerateSequentialButton;
    QPushButton* bootloaderEnumerateBinaryButton;
    QPushButton* bootloaderEnumerateBothButton;
    QPushButton* bootloaderStopEnumerateButton;

    QPushButton* dynamixelStartInquiry_;
    LineEditExt* dynamixelFromEdit_;
    LineEditExt* dynamixelToEdit_;
    QIntValidator* dynamixelFromValidator_;
    QIntValidator* dynamixelToValidator_;
    QList<DynamixelDeviceWrapper> dynamixelDevices_;

    QGroupBox* masterGroupBox_;
    QLabel* masterNoErrorPackages_;
    QLabel* masterNoAnswer_;
    QLabel* masterMissingData_;
    QLabel* masterChecksumError_;
    QLabel* masterSendError_;
    QGroupBox* slaveGroupBox_;
    QLabel* slaveOverflow_;
    QLabel* slaveChecksumError_;
    QLabel* slaveLostPackages_;
    QLabel* slaveAcceptedPackages_;
    QLabel* slaveUptime_;
    QPushButton* updateStatisticsButton_;
    CheckBoxExt* updateStatisticsAuto_;

    QSplitter* splitter;
    PlainTextFrontend* busLog_;

    QTimer availabilityChecker_;
    QTimer sendBroadcastTimer_;
    QTimer updateStatisticsTimer_;

    FeldbusDeviceWrapper* selectedDevice_;
    TURAG::Feldbus::Bootloader* broadcastBootloader;

private:
	void setInquiryWidgetsEnabled(bool enabled);
	void setBootInquiryWidgetsEnabled(bool enabled);
    void setDynamixelInquiryWidgetsEnabled(bool enabled);
    void setEnumerateWidgetsEnabled(bool enabled, bool boot);

    void validateAdressFields();
    void dynamixelValidateAdressFields();
    void disableStatistics(void);
    void enableStatistics(void);
    void resetStatistics(void);
    void enumerate(bool useSequentialSearch, bool useBinarySearch, bool boot);

    std::tuple<QList<uint32_t>, bool>
    enumerateBusNodes(Feldbus::DeviceLocator& locator, bool useSequentialSearch, bool useBinarySearch, bool* keepRunning,
                      std::function<void(int)> onDeviceCountChanged = {});

	bool inquiryRunning;
    bool enumerationRunning;
	bool bootloaderActivationRunning;
    bool connected;

	QIODevice* turag_rs485_io_device;
	QByteArray busDataBuffer;
	int feldbusTimeout_ms;
};

#endif // TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSFRONTEND_H
