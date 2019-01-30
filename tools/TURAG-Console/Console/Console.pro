defineTest(minQtVersion) {
    maj = $$1
    min = $$2
    patch = $$3
    isEqual(QT_MAJOR_VERSION, $$maj) {
        isEqual(QT_MINOR_VERSION, $$min) {
            isEqual(QT_PATCH_VERSION, $$patch) {
                return(true)
            }
            greaterThan(QT_PATCH_VERSION, $$patch) {
                return(true)
            }
        }
        greaterThan(QT_MINOR_VERSION, $$min) {
            return(true)
        }
    }
    greaterThan(QT_MAJOR_VERSION, $$maj) {
        return(true)
    }
    return(false)
}

#-------------------------------------------------
#
# Project created by QtCreator 2013-03-25T00:00:13
#
#-------------------------------------------------

VERSION = 3.5.3

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

TINA   += feldbus-host feldbus-dynamixel feldbus-protocol crc base64 debug geometry
    
TARGET = turag-console
TEMPLATE = app

CONFIG += c++11


#version check qt
!minQtVersion(5, 2, 0) {
    message("Cannot build Qt Creator with Qt version $${QT_VERSION}.")
    error("Use at least Qt 5.2.0.")
}




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

QMAKE_CXXFLAGS += -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast \
    -Wcast-align -Wunused -Winit-self -Woverloaded-virtual -Wno-unused-parameter

win32: {
  # don't use broken -mms-bitfields flag for windows
  QMAKE_CFLAGS += -mno-ms-bitfields
  QMAKE_CXXFLAGS += -mno-ms-bitfields

  # winsocks2 library is required for tcp keepalive in
  # tcp backend
  LIBS += -lws2_32
}

QMAKE_CFLAGS += -std=c99

QMAKE_CFLAGS_RELEASE += -flto
QMAKE_CXXFLAGS_RELEASE += -flto
QMAKE_LFLAGS_RELEASE += -flto

SOURCES +=\
        mainwindow.cpp \
    backend/basebackend.cpp \
    backend/filebackend.cpp \
    backend/serialbackend.cpp \
    backend/tcpbackend.cpp \
    connectionwidgets/connectionwidget.cpp \
    connectionwidgets/connectionwidgetfile.cpp \
    connectionwidgets/connectionwidgetserial.cpp \
    connectionwidgets/connectionwidgettcp.cpp \
    controller.cpp \
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
    libs/loggerwidget.cpp \
    libs/log.cpp \
    frontend/feldbusviews/feldbusbootloaderatmegaview.cpp \
    frontend/feldbusviews/feldbusbootloaderbaseview.cpp \
    frontend/feldbusviews/feldbusbootloaderxmegaview.cpp \
    frontend/rawfrontend.cpp \
    frontend/robotfrontend.cpp \
    connectionwidgets/connectionwidgetwebdav.cpp \
    libs/logindialog.cpp \
    backend/webdavbackend.cpp \
    connectionwidgets/connectionwidgetwebdav/webdavtreeitem.cpp \
    connectionwidgets/connectionwidgetwebdav/webdavtreemodel.cpp \
    libs/iconmanager.cpp \
    frontend/scfrontend.cpp \
    main.cpp \
    libs/tcpsocketext.cpp \
    libs/popup/resizableframe.cpp \
    libs/popup/popupwidget.cpp \
    libs/popup/popupcontainerwidget.cpp \
    libs/popup/popuptoolbutton.cpp \
    frontend/graphutils/plotmagnifier.cpp \
    frontend/feldbusviews/feldbusmuxerview.cpp

HEADERS  += \
    mainwindow.h \
    backend/basebackend.h \
    backend/filebackend.h \
    backend/serialbackend.h \
    backend/tcpbackend.h \
    connectionwidgets/connectionwidget.h \
    connectionwidgets/connectionwidgetfile.h \
    connectionwidgets/connectionwidgetserial.h \
    connectionwidgets/connectionwidgettcp.h \
    controller.h \
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
    libs/comboboxext.h \
    config_tina.h \
    frontend/feldbusviews/feldbusbootloaderatmegaview.h \
	libs/buttongroupext.h \
    frontend/feldbusviews/feldbusbootloaderbaseview.h \
    frontend/feldbusviews/feldbusbootloaderxmegaview.h \
    frontend/rawfrontend.h \
    frontend/robotfrontend.h \
    connectionwidgets/connectionwidgetwebdav.h \
    libs/logindialog.h \
    backend/webdavbackend.h \
    connectionwidgets/connectionwidgetwebdav/webdavtreeitem.h \
    connectionwidgets/connectionwidgetwebdav/webdavtreemodel.h \
    libs/iconmanager.h \
    frontend/scfrontend.h \
    libs/tcpsocketext.h \
    libs/popup/resizableframe.h \
    libs/popup/popupwidget.h \
    libs/popup/popupcontainerwidget.h \
    libs/popup/popuptoolbutton.h \
    frontend/graphutils/plotmagnifier.h \
    frontend/feldbusviews/feldbusmuxerview.h

INCLUDEPATH += \
    tina-platform/public \
    .

DISTR_FILES += \
    $$files(images/*.png) \
    Console.pro

RESOURCES += \
    images.qrc

OTHER_FILES += TODO.txt

PIXMAPS      = images/turag-55.png
APPLICATIONS = turag-console.desktop
MIMES        = turag-console.xml

TURAG_REPRO = ../../..
include($$TURAG_REPRO/libs/libsimeurobot.pri)
include($$TURAG_REPRO/libs/qt/expander-widget/expanderwidget.pri)
include($$TURAG_REPRO/libs/libcintelhex.pri)
include($$TURAG_REPRO/libs/qwebdav.pri)
include($$TURAG_REPRO/tools/Debug-Server/Debug-Server-Protocol.pri)
include($$TURAG_REPRO/common/TURAG-common.pri)
include($$TURAG_REPRO/tina/tina.pri)
include($$TURAG_REPRO/tina/platform/desktop/tina-desktop.pri)


# install & distr
unix:!mac {

  #
  # paths

  PREFIX = $$(PREFIX)
  isEmpty(PREFIX):PREFIX = /usr/local
  
  BINDIR = $$PREFIX/bin
  DATADIR = $$PREFIX/share
  PKGDATADIR = $$DATADIR/turag-console

  #
  # make distr (own make dist)

  MAKEDISTRFILES = \
    $${SOURCES} $${HEADERS} $${RESOURCES} $${FORMS} \
    $${DISTR_FILES} $${DIST_FILES} $${DISTFILES} $${OTHER_FILES} \
    $${PIXMAPS} $${APPLICATIONS} $${MIMES}
  MAKEDISTRFILES = $$unique(MAKEDISTRFILES)

  PACKAGE_STRING = $${TARGET}-$${VERSION}$${EXT_VERSION}
  TMP_DIR = $$(PWD)/.tmp
  DIST_DIR = $${TMP_DIR}/$${PACKAGE_STRING}/tools/TURAG-Console/Console

  distr.commands =
  distr.depends = $${PACKAGE_STRING}.tar.gz
  distr.target = distr

  pack.input = MAKEDISTRFILES
  pack.output = $${PACKAGE_STRING}.tar.gz
  pack.CONFIG = combine no_link explicit_dependencies

  pack.commands = \
    realpath --relative-to $$PWD/$$TURAG_REPRO ${QMAKE_FILE_IN} | \
    tar -acf $${PACKAGE_STRING}.tar.bz2 --xform='s%^%$${PACKAGE_STRING}/%' -C $$PWD/$$TURAG_REPRO -T -

  QMAKE_EXTRA_COMPILERS += pack
  QMAKE_EXTRA_TARGETS += distr

  #
  # make install

  target.path = $$PREFIX/bin

  pixmaps.path = $$DATADIR/pixmaps
  pixmaps.files += $${PIXMAPS}

  desktop.path = $$DATADIR/applications
  desktop.files += $${APPLICATIONS}

  mime.path = $$DATADIR/mime/packages
  mime.files += $${MIMES}

  INSTALLS += target pixmaps desktop mime
}
