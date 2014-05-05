#define TURAG_DEBUG_LOG_SOURCE "H"

#include <tina/tina.h>
#include <tina/time.h>
#include <tina/rs485.h>
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

extern "C" bool turag_rs485_ready(void) {
    return true;
}

extern "C" bool turag_rs485_transceive(uint8_t *input, int input_length, uint8_t *output, int output_length) {
    if (!turag_rs485_io_device) {
        return false;
    }

    if (input_length > 0 && input) {
        QString inputmsg = QString("_Write: %1 {").arg(input_length);
        for (int i = 0; i < input_length; ++i) {
            inputmsg += QString("%1 ").arg((uint8_t)input[i]);
        }
        inputmsg += "}:";

        if (input_length != turag_rs485_io_device->write((const char*)input, input_length)) {
            rs485Debug.generateDebugMessage(inputmsg + " failed");
            return false;
        }
        if (!turag_rs485_io_device->waitForBytesWritten(turag_ticks_to_ms(rs485_timeout))) {
            rs485Debug.generateDebugMessage(inputmsg + " timeout");
            return false;
        }
        rs485Debug.generateDebugMessage(inputmsg + " ok");
    }

    if (output_length > 0 && output) {
        QString outmsg = QString("_Read required: %1").arg(output_length);

        while (turag_rs485_data_buffer.size() < output_length) {
            if (!turag_rs485_io_device->waitForReadyRead(turag_ticks_to_ms(rs485_timeout))) {
                rs485Debug.generateDebugMessage(outmsg + QString(" Timeout (%1)").arg(turag_rs485_data_buffer.size()));
                turag_rs485_buffer_clear();
                return false;
            }
        }

        rs485Debug.generateDebugMessage(outmsg + QString(" available: %1").arg(turag_rs485_data_buffer.size()));
        outmsg = "data: {";
        for (int i = 0; i < output_length; ++i) {
            output[i] = turag_rs485_data_buffer.at(i);
            outmsg += QString("%1 ").arg((uint8_t)output[i]);
        }
        rs485Debug.generateDebugMessage(outmsg + "}");
        turag_rs485_data_buffer.remove(0, output_length);
    }

    return true;
}

extern "C" void turag_rs485_buffer_clear(void) {
	turag_rs485_data_buffer.clear();
}
