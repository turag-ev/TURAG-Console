#ifndef FELDBUSBOOTLOADERVIEW_H
#define FELDBUSBOOTLOADERVIEW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <fstream>
#include <tina++/feldbus/host/device.h>

#include "../bootloader/hex2bin.h"

class FeldbusBootloaderView : public QWidget//, public TURAG::Feldbus::Device
{
    Q_OBJECT
public:
    explicit FeldbusBootloaderView(TURAG::Feldbus::Device* dev, QWidget *parent = 0);

protected:
    QLabel *labelFilePath;
    QLineEdit *textFilePath;
    QPushButton *buttonGetFilePath_ ;
    QPushButton *button_transferToMC_;
    QByteArray memblock;
    char *input_filename;

signals:

public slots:

protected slots:
    void onOpenFile(void);
    void onTransferFirmware(void);
    void onCreateBinary(void);
    bool onReadBinary(void);

private:
    TURAG::Feldbus::Device* dev;
    QString deviceName;
    int page_size;
    int pages_max;
    int fsize;
};

#endif // FELDBUSBOOTLOADERVIEW_H
