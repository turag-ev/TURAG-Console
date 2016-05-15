
QT       *= gui
QT       *= widgets


SOURCES +=\
    $$PWD/expanderwidget.cpp
    
HEADERS  += \
    $$PWD/expanderwidget.h
    
RESOURCES += \
    $$PWD/expanderwidget.qrc

DISTR_FILES += \
    $$PWD/expanderwidget.pri \
    $$files($$PWD/*.png)
