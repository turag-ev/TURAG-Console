#include "tinainterface.h"

#include <algorithm>
#include <QDebug>

TinaInterface::TinaInterface(QObject *parent) :
    QObject(parent), content_(BufferContentType::CMENU)
{
}


void TinaInterface::dataInput(const QByteArray data) {
    emit beginUpdate();

    const char* msg_begin = data.constBegin(); // begin of message
    const char* iter = msg_begin; // Index in data buffer

    // try to fill incomplete tina package in buffer with incoming data
    if (content_ == BufferContentType::TINA_DEBUG) {
        for (; iter < data.end(); iter++) {
            if (*iter == '\n') {
                packageBuffer_.append(data.constBegin(), iter - data.constBegin());
                emit tinaPackageReady(trimmedBuffer(packageBuffer_));
                packageBuffer_.clear();
                msg_begin = iter + 1;
                break;
            }
        }
    }

    // search for tina packages with cmenu outputs in between
    while (iter < data.end()) {
        if (content_ == BufferContentType::CMENU) {
            iter = std::find(iter, data.constEnd(), '\x02');
            if (iter != data.constEnd()) {
                if (iter - msg_begin > 0) {
                    emit cmenuDataReady(trimmedBuffer(msg_begin, iter));
                }
                content_ = BufferContentType::TINA_DEBUG;
                msg_begin = iter + 1;

            } else {
                // we are at the end, print the rest
                if (data.end() - msg_begin > 0) {
                    emit cmenuDataReady(trimmedBuffer(msg_begin, data.end()));
                }
                break;
            }

        } else { // if (content_ == BufferContentType::TINA_DEBUG)
            iter = std::find(iter, data.constEnd(), '\n');
            if (iter != data.constEnd()) {
                if (iter - msg_begin > 0) {
                    emit tinaPackageReady(trimmedBuffer(msg_begin, iter));
                }
                content_ = BufferContentType::CMENU;
                msg_begin = iter + 1;

            } else {
                // we are at the end, save incomplete tina packages in buffer
                packageBuffer_.append(msg_begin, data.end() - msg_begin);
                break;
            }
        }

        iter++;
    }

    emit endUpdate();
}

/**
 * @brief TinaInterface::trimmedBuffer trim part of buffer and return a copy of it
 * @param data buffer
 * @param begin pointer to begin of part of buffer to process
 * @param end pointer to end of part of buffer to process (to one byte after last byte to process)
 * @return new trimmed buffer
 */
QByteArray TinaInterface::trimmedBuffer(const char* begin, const char* end) {
    for (; *begin == '\r' && begin < end; begin++) { }
    for (; *(end-1) == '\r' && end > begin; end--) { }

    return QByteArray(begin, end - begin);
}
