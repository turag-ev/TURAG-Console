#ifndef TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSVIEWS_FELDBUSBOOTLOADERXMEGAVIEW_H
#define TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSVIEWS_FELDBUSBOOTLOADERXMEGAVIEW_H

#include "feldbusbootloaderbaseview.h"


class FeldbusBootloaderXmegaView : public FeldbusBootloaderBaseView
{
	Q_OBJECT
public:
    explicit FeldbusBootloaderXmegaView(TURAG::Feldbus::BootloaderXmega *bootloader, FeldbusFrontend* bus_, QWidget *parent = 0);

};

#endif // TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSVIEWS_FELDBUSBOOTLOADERXMEGAVIEW_H
