#include "tinainterface.h"

TinaInterface::TinaInterface(QObject *parent) :
    QObject(parent)
{
}


void TinaInterface::dataInput(QByteArray data) {
    emit beginUpdate();

    int msg_begin = 0;

    // try to fill incomplete tina package in buffer with incoming data
    if (!packageBuffer_.isEmpty()) {
        for (int i = 0; i != data.size(); i++) {
            if (data.at(i) == '\n') {
                packageBuffer_.append(data.left(i));
                emit tinaPackageReady(trimCarriageReturn(packageBuffer_));
                packageBuffer_.clear();
                msg_begin = i + 1;
                break;
            }
        }
    }

    char lastDelim = '\n';

    // search for tina packages with cmenu outputs in between
    for (int i = msg_begin; i < data.size(); i++) {
        switch (lastDelim) {
        case '\n':
            if (data.at(i) == '\x02') {
                if (i - msg_begin > 0) {
                    emit cmenuDataReady(trimCarriageReturn(data.mid(msg_begin, i - msg_begin)));
                }
                msg_begin = i + 1;
                lastDelim = '\x02';
            }
            break;

        case '\x02':
            if (data.at(i) == '\n') {
                if (i - msg_begin > 0) {
                    emit tinaPackageReady(trimCarriageReturn(data.mid(msg_begin, i - msg_begin)));
                }
                msg_begin = i + 1;
                lastDelim = '\n';
            }
            break;
        }
    }

    // save incomplete tina packages in buffer; cmenu outputs are printed anyway
    switch (lastDelim) {
    case '\x02':
        if (data.size() - msg_begin > 0) {
            packageBuffer_.append(data.right(data.size() - msg_begin));
        } else {
            // here we cheat a little bit: if the \x02 char was the only one
            // we would not add anything to the buffer and miss the package
            // when the following data arrives. For that reason we add \r what
            // will be removed before emitting the whole package.
            packageBuffer_.append('\r');
        }
        break;

    case '\n':
        if (data.size() - msg_begin > 0) {
            emit cmenuDataReady(trimCarriageReturn(data.right(data.size() - msg_begin)));
        }
        break;
    }


    emit endUpdate();
}


QByteArray TinaInterface::trimCarriageReturn(QByteArray data) {
    while (data.endsWith('\r')) {
        data.remove(data.size()-1, 1);
    }
    while (data.startsWith('\r')) {
        data.remove(0, 1);
    }
    return data;
}
