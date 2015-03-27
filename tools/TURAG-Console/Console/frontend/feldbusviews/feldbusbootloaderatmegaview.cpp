#include "feldbusbootloaderatmegaview.h"
#include <QFormLayout>
#include <QLineEdit>


FeldbusBootloaderAtmegaView::FeldbusBootloaderAtmegaView(TURAG::Feldbus::BootloaderAtmega *bootloader, QWidget *parent) :
	FeldbusBootloaderBaseView(bootloader, parent)
{
	QLineEdit* lowFuseEdit = new QLineEdit;
	setDisabledTheme(lowFuseEdit);
	infoLayout->addRow("Low-Fuse:", lowFuseEdit);

	QLineEdit* highFuseEdit = new QLineEdit;
	setDisabledTheme(highFuseEdit);
	infoLayout->addRow("High-Fuse:", highFuseEdit);

	QLineEdit* extFuseEdit = new QLineEdit;
	setDisabledTheme(extFuseEdit);
	infoLayout->addRow("Ext-Fuse:", extFuseEdit);


	if (!bootloader || !bootloader->isUnlocked()) {
		return;
	}

	// read mcu ID
	uint16_t mcuId = bootloader->getMcuId();
	QString mcuName;

	switch (mcuId) {
		case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA8: mcuName = "AtMega8"; break;
		case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA16: mcuName = "AtMega16"; break;
		case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA32: mcuName = "AtMega32"; break;
		case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA128: mcuName = "AtMega128"; break;
		case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA88: mcuName = "AtMega88"; break;
		case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA168: mcuName = "AtMega168"; break;
		case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA644: mcuName = "AtMega644"; break;
		default: mcuName = "Error";
	}
	mcuIdEdit->setText(mcuName);

	// read fuses
	TURAG::Feldbus::BootloaderAtmega::Fuses fuses;
	if (!bootloader->readFuses(&fuses)) {
		lowFuseEdit->setText("Error");
		highFuseEdit->setText("Error");
		extFuseEdit->setText("Error");
	} else {
		lowFuseEdit->setText(QString("0x%1").arg(static_cast<ulong>(fuses.low), 0, 16));
		highFuseEdit->setText(QString("0x%1").arg(static_cast<ulong>(fuses.high), 0, 16));
		extFuseEdit->setText(QString("0x%1").arg(static_cast<ulong>(fuses.extended), 0, 16));
	}
}

