#ifndef FELDBUSBOOTLOADERXMEGAVIEW_H
#define FELDBUSBOOTLOADERXMEGAVIEW_H

#include "feldbusbootloaderbaseview.h"


class FeldbusBootloaderXmegaView : public FeldbusBootloaderBaseView
{
	Q_OBJECT
public:
	explicit FeldbusBootloaderXmegaView(TURAG::Feldbus::BootloaderXmega *bootloader, QWidget *parent = 0);

};

#endif // FELDBUSBOOTLOADERXMEGAVIEW_H
