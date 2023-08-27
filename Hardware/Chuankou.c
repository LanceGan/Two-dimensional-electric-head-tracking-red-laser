#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>

void Serialg_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART3, &USART_InitStructure);
	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART3, ENABLE);
}

void Serialg_SendByte(uint8_t Byte)
{
	USART_SendData(USART3, Byte);
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

void Serialg_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)
	{
		Serialg_SendByte(Array[i]);
	}
}

void Serialg_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)
	{
		Serialg_SendByte(String[i]);
	}
}

uint32_t Serialg_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y --)
	{
		Result *= X;
	}
	return Result;
}

void Serialg_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)
	{
		Serialg_SendByte(Number / Serialg_Pow(10, Length - i - 1) % 10 + '0');
	}
}

int fputcg(int ch, FILE *f)
{
	Serialg_SendByte(ch);
	return ch;
}

void Serialg_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	Serialg_SendString(String);
}


//void Serial1_SendPacket(void)
//{
//	Serial1_SendByte(0xFF);
//	Serial1_SendArray(Serial_TxPacket, 4);
//	Serial1_SendByte(0xFE);
//}

//uint8_t Serial1_GetRxFlag(void)
//{
//	if (Serial_RxFlag == 1)
//	{
//		Serial_RxFlag = 0;
//		return 1;
//	}
//	return 0;
//}

//void USART3_IRQHandler(void)
//{
//	static uint8_t RxState = 0;
//	static uint8_t pRxPacket = 0;
//	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
//	{
//		uint8_t RxData = USART_ReceiveData(USART3);
//		
//		if (RxState == 0)
//		{
//			if (RxData == 0xAB)
//			{
//				RxState = 1;
//				pRxPacket = 0;
//			}
//		}
//		else if (RxState == 1)
//		{
//			Serial_RxPacket[pRxPacket] = RxData;
//			pRxPacket ++;
//			if (pRxPacket >= 4)
//			{
//				RxState = 2;
//			}
//		}
//		else if (RxState == 2)
//		{
//			if (RxData == 0xCD)
//			{
//				RxState = 0;
//				Serial_RxFlag = 1;
//			}
//		}
//		
//		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
//	}
//}
