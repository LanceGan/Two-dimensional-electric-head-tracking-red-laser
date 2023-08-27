#include "stm32f10x.h"                  // Device header
#include "Delay.h"
void buzzer_init()
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//使能GPIOA的时钟		
	  GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;//IO管脚模式配置为浮空输入，这样就能获取传感器传回来的数字信号(高低电平)
	  GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;
	  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	  GPIO_Init(GPIOB,&GPIO_InitStructure);//PA6,PA7管脚的初始化  
}

void buzzer_on(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_8);
}

void buzzer_down(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);
	Delay_ms(500);
}