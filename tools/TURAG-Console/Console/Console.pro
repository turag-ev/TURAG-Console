#-------------------------------------------------
#
# Project created by QtCreator 2013-03-25T00:00:13
#
#-------------------------------------------------

VERSION = 2.15

CONFIG +=  qwt
QT     += core gui
QT     += network
QT     += printsupport
QT     += serialport
QT     += widgets
QT     += xml
# camera stuff
QT     += multimedia
QT     += multimediawidgets
    
TARGET = turag-console
TEMPLATE = app

CONFIG += c++11

# Schneller als die Standardimplementierung wenn Strings mit + verbunden werden
DEFINES *= QT_USE_QSTRINGBUILDER

# non-const Iteratoren nicht in const-Iteratoren umwandeln
# damit kann entdeckt werden, wenn fälschlicher weise ein non-const Iterator erstellt wurde,
# was langsam sein kann.
DEFINES += QT_STRICT_ITERATORS

# causes segfaults in qwt ?!?!?!?!
#DEFINES += QT_COORD_TYPE=float

# Version für Quelldateien
DEFINES += PACKAGE_VERSION=$$VERSION
DEFINES += PACKAGE_NAME=$$TARGET

#DEFINES += DEBUG_WEBDAV

win32: {
  # don't use broken -mms-bitfields flag for windows
  QMAKE_CFLAGS += -mno-ms-bitfields
  QMAKE_CXXFLAGS += -mno-ms-bitfields
}

QMAKE_CXXFLAGS_RELEASE += -flto

SOURCES +=\
        mainwindow.cpp \
    ../../../tina/tina/crc/crc_checksum.c \
    ../../../tina/tina/feldbus/dynamixel/dxl_hal.c \
    ../../../tina/tina/feldbus/dynamixel/dynamixel.c \
    ../../../tina/tina/utils/base64.c \
    ../../../tina/tina++/feldbus/dynamixel/dynamixeldevice.cpp \
    ../../../tina/tina++/feldbus/host/aktor_tina.cpp \
    ../../../tina/tina++/feldbus/host/aseb_tina.cpp \
    ../../../tina/tina++/feldbus/host/dcmotor_tina.cpp \
    ../../../tina/tina++/feldbus/host/device_tina.cpp \
    ../../../tina/tina++/feldbus/host/farbsensor_tina.cpp \
    ../../../tina/tina++/feldbus/host/servo_tina.cpp \
    backend/basebackend.cpp \
    backend/filebackend.cpp \
    backend/serialbackend.cpp \
    backend/tcpbackend.cpp \
    connectionwidgets/connectionwidget.cpp \
    connectionwidgets/connectionwidgetfile.cpp \
    connectionwidgets/connectionwidgetserial.cpp \
    connectionwidgets/connectionwidgettcp.cpp \
    controller.cpp \
    debugprintclass.cpp \
    frontend/basefrontend.cpp \
    frontend/feldbusfrontend.cpp \
    frontend/feldbusviews/dynamixelview.cpp \
    frontend/feldbusviews/feldbusaktorview.cpp \
    frontend/feldbusviews/feldbusasebview.cpp \
    frontend/feldbusviews/feldbusfarbsensorview.cpp \
    frontend/graphutils/canvaspicker.cpp \
    frontend/graphutils/colormap.cpp \
    frontend/graphutils/datagraph.cpp \
    frontend/logview.cpp \
    frontend/oscilloscope.cpp \
    frontend/plaintextfrontend.cpp \
    frontend/stmcamfrontend.cpp \
    frontend/tinacamerafrontend.cpp \
    frontend/tinagraphfrontend.cpp \
    frontend/util/datapointinterface.cpp \
    frontend/util/feldbusdevicefactory.cpp \
    frontend/util/tinainterface.cpp \
    libs/elidedbutton.cpp \
    libs/keyboardenabledcombobox.cpp \
    libs/lineeditext.cpp \
    tina-platform/debug_printf.cpp \
    tina-platform/rs485.cpp \
    libs/loggerwidget.cpp \
    libs/log.cpp \
    ../../../tina/tina/debug/image.c \
    ../../../tina/tina/debug/print.c \
    ../../../tina/tina++/feldbus/host/bootloader_tina.cpp \
    ../../../tina/platform/desktop/thread.cpp \
    frontend/feldbusviews/feldbusbootloaderatmegaview.cpp \
    frontend/feldbusviews/feldbusbootloaderbaseview.cpp \
    frontend/feldbusviews/feldbusbootloaderxmegaview.cpp \
    ../../../tina/tina/debug/binary.cpp \
    frontend/rawfrontend.cpp \
    frontend/robotfrontend.cpp \
    ../../../tina/tina/debug/graph.cpp \
    ../../../libs/qt/qwebdav/qwebdavlib/qnaturalsort.cpp \
    ../../../libs/qt/qwebdav/qwebdavlib/qwebdav.cpp \
    ../../../libs/qt/qwebdav/qwebdavlib/qwebdavdirparser.cpp \
    ../../../libs/qt/qwebdav/qwebdavlib/qwebdavitem.cpp \
    connectionwidgets/connectionwidgetwebdav.cpp \
    libs/logindialog.cpp \
    backend/webdavbackend.cpp \
    connectionwidgets/connectionwidgetwebdav/webdavtreeitem.cpp \
    connectionwidgets/connectionwidgetwebdav/webdavtreemodel.cpp \
    libs/iconmanager.cpp \
    frontend/scfrontend.cpp

HEADERS  += \
    mainwindow.h \
    ../../../tina/platform/desktop/public/tina++/thread.h \
    ../../../tina/tina/bytes.h \
    ../../../tina/tina/crc/crc_checksum.h \
    ../../../tina/tina/crc/xor_checksum.h \
    ../../../tina/tina/debug.h \
    ../../../tina/tina/debug/defines.h \
    ../../../tina/tina/debug/print.h \
    ../../../tina/tina/feldbus/dynamixel/dxl_hal.h \
    ../../../tina/tina/feldbus/dynamixel/dynamixel.h \
    ../../../tina/tina/feldbus/protocol/turag_feldbus_bus_protokoll.h \
    ../../../tina/tina/feldbus/protocol/turag_feldbus_fuer_aseb.h \
    ../../../tina/tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h \
    ../../../tina/tina/feldbus/protocol/turag_feldbus_fuer_lokalisierungssensoren.h \
    ../../../tina/tina/feldbus/protocol/turag_feldbus_fuer_stellantriebe.h \
    ../../../tina/tina/helper/macros.h \
    ../../../tina/tina/helper/normalize.h \
    ../../../tina/tina/helper/types.h \
    ../../../tina/tina/math.h \
    ../../../tina/tina/tina.h \
    ../../../tina/tina/utils/base64.h \
    ../../../tina/tina++/algorithm.h \
    ../../../tina/tina++/crc/crc.h \
    ../../../tina/tina++/crc/xor.h \
    ../../../tina/tina++/debug.h \
    ../../../tina/tina++/feldbus/dynamixel/dynamixeldevice.h \
    ../../../tina/tina++/feldbus/host/aktor.h \
    ../../../tina/tina++/feldbus/host/aseb.h \
    ../../../tina/tina++/feldbus/host/dcmotor.h \
    ../../../tina/tina++/feldbus/host/device.h \
    ../../../tina/tina++/feldbus/host/farbsensor.h \
    ../../../tina/tina++/feldbus/host/sensor.h \
    ../../../tina/tina++/feldbus/host/servo.h \
    ../../../tina/tina++/helper/construct.h \
    ../../../tina/tina++/helper/init.h \
    ../../../tina/tina++/helper/integer.h \
    ../../../tina/tina++/helper/macros.h \
    ../../../tina/tina++/helper/normalize.h \
    ../../../tina/tina++/helper/packed.h \
    ../../../tina/tina++/helper/scoped_lock.h \
    ../../../tina/tina++/math.h \
    ../../../tina/tina++/range/algorithm.h \
    ../../../tina/tina++/time.h \
    ../../../tina/tina++/tina.h \
    ../../../tina/tina++/utils/base64.h \
    ../../Debug-Server/Debug_Server/debugserver_protocol.h \
    backend/basebackend.h \
    backend/filebackend.h \
    backend/serialbackend.h \
    backend/tcpbackend.h \
    connectionwidgets/connectionwidget.h \
    connectionwidgets/connectionwidgetfile.h \
    connectionwidgets/connectionwidgetserial.h \
    connectionwidgets/connectionwidgettcp.h \
    controller.h \
    debugprintclass.h \
    frontend/basefrontend.h \
    frontend/feldbusfrontend.h \
    frontend/feldbusviews/dynamixelview.h \
    frontend/feldbusviews/feldbusaktorview.h \
    frontend/feldbusviews/feldbusasebview.h \
    frontend/feldbusviews/feldbusfarbsensorview.h \
    frontend/graphutils/canvaspicker.h \
    frontend/graphutils/colormap.h \
    frontend/graphutils/datagraph.h \
    frontend/graphutils/hoverableqwtlegend.h \
    frontend/graphutils/hoverableqwtlegendlabel.h \
    frontend/logview.h \
    frontend/oscilloscope.h \
    frontend/plaintextfrontend.h \
    frontend/stmcamfrontend.h \
    frontend/tinacamerafrontend.h \
    frontend/tinagraphfrontend.h \
    frontend/util/datapointinterface.h \
    frontend/util/feldbusdevicefactory.h \
    frontend/util/tinainterface.h \
    libs/checkactionext.h \
    libs/checkboxext.h \
    libs/elidedbutton.h \
    libs/fastlistwidget.h \
    libs/keyboardenabledcombobox.h \
    libs/lineeditext.h \
    libs/splitterext.h \
    libs/loggerwidget.h \
    libs/log.h \
    ../../../tina/tina/helper/config_tina_default.h \
    ../../../tina/tina/helper/locked_type.h \
    ../../../tina/tina/debug/image.h \
    ../../../tina/tina/utils/bit_macros.h \
    ../../../tina/tina++/crc.h \
    ../../../tina/tina++/geometry.h \
    ../../../tina/tina++/statemachine.h \
    ../../../tina/tina++/feldbus/host/bootloader.h \
    ../../../tina/tina++/container/array_buffer.h \
    ../../../tina/tina++/container/array_storage.h \
    ../../../tina/tina++/container/circular_buffer.h \
    ../../../tina/tina++/container/container.h \
    ../../../tina/tina++/container/thread_fifo.h \
    ../../../tina/tina++/container/variant_class.h \
    ../../../tina/tina++/geometry/units/config.h \
    ../../../tina/tina++/geometry/units/math.h \
    ../../../tina/tina++/geometry/units/unit.h \
    ../../../tina/tina++/geometry/units/units.h \
    ../../../tina/tina++/geometry/fieldpose.h \
    ../../../tina/tina++/geometry/geometry.h \
    ../../../tina/tina++/geometry/units.h \
    ../../../tina/tina++/helper/locked.h \
    ../../../tina/tina++/helper/static_const.h \
    ../../../tina/tina++/utils/flags.h \
    ../../../tina/tina++/utils/misc-doc.h \
    ../../../tina/tina++/utils/timer.h \
    libs/comboboxext.h \
    config_tina.h \
    ../../../tina/tina/feldbus/host/rs485.h \
    ../../../tina/tina/helper/static_assert.h \
    ../../../tina/tina++/debug/binary.h \
    ../../../tina/platform/desktop/public/tina/can.h \
    ../../../tina/platform/desktop/public/tina/debugprint.h \
    ../../../tina/platform/desktop/public/tina/thread.h \
    ../../../tina/platform/desktop/public/tina/time.h \
    ../../../tina/platform/desktop/public/tina/timetype.h \
    ../../../tina/platform/desktop/public/tina/config_tina_platform.h \
    ../../../tina/platform/desktop/public/tina++/can.h \
    frontend/feldbusviews/feldbusbootloaderatmegaview.h \
	libs/buttongroupext.h \
    frontend/feldbusviews/feldbusbootloaderbaseview.h \
    frontend/feldbusviews/feldbusbootloaderxmegaview.h \
    ../../../tina/tina/debug/binary.h \
    ../../../tina/tina/debug/internal-debug.h \
    ../../../tina/tina/debug/log-source.h \
    frontend/rawfrontend.h \
    frontend/robotfrontend.h \
    ../../../tina/tina++/debug/graph.h \
    ../../../libs/qt/qwebdav/qwebdavlib/qnaturalsort.h \
    ../../../libs/qt/qwebdav/qwebdavlib/qwebdav_global.h \
    ../../../libs/qt/qwebdav/qwebdavlib/qwebdav.h \
    ../../../libs/qt/qwebdav/qwebdavlib/qwebdavdirparser.h \
    ../../../libs/qt/qwebdav/qwebdavlib/qwebdavitem.h \
    connectionwidgets/connectionwidgetwebdav.h \
    libs/logindialog.h \
    backend/webdavbackend.h \
    connectionwidgets/connectionwidgetwebdav/webdavtreeitem.h \
    connectionwidgets/connectionwidgetwebdav/webdavtreemodel.h \
    libs/iconmanager.h \
    frontend/scfrontend.h

INCLUDEPATH += \
    ../../../tina \
    ../../../tina/platform/desktop/public \
    tina-platform/public \
    ../../Debug-Server \
    . \
    ../../../libs

DISTR_FILES += \
    $$files(images/*.png) \
    turag-console.desktop

RESOURCES += \
    images.qrc

OTHER_FILES += \
    TODO.txt \
    images/turag-55.png

# install
unix:!mac {
  isEmpty(PREFIX):PREFIX = /usr
  
  BINDIR = $$PREFIX/bin
  DATADIR = $$PREFIX/share
  PKGDATADIR = $$DATADIR/minitube
    
  target.path = $$PREFIX/bin
  
  sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS Console.pro

  pixmaps.path = $$DATADIR/pixmaps
  pixmaps.files += images/turag-55.png

  desktop.path = $$DATADIR/applications
  desktop.files += turag-console.desktop
  
  mime.path = $$DATADIR/mime/packages
  mime.files += turag-console.xml

  INSTALLS += target pixmaps desktop mime

  # own make dist :P -> make distr
  PACKAGE_STRING = $(TARGET)-$${VERSION}$${EXT_VERSION}
  TMP_DIR = .tmp
  DIST_DIR = $${TMP_DIR}/$${PACKAGE_STRING}
  DISTFILES += $${SOURCES} $${HEADERS} $${RESOURCES} $${FORMS} $${DISTR_FILES} Console.pro

  distr.commands = (test -d $${DIST_DIR}/src || mkdir -p $${DIST_DIR}/src) && \
                   $(COPY_FILE) --parents $${DISTFILES} $${DIST_DIR}/src && \
                   (cd $${TMP_DIR} && \
                    $(TAR) $${PACKAGE_STRING}.tar $${PACKAGE_STRING} && \
                    $(COMPRESS) $${PACKAGE_STRING}.tar) && \
                   $(MOVE) $${TMP_DIR}/$${PACKAGE_STRING}.tar.gz . && \
                   $(DEL_FILE) -r $${DIST_DIR}

  QMAKE_EXTRA_TARGETS += distr
}

include(../../../libs/libsimeurobot.pri)
