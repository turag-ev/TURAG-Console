/**
 *  @brief		Base64 encoding/decoding routines
 *  @file		tina/utils/base64.h
 *  @date		02.12.2013
 *  @author		Martin Oemus <martin@oemus.net>
 *
 */

/**
 * @defgroup base64 Base64 encoder/decoder
 * @ingroup utilities
 * 
 * Base64 is a group of similar binary-to-text encoding schemes that 
 * represent binary data in an ASCII string format by translating it 
 * into a radix-64 representation. The term Base64 originates from a 
 * specific MIME content transfer encoding.
 * 
 * \warning These functions do not comply with any RFC as it does not
 * produce any trailing `=' padding characters!\n It is not possible to 
 * concatenate resulting encoded strings.
 * 
 * \see http://en.wikipedia.org/wiki/Base64
 */

#ifndef TURAG_CONSOLE_UTIL_GENERATE_TINA_GRAPH_DATA_SRC_BASE64_H
#define TURAG_CONSOLE_UTIL_GENERATE_TINA_GRAPH_DATA_SRC_BASE64_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup base64
 * @{
 */
 

/**
 * @brief Calculates the length of base64 encoded data.
 * @param[in] len Length of data that is to be encoded
 * @return length of base64 encoded data
 */
unsigned turag_base64_encode_len(unsigned len);

/**
 * @brief Encodes data to base64 encoded data.
 * @param[in] data data to encode
 * @param[in] len length of data
 * @param[out] encoded buffer holding base64 encoded data
 * @return length of base64 encoded data
 */
int turag_base64_encode(const uint8_t *data, unsigned len, uint8_t *encoded);


/**
 * @brief Calculates length of decoded data.
 * @param[in] len length of base64 encoded data
 * @return length of decoded data
 */
unsigned turag_base64_decode_len(unsigned len);

/**
 * @brief Decodes base64 encoded data.
 * @param[in] encoded base64 encoded data
 * @param[in] len length of base64 encoded data
 * @param[out] data buffer holding decoded data
 * @return Length of decoded data
 */
int turag_base64_decode(const uint8_t *encoded, unsigned len, uint8_t *data);

/**
 * @}
 */


#ifdef __cplusplus
} // extern "C"
#endif


#endif // TURAG_CONSOLE_UTIL_GENERATE_TINA_GRAPH_DATA_SRC_BASE64_H
