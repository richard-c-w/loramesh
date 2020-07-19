#include "RadioengeLoraMesh.h"
#include "utils.h"

#ifdef DEBUG_RADIOENGE    
#endif


RadioengeLoraMesh::RadioengeLoraMesh(uint8_t rxPin, uint8_t txPin, uint32_t baudRate)
{
    this->_rxPin = rxPin;
    this->_txPin = txPin;
    this->_baudRate = baudRate;
    this->packet.size = 0;
    this->_timeout = 5000;
}

void RadioengeLoraMesh::begin()
{
    static SerialPort uart(this->_rxPin, this->_txPin);    
    uart.begin(this->_baudRate);
    this->uartComandos = &uart;
}

uint8_t RadioengeLoraMesh::available()
{
    if(uartComandos == NULL)
    {
        return 0;
    }
   
    if(packet.size != 0) //Packet without processing
    {
        return packet.size;
    }
    while(uartComandos->available() && packet.size < MAX_BUFFER_SIZE)
    {
        packet.data[packet.size++] = uartComandos->read();
        delay(2);
    }
    if(checkPacketIntegrity() != RADIO_OK)
    {
        packet.size = 0;
    }
    return packet.size;
}

#ifdef RADIO_SYNC_FUNCTIONS

void RadioengeLoraMesh::setResponseTimeout(uint32_t millis)
{
    this->_timeout = millis;
}

radio_result_t RadioengeLoraMesh::localRead(device_t* device)
{
    if(localReadAsync() != RADIO_OK) //Sends a local read command
    {
        return RADIO_ERROR;
    }
    
    if(awaitForResponse() != RADIO_OK)
    {
        return RADIO_TIMEOUT_ERROR; //Response timeout
    }    
    if(packet.data[RADIO_CMD_POS] != RADIO_CMD_LOCALREAD || packet.size != SIZEOF_LOCALREAD_RESPONSE)
    {
        return RADIO_ERROR; //Wrong response
    }
    mountReadPacket(device);
    return RADIO_OK;
}

radio_result_t RadioengeLoraMesh::remoteRead(device_t* device, uint8_t id)
{
    remoteReadAsync(id); //Sends a local read command
    
    if(awaitForResponse() != RADIO_OK)
    {
        return RADIO_TIMEOUT_ERROR; //Response timeout
    }    
    if(packet.data[RADIO_CMD_POS] != RADIO_CMD_REMOTEREAD || packet.size != SIZEOF_LOCALREAD_RESPONSE)
    {
        return RADIO_ERROR; //Wrong response
    }
    mountReadPacket(device);
    return RADIO_OK;
}

radio_result_t RadioengeLoraMesh::writeConfig(device_t* device)
{
    if(writeConfigAsync(device) != RADIO_OK) //Sends a local read command
    {
        return RADIO_ERROR;
    }

    if(awaitForResponse() != RADIO_OK)
    {
        return RADIO_TIMEOUT_ERROR; //Response timeout
    }    
    if(packet.data[RADIO_CMD_POS] != RADIO_CMD_WRITECONFIG || packet.size != SIZEOF_LOCALREAD_RESPONSE)
    {
        return RADIO_ERROR; //Wrong response
    }
    mountReadPacket(device);
    return RADIO_OK;
}

radio_result_t RadioengeLoraMesh::setPassword(uint16_t id, uint32_t* password)
{
    setPasswordAsync(id, password); 
    
    if(awaitForResponse() != RADIO_OK)
    {
        return RADIO_TIMEOUT_ERROR; //Response timeout
    }
    return RADIO_OK;
}

radio_result_t RadioengeLoraMesh::getModulation(uint16_t id, mod_param_t* modulation)
{
    if(getModulationAsync(id) != RADIO_OK) //Request modulation parameters
    {
        return RADIO_ERROR;
    }

    if(awaitForResponse() != RADIO_OK)
    {
        return RADIO_TIMEOUT_ERROR; //Response timeout
    } 

    if(packet.data[RADIO_CMD_POS] != RADIO_CMD_GET_MODULATION || packet.size != SIZEOF_GET_MODULATION_RESPONSE)
    {
        return RADIO_ERROR; //Wrong response
    }
    modulation->power = packet.data[RADIO_DATA_POS + 1];
    modulation->bandwidth = packet.data[RADIO_DATA_POS + 2];
    modulation->spreadFactor = packet.data[RADIO_DATA_POS + 3];
    modulation->codingRate = packet.data[RADIO_DATA_POS + 4];
    return RADIO_OK;
}

radio_result_t RadioengeLoraMesh::pinMode(uint16_t id, uint8_t pin, uint8_t mode, uint8_t pull)
{
    pinModeAsync(id, pin, mode, pull);

    radio_result_t result = awaitForResponse();
    if(result != RADIO_OK)
    {
        return result;
    }
    if(packet.data[RADIO_CMD_POS] != RADIO_CMD_GPIOCONFIG)
    {
        return RADIO_ERROR;
    }
    /*0 -> Success, 1 -> Error*/
    if(packet.data[RADIO_DATA_POS + 2] != pin)
    {
        return RADIO_ERROR;
    }
    if(packet.data[RADIO_DATA_POS + 1] == 0)
    {
        return RADIO_OK;
    }
    return RADIO_ERROR;
}

radio_result_t RadioengeLoraMesh::digitalWrite(uint16_t id, uint8_t pin, uint8_t value)
{
    uint16_t expectedValue;    
    if(digitalWriteAsync(id, pin, value) != RADIO_OK)
    {
        return RADIO_ERROR;
    }
    if(awaitForResponse() != RADIO_OK)
    {
        return RADIO_ERROR;
    }
    expectedValue =  packet.data[RADIO_DATA_POS + 3] & 0x0F;
    expectedValue <<= 8;
    expectedValue =  packet.data[RADIO_DATA_POS + 4] & 0xFF;
    if(expectedValue > 1 || ((uint8_t)expectedValue) != value)
    {
        return RADIO_ERROR;
    }
    if(packet.data[RADIO_DATA_POS + 1] == 0)
    {
        return RADIO_OK;
    }
    return RADIO_ERROR;    
}

int8_t RadioengeLoraMesh::digitalRead(uint16_t id, uint8_t pin)
{
    if(analogReadAsync(id, pin) != RADIO_OK)
    {
        return INVALID_DIGITAL_READ;
    }
    if(awaitForResponse() != RADIO_OK)
    {
        return INVALID_DIGITAL_READ;
    }

    if(packet.data[RADIO_CMD_POS] != RADIO_CMD_GPIOCONFIG || packet.data[RADIO_DATA_POS] != 0x00)
    {
        return INVALID_DIGITAL_READ; //Wrong response
    }

    if(packet.data[RADIO_DATA_POS +  2] != pin || (packet.data[RADIO_DATA_POS +  3] & 0x80) != 0x80)
    {
        return INVALID_DIGITAL_READ;
    }
        
    if(packet.data[RADIO_DATA_POS + 1] == 0)
    {
        return packet.data[RADIO_DATA_POS + 4] & 0x01;
    }
    return INVALID_DIGITAL_READ;
}

uint16_t RadioengeLoraMesh::analogRead(uint16_t id, uint8_t pin)
{
    uint16_t read = INVALID_ANALOG_READ;
    if(analogReadAsync(id, pin) != RADIO_OK)
    {
        return INVALID_ANALOG_READ;
    }
    if(awaitForResponse() != RADIO_OK)
    {
        return INVALID_ANALOG_READ;
    }

    if(packet.data[RADIO_CMD_POS] != RADIO_CMD_GPIOCONFIG || packet.data[RADIO_DATA_POS] != 0x00)
    {
        return INVALID_ANALOG_READ; //Wrong response
    }

    if(packet.data[RADIO_DATA_POS +  2] != pin || (packet.data[RADIO_DATA_POS +  3] & 0x80) == 0x80)
    {
        return INVALID_ANALOG_READ;
    }     
    if(packet.data[RADIO_DATA_POS + 1] == 0)
    {
        read = packet.data[RADIO_DATA_POS + 3] & 0x0F;
        read <<= 8;
        read |= packet.data[RADIO_DATA_POS + 4] & 0xFF;  
        return read;
    }
    return INVALID_ANALOG_READ;
    
    
    
}

#endif

radio_result_t RadioengeLoraMesh::localReadAsync()
{
    uint8_t payload[] = {0x00, 0x00, 0x00};
    return sendPacket(0, RADIO_CMD_LOCALREAD, payload, sizeof(payload));
}

radio_result_t RadioengeLoraMesh::writeConfigAsync(device_t* device)
{
    if(device->net > 2047) return RADIO_ERROR;
    
    uint8_t payload[11] = 
    {
        0, 0, 
        device->uniqueId[0],
        device->uniqueId[1], 
        device->uniqueId[2], 
        device->uniqueId[3],
        0, 0, 0, 0, 0};
    return sendPacket(device->id, RADIO_CMD_WRITECONFIG, payload, sizeof(payload));
}

radio_result_t RadioengeLoraMesh::setPasswordAsync(uint16_t id, uint32_t* password)
{
    if(id > 1023) return RADIO_ERROR;
    uint8_t payload[5] = 
    {
        4,
        (uint8_t) (*password & 0xFF),
        (uint8_t) (*password  >>  8),
        (uint8_t) (*password  >>  16),
        (uint8_t) (*password  >>  16)

    };
    return sendPacket(id, RADIO_CMD_SET_PASSWORD, (uint8_t*)payload, sizeof(payload));
}

radio_result_t RadioengeLoraMesh::getModulationAsync(uint16_t id)
{
    uint8_t payload[3] = {0x00, 0x01, 0x00};
    return sendPacket(id, RADIO_CMD_GET_MODULATION, payload, sizeof(payload));
}

radio_result_t RadioengeLoraMesh::remoteReadAsync(uint8_t id)
{
    uint8_t payload[] = {0x00, 0x00, 0x00};
    if(id > 1023)
    {
        return RADIO_ERROR;
    }
    sendPacket(id, RADIO_CMD_REMOTEREAD, payload, sizeof(payload));
    return RADIO_OK;
}

radio_result_t RadioengeLoraMesh::pinModeAsync(uint16_t id, uint8_t pin, uint8_t mode, uint8_t pull)
{
    uint8_t payload[] = {0x02, pin, pull, mode};
    return sendPacket(id, RADIO_CMD_GPIOCONFIG, payload, 4);
}

radio_result_t RadioengeLoraMesh::digitalWriteAsync(uint16_t id, uint8_t pin, uint8_t value)
{
  if(id > 1023) return RADIO_ERROR;
  if(pin > RADIO_GPIO7) return RADIO_ERROR;
  if(uartComandos == NULL) return RADIO_ERROR;
  if(value != 0) value = 1;
  uint8_t payload[] = {0x01, pin, value, 0};
  return sendPacket(id, RADIO_CMD_GPIOCONFIG, payload, sizeof(payload));
}

radio_result_t RadioengeLoraMesh::digitalReadAsync(uint16_t id, uint8_t pin)
{
    if(id > 1023) return RADIO_ERROR;
    if(pin > RADIO_GPIO7) return RADIO_ERROR;
    if(uartComandos == NULL) return RADIO_ERROR;
    uint8_t payload[] = {0x00, pin, 0};
    return sendPacket(id, RADIO_CMD_GPIOCONFIG, payload, sizeof(payload));
}

radio_result_t RadioengeLoraMesh::analogReadAsync(uint16_t id, uint8_t pin)
{
    if(id > 1023) return RADIO_ERROR;
    if(pin > RADIO_GPIO7) return RADIO_ERROR;
    if(uartComandos == NULL) return RADIO_ERROR;
    uint8_t payload[] = {0x00, pin, 0};
    return sendPacket(id, RADIO_CMD_GPIOCONFIG, payload, sizeof(payload));
}

radio_result_t RadioengeLoraMesh::awaitForResponse()
{
    uint32_t remaining = _timeout;
    packet.size = 0;
    while(!uartComandos->available() && remaining > 0)
    {
        remaining--;
        delay(1);
    }
    if(uartComandos->available() == 0 && remaining == 0)
    {
        return RADIO_TIMEOUT_ERROR;
    }
    remaining = 300;
    while(remaining > 0)
    {
        remaining--;
        delay(1);
        while(uartComandos->available() > 0)
        {
            packet.data[packet.size++] = uartComandos->read();
            remaining = 300;
        }
    }

    
    
    if(checkPacketIntegrity() != RADIO_OK)
    {
        return RADIO_ERROR;
    }    
    return RADIO_OK;
}


void RadioengeLoraMesh::mountReadPacket(device_t* device)
 {
     /* Sets device's ID */
    device->id = packet.data[RADIO_ID_MSB_POS];
    device->id <<= 8;
    device->id |= packet.data[RADIO_ID_LSB_POS] & 0xFF;
    
    /* Sets device's net */
    device->net = (packet.data[RADIO_DATA_POS + 1] & 0xFF);
    device->net <<= 8;
    device->net |= (packet.data[RADIO_DATA_POS] & 0xFF);

    device->hwVersion = packet.data[RADIO_DATA_POS + 6];
    device->fwVersion = packet.data[RADIO_DATA_POS + 12];
    device->fwRevision = packet.data[RADIO_DATA_POS + 8];
    device->bank = 'A' +packet.data[RADIO_DATA_POS + 13];
    device->modulation = packet.data[RADIO_DATA_POS + 10];
    
    /*Set unique id from device*/
    memcpy(device->uniqueId, &packet.data[RADIO_DATA_POS + 2], 4);
 }

radio_result_t RadioengeLoraMesh::checkPacketIntegrity()
{
    if(packet.size < 5)
    {
        return RADIO_ERROR;
    }
    uint16_t crc = (uint16_t)packet.data[packet.size - 2] | ((uint16_t)packet.data[packet.size - 1] << 8);
    
    if(computeCRC(packet.data, packet.size - 2) != crc)
    {
        return RADIO_ERROR;
    }
    return RADIO_OK;
}

radio_result_t RadioengeLoraMesh::sendPacket(uint16_t id, uint8_t command, uint8_t* payload, uint8_t length)
{
    uint16_t crc;
    if(payload == NULL)
    {
        return RADIO_ERROR;
    }
    if(id > 1023)
    {
        return RADIO_ERROR;
    }
    if(length > MAX_PAYLOAD_SIZE)
    {
        return RADIO_ERROR;
    }
    if(uartComandos == NULL)
    {
        return RADIO_ERROR;
    }
    this->packet.data[RADIO_ID_LSB_POS] = (id & 0xFF);
    this->packet.data[RADIO_ID_MSB_POS] = ((id >> 8) & 0x03);
    this->packet.data[RADIO_CMD_POS] = command;
    
    memcpy(&this->packet.data[RADIO_DATA_POS], payload, length);

    crc = computeCRC(packet.data, length + 3);
    this->packet.data[length + 3] = (crc & 0xFF);
    this->packet.data[length + 4] = ((crc >> 8) & 0xFF);
    this->packet.size = 0;
    uartComandos->write(this->packet.data, length + 5);
    return RADIO_OK;
}

uint16_t RadioengeLoraMesh::readBytes(uint8_t buffer[], uint8_t* command)
{
    int ret = 0;
    uint16_t id = packet.data[RADIO_ID_LSB_POS];
    id <<= 8;
    id |= packet.data[RADIO_ID_MSB_POS];
    ret = packet.size - 5;
    command[0] = packet.data[RADIO_CMD_POS];
    memcpy(buffer, &packet.data[3], ret);
    packet.size = 0;
    return ret;
}

uint16_t RadioengeLoraMesh::readPacket(uint8_t buffer[])
{
    if(packet.size == 0)
    {
        return 0;
    }
    uint16_t ret = packet.size;
    memcpy(buffer, packet.data, packet.size);
    packet.size = 0;
    return ret;
}

uint16_t RadioengeLoraMesh::readMessage(uint8_t buffer[], uint8_t &command, uint16_t &dst)
{
    int ret = 0;
    if(packet.size < 5)
    {
        return 0;
    }
    ret = packet.size - 5;
    command = packet.data[RADIO_CMD_POS];
    dst = (packet.data[RADIO_ID_MSB_POS] & 0xFFF);
    dst = (dst << 8);
    dst |= (packet.data[RADIO_ID_LSB_POS] & 0xFF);
    memcpy(buffer, &packet.data[3], ret);
    packet.size = 0;
    return ret;
}


