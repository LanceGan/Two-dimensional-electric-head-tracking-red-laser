#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>

void Serialg_Init(void);
void Serialg_SendByte(uint8_t Byte);
void Serialg_SendArray(uint8_t *Array, uint16_t Length);
void Serialg_SendString(char *String);
void Serialg_SendNumber(uint32_t Number, uint8_t Length);
void Serialg_Printf(char *format, ...);



#endif
