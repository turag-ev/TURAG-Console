#include "lineinterface.h"

LineInterface::LineInterface(QObject *parent) :
    QObject(parent)
{
}


void LineInterface::writeData(QByteArray data) {
    emit beginUpdate();

    int msg_begin = 0;

    // Zeilen-Puffer leeren:
    if (!buffer_.isEmpty()) {
        for (int i = 0; i != data.size(); i++) {
            if (data.at(i) == '\n') {
                buffer_.append(data.left(i));
                lineComplete(buffer_);
                buffer_.clear();
                msg_begin = i + 1;
                break;
            }
        }
    }

    // weiter mit richtigem Puffer
    for (int i = msg_begin; i < data.size(); i++) {
        if (data.at(i) == '\n') {
            lineComplete(data.mid(msg_begin, i - msg_begin));
            msg_begin = i + 1;
        }
    }

    // Zeilenanfang speichern in Zeilenpuffer:
    if (data.size() - msg_begin > 0) {
        buffer_.append(data.right(data.size() - msg_begin));
    }

    emit endUpdate();
}



void LineInterface::lineComplete(QByteArray line) {
    if (line.endsWith('\r')) {
        line.remove(line.size()-1, 1);
    }
    if (line.size() > 0 && line.at(0) == '\r') {
        line.remove(0, 1);
    }

    emit lineReady(line);
}
