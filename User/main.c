#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Servo.h"
#include "Key.h"
#include "Serial.h"
#include "Button.h"
#include "Timer.h"
#include "buzzer.h"
#include "LED.h"
uint8_t KeyNum;
float Angle;
extern int16_t Px,Py;
extern float Ax,Ay;
extern float PPx,PPy;
int track=0;
int flag=0;
int main(void)
{
	Servo_Init();
	Serial_Init();
	Key_Init();
	Timer_Init();
	OLED_Init();
	stop_init();
	buzzer_init();
	Servo_SetAngle1(0);//Y,正为向上
	Servo_SetAngle2(0);//X,正为向左
//	buzzer_on();
//	buzzer_down();
	LED_Init();
	while (1)
	{
//		LED_show();
		if(Ax>90||Ax<-90)Ax=0;
		if(Ay>90||Ay<-90)Ay=0;
		OLED_ShowSignedNum(1,1,PPx*1000,8);
		OLED_ShowSignedNum(2,1,PPy*1000,8);	
		OLED_ShowSignedNum(3,1,Ax,8);
		OLED_ShowSignedNum(4,1,Ay,8);	
		read_stop();
		is_track();
	}
}

void TIM4_IRQHandler(void)
{
	static int cnt=0;
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
	{
		
		cnt++;
		if(flag)
		{
		Servo_SetAngle1(Ay);//Y
		Servo_SetAngle2(Ax);//X
		}
		
//		if(cnt==100)
//		{
//			cnt=0;
//			LED1_Turn();
//			LED2_Turn();
//		}
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	}
}
