#include "feldbusbootloaderstm32v2view.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QDebug>


FeldbusBootloaderStm32v2View::FeldbusBootloaderStm32v2View(TURAG::Feldbus::BootloaderStm32v2 *stm32Bootloader, FeldbusFrontend *bus_, QWidget *parent) :
    FeldbusBootloaderBaseView(stm32Bootloader, bus_, parent), bootloader(stm32Bootloader)
{
    QLineEdit* resetHandlerStorageAddress = new QLineEdit;
    setDisabledTheme(resetHandlerStorageAddress);
    infoLayout->addRow("Bootloader reset handler address:", resetHandlerStorageAddress);

	if (!bootloader || !bootloader->isUnlocked()) {
		return;
	}

    // read reset handler storage address
    uint32_t storageAddress = bootloader->getResetVectorStorageAddress();
    if (storageAddress == 0xFFFFFFFF) {
        resetHandlerStorageAddress->setText("Error");
    } else {
        resetHandlerStorageAddress->setText(QString("0x%1").arg(storageAddress, 2, 16));
    }
}


TURAG::Feldbus::BootloaderAvrBase::ErrorCode FeldbusBootloaderStm32v2View::doFlashImage(uint8_t* imageData, uint32_t offset, uint32_t length) {
    uint32_t resetVectorStorageAddress = bootloader->getResetVectorStorageAddress();

    if (resetVectorStorageAddress == 0xFFFFFFFF) {
        return TURAG::Feldbus::BootloaderAvrBase::ErrorCode::invalid_args;
    }

    // save original addresses
    uint32_t userAppStackAddress, userAppResetHandlerAddress;
    memcpy(&userAppStackAddress, imageData, 4);
    memcpy(&userAppResetHandlerAddress, imageData + 4, 4);

    // send these addresses to the device
    qDebug() << "transmitting app reset vectors\n";
    auto result = bootloader->transmitAppResetVectors(userAppStackAddress, userAppResetHandlerAddress);
    if (result != TURAG::Feldbus::BootloaderAvrBase::ErrorCode::success) {
        return result;
    }

    // write image to device
    qDebug() << "writing " << length << " bytes to address " << offset << "\n";
    result = bootloader->writeFlash(offset, length, imageData);
    if (result != TURAG::Feldbus::BootloaderAvrBase::ErrorCode::success) {
        return result;
    }

    // commit user app addresses
    qDebug() << "committing app reset vectors\n";
    result = bootloader->commitAppResetVectors(userAppStackAddress, userAppResetHandlerAddress);
    if (result != TURAG::Feldbus::BootloaderAvrBase::ErrorCode::success) {
        return result;
    }

    return TURAG::Feldbus::BootloaderAvrBase::ErrorCode::success;
}

TURAG::Feldbus::BootloaderAvrBase::ErrorCode FeldbusBootloaderStm32v2View::doVerifyImage(uint8_t* imageData, uint32_t offset, uint32_t length) {
    // verify
    qDebug() << "verifying flash content\n";
    uint8_t readData[length];

    auto result = bootloader->readFlash(offset, length, readData);
    if (result != TURAG::Feldbus::BootloaderAvrBase::ErrorCode::success) {
        return result;
    }

    // replace the first 8 byte because they are changed by the bootloader
    memcpy(readData, imageData, 8);

    // if the reset vector is stored someplace within the image we need to replace this data
    // with the original, otherwise the verification would fail
    uint32_t maskOffset = bootloader->getResetVectorStorageAddress() & ~bootloader->getFlashBaseAddress();
    if (maskOffset + 8 <= length) {
        memcpy(readData + maskOffset, imageData + maskOffset, 8);
    }

    for (uint32_t i = 0; i < length; ++i) {
        if (imageData[i] != readData[i]) {
            return TURAG::Feldbus::BootloaderAvrBase::ErrorCode::content_mismatch;
        }
    }

    return TURAG::Feldbus::BootloaderAvrBase::ErrorCode::success;
}

