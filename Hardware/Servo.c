#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Servo_Init(void)
{
	PWM2_Init();
//	PWM1_Init();
}


void Servo_SetAngle2(float Angle)
{
	PWM_SetCompare2(Angle*7.407f + 1500.0f);
}

void Servo_SetAngle1(float Angle)
{
	PWM_SetCompare1(Angle*7.407f+ 1500.0f);
}
