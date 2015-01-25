#ifndef FELDBUSFRONTEND_H
#define FELDBUSFRONTEND_H

#include "basefrontend.h"
#include <tina++/feldbus/host/device.h>
#include <tina++/feldbus/host/bootloader.h>
#include <QList>
#include "util/feldbusdevicefactory.h"
#include <tina++/feldbus/dynamixel/dynamixeldevice.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>
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


class FeldbusFrontend : public BaseFrontend
{
    Q_OBJECT

protected:
    QWidget* feldbusWidget;
    QListWidget* deviceList_;
    QTextEdit* deviceInfo_;
    QList<FeldbusDeviceWrapper> devices_;
    QTabWidget* inquiryTabwidget;

    ComboBoxExt* checksumCombobox_;
    CheckBoxExt* twoByteAddressCheckbox_;
    QPushButton* startInquiry_;
    LineEditExt* fromEdit_;
    LineEditExt* toEdit_;
    QIntValidator* fromValidator_;
    QIntValidator* toValidator_;

    ComboBoxExt* bootloaderChecksumCombobox_;
    CheckBoxExt* bootloaderTwoByteAddressCheckbox_;
    QPushButton* startBootloader_;
    LineEditExt* broadcastTime_;
    QPushButton* bootloadertoolsStartInquiry_;
    LineEditExt* bootFromEdit_;
    LineEditExt* bootToEdit_;
    QIntValidator* bootloaderFromValidator_;
    QIntValidator* bootloaderToValidator_;

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

    Feldbus::Device* selectedDevice_;
    TURAG::Feldbus::Bootloader* broadcastBootloader;

    TURAG::Feldbus::Device::AddressLength deviceAddressLength;
    TURAG::Feldbus::Device::AddressLength bootloaderAddressLength;

protected slots:
    void onInquiry(bool boot);

    void onStartInquiry(void);
    void onDeviceSelected(int row);

    void onStartBootInquiry(void);

    void onStartDynamixelInquiry(void);
    void onRs485DebugMsg(QString msg);

    void onCheckDeviceAvailability(void);

    void onStartBoot(void);
    void onIKnowWhatImDoingBoot(void);
    void requestStartBootBroad(void);

    void onUpdateStatistics(void);
    void onUpdateStatisticsSlave(void);

    void onTwoByteAddressCheckBoxToggled(bool state);
    void onBootloaderTwoByteAddressCheckBoxToggled(bool state);

public:
    FeldbusFrontend(QWidget *parent = 0);
    ~FeldbusFrontend();

public slots:
    virtual void onConnected(bool readOnly, bool isBuffered, QIODevice*);
    virtual void onDisconnected(bool reconnecting);
    virtual void writeData(QByteArray data);
    virtual void clear(void);

private:
    void validateAdressFields();
    void dynamixelValidateAdressFields();
    void disableStatistics(void);
    void enableStatistics(void);
    void resetStatistics(void);

    int  sendBroadcastsBoot;
    int  requiredBroadcastsBoot;

    TURAG::Feldbus::Device* dev;

};

#endif // FELDBUSFRONTEND_H
