#ifndef TINA_CHIBIOS_RS485_H
#define TINA_CHIBIOS_RS485_H

#include <tina/time.h>

#ifdef __cplusplus
extern "C" {
#endif

bool turag_rs485_init(uint32_t baud_rate, TuragSystemTime timeout);
bool turag_rs485_transceive(uint8_t* input, int input_length, uint8_t* output, int output_length);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // TINA_CHIBIOS_RS485_H
