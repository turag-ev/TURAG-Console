#include "feldbusbootloaderxmegaview.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QDebug>


FeldbusBootloaderXmegaView::FeldbusBootloaderXmegaView(TURAG::Feldbus::BootloaderXmega *bootloader, QWidget *parent) :
	FeldbusBootloaderBaseView(bootloader, parent)
{
	QLineEdit* revisionId = new QLineEdit;
	setDisabledTheme(revisionId);
	infoLayout->addRow("Revision-ID:", revisionId);


	if (!bootloader || !bootloader->isUnlocked()) {
		return;
	}

	// read revision
	char revision = bootloader->getRevisionId();
	if (!revision) {
		revisionId->setText("Error");
	} else {
		revisionId->setText(QString("%1").arg(revision));
	}

	// read fuses
	TURAG::Feldbus::BootloaderXmega::Fuses fuses;
	if (!bootloader->readFuses(&fuses)) {
		qWarning() << "Couldn't read fuses";
	} else {
		if (fuses.fuse0.available) {
			QLineEdit* fuse0 = new QLineEdit;
			setDisabledTheme(fuse0);
			fuse0->setText(QString("0x%1").arg(static_cast<ulong>(fuses.fuse0.value), 0, 16));
			infoLayout->addRow("Fuse 0:", fuse0);
		}
		if (fuses.fuse1.available) {
			QLineEdit* fuse1 = new QLineEdit;
			setDisabledTheme(fuse1);
			fuse1->setText(QString("0x%1").arg(static_cast<ulong>(fuses.fuse1.value), 0, 16));
			infoLayout->addRow("Fuse 1:", fuse1);
		}
		if (fuses.fuse2.available) {
			QLineEdit* fuse2 = new QLineEdit;
			setDisabledTheme(fuse2);
			fuse2->setText(QString("0x%1").arg(static_cast<ulong>(fuses.fuse2.value), 0, 16));
			infoLayout->addRow("Fuse 2:", fuse2);
		}
		if (fuses.fuse3.available) {
			QLineEdit* fuse3 = new QLineEdit;
			setDisabledTheme(fuse3);
			fuse3->setText(QString("0x%1").arg(static_cast<ulong>(fuses.fuse3.value), 0, 16));
			infoLayout->addRow("Fuse 3:", fuse3);
		}
		if (fuses.fuse4.available) {
			QLineEdit* fuse4 = new QLineEdit;
			setDisabledTheme(fuse4);
			fuse4->setText(QString("0x%1").arg(static_cast<ulong>(fuses.fuse4.value), 0, 16));
			infoLayout->addRow("Fuse 4:", fuse4);
		}
		if (fuses.fuse5.available) {
			QLineEdit* fuse5 = new QLineEdit;
			setDisabledTheme(fuse5);
			fuse5->setText(QString("0x%1").arg(static_cast<ulong>(fuses.fuse5.value), 0, 16));
			infoLayout->addRow("Fuse 5:", fuse5);
		}
		if (fuses.fuse6.available) {
			QLineEdit* fuse6 = new QLineEdit;
			setDisabledTheme(fuse6);
			fuse6->setText(QString("0x%1").arg(static_cast<ulong>(fuses.fuse6.value), 0, 16));
			infoLayout->addRow("Fuse 6:", fuse6);
		}
	}
}

