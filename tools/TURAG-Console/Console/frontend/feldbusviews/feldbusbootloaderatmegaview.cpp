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

