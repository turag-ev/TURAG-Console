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

VERSION = 3.1.0

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
    tina-platform/debug_printf.cpp \
    tina-platform/rs485.cpp \
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
    libs/tcpsocketext.cpp

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
    libs/debugprintclass.h

INCLUDEPATH += \
    tina-platform/public \
    .

DISTR_FILES += \
    $$files(images/*.png) \
    turag-console.desktop \
    ../../../libs/libsimeurobot.pri \
    ../../../libs/libsimeurobot/plugin/libsimeurobot-plugin.pri \
    ../../../libs/qt/expander-widget/expanderwidget.pri \
    Console.pro

RESOURCES += \
    images.qrc

OTHER_FILES += \
    TODO.txt \
    images/turag-55.png

include(../../../libs/libsimeurobot.pri)
include(../../../libs/qt/expander-widget/expanderwidget.pri)
include(../../../libs/libcintelhex.pri)
include(../../../libs/qwebdav.pri)
include(../../Debug-Server/Debug-Server-Protocol.pri)
include(../../../src/common/TURAG-common.pri)
include(../../../tina/tina.pri)
include(../../../tina/platform/desktop/tina-desktop.pri)

# install
unix:!mac {
  isEmpty(PREFIX):PREFIX = /usr/local
  
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
  PACKAGE_STRING = $${TARGET}-$${VERSION}$${EXT_VERSION}
  TMP_DIR = .tmp
  DIST_DIR = $${TMP_DIR}/$${PACKAGE_STRING}/tools/TURAG-Console/Console
  DISTRFILES += $${SOURCES} $${HEADERS} $${RESOURCES} $${FORMS} $${DISTR_FILES}

  distr.commands =
  distr.depends = $${PACKAGE_STRING}.tar.gz
  distr.target = distr

  pack.input = DISTRFILES
  pack.output = $${PACKAGE_STRING}.tar.gz
  pack.CONFIG = combine no_link explicit_dependencies
  pack.dependency_type = TYPE_C
  pack.commands =  (test -d $${DIST_DIR} || mkdir -p $${DIST_DIR}) && \
                   $(COPY_FILE) --parents ${QMAKE_FILE_IN} $${DIST_DIR} && \
                   (cd $${TMP_DIR} && \
                    $(TAR) $${PACKAGE_STRING}.tar $${PACKAGE_STRING} && \
                    $(COMPRESS) $${PACKAGE_STRING}.tar) && \
                   $(MOVE) $${TMP_DIR}/$${PACKAGE_STRING}.tar.gz . && \
                   $(DEL_FILE) -r $${TMP_DIR}

  QMAKE_EXTRA_COMPILERS += pack
  QMAKE_EXTRA_TARGETS += distr
}
