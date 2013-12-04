#-------------------------------------------------
#
# Project created by QtCreator 2013-03-25T00:00:13
#
#-------------------------------------------------

CONFIG += serialport qwt
QT       += core gui widgets serialport

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += printsupport
}

TARGET = Console
TEMPLATE = app

# Schneller als die Standardimplementierung wenn Strings mit + verbunden werden
DEFINES *= QT_USE_QSTRINGBUILDER TURAG_STD_CONFIG

QMAKE_CXXFLAGS         += -std=gnu++0x
QMAKE_LFLAGS           += -std=gnu++0x

# Release
QMAKE_CXXFLAGS_RELEASE += -Ofast -flto -march=native -funroll-loops -mfpmath=sse
QMAKE_CFLAGS_RELEASE   += -Ofast -flto -march=native -funroll-loops -mfpmath=sse
QMAKE_LFLAGS_RELEASE   += -Ofast -flto -march=native -funroll-loops -mfpmath=sse

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
    frontend/cmenufrontend.cpp

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
    frontend/cmenufrontend.h

INCLUDEPATH += \
    ../../../tina \

RESOURCES += \
    images.qrc

OTHER_FILES += \
    TODO.txt

