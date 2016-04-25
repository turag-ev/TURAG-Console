#ifndef FELDBUSBOOTLOADERATMEGAVIEW_H
#define FELDBUSBOOTLOADERATMEGAVIEW_H

#include "feldbusbootloaderbaseview.h"


class FeldbusBootloaderAtmegaView : public FeldbusBootloaderBaseView
{
	Q_OBJECT
public:
    explicit FeldbusBootloaderAtmegaView(TURAG::Feldbus::BootloaderAtmega *bootloader, FeldbusFrontend* bus_, QWidget *parent = 0);

};

#endif // FELDBUSBOOTLOADERATMEGAVIEW_H
