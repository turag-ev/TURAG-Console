#ifndef TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSVIEWS_FELDBUSBOOTLOADERATMEGAVIEW_H
#define TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSVIEWS_FELDBUSBOOTLOADERATMEGAVIEW_H

#include "feldbusbootloaderbaseview.h"


class FeldbusBootloaderAtmegaView : public FeldbusBootloaderBaseView
{
	Q_OBJECT
public:
    explicit FeldbusBootloaderAtmegaView(TURAG::Feldbus::BootloaderAtmega *bootloader, FeldbusFrontend* bus_, QWidget *parent = 0);

};

#endif // TURAG_CONSOLE_CONSOLE_FRONTEND_FELDBUSVIEWS_FELDBUSBOOTLOADERATMEGAVIEW_H
