#include "stm32f10x_rcc.h"
#include "stm32f10x_iwdg.h"


void iwdgConfigure(void)
{
	// д��0x5555,�����������Ĵ���д�빦�� 
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);	
	// ����ʱ�ӷ�Ƶ,40K/256=156HZ(6.4ms)  ���Է�Ϊ 4��8��16��32��64��128��256
	IWDG_SetPrescaler(IWDG_Prescaler_256);
	//ι��ʱ�� 1s=156 @IWDG_Prescaler_256.ע�ⲻ�ܴ���4096
	IWDG_SetReload(1560);     //10��
	IWDG_Enable();	
}
