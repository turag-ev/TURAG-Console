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
	// append new data to already buffered data, removing all carriage returns
    buffer.append(data.replace("\r", ""));

	int currentBufferIndex = 0;
	int nextDelimIndex = 0;

    // start time if that is a fresh package
    if (detectChannels && !firstChannelIsTime_) {
        clock.start();
    }

    // search for packet delim
	while ((nextDelimIndex = buffer.indexOf(delim_, currentBufferIndex)) != -1) {

		// The following code removes the first set of data.
		// This is important because for sequential devices
		// the first line could be incomplete thus rendering
		// the channel detection faulty.
		// FIXME: do this only for sequential devices.
		if (detectChannels) {
			// search for a second delim
			currentBufferIndex = nextDelimIndex + delim_.size();
			if ((nextDelimIndex = buffer.indexOf(delim_, currentBufferIndex)) == -1) {
				return;
			}
		}

        QByteArray packet;
		packet = buffer.mid(currentBufferIndex, nextDelimIndex - currentBufferIndex);
		currentBufferIndex = nextDelimIndex + delim_.size();

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
	buffer.remove(0, currentBufferIndex);
}

void TextDataPointInterface::clear() {
    buffer.clear();
    detectChannels = true;
}
