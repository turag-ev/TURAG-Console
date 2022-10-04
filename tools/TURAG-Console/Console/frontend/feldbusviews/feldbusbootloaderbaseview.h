#ifndef TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSVIEWS_FELDBUSBOOTLOADERBASEVIEW_H
#define TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSVIEWS_FELDBUSBOOTLOADERBASEVIEW_H

#include <QWidget>
#include <tina++/feldbus/host/bootloader.h>


class QPushButton;
class QLineEdit;
class QFormLayout;
class FeldbusFrontend;
class QCheckBox;


class FeldbusBootloaderBaseView : public QWidget
{
	Q_OBJECT
public:
    explicit FeldbusBootloaderBaseView(TURAG::Feldbus::BootloaderAvrBase *bootloader, FeldbusFrontend* bus_, QWidget *parent = 0);

signals:

public slots:

protected slots:
	void openImageFile(void);
	void checkImageFile(QString path);
    void flashImageFile(void);
    void verifyImageFile(void);

	void openReadImageFile(void);
	void checkReadImageFile(QString path);
	void readImageFile(void);

	void leaveBootloader(void);

protected:
    virtual TURAG::Feldbus::BootloaderAvrBase::ErrorCode doFlashImage(uint8_t* data, uint32_t length);
    virtual TURAG::Feldbus::BootloaderAvrBase::ErrorCode doVerifyImage(uint8_t* data, uint32_t length);
    void setDisabledTheme(QLineEdit *lineedit);

	QFormLayout* infoLayout;
	QLineEdit* mcuIdEdit;
    FeldbusFrontend* bus;

private:
    void flashAndVerifyImageFile(bool flashImage, bool verifyImage);

	TURAG::Feldbus::BootloaderAvrBase* bootloader_;

	QPushButton* openFileDialogButton;
	QPushButton* flashHexFileButton;
	QLineEdit* hexFilePathEdit;

	QLineEdit* imagePathEdit;
	QPushButton* loadImageFileButton;
    QPushButton* flashImageFileButton;
    QPushButton* verifyImageFileButton;
    QLineEdit* imageFileSizeEdit;
    QCheckBox* verifyImageCheckbox;

	QLineEdit* imageReadPathEdit;
	QPushButton* selectImageReadFileButton;
	QPushButton* readImageFileButton;
};

#endif // TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSVIEWS_FELDBUSBOOTLOADERBASEVIEW_H
