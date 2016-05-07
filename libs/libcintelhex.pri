INCLUDEPATH *= $$PWD
DEPENDPATH  *= $$PWD

SOURCES += \
    $$PWD/libcintelhex/ihex_copy.c \
    $$PWD/libcintelhex/ihex_parse.c \
    $$PWD/libcintelhex/ihex_record.c


HEADERS  += \
    $$PWD/libcintelhex/cintelhex.h \
    $$PWD/libcintelhex/config.h
