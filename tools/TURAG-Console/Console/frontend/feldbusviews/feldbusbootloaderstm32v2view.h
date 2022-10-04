#ifndef FELDBUSBOOTLOADERSTM32V2VIEW_H
#define FELDBUSBOOTLOADERSTM32V2VIEW_H

#include "feldbusbootloaderbaseview.h"


class FeldbusBootloaderStm32v2View : public FeldbusBootloaderBaseView
{
	Q_OBJECT
public:
    const uint32_t stmFlashBaseAddress = 0x08000000;

    explicit FeldbusBootloaderStm32v2View(TURAG::Feldbus::BootloaderStm32v2 *bootloader, FeldbusFrontend* bus_, QWidget *parent = 0);

protected:
    virtual TURAG::Feldbus::BootloaderAvrBase::ErrorCode doFlashImage(uint8_t* data, uint32_t length) override;
    virtual TURAG::Feldbus::BootloaderAvrBase::ErrorCode doVerifyImage(uint8_t* data, uint32_t length) override;

private:
    TURAG::Feldbus::BootloaderStm32v2* bootloader;
};

#endif // FELDBUSBOOTLOADERSTM32V2VIEW_H
