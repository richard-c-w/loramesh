/*
  This file contains general purpose functions
  Author: Richard C. W.
  Version: 0.1
*/

#include "utils.h"


uint16_t computeCRC(uint8_t* data_in, uint16_t length)
{
    uint16_t i;
    uint8_t bitbang, j;
    uint16_t crc_calc;

    crc_calc = 0xC181;
    for(i=0; i<length; i++)
    {
        crc_calc ^= (((uint16_t)data_in[i]) & 0x00FF);
        for(j=0; j<8; j++)
        {
            bitbang = crc_calc;
            crc_calc >>= 1;            
            if(bitbang & 1)
            {
                crc_calc ^= 0xA001;
            }
        }
    }
    return (crc_calc&0xFFFF);
}