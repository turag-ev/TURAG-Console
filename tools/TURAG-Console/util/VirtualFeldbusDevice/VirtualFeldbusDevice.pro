#-------------------------------------------------
#
# Project created by QtCreator 2015-02-15T21:41:33
#
#-------------------------------------------------

QT       += core
QT       -= gui
QT     += serialport

TARGET = VirtualFeldbusDevice
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += c++11

TEMPLATE = app


DEFINES += TURAG_NO_PLATFORM_CONFIG
DEFINES += MY_ADDR=1

INCLUDEPATH += \
    ../../../../libs/qt \
    ../../../../tina/tina/feldbus \
    ../../../../tina \
    .


SOURCES += \
    C:/Users/martin/Documents/programmierung/libs/qt/signal_handler/signalhandler.cpp \
    feldbusdevice.cpp \
    ../../../../tina/tina/feldbus/slave/feldbus.c \
    ../../../../tina/tina/crc/crc_checksum.c

HEADERS += \
    C:/Users/martin/Documents/programmierung/libs/qt/signal_handler/signalhandler.h \
    ../../../../tina/tina/feldbus/protocol/turag_feldbus_bus_protokoll.h \
    feldbusdevice.h \
    ../../../../tina/tina/feldbus/slave/feldbus.h \
    config_tina.h \
    feldbus_config.h \
    ../../../../tina/tina/crc/crc_checksum.h
