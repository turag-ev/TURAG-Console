#-------------------------------------------------
#
# Project created by QtCreator 2013-03-25T00:00:13
#
#-------------------------------------------------

VERSION = 2.3

CONFIG +=  qwt
QT     += core gui
QT     += network

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += printsupport
    QT += serialport
    QT += widgets
    # camera stuff
    QT += multimedia
    QT += multimediawidgets
} else {
    CONFIG += serialport
}

TARGET = turag-console
TEMPLATE = app

# Schneller als die Standardimplementierung wenn Strings mit + verbunden werden
DEFINES *= QT_USE_QSTRINGBUILDER

# causes segfaults in qwt ?!?!?!?!
#DEFINES += QT_COORD_TYPE=float

# Version für Quelldateien
DEFINES += PACKAGE_VERSION=$$VERSION

QMAKE_CXXFLAGS         += -std=gnu++0x
QMAKE_LFLAGS           += -std=gnu++0x

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
    frontend/oscilloscope.cpp \
    frontend/util/colormap.cpp \
    frontend/util/tinainterface.cpp \
    ../../../tina/platform/desktop/thread.cpp \
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
    backend/tcpbackend.cpp \
    frontend/feldbusviews/feldbusaktorview.cpp \
    ../../../tina/tina++/feldbus/host/dcmotor_tina.cpp \
    ../../../tina/tina++/feldbus/host/servo_tina.cpp \
    ../../../tina/tina++/feldbus/host/aseb_tina.cpp \
    frontend/feldbusviews/feldbusasebview.cpp \
    libs/qlineedit2.cpp \
    frontend/mcfrontend.cpp \
    frontend/tinagraphfrontend.cpp \
    ../../../tina/tina/utils/base64.c \
    ../../../tina/tina/debug/graph.c \
    libs/keyboardenabledcombobox.cpp \
    frontend/stmcamfrontend.cpp \
    frontend/tinacamerafrontend.cpp \
    frontend/feldbusviews/feldbusbootloaderview.cpp

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
    ../../Debug-Server/Debug_Server/debugserver_protocol.h \
    backend/tcpbackend.h \
    frontend/feldbusviews/feldbusaktorview.h \
    ../../../tina/tina++/feldbus/host/device.h \
    ../../../tina/tina++/feldbus/host/aktor.h \
    ../../../tina/tina++/feldbus/host/aseb.h \
    ../../../tina/tina++/feldbus/host/servo.h \
    ../../../tina/tina++/feldbus/host/dcmotor.h \
    ../../../tina/tina++/feldbus/host/farbsensor.h \
    frontend/feldbusviews/feldbusasebview.h \
    ../../../tina/tina++/tina.h \
    ../../../tina/tina++/math.h \
    ../../../tina/tina++/debug.h \
    ../../../tina/tina++/algorithm.h \
    ../../../tina/tina/tina.h \
    ../../../tina/tina/math.h \
    ../../../tina/tina/debug.h \
    ../../../tina/platform/desktop/public/tina/timetype.h \
    ../../../tina/platform/desktop/public/tina/time.h \
    ../../../tina/platform/desktop/public/tina/debugprint.h \
    ../../../tina/platform/desktop/public/tina/config.h \
    ../../../tina/tina++/time.h \
    ../../../tina/tina/helper/types.h \
    ../../../tina/tina/helper/normalize.h \
    ../../../tina/tina/helper/macros.h \
    ../../../tina/tina++/helper/normalize.h \
    ../../../tina/tina++/helper/macros.h \
    ../../../tina/platform/desktop/public/tina/thread.h \
    ../../../tina/platform/desktop/public/tina++/thread.h \
    ../../../tina/platform/desktop/public/tina/can.h \
    ../../../tina/platform/desktop/public/tina++/can.h \
    ../../../tina/tina/feldbus/protocol/turag_feldbus_fuer_stellantriebe.h \
    ../../../tina/tina/feldbus/protocol/turag_feldbus_fuer_lokalisierungssensoren.h \
    ../../../tina/tina/feldbus/protocol/turag_feldbus_fuer_aseb.h \
    ../../../tina/tina/feldbus/protocol/turag_feldbus_bus_protokoll.h \
    ../../../tina/tina++/helper/scoped_lock.h \
    ../../../tina/tina++/helper/packed.h \
    ../../../tina/tina++/helper/integer.h \
    ../../../tina/tina++/helper/init.h \
    ../../../tina/tina++/helper/construct.h \
    ../../../tina/tina/feldbus/dynamixel/dynamixel.h \
    ../../../tina/tina/feldbus/dynamixel/dxl_hal.h \
    ../../../tina/tina/crc/crc8_icode/crc8_icode.h \
    ../../../tina/tina++/feldbus/host/sensor.h \
    ../../../tina/tina++/range/algorithm.h \
    ../../../tina/tina++/crc/xor.h \
    ../../../tina/tina++/crc/crc16.h \
    ../../../tina/tina++/crc/crc8.h \
    ../../../tina/tina/crc/xor_checksum.h \
    ../../../tina/tina/bytes.h \
    libs/qlineedit2.h \
    frontend/mcfrontend.h \
    frontend/tinagraphfrontend.h \
    ../../../tina/tina/utils/base64.h \
    ../../../tina/tina++/utils/base64.h \
    ../../../tina/tina/debug/defines.h \
    ../../../tina/tina/debug/graph.h \
    ../../../tina/tina/debug/print.h \
    libs/keyboardenabledcombobox.h \
    frontend/stmcamfrontend.h \
    frontend/tinacamerafrontend.h \
    frontend/feldbusviews/feldbusbootloaderview.h

INCLUDEPATH += \
    ../../../tina \
    ../../../tina/platform/desktop/public \
    tina-platform/public \
    ../../Debug-Server \
    /usr/include/qwt

DISTR_FILES += \
    images/ok.png \
    images/nok.png \
    images/turag-55.png \
    images/lock.png \
    images/warning-orange-16.png \
    images/error-orange-16.png \
    images/error-red-16.png

RESOURCES += \
    images.qrc

OTHER_FILES += \
    TODO.txt

# install
target.path = $$PREFIX/bin
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS Console.pro
INSTALLS += target

# own make dist :P
PACKAGE_STRING = $(TARGET)-$${VERSION}$${EXT_VERSION}
TMP_DIR = .tmp
DIST_DIR = $${TMP_DIR}/$${PACKAGE_STRING}
DISTFILES = $${SOURCES} $${HEADERS} $${RESOURCES} $${FORMS} $${DISTR_FILES} Console.pro

distr.commands = (test -d $${DIST_DIR}/src || mkdir -p $${DIST_DIR}/src) && \
                 $(COPY_FILE) --parents $${DISTFILES} $${DIST_DIR}/src && \
                 (cd $${TMP_DIR} && \
                  $(TAR) $${PACKAGE_STRING}.tar $${PACKAGE_STRING} && \
                  $(COMPRESS) $${PACKAGE_STRING}.tar) && \
                 $(MOVE) $${TMP_DIR}/$${PACKAGE_STRING}.tar.gz . && \
                 $(DEL_FILE) -r $${DIST_DIR}

QMAKE_EXTRA_TARGETS += distr
