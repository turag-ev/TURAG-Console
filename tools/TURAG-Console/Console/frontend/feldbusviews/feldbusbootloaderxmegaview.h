#ifndef FELDBUSBOOTLOADERXMEGAVIEW_H
#define FELDBUSBOOTLOADERXMEGAVIEW_H

#include "feldbusbootloaderbaseview.h"


class FeldbusBootloaderXmegaView : public FeldbusBootloaderBaseView
{
	Q_OBJECT
public:
    explicit FeldbusBootloaderXmegaView(TURAG::Feldbus::BootloaderXmega *bootloader, FeldbusFrontend* bus_, QWidget *parent = 0);

};

#endif // FELDBUSBOOTLOADERXMEGAVIEW_H
