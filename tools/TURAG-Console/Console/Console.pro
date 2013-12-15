#-------------------------------------------------
#
# Project created by QtCreator 2013-03-25T00:00:13
#
#-------------------------------------------------

CONFIG +=  qwt
QT     += core gui
QT     += network

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += printsupport
    QT += serialport
    QT += widgets
} else {
    CONFIG += serialport
}

TARGET = Console
TEMPLATE = app

# Schneller als die Standardimplementierung wenn Strings mit + verbunden werden
DEFINES *= QT_USE_QSTRINGBUILDER TURAG_STD_CONFIG

QMAKE_CXXFLAGS         += -std=gnu++0x
QMAKE_LFLAGS           += -std=gnu++0x

# Release
QMAKE_CXXFLAGS_RELEASE += -O3 -flto -march=native -funroll-loops -mfpmath=sse
QMAKE_CFLAGS_RELEASE   += -O3 -flto -march=native -funroll-loops -mfpmath=sse
QMAKE_LFLAGS_RELEASE   += -O3 -flto -march=native -funroll-loops -mfpmath=sse

SOURCES +=\
        mainwindow.cpp \
    libs/elidedbutton.cpp \
    frontend/basefrontend.cpp \
    backend/basebackend.cpp \
    backend/filebackend.cpp \
    backend/serialbackend.cpp \
    connectionwidgets/connectionwidget.cpp \
    connectionwidgets/connectionwidgetfile.cpp \
    connectionwidgets/connectionwidgetserial.cpp \
    controller.cpp \
    frontend/plaintextfrontend.cpp \
    frontend/logview.cpp \
    frontend/scfrontend.cpp \
    frontend/util/datagraph.cpp \
    frontend/util/lineinterface.cpp \
    frontend/oscilloscope.cpp \
    frontend/util/colormap.cpp \
    frontend/util/tinainterface.cpp \
    ../../../tina/platform/desktop-qt/thread.cpp \
    tina-platform/rs485.cpp \
    frontend/feldbusfrontend.cpp \
    ../../../tina/tina++/feldbus/host/device_tina.cpp \
    ../../../tina/tina/crc/crc8_icode/crc8_icode.c \
    tina-platform/debug_printf.cpp \
    frontend/util/feldbusdevicefactory.cpp \
    ../../../tina/tina++/feldbus/host/farbsensor_tina.cpp \
    ../../../tina/tina++/feldbus/host/aktor_tina.cpp \
    frontend/feldbusviews/feldbusfarbsensorview.cpp \
    frontend/util/datapointinterface.cpp \
    connectionwidgets/connectionwidgettcp.cpp \
    debugprintclass.cpp \
    ../../../tina/tina++/feldbus/dynamixel/dynamixeldevice.cpp \
    ../../../tina/tina/feldbus/dynamixel/dynamixel.c \
    ../../../tina/tina/feldbus/dynamixel/dxl_hal.c \
    frontend/feldbusviews/dynamixelview.cpp \
    backend/tcpbackend.cpp

HEADERS  += mainwindow.h \
    libs/elidedbutton.h \
    frontend/basefrontend.h \
    backend/basebackend.h \
    backend/filebackend.h \
    backend/serialbackend.h \
    connectionwidgets/connectionwidget.h \
    connectionwidgets/connectionwidgetfile.h \
    connectionwidgets/connectionwidgetserial.h \
    controller.h \
    frontend/plaintextfrontend.h \
    frontend/logview.h \
    frontend/scfrontend.h \
    frontend/util/datagraph.h \
    frontend/util/lineinterface.h \
    frontend/oscilloscope.h \
    frontend/util/colormap.h \
    frontend/util/tinainterface.h \
    tina-platform/public/tina/rs485.h \
    frontend/feldbusfrontend.h \
    frontend/util/feldbusdevicefactory.h \
    frontend/feldbusviews/feldbusfarbsensorview.h \
    frontend/util/datapointinterface.h \
    connectionwidgets/connectionwidgettcp.h \
    debugprintclass.h \
    ../../../tina/tina++/feldbus/dynamixel/dynamixeldevice.h \
    frontend/feldbusviews/dynamixelview.h \
    backend/tcpBackend_protocol.h \
    backend/tcpbackend.h

INCLUDEPATH += \
    ../../../tina \
    ../../../tina/platform/desktop-qt/public \
    tina-platform/public

RESOURCES += \
    images.qrc

OTHER_FILES += \
    TODO.txt

