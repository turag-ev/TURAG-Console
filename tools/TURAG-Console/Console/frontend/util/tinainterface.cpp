#include "tinainterface.h"

#include <algorithm>
#include <QDebug>

TinaInterface::TinaInterface(QObject *parent) :
    QObject(parent), tina_package_depth_(0)
{
}


void TinaInterface::dataInput(const QByteArray data) {
    const char* msg_begin = data.constBegin(); // begin of message
    const char* iter = msg_begin; // Index in data buffer
    int tina_separators[] = {'\n','\x02'};

    // search for tina packages with cmenu outputs in between
    while (iter < data.end()) {
        if (tina_package_depth_ == 0) {
            // look for separator in stream
            iter = std::find(iter, data.cend(), '\x02');

            // output data until separator or end
            if (iter - msg_begin > 0) {
                emit cmenuDataReady(trimmedBuffer(msg_begin, iter));
            }
            if (iter != data.constEnd()) {
                ++tina_package_depth_;
                packageBuffer_.push_back(QByteArray(""));
                ++iter;
                msg_begin = iter;
            }
        } else { // if (tina_package_depth_ > 0)
            iter = std::find_first_of(iter, data.cend(), tina_separators, tina_separators + 2);

            // append data to buffer until separator or end
            if (iter - msg_begin > 0) {
                packageBuffer_.back().append(msg_begin, iter - msg_begin);
            }

            if (iter != data.constEnd()) {
                if (*iter == '\x02') {
                    ++tina_package_depth_;
                    packageBuffer_.push_back(QByteArray(""));
                } else { // if (*iter == '\n')
                    emit tinaPackageReady(trimmedBuffer(packageBuffer_.back()));
                    --tina_package_depth_;
                    packageBuffer_.pop_back();
                }
                ++iter;
                msg_begin = iter;
            }
        }
    }
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


void TinaInterface::clear(void) {
    tina_package_depth_ = 0;
    packageBuffer_.clear();
}
