/**
 *  @file		feldbus_config.h
 *  @date		25.11.2013
 *  @author		Martin Oemus <martin@oemus.net>
 * 
 */
#ifndef FELDBUS_CONFIG_H_
#define FELDBUS_CONFIG_H_

#include <tina/feldbus/protocol/turag_feldbus_fuer_lokalisierungssensoren.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_bootloader.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_stellantriebe.h>
#include <tina/feldbus/protocol/turag_feldbus_fuer_aseb.h>

//#define TURAG_FELDBUS_DEVICE_PROTOCOL                           0xff
//#define TURAG_FELDBUS_DEVICE_TYPE_ID							0x00
#define TURAG_FELDBUS_DEVICE_PROTOCOL							TURAG_FELDBUS_DEVICE_PROTOCOL_BOOTLOADER
#define TURAG_FELDBUS_DEVICE_TYPE_ID							TURAG_FELDBUS_BOOTLOADER_XMEGA

#define TURAG_FELDBUS_SLAVE_CONFIG_CRC_TYPE 					TURAG_FELDBUS_CHECKSUM_CRC8_ICODE
#define TURAG_FELDBUS_SLAVE_CONFIG_BUFFER_SIZE					(80)
#define TURAG_FELDBUS_SLAVE_CONFIG_DEBUG_ENABLED				0
#define TURAG_FELDBUS_SLAVE_CONFIG_UPTIME_FREQUENCY				100
#define TURAG_FELDBUS_SLAVE_BROADCASTS_AVAILABLE				0
#define TURAG_FELDBUS_SLAVE_CONFIG_ADDRESS_LENGTH				1
#define TURAG_FELDBUS_SLAVE_CONFIG_USE_LED_CALLBACK 			0
#define TURAG_FELDBUS_SLAVE_CONFIG_PACKAGE_STATISTICS_AVAILABLE 1



#endif /* FELDBUS_CONFIG_H_ */
