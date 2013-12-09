#include "datapointinterface.h"
#include <QByteArray>


TextDataPointInterface::TextDataPointInterface() :
    delim_(" "),
    channelDelim_(","), decimalPoint_('.'),
    firstChannelIsTime_(false), detectChannels(true)
{
}


void TextDataPointInterface::setDecimalPoint(char decimalPoint) {
    decimalPoint_ = decimalPoint;
    buffer.clear();
    detectChannels = true;
}

void TextDataPointInterface::setDelim(QByteArray delim) {
    delim_ = delim;
    buffer.clear();
    detectChannels = true;
}


void TextDataPointInterface::setChannelDelim(QByteArray delim) {
    channelDelim_ = delim;
    buffer.clear();
    detectChannels = true;
}

void TextDataPointInterface::setFirstChannelIsTime(bool isTime) {
    firstChannelIsTime_ = isTime;
    buffer.clear();
    detectChannels = true;
}

void TextDataPointInterface::writeData(QByteArray data) {
    // append new data to already buffered data, removing all line feeds
    buffer.append(data.replace("\r", ""));

    // start time if that is a fresh package
    if (detectChannels && !firstChannelIsTime_) {
        clock.start();
    }

    // search for packet delim
    while (buffer.contains(delim_)) {
        if (detectChannels) {
            if (buffer.count(delim_) > 1) {
                buffer = buffer.remove(0, buffer.indexOf(delim_) + delim_.size());
            } else {
                break;
            }
        }

        QByteArray packet;

        int index = buffer.indexOf(delim_);
        packet = buffer.left(index);
        buffer.remove(0, index + delim_.length());

        QList<QByteArray> parts = packet.split(channelDelim_.at(0));

        // data invalid if we only have the time channel
        if (firstChannelIsTime_ && parts.size() <= 1) {
            break;
        }

        QList<DataEntry> list;

        float time = 0;
        int start_i = 0;

        if (firstChannelIsTime_) {
            start_i = 1;

            QByteArray part = parts.at(0).trimmed().replace("\n", "");
            bool success = false;
            time = part.toFloat(&success);

            if (!success) {
                break;
            }
        } else {
            time = clock.elapsed();
        }

        for (int i = start_i; i < parts.size(); ++i) {
            QByteArray part = parts.at(i).trimmed().replace("\n", "");
            bool success = false;
            float value = part.toFloat(&success);

            if (success) {
                list.append(DataEntry(i - start_i, time, value));
            }
        }
        if (detectChannels) {
            emit channelsDetected(parts.size() - start_i);
            detectChannels = false;
        }
        emit dataPointsReady(list);
    }
}

void TextDataPointInterface::clear() {
    buffer.clear();
    detectChannels = true;
}
