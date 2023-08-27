#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>



void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);
void K210_Data_Receive_Anl(uint8_t *data_buf,uint8_t num);
void Serial_SendPacket(void);
uint8_t Serial_GetRxFlag(void);
void is_track(void);
void TTRu(float x);
#endif
