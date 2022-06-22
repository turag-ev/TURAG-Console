#include "feldbusbootloaderstm32v2view.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QDebug>


FeldbusBootloaderStm32v2View::FeldbusBootloaderStm32v2View(TURAG::Feldbus::BootloaderStm32v2 *bootloader, FeldbusFrontend *bus_, QWidget *parent) :
    FeldbusBootloaderBaseView(bootloader, bus_, parent), bootloader(bootloader)
{
    QLineEdit* bootloaderInitialStackAddress = new QLineEdit;
    setDisabledTheme(bootloaderInitialStackAddress);
    infoLayout->addRow("Bootloader initial stack address:", bootloaderInitialStackAddress);

    QLineEdit* bootloaderResethandlerAddress = new QLineEdit;
    setDisabledTheme(bootloaderResethandlerAddress);
    infoLayout->addRow("Bootloader reset handler address:", bootloaderResethandlerAddress);

	if (!bootloader || !bootloader->isUnlocked()) {
		return;
	}


    // read stack address
    uint32_t stackAddress = bootloader->getBootloaderInitialStackAddress();
    if (stackAddress == 0xFFFFFFFF) {
        bootloaderInitialStackAddress->setText("Error");
    } else {
        bootloaderInitialStackAddress->setText(QString("0x%1").arg(stackAddress, 2, 16));
    }

    // reset handler address
    uint32_t resetHandler = bootloader->getBootloaderResetHandlerAddress();
    if (resetHandler == 0xFFFFFFFF) {
        bootloaderResethandlerAddress->setText("Error");
    } else {
        bootloaderResethandlerAddress->setText(QString("0x%1").arg(resetHandler, 2, 16));
    }
}


TURAG::Feldbus::BootloaderAvrBase::ErrorCode FeldbusBootloaderStm32v2View::doFlashImage(uint8_t* data, uint32_t length) {
    uint32_t stmFlashBaseAddress = 0x08000000;

    uint32_t bootloaderStackAddress = bootloader->getBootloaderInitialStackAddress();
    uint32_t bootloaderResetHandlerAddress = bootloader->getBootloaderResetHandlerAddress();

    if (bootloaderStackAddress == 0xFFFFFFFF || bootloaderResetHandlerAddress == 0xFFFFFFFF) {
        return TURAG::Feldbus::BootloaderAvrBase::ErrorCode::invalid_args;
    }

    // save original addresses
    uint32_t userAppStackAddress, userAppResetHandlerAddress;
    memcpy(&userAppStackAddress, data, 4);
    memcpy(&userAppResetHandlerAddress, data + 4, 4);

    // send these addresses to the device
    qDebug() << "writing app reset vectors\n";
    auto result = bootloader->writeAppResetVectors(userAppStackAddress, userAppResetHandlerAddress);
    if (result != TURAG::Feldbus::BootloaderAvrBase::ErrorCode::success) {
        return result;
    }

    // replace app reset vectors with the addresses as reported by the bootloader
    memcpy(data, &bootloaderStackAddress, 4);
    memcpy(data + 4, &bootloaderResetHandlerAddress, 4);

    // write modified image to device
    qDebug() << "writing " << length << " bytes to address " << stmFlashBaseAddress << "\n";
    result = bootloader->writeFlash(stmFlashBaseAddress, length, data);
    if (result != TURAG::Feldbus::BootloaderAvrBase::ErrorCode::success) {
        return result;
    }

    return TURAG::Feldbus::BootloaderAvrBase::ErrorCode::success;
}
