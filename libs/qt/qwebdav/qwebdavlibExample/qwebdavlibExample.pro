QT       += core network xml

QT       -= gui

TARGET = testConsole
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    qexample.cpp

win32:CONFIG(release, debug|release) {
    # WINDOWS RELEASE
    PRE_TARGETDEPS += $$OUT_PWD/../qwebdavlib/release/libqwebdav.a
    INCLUDEPATH += $$OUT_PWD/../qwebdavlib/
    LIBS += -L$$OUT_PWD/../qwebdavlib/release/
} else:win32:CONFIG(debug, debug|release) {
    # WINDOWS DEBUG
    PRE_TARGETDEPS += $$OUT_PWD/../qwebdavlib/debug/libqwebdav.a
    INCLUDEPATH += $$OUT_PWD/../qwebdavlib/
    LIBS += -L$$OUT_PWD/../qwebdavlib/debug/
}

LIBS += -lqwebdav

HEADERS += \
    qexample.h
