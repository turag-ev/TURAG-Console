QT       *= gui
QT       *= widgets

INCLUDEPATH *= $$PWD
DEPENDPATH  *= $$PWD

SOURCES += \
    $$PWD/qt/qwebdav/qwebdavlib/qwebdav.cpp \
    $$PWD/qt/qwebdav/qwebdavlib/qnaturalsort.cpp \
    $$PWD/qt/qwebdav/qwebdavlib/qwebdavdirparser.cpp \
    $$PWD/qt/qwebdav/qwebdavlib/qwebdavitem.cpp


HEADERS  += \
    $$PWD/qt/qwebdav/qwebdavlib/qwebdav.h \
    $$PWD/qt/qwebdav/qwebdavlib/qnaturalsort.h \
    $$PWD/qt/qwebdav/qwebdavlib/qwebdav_global.h \
    $$PWD/qt/qwebdav/qwebdavlib/qwebdavdirparser.h \
    $$PWD/qt/qwebdav/qwebdavlib/qwebdavitem.h
