#ifndef FELDBUSFRONTEND_H
#define FELDBUSFRONTEND_H

#include "basefrontend.h"
#include "util/feldbusdevicefactory.h"

#include <tina++/feldbus/host/device.h>
#include <tina++/feldbus/host/bootloader.h>
#include <tina++/feldbus/dynamixel/dynamixeldevice.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>

#include <QList>
#include <QTimer>
#include <QCheckBox>



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


using namespace TURAG;


class FeldbusFrontend : public BaseFrontend, public Feldbus::FeldbusAbstraction
{
    Q_OBJECT

public:
	FeldbusFrontend(QWidget *parent = 0);
	~FeldbusFrontend();

    virtual bool transceive(uint8_t *transmit, int *transmit_length, uint8_t *receive, int *receive_length, bool delayTransmission);
    virtual void clearBuffer(void);
    void setFeldbusTimeout(unsigned milliSeconds);
    unsigned getFeldbusTimeout(void);


public slots:
	virtual void onConnected(bool readOnly, QIODevice*);
	virtual void onDisconnected(bool reconnecting);
	virtual void writeData(QByteArray data);
	virtual void clear(void);

protected slots:
	void onInquiry(bool boot);

	void onStartInquiry(void);
	void onDeviceSelected(int row);

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

    ComboBoxExt* checksumCombobox_;
    CheckBoxExt* twoByteAddressCheckbox_;
    QPushButton* startInquiry_;
    LineEditExt* fromEdit_;
    LineEditExt* toEdit_;
    QIntValidator* fromValidator_;
    QIntValidator* toValidator_;
	QList<QWidget*> inquiryWidgetList;

    ComboBoxExt* bootloaderChecksumCombobox_;
    CheckBoxExt* bootloaderTwoByteAddressCheckbox_;
    QPushButton* startBootloader_;
    QPushButton* bootloadertoolsStartInquiry_;
    LineEditExt* bootFromEdit_;
    LineEditExt* bootToEdit_;
    QIntValidator* bootloaderFromValidator_;
    QIntValidator* bootloaderToValidator_;
	QList<QWidget*> bootloaderInquiryWidgetList;
	QList<QWidget*> bootloaderStartBootloaderWidgetList;

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

    TURAG::Feldbus::Device::AddressLength deviceAddressLength;
    TURAG::Feldbus::Device::AddressLength bootloaderAddressLength;

private:
	void setInquiryWidgetsEnabled(bool enabled);
	void setBootInquiryWidgetsEnabled(bool enabled);
	void setDynamixelInquiryWidgetsEnabled(bool enabled);

    void validateAdressFields();
    void dynamixelValidateAdressFields();
    void disableStatistics(void);
    void enableStatistics(void);
    void resetStatistics(void);

	bool inquiryRunning;
	bool bootloaderActivationRunning;
    bool connected;

    TURAG::Feldbus::Device* dev;
    QByteArray busDataBuffer;
    QTimer feldbusReceiveTimer;
    unsigned feldbusTimeout;
};

#endif // FELDBUSFRONTEND_H
