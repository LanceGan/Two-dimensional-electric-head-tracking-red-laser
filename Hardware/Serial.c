#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>
#include "Button.h"
#include "buzzer.h"
#include "LED.h"
#include "Delay.h"
uint8_t Serial_TxPacket[4];				//FF 01 02 03 04 FE
uint8_t Serial_RxPacket[4];
uint8_t Serial_RxFlag;
int16_t Px;
int16_t Py;
float PPx=1e5+1,PPy=1e5+1;
float Ax=0,Ay=0;
extern int flag;
extern int track;
void Serial_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1, ENABLE);
}

void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)
	{
		Serial_SendByte(Array[i]);
	}
}

void Serial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)
	{
		Serial_SendByte(String[i]);
	}
}

uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y --)
	{
		Result *= X;
	}
	return Result;
}

void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)
	{
		Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
	}
}

int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);
	return ch;
}

void Serial_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	Serial_SendString(String);
}


void Serial_SendPacket(void)
{
	Serial_SendByte(0xFF);
	Serial_SendArray(Serial_TxPacket, 4);
	Serial_SendByte(0xFE);
}

uint8_t Serial_GetRxFlag(void)
{
	if (Serial_RxFlag == 1)
	{
		Serial_RxFlag = 0;
		return 1;
	}
	return 0;
}

void is_track(void)
{
	if((PPx*1000)>-100&&(PPx*1000)<100&&(PPy*1000)>-100&&(PPy*1000)<100&&track==0)
	{
		track=1;
		buzzer_on();
		LED1_ON();
		Delay_ms(500);
		LED1_OFF();
		buzzer_down();
	}
}
void K210_Data_Receive_Anl(uint8_t *data_buf,uint8_t num)
{
    uint8_t sum = 0;
    for(uint8_t i=0;i<(num-1);i++)  sum+=*(data_buf+i);
    if(!(sum==*(data_buf+num-1)))     return;//????????
    if(!(*(data_buf)==0xFF && *(data_buf+1)==0xFC))return;//???????
    Px=-(int16_t)(*(data_buf+4)<<8|*(data_buf+5));
		Py=(int16_t)(*(data_buf+6)<<8|*(data_buf+7));
		PPx=(Px/1000.0);
		PPy=(Py/1000.0);
		if(flag)
		{
			Ax+=PPx;
			Ay+=PPy;
		}
		
}

void USART1_IRQHandler(void)
{
	static uint8_t state[2] = {0};
	static uint8_t _data_len[2] = {0},_data_cnt[2] = {0};
	static uint8_t _buf[2][50];	
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		uint8_t label=0;
		uint8_t data=USART_ReceiveData(USART1);
    if(state[label]==0&&data==0xFF)//??1
    {
        state[label]=1;
        _buf[label][0]=data;
    }
    else if(state[label]==1&&data==0xFC)//??2
    {
        state[label]=2;
        _buf[label][1]=data;
    }
    else if(state[label]==2&&data<0XFF)//????
    {
        state[label]=3;
        _buf[label][2]=data;
    }
    else if(state[label]==3&&data<50)//????
    {
        state[label] = 4;
        _buf[label][3]=data;
        _data_len[label] = data;
        _data_cnt[label] = 0;
    }
    else if(state[label]==4&&_data_len>0)//???????,?????
    {
        _data_len[label]--;
        _buf[label][4+_data_cnt[label]++]=data;
        if(_data_len[label]==0) state[label] = 5;
    }
    else if(state[label]==5)//?????????
    {
        state[label] = 0;
        _buf[label][4+_data_cnt[label]]=data;
        K210_Data_Receive_Anl(_buf[label], _data_cnt[label] + 5);
        // Openmv_Data_Receive_Anl_1(_buf[label],_data_cnt[label]+5,&camera1);
    }
    else state[label] = 0;
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}
