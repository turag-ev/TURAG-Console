#include "feldbusbootloaderatmegaview.h"
#include <QFormLayout>
#include <QLineEdit>


FeldbusBootloaderAtmegaView::FeldbusBootloaderAtmegaView(TURAG::Feldbus::BootloaderAtmega *bootloader, FeldbusFrontend *bus_, QWidget *parent) :
    FeldbusBootloaderBaseView(bootloader, bus_, parent)
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
		lowFuseEdit->setText(QString("0x%1\t0b%2").arg(static_cast<ulong>(fuses.low), -2, 16, static_cast<QChar>('0')).arg(static_cast<ulong>(fuses.low), -8, 2, static_cast<QChar>('0')));
		highFuseEdit->setText(QString("0x%1\t0b%2").arg(static_cast<ulong>(fuses.high), -2, 16, static_cast<QChar>('0')).arg(static_cast<ulong>(fuses.high), -8, 2, static_cast<QChar>('0')));
		extFuseEdit->setText(QString("0x%1\t0b%2").arg(static_cast<ulong>(fuses.extended), -2, 16, static_cast<QChar>('0')).arg(static_cast<ulong>(fuses.extended), -8, 2, static_cast<QChar>('0')));
	}
}

