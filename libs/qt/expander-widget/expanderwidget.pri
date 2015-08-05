
QT       *= gui
QT       *= widgets


SOURCES +=\
    $$PWD/expanderwidget.cpp
    
HEADERS  += \
    $$PWD/expanderwidget.h
    
RESOURCES += \
    $$PWD/expanderwidget.qrc

DISTR_FILES += \
    $$files($$PWD/*.png)
