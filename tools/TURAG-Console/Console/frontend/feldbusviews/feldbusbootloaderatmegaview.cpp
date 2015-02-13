#include "feldbusbootloaderatmegaview.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QFileDialog>
#include <QFile>
#include <QByteArray>
#include <QMessageBox>
#include <QCoreApplication>


extern "C" TuragSystemTime turag_rs485_get_timeout(void);
extern "C" void turag_rs485_set_timeout(TuragSystemTime timeout);


FeldbusBootloaderAtmegaView::FeldbusBootloaderAtmegaView(TURAG::Feldbus::BootloaderAtmega *bootloader, QWidget *parent) :
	QWidget(parent), bootloader_(bootloader)
{
	QVBoxLayout* layout = new QVBoxLayout;

	QFormLayout* infoLayout = new QFormLayout;
	QLineEdit* mcuIdEdit = new QLineEdit;
	setDisabledTheme(mcuIdEdit);
	infoLayout->addRow("Controller-Typ:", mcuIdEdit);
	QLineEdit* flashSizeEdit = new QLineEdit;
	setDisabledTheme(flashSizeEdit);
	infoLayout->addRow("Flash-Größe:", flashSizeEdit);
	QLineEdit* writableFlashSizeEdit = new QLineEdit;
	setDisabledTheme(writableFlashSizeEdit);
	infoLayout->addRow("beschreibbar:", writableFlashSizeEdit);
	QLineEdit* pageSizeEdit = new QLineEdit;
	setDisabledTheme(pageSizeEdit);
	infoLayout->addRow("Page-Größe:", pageSizeEdit);
	layout->addLayout(infoLayout);

	QHBoxLayout* fuseLayout = new QHBoxLayout;
	fuseLayout->addWidget(new QLabel("Low-Fuse:"));
	QLineEdit* lowFuseEdit = new QLineEdit;
	setDisabledTheme(lowFuseEdit);
	fuseLayout->addWidget(lowFuseEdit);
	fuseLayout->addWidget(new QLabel("High-Fuse:"));
	QLineEdit* highFuseEdit = new QLineEdit;
	setDisabledTheme(highFuseEdit);
	fuseLayout->addWidget(highFuseEdit);
	fuseLayout->addWidget(new QLabel("Ext-Fuse:"));
	QLineEdit* extFuseEdit = new QLineEdit;
	setDisabledTheme(extFuseEdit);
	fuseLayout->addWidget(extFuseEdit);
	layout->addLayout(fuseLayout);

	layout->addSpacing(30);

//	QLabel* writeFlashLabel = new QLabel("Intel Hexfile in Flash schreiben");
//	QFont label_font = writeFlashLabel->font();
//	label_font.setPointSize(14);
//	label_font.setWeight(QFont::Bold);
//	writeFlashLabel->setFont(label_font);
//	layout->addWidget(writeFlashLabel);
//	QHBoxLayout* hexLayout = new QHBoxLayout;
//	hexLayout->addWidget(new QLabel("Datei:"));
//	hexFilePathEdit = new QLineEdit;
//	connect(hexFilePathEdit, SIGNAL(textChanged(QString)), this, SLOT(checkHexFile(QString)));
//	hexLayout->addWidget(hexFilePathEdit);
//	openFileDialogButton = new QPushButton("...");
//	connect(openFileDialogButton, SIGNAL(clicked()), this, SLOT(openHexFile()));
//	hexLayout->addWidget(openFileDialogButton);
//	layout->addLayout(hexLayout);
//	layout->addSpacing(30);

	QLabel* writeImageLabel = new QLabel("Binäres Image in Flash schreiben");
	QFont label_font = writeImageLabel->font();
	label_font.setPointSize(14);
	label_font.setWeight(QFont::Bold);
	writeImageLabel->setFont(label_font);
	layout->addWidget(writeImageLabel);
	QHBoxLayout* imageLayout = new QHBoxLayout;
	imageLayout->addWidget(new QLabel("Datei:"));
	imagePathEdit = new QLineEdit;
	connect(imagePathEdit, SIGNAL(textChanged(QString)), this, SLOT(checkImageFile(QString)));
	imageLayout->addWidget(imagePathEdit);
	loadImageFileButton = new QPushButton("...");
	connect(loadImageFileButton, SIGNAL(clicked()), this, SLOT(openImageFile()));
	imageLayout->addWidget(loadImageFileButton);
	layout->addLayout(imageLayout);
	QHBoxLayout* imageLayout2 = new QHBoxLayout;
	imageLayout2->addWidget(new QLabel("Größe:"));
	imageFileSizeEdit = new QLineEdit;
	setDisabledTheme(imageFileSizeEdit);
	imageFileSizeEdit->setMinimumWidth(500);
	imageLayout2->addWidget(imageFileSizeEdit);
	flashImageFileButton = new QPushButton("Flashen");
	flashImageFileButton->setDisabled(true);
	connect(flashImageFileButton, SIGNAL(clicked()), this, SLOT(flashImageFile()));
	imageLayout2->addWidget(flashImageFileButton);
	layout->addLayout(imageLayout2);
	layout->addSpacing(30);

	QLabel* readImageLabel = new QLabel("Binäres Image aus dem Flash lesen");
	label_font = readImageLabel->font();
	label_font.setPointSize(14);
	label_font.setWeight(QFont::Bold);
	readImageLabel->setFont(label_font);
	layout->addWidget(readImageLabel);
	QHBoxLayout* readImageLayout = new QHBoxLayout;
	readImageLayout->addWidget(new QLabel("Datei:"));
	imageReadPathEdit = new QLineEdit;
	connect(imageReadPathEdit, SIGNAL(textChanged(QString)), this, SLOT(checkReadImageFile(QString)));
	readImageLayout->addWidget(imageReadPathEdit);
	selectImageReadFileButton = new QPushButton("...");
	connect(selectImageReadFileButton, SIGNAL(clicked()), this, SLOT(openReadImageFile()));
	readImageLayout->addWidget(selectImageReadFileButton);
	layout->addLayout(readImageLayout);
	QHBoxLayout* readImageLayout2 = new QHBoxLayout;
	readImageLayout2->addStretch();
	readImageFileButton = new QPushButton("Image auslesen");
	readImageFileButton->setEnabled(false);
	connect(readImageFileButton, SIGNAL(clicked()), this, SLOT(readImageFile()));
	readImageLayout2->addWidget(readImageFileButton);
	layout->addLayout(readImageLayout2);
	layout->addSpacing(30);

	QLabel* startProgramLabel = new QLabel("Bootloader verlassen");
	label_font = startProgramLabel->font();
	label_font.setPointSize(14);
	label_font.setWeight(QFont::Bold);
	startProgramLabel->setFont(label_font);
	layout->addWidget(startProgramLabel);
	QHBoxLayout* startProgramLayout = new QHBoxLayout;
	QPushButton* startProgramButton = new QPushButton("Bootloader jetzt in allen Geräten verlassen");
	connect(startProgramButton, SIGNAL(clicked()), this, SLOT(leaveBootloader()));
	startProgramLayout->addWidget(startProgramButton);
	startProgramLayout->addStretch();
	layout->addLayout(startProgramLayout);

	layout->addStretch();

	QHBoxLayout* wrapLayout = new QHBoxLayout;
	wrapLayout->addLayout(layout);
	wrapLayout->addStretch();
	setLayout(wrapLayout);

	if (!bootloader_) {
		return;
	}

	// read mcu ID
	uint16_t mcuId = bootloader_->getMcuId();
	QString mcuName;

	switch (mcuId) {
		case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA8: mcuName = "AtMega8"; break;
		case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA32: mcuName = "AtMega32"; break;
		case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA128: mcuName = "AtMega128"; break;
		case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA88: mcuName = "AtMega88"; break;
		case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA168: mcuName = "AtMega168"; break;
		case TURAG_FELDBUS_BOOTLOADER_MCU_ID_ATMEGA644: mcuName = "AtMega644"; break;
		default: mcuName = "Error";
	}
	mcuIdEdit->setText(mcuName);

	// unlock bootloader
	if (!bootloader_->unlockBootloader()) {
		return;
	}

	// read flash size
	uint32_t flashSize = bootloader_->getFlashSize(false);
	if (flashSize == 0) {
		flashSizeEdit->setText("Error");
	} else {
		flashSizeEdit->setText(QString("%1").arg(flashSize));
	}

	// read writable flash size
	flashSize = bootloader_->getFlashSize(true);
	if (flashSize == 0) {
		writableFlashSizeEdit->setText("Error");
	} else {
		writableFlashSizeEdit->setText(QString("%1").arg(flashSize));
	}

	// read page size
	uint16_t pageSize = bootloader_->getPageSize();
	if (pageSize == 0) {
		pageSizeEdit->setText("Error");
	} else {
		pageSizeEdit->setText(QString("%1").arg(pageSize));
	}

	// read fuses
	TURAG::Feldbus::BootloaderAtmega::Fuses fuses;
	if (!bootloader_->readFuses(&fuses)) {
		lowFuseEdit->setText("Error");
		highFuseEdit->setText("Error");
		extFuseEdit->setText("Error");
	} else {
		lowFuseEdit->setText(QString("0x%1").arg(static_cast<ulong>(fuses.low), 0, 16));
		highFuseEdit->setText(QString("0x%1").arg(static_cast<ulong>(fuses.high), 0, 16));
		extFuseEdit->setText(QString("0x%1").arg(static_cast<ulong>(fuses.extended), 0, 16));
	}
}

void FeldbusBootloaderAtmegaView::setDisabledTheme(QLineEdit* lineedit) {
	QPalette pal = lineedit->palette();
	QColor clr = this->palette().color(QPalette::Window);
	pal.setColor(QPalette::Active, QPalette::Base, clr);
	pal.setColor(QPalette::Inactive, QPalette::Base, clr);
	lineedit->setPalette(pal);
	lineedit->setReadOnly(true);
}

void FeldbusBootloaderAtmegaView::leaveBootloader(void) {
	if (bootloader_->sendStartProgramBroadcast()) {
		this->setEnabled(false);
	}
}

// void FeldbusBootloaderAtmegaView::openHexFile(void) {
	// QFileDialog dialog(this, "Firmwaredatei auswählen", "");
	// dialog.setFileMode(QFileDialog::ExistingFile);
	// dialog.setNameFilter(tr("Intel Hex-file (*.hex)"));
	// if (dialog.exec() == QDialog::Accepted) {
		// QStringList files = dialog.selectedFiles();
		// hexFilePathEdit->setText(QString("%1").arg(files[0]));
	// }
// }

// void FeldbusBootloaderAtmegaView::checkHexFile(QString path) {

// }

// void FeldbusBootloaderAtmegaView::flashHexFile(void) {

// }

void FeldbusBootloaderAtmegaView::openImageFile(void) {
	QFileDialog dialog(this, "Firmwaredatei auswählen", "");
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setNameFilter(tr("Intel Hex-file (*.bin)"));
	if (dialog.exec() == QDialog::Accepted) {
		QStringList files = dialog.selectedFiles();
		imagePathEdit->setText(QString("%1").arg(files[0]));
	}
}

void FeldbusBootloaderAtmegaView::checkImageFile(QString path) {
	QFile image(path);
	if (image.exists()) {
		imageFileSizeEdit->setText(QString("%1 KB (%2 Byte) %3 %").arg(image.size() / 1024).arg(image.size()).arg(100*image.size()/bootloader_->getFlashSize(true)));
		flashImageFileButton->setEnabled(true);
	} else {
		imageFileSizeEdit->setText("");
		flashImageFileButton->setEnabled(false);
	}
}

void FeldbusBootloaderAtmegaView::flashImageFile(void) {
	QFile image(imagePathEdit->text());
	if (!image.exists()) {
		return;
	}

	if (!image.open(QIODevice::ReadOnly)) {
		QMessageBox msg(QMessageBox::Warning, "Fehler", "Datei konnte nicht geöffnet werden.", QMessageBox::Ok);
		msg.exec();
		return;
	}

	this->setEnabled(false);
	QCoreApplication::processEvents();

	// the bootloader might take a little longer to reply
	TuragSystemTime originalTimeout = turag_rs485_get_timeout();
	turag_rs485_set_timeout(turag_ms_to_ticks(100));

	QByteArray data(image.readAll());
	TURAG::Feldbus::BootloaderAtmega::ErrorCode result = bootloader_->writeFlash(0, data.size(), (uint8_t*)data.constData());

	if (result == TURAG::Feldbus::BootloaderAtmega::ErrorCode::success) {
		QMessageBox msg(QMessageBox::Information, "Erfolgreich", "Image erfolgreich zum Gerät übertragen", QMessageBox::Ok);
		msg.exec();
	} else {
		QMessageBox msg(QMessageBox::Critical, bootloader_->errorName(result), bootloader_->errorDescription(result), QMessageBox::Ok);
		msg.exec();
	}

	turag_rs485_set_timeout(originalTimeout);

	image.close();

	this->setEnabled(true);
}


void FeldbusBootloaderAtmegaView::openReadImageFile(void) {
	QFileDialog dialog(this, "Firmwaredatei speichern als", "");
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setNameFilter(tr("Intel Hex-file (*.bin)"));
	dialog.setDefaultSuffix("bin");
	if (dialog.exec() == QDialog::Accepted) {
		QStringList files = dialog.selectedFiles();
		imageReadPathEdit->setText(QString("%1").arg(files[0]));
	}
}

void FeldbusBootloaderAtmegaView::checkReadImageFile(QString path) {
	if (!path.isEmpty()) {
		readImageFileButton->setEnabled(true);
	} else {
		readImageFileButton->setEnabled(false);
	}
}

void FeldbusBootloaderAtmegaView::readImageFile(void) {
	if (imageReadPathEdit->text().isEmpty()) {
		return;
	}

	this->setEnabled(false);
	QCoreApplication::processEvents();

	// the bootloader might take a little longer to reply
	TuragSystemTime originalTimeout = turag_rs485_get_timeout();
	turag_rs485_set_timeout(turag_ms_to_ticks(100));

	uint32_t sizeToRead = bootloader_->getFlashSize(true);

	uint8_t* buffer = new uint8_t[sizeToRead];
	TURAG::Feldbus::BootloaderAtmega::ErrorCode errorCode = bootloader_->readFlash(0, sizeToRead, buffer);

	if (errorCode != TURAG::Feldbus::BootloaderAtmega::ErrorCode::success) {
		QMessageBox msg(QMessageBox::Critical, bootloader_->errorName(errorCode), bootloader_->errorDescription(errorCode), QMessageBox::Ok);
		msg.exec();
		turag_rs485_set_timeout(originalTimeout);
		this->setEnabled(true);
		delete[] buffer;
		return;
	}

	QFile outfile(imageReadPathEdit->text());

	bool success = outfile.open(QIODevice::WriteOnly);
	if (success) {
		success = outfile.write((const char*)buffer, sizeToRead);
		outfile.close();
	}

	if (success) {
		QMessageBox msg(QMessageBox::Information, "Erfolgreich", "Image erfolgreich vom Gerät gelesen", QMessageBox::Ok);
		msg.exec();
	} else {
		QMessageBox msg(QMessageBox::Critical, "Fehler", "Image konnte nicht in Datei geschrieben werden.", QMessageBox::Ok);
		msg.exec();
	}

	turag_rs485_set_timeout(originalTimeout);
	this->setEnabled(true);
	delete[] buffer;
}

