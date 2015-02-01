#define TURAG_DEBUG_LOG_SOURCE "H"

#include <tina/tina.h>
#include <tina/time.h>
#include <tina/feldbus/host/rs485.h>
#include <QIODevice>
#include <QDebug>
#include <QCoreApplication>
#include <QByteArray>
#include <debugprintclass.h>


static TuragSystemTime rs485_timeout;

QIODevice* turag_rs485_io_device;
QByteArray turag_rs485_data_buffer;


extern "C" bool turag_rs485_init(uint32_t baud_rate, TuragSystemTime timeout) {
    (void)baud_rate;
    rs485_timeout = timeout;
    return true;
}

extern "C" bool turag_rs485_transceive(uint8_t *transmit, int* transmit_length, uint8_t *receive, int* receive_length) {
    if (!turag_rs485_io_device) {
        return false;
    }

    if (transmit_length && transmit) {
        int transmit_length_copy = *transmit_length;

        if (transmit_length_copy > 0) {
            QString transmitmsg = QString("_Write: %1 {").arg(transmit_length_copy);
            for (int i = 0; i < transmit_length_copy; ++i) {
                transmitmsg += QString("%1 ").arg((uint8_t)transmit[i]);
            }
            transmitmsg += "}:";

            *transmit_length = turag_rs485_io_device->write((const char*)transmit, transmit_length_copy);

            if (transmit_length_copy != *transmit_length) {
                rs485Debug.generateDebugMessage(transmitmsg + " failed");
                if (receive_length) {
                    *receive_length = 0;
                }
                return false;
            }
            if (!turag_rs485_io_device->waitForBytesWritten(turag_ticks_to_ms(rs485_timeout))) {
                rs485Debug.generateDebugMessage(transmitmsg + " timeout");
                if (receive_length) {
                    *receive_length = 0;
                }
                return false;
            }
            rs485Debug.generateDebugMessage(transmitmsg + " ok");
        }
    }

    if (receive_length && receive) {
        int receive_length_copy = *receive_length;

        if (receive_length_copy > 0) {
            QString outmsg = QString("_Read required: %1").arg(receive_length_copy);

            while (turag_rs485_data_buffer.size() < receive_length_copy) {
                if (!turag_rs485_io_device->waitForReadyRead(turag_ticks_to_ms(rs485_timeout))) {
                    rs485Debug.generateDebugMessage(outmsg + QString(" Timeout (%1)").arg(turag_rs485_data_buffer.size()));
                    *receive_length = turag_rs485_data_buffer.size();
                    turag_rs485_buffer_clear();
                    return false;
                }
            }

            rs485Debug.generateDebugMessage(outmsg + QString(" available: %1").arg(turag_rs485_data_buffer.size()));
            outmsg = "data: {";
            for (int i = 0; i < receive_length_copy; ++i) {
                receive[i] = turag_rs485_data_buffer.at(i);
                outmsg += QString("%1 ").arg((uint8_t)receive[i]);
            }
            rs485Debug.generateDebugMessage(outmsg + "}");
            turag_rs485_data_buffer.remove(0, receive_length_copy);
        }
    }

    return true;
}

extern "C" void turag_rs485_buffer_clear(void) {
	turag_rs485_data_buffer.clear();
}

extern "C" void turag_rs485_set_timeout(TuragSystemTime timeout) {
	rs485_timeout = timeout;
}

extern "C" TuragSystemTime turag_rs485_get_timeout(void) {
	return rs485_timeout;
}
