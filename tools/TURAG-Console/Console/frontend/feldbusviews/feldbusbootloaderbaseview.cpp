#include "feldbusbootloaderbaseview.h"
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

#include <libcintelhex/cintelhex.h>


extern "C" TuragSystemTime turag_rs485_get_timeout(void);
extern "C" void turag_rs485_set_timeout(TuragSystemTime timeout);


FeldbusBootloaderBaseView::FeldbusBootloaderBaseView(TURAG::Feldbus::BootloaderAvrBase *bootloader, QWidget *parent) :
	QWidget(parent), bootloader_(bootloader)
{
	QVBoxLayout* layout = new QVBoxLayout;

	infoLayout = new QFormLayout;
	mcuIdEdit = new QLineEdit;
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

	layout->addSpacing(30);

	QLabel* writeImageLabel = new QLabel("Binäres Image/Intel Hexfile in Flash schreiben");
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

    setDisabled(true);

	if (!bootloader_) {
		mcuIdEdit->setText("ERROR: no device pointer given");
		return;
	}

	// unlock bootloader
	if (!bootloader_->unlockBootloader()) {
		mcuIdEdit->setText("ERROR: unable to unlock bootloader");
		return;
	}

	// read flash size
	uint32_t flashSize = bootloader_->getFlashSize(false);
	if (flashSize == 0) {
		flashSizeEdit->setText("Error");
		return;
	} else {
		flashSizeEdit->setText(QString("%1").arg(flashSize));
	}

	// read writable flash size
	flashSize = bootloader_->getFlashSize(true);
	if (flashSize == 0) {
		writableFlashSizeEdit->setText("Error");
		return;
	} else {
		writableFlashSizeEdit->setText(QString("%1").arg(flashSize));
	}

	// read page size
	uint16_t pageSize = bootloader_->getPageSize();
	if (pageSize == 0) {
		pageSizeEdit->setText("Error");
		return;
	} else {
		pageSizeEdit->setText(QString("%1").arg(pageSize));
	}

	setDisabled(false);
}


void FeldbusBootloaderBaseView::setDisabledTheme(QLineEdit* lineedit) {
	QPalette pal = lineedit->palette();
	QColor clr = this->palette().color(QPalette::Window);
	pal.setColor(QPalette::Active, QPalette::Base, clr);
	pal.setColor(QPalette::Inactive, QPalette::Base, clr);
	lineedit->setPalette(pal);
	lineedit->setReadOnly(true);
}

void FeldbusBootloaderBaseView::leaveBootloader(void) {
	if (bootloader_->sendStartProgramBroadcast()) {
		this->setEnabled(false);
	}
}

void FeldbusBootloaderBaseView::openImageFile(void) {
	QFileDialog dialog(this, "Firmwaredatei auswählen", "");
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setNameFilter(tr("Firmware image files (*.bin *.hex)"));
	if (dialog.exec() == QDialog::Accepted) {
		QStringList files = dialog.selectedFiles();
		imagePathEdit->setText(QString("%1").arg(files[0]));
	}
}

void FeldbusBootloaderBaseView::checkImageFile(QString path) {
	flashImageFileButton->setEnabled(false);
	imageFileSizeEdit->setText("");

	QFile image(path);
	if (image.exists()) {
		if (path.endsWith(".bin")) {
			QString imageFileSizeEditString = QString("%1 KB (%2 Byte)")
					.arg((image.size() + 512) / 1024)
					.arg(image.size());
			if (bootloader_->getFlashSize(true) != 0) {
				imageFileSizeEditString = QString(imageFileSizeEditString + " %3 %").arg(100 * image.size() / bootloader_->getFlashSize(true));
			}
			imageFileSizeEdit->setText(imageFileSizeEditString);
			flashImageFileButton->setEnabled(true);
		} else if (path.endsWith(".hex")) {
			ihex_recordset_t* hexRecordset = ihex_rs_from_file(path.toLatin1().constData());
			if (!hexRecordset) {
				imageFileSizeEdit->setText("Error parsing hex file");
				return;
			}
			uint32_t max, min;
			if (ihex_rs_get_address_range(hexRecordset, &min, &max) != 0) {
				imageFileSizeEdit->setText("Error parsing hex file");
				return;
			}

			int size = max - min;

			QString imageFileSizeEditString = QString("%1 KB (%2 Byte)")
					.arg((size + 512) / 1024)
					.arg(size);
			if (bootloader_->getFlashSize(true) != 0) {
				imageFileSizeEditString = QString(imageFileSizeEditString + " %3 %").arg(100 * size / bootloader_->getFlashSize(true));
			}
			if (min > 0) {
				imageFileSizeEditString = QString(imageFileSizeEditString + " (Adresse %1 - %2)")
						.arg(min)
						.arg(max - 1);
			}
			imageFileSizeEdit->setText(imageFileSizeEditString);
			flashImageFileButton->setEnabled(true);
		} else {
			imageFileSizeEdit->setText("only *.hex and *.bin files are supported");
			flashImageFileButton->setEnabled(false);
		}
	}
}

void FeldbusBootloaderBaseView::flashImageFile(void) {
	QFile image(imagePathEdit->text());
	if (!image.exists()) {
		return;
	}

	if (!image.open(QIODevice::ReadOnly)) {
		QMessageBox msg(QMessageBox::Warning, "Fehler", "Datei konnte nicht geöffnet werden.", QMessageBox::Ok);
		msg.exec();
		return;
	}

	QByteArray data(image.readAll());
	image.close();
	uint8_t* rawData;
	unsigned dataLength = 0;

	if (imagePathEdit->text().endsWith(".hex")) {
		ihex_recordset_t* hexRecordset = ihex_rs_from_mem(data.constData(), data.size());
		if (!hexRecordset) {
			QMessageBox msg(QMessageBox::Warning, "Fehler", "Hex-Datei konnte nicht verarbeitet werden", QMessageBox::Ok);
			msg.exec();
			return;
		}
		uint32_t max, min;
		if (ihex_rs_get_address_range(hexRecordset, &min, &max) != 0) {
			QMessageBox msg(QMessageBox::Warning, "Fehler", "Hex-Datei konnte nicht verarbeitet werden", QMessageBox::Ok);
			msg.exec();
			return;
		}

		// TODO: increase efficiency for images that do not start at address zero by only transmitting
		// the data from where it actually starts.
		int size = max;
		uint8_t imageBuffer[size];

		if (ihex_mem_copy(hexRecordset, imageBuffer, size) != 0) {
			QMessageBox msg(QMessageBox::Warning, "Fehler", "Hex-Datei konnte nicht verarbeitet werden", QMessageBox::Ok);
			msg.exec();
			return;
		}
		rawData = imageBuffer;
		dataLength = size;

	} else if (imagePathEdit->text().endsWith(".bin")) {
		rawData = (uint8_t*)data.constData();
		dataLength = data.size();
	} else {
		return;
	}

	this->setEnabled(false);
	QCoreApplication::processEvents();

	// the bootloader might take a little longer to reply
	TuragSystemTime originalTimeout = turag_rs485_get_timeout();
	turag_rs485_set_timeout(turag_ms_to_ticks(100));

	TURAG::Feldbus::BootloaderAtmega::ErrorCode result = bootloader_->writeFlash(0, dataLength, rawData);

	if (result == TURAG::Feldbus::BootloaderAtmega::ErrorCode::success) {
		QMessageBox msg(QMessageBox::Information, "Erfolgreich", "Image erfolgreich zum Gerät übertragen", QMessageBox::Ok);
		msg.exec();
	} else {
		QMessageBox msg(QMessageBox::Critical, bootloader_->errorName(result), bootloader_->errorDescription(result), QMessageBox::Ok);
		msg.exec();
	}

	turag_rs485_set_timeout(originalTimeout);

	this->setEnabled(true);
}


void FeldbusBootloaderBaseView::openReadImageFile(void) {
	QFileDialog dialog(this, "Firmwaredatei speichern als", "");
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setNameFilter(tr("Binary Image file (*.bin)"));
	dialog.setDefaultSuffix("bin");
	if (dialog.exec() == QDialog::Accepted) {
		QStringList files = dialog.selectedFiles();
		imageReadPathEdit->setText(QString("%1").arg(files[0]));
	}
}

void FeldbusBootloaderBaseView::checkReadImageFile(QString path) {
	if (!path.isEmpty()) {
		readImageFileButton->setEnabled(true);
	} else {
		readImageFileButton->setEnabled(false);
	}
}

void FeldbusBootloaderBaseView::readImageFile(void) {
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

