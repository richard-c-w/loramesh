/*
  Header file of RadioengeLoraMesh Class
  Author: Richard C. W.
  Version: 0.1
*/
#ifndef __RADIOENGE_LORAMESH_H__
#define __RADIOENGE_LORAMESH_H__


#include "Arduino.h"
#include "defines.h"

#define DEBUG_RADIOENGE


#define RADIO_SYNC_FUNCTIONS 1

/* Buffer structure */
typedef struct packet_t
{
    uint8_t data[MAX_BUFFER_SIZE];
    uint16_t size;
} packet_t;

class RadioengeLoraMesh
{
    public:
        /*Constructor*/
        RadioengeLoraMesh(uint8_t rxPin, uint8_t txPin, uint32_t baudRate = 9600);        
        void begin(); // Serial initialization
        uint8_t available(); //Data available for processing

        /**
         * Synchronous functions:
         * It sends and waits for response.
         * In case of success it returns RADIO_OK,
         * otherwise it returns the code of error.
        */
#ifdef RADIO_SYNC_FUNCTIONS
        void setResponseTimeout(uint32_t millis); //Timeout for sync responses        
        radio_result_t localRead(device_t* device);
        radio_result_t remoteRead(device_t* device, uint8_t id);
        radio_result_t writeConfig(device_t* device);
        radio_result_t setPassword(uint16_t id, uint32_t* password);
        radio_result_t getModulation(uint16_t id, mod_param_t* modulation);
        radio_result_t pinMode(uint16_t id, uint8_t pin, uint8_t mode, uint8_t pull);
        radio_result_t digitalWrite(uint16_t id, uint8_t pin, uint8_t value);   
        int8_t digitalRead(uint16_t id, uint8_t pin);
        uint16_t analogRead(uint16_t id, uint8_t pin);    
#endif

        /**
         * Synchronous functions:
         * It sends and waits for response.
         * In case of success it returns RADIO_OK,
         * otherwise it returns the code of error.
        */
        radio_result_t localReadAsync();
        radio_result_t writeConfigAsync(device_t* device);
        radio_result_t setPasswordAsync(uint16_t id, uint32_t* password);
        radio_result_t getModulationAsync(uint16_t id);
        radio_result_t remoteReadAsync(uint8_t id);
        radio_result_t pinModeAsync(uint16_t id, uint8_t pin, uint8_t mode, uint8_t pull);
        radio_result_t digitalWriteAsync(uint16_t id, uint8_t pin, uint8_t value);
        radio_result_t digitalReadAsync(uint16_t id, uint8_t pin);
        radio_result_t analogReadAsync(uint16_t id, uint8_t pin);
        radio_result_t awaitForResponse(); 

        radio_result_t sendPacket(uint16_t id, uint8_t command, uint8_t* payload, uint8_t size);
        uint16_t readPacket(uint8_t buffer[]);
        uint16_t readBytes(uint8_t buffer[], uint8_t* command);
        uint16_t readMessage(uint8_t buffer[], uint8_t &command, uint16_t &dsc);
        
    private:
        uint8_t _rxPin;
        uint8_t _txPin;
        uint32_t _baudRate;
        uint32_t _timeout;

        SerialPort* uartComandos;

        packet_t packet;
        
        void mountReadPacket(device_t* device);
        radio_result_t checkPacketIntegrity();
};

#endif