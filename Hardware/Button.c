#include "stm32f10x.h"                  // Device header
#include "Delay.h"
extern int flag;
void stop_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPU ;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

int read_stop(void)
{
	if(!GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)) 
	{
		Delay_ms(10);
		if(!GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4))flag=!flag;
		while(!GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4));
	}
}