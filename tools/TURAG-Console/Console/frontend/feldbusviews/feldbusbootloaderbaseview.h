#ifndef FELDBUSBOOTLOADERBASEVIEW_H
#define FELDBUSBOOTLOADERBASEVIEW_H

#include <QWidget>
#include <tina++/feldbus/host/bootloader.h>


class QPushButton;
class QLineEdit;
class QFormLayout;


class FeldbusBootloaderBaseView : public QWidget
{
	Q_OBJECT
public:
	explicit FeldbusBootloaderBaseView(TURAG::Feldbus::BootloaderAvrBase *bootloader, QWidget *parent = 0);

signals:

public slots:

protected slots:
	void openImageFile(void);
	void checkImageFile(QString path);
	void flashImageFile(void);

	void openReadImageFile(void);
	void checkReadImageFile(QString path);
	void readImageFile(void);

	void leaveBootloader(void);

protected:
	void setDisabledTheme(QLineEdit *lineedit);

	QFormLayout* infoLayout;

private:
	TURAG::Feldbus::BootloaderAvrBase* bootloader_;

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

#endif // FELDBUSBOOTLOADERBASEVIEW_H
