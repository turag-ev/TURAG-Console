#ifndef FELDBUSBOOTLOADERVIEW_H
#define FELDBUSBOOTLOADERVIEW_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <tina++/feldbus/host/device.h>

class FeldbusBootloaderView : public QWidget
{
    Q_OBJECT
public:
    explicit FeldbusBootloaderView(TURAG::Feldbus::Device* dev, QWidget *parent = 0);

protected:
    QLabel *labelFilePath;
    QLineEdit *textFilePath;
    QPushButton *buttonGetFilePath_ ;

signals:

public slots:

protected slots:
    void onOpenFile();

private:
    TURAG::Feldbus::Device* dev;

    QString deviceName;
};

#endif // FELDBUSBOOTLOADERVIEW_H
