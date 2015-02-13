#ifndef FELDBUSBOOTLOADERATMEGAVIEW_H
#define FELDBUSBOOTLOADERATMEGAVIEW_H

#include <QWidget>
#include <tina++/feldbus/host/bootloader.h>


class QPushButton;
class QLineEdit;

class FeldbusBootloaderAtmegaView : public QWidget
{
	Q_OBJECT
public:
	explicit FeldbusBootloaderAtmegaView(TURAG::Feldbus::BootloaderAtmega *bootloader, QWidget *parent = 0);

signals:

public slots:

protected slots:
	//void openHexFile(void);
	//void checkHexFile(QString path);
	//void flashHexFile(void);

	void openImageFile(void);
	void checkImageFile(QString path);
	void flashImageFile(void);

	void openReadImageFile(void);
	void checkReadImageFile(QString path);
	void readImageFile(void);

	void leaveBootloader(void);

private:
	void setDisabledTheme(QLineEdit *lineedit);

	TURAG::Feldbus::BootloaderAtmega* bootloader_;

	QPushButton* openFileDialogButton;
	QPushButton* flashHexFileButton;
	QLineEdit* hexFilePathEdit;

	QLineEdit* imagePathEdit;
	QPushButton* loadImageFileButton;
	QPushButton* flashImageFileButton;
	QLineEdit* imageFileSizeEdit;

	QLineEdit* imageReadPathEdit;
	QPushButton* selectImageReadFileButton;
	QPushButton* readImageFileButton;
};

#endif // FELDBUSBOOTLOADERATMEGAVIEW_H
