#define LOG_SOURCE "H"

#include <tina/tina.h>
#include <tina/time.h>
#include <QIODevice>
#include <QDebug>
#include <QCoreApplication>
#include <QByteArray>


static TuragSystemTime rs485_timeout;

QIODevice* turag_rs485_io_device;
QByteArray turag_rs485_data_buffer;


extern "C" bool turag_rs485_init(uint32_t baud_rate, TuragSystemTime timeout) {
    (void)baud_rate;
    rs485_timeout = timeout;
    return true;
}

extern "C" bool turag_rs485_transceive(uint8_t *input, int input_length, uint8_t *output, int output_length) {
    if (!turag_rs485_io_device) {
        return false;
    }

    if (input_length != turag_rs485_io_device->write((const char*)input, input_length)) {
        return false;
    }
    if (!turag_rs485_io_device->waitForBytesWritten(turag_ticks_to_ms(rs485_timeout))) {
        return false;
    }


    while (turag_rs485_data_buffer.size() < output_length) {
        if (!turag_rs485_io_device->waitForReadyRead(turag_ticks_to_ms(rs485_timeout))) {
            return false;
        }
    }


    for (int i = 0; i < output_length; ++i) {
        output[i] = turag_rs485_data_buffer.at(i);
    }
    turag_rs485_data_buffer.clear();

    return true;
}
