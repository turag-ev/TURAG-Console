#include "feldbusbootloaderview.h"
#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>

FeldbusBootloaderView::FeldbusBootloaderView(TURAG::Feldbus::Device *dev_, QWidget *parent) :
    QWidget(parent), dev(dev_)
{
    QVBoxLayout* layout = new QVBoxLayout;
    QHBoxLayout* hlayout = new QHBoxLayout;
    QLabel* label = new QLabel("Gerätetyp");
    hlayout->addWidget(label);

    QLineEdit* text = new QLineEdit;
    text->setReadOnly(true);
    hlayout->addWidget(text);

    layout->addLayout(hlayout);
    setLayout(layout);

    if (dev) {
        TURAG::Feldbus::Request<uint8_t> request;
        request.data = TURAG_FELDBUS_BOOTLOADER_COMMAND_GET_MCUID;

        TURAG::Feldbus::Response<uint8_t> response;

        if (dev->transceive(request, &response)) {
            text->setText(QString("%1").arg(response.data));
        } else {
            text->setText("ERROR");
        }
    }
}
