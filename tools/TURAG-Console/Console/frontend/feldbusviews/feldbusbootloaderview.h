#ifndef FELDBUSBOOTLOADERVIEW_H
#define FELDBUSBOOTLOADERVIEW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <fstream>
#include <QThread>
#include <QTimer>
#include <tina++/feldbus/host/device.h>
#include <tina++/feldbus/host/bootloader.h>

#include "../hex2bin/hex2bin.h"

class FeldbusBootloaderView : public QWidget
{
    Q_OBJECT
public:
    explicit FeldbusBootloaderView(TURAG::Feldbus::Bootloader* bootloader_, QWidget *parent = 0);

protected:
    QLabel *labelFilePath;
    QLineEdit *textFilePath;
    QPushButton *buttonGetFilePath_ ;
    QPushButton *button_transferToMC_;
    QByteArray memblock;
    QTimer sendPageTimer_;
    char *input_filename;

signals:

public slots:

protected slots:
    void onOpenFile(void);
    void onTransferFirmware(void);
    void onCreateBinary(void);
    bool onReadBinary(void);
    void onWritePages(void);

private:
    TURAG::Feldbus::Bootloader* bootloader_;
    QString deviceName;
    int page_size;
    int pages_max;
    int page_cur;
    int fsize;
    bool transmitError;
};

class Sleeper : public QThread
{
public:
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
    static void sleep(unsigned long secs){QThread::sleep(secs);}
};

#endif // FELDBUSBOOTLOADERVIEW_H
