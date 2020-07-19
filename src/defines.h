/*
  This file contains structs and defines used by Radioenge LoraMesh module
  Author: Richard C. W.
  Version: 0.1
*/

#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <stdint.h>

#ifndef CUSTOM_SERIAL
  #include "SoftwareSerial.h"
  #define SerialPort SoftwareSerial
#endif
/* RX/TX Buffer sizes */
#define MAX_PAYLOAD_SIZE 232 
#define MAX_BUFFER_SIZE (MAX_PAYLOAD_SIZE + 5)

/* Packet fields definition */
#define RADIO_ID_LSB_POS 0
#define RADIO_ID_MSB_POS 1
#define RADIO_CMD_POS    2
#define RADIO_DATA_POS   3

/*Size of local read pacote response size */
#define SIZEOF_LOCALREAD_RESPONSE 31
#define SIZEOF_GET_MODULATION_RESPONSE 10
/* Invalid packets */
#define INVALID_ANALOG_READ (uint16_t)0xFFFF
#define INVALID_DIGITAL_READ  0xFF


/* GPIO Enum */
typedef uint8_t GPIO_Typedef;

/* -----  Default Comands List ----- */
typedef enum
{
	RADIO_CMD_LORAPARAMETER   = 0xD6,   /* Gets or Sets the LoRa modulation parameters */
	RADIO_CMD_LOCALREAD       = 0xE2,   /* Gets the ID, NET and UNIQUE ID info from the local device */
	RADIO_CMD_REMOTEREAD      = 0xD4,   /* Gets the ID, NET and UNIQUE ID info from a remote device */
	RADIO_CMD_GET_MODULATION  = 0xD6,   /* Gets modulation params */
	RADIO_CMD_WRITECONFIG     = 0xCA,   /* Writes configuration info to the device, i.e. NET and ID */
	RADIO_CMD_SET_PASSWORD    = 0xCD,   /* Sets a new password of Radio communication */
	RADIO_CMD_DIAGNOSIS       = 0xE7,   /* Gets diagnosis information from the device */
	RADIO_CMD_GPIOCONFIG      = 0xC2,   /* Configures a given GPIO pin to a desired mode, gets or sets its value */
	RADIO_CMD_READRSSI        = 0xD5,   /* Reads the RSSI between the device and a neighbour */
	RADIO_CMD_READNOISE       = 0xD8,   /* Reads the noise floor on the current channel */
	RADIO_CMD_TRACEROUTE      = 0xD2,   /* Traces the hops from the device to the master */
	RADIO_CMD_SENDTRANSP      = 0x28    /* Sends a packet to the device's transparent serial port */
} radio_cmd_t;


/* LoraMesh GPIO Pins */
typedef enum {
	RADIO_GPIO0 = 0,
	RADIO_GPIO1,
	RADIO_GPIO2,
	RADIO_GPIO3,
	RADIO_GPIO4,
	RADIO_GPIO5,
	RADIO_GPIO6,
	RADIO_GPIO7
} radio_gpio_t;

/* GPIO mode enum */
typedef enum
{
	RADIO_INPUT,
	RADIO_OUTPUT,
	RADIO_ANALOG_IN = 3
} radio_mode_t;

/* Pull resistor enum */
typedef enum
{
	RADIO_PULL_OFF,
	RADIO_PULLUP,
	RADIO_PULLDOWN
} radio_pull_t;

/* Result of operations*/
typedef enum
{
	RADIO_OK,
	RADIO_ERROR,
	RADIO_TIMEOUT_ERROR,
RADIO_CRC_ERROR
} radio_result_t;

/* Modulation options*/
typedef enum
{
	MODULATION_LORA = 0,
	MODULATION_FSK
} modulation_t;

/* bandwidth options */
typedef enum
{
	BANDWIDTH_125_kHz = 0,
	BANDWIDTH_250_kHz,
	BANDWIDTH_500_kHz
} bandwidth_t;

/* Coding Rates options */
typedef enum
{
	CODING_RATE_4_5 = 1,
	CODING_RATE_4_6 = 2,
	CODING_RATE_4_7 = 3,
	CODING_RATE_4_8 = 4
} coding_rate_t;

/* Spreading Factor options */
typedef enum
{
	SPREAD_FACTOR_FSK = 5,
	SPREAD_FACTOR_7 = 7,
	SPREAD_FACTOR_8,
	SPREAD_FACTOR_9,
	SPREAD_FACTOR_10,
	SPREAD_FACTOR_11,
	SPREAD_FACTOR_12,
} sp_factor_t;

/* Device attributes */
typedef struct device_t
{
    uint16_t id;
    uint8_t uniqueId[4];
    uint8_t net;
    uint8_t hwVersion;
    uint8_t fwVersion;
    uint8_t fwRevision;
    modulation_t modulation;
    char bank;
} device_t;

typedef struct mod_param_t
{
	uint8_t power;
	bandwidth_t bandwidth;
	sp_factor_t spreadFactor;
 	coding_rate_t codingRate;
} mod_param_t;

#endif