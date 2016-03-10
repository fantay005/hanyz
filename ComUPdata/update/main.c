//˵����������һЩ��־�������Ĳ����������ڲ�flash�����һҳ���׵�ַ��0x0807F800
//�����ڶ�ҳ�����ڴ������ʧ�ܵĴ�������������3��ʱ�˴�����ʧ�ܣ�������־����ַ��0x0807F000
//�ڲ�flashÿһҳ�Ĵ�СΪ2K�ֽ�
//0-64K���bootloader����65-508K���Ӧ�ó���

#include <stdio.h>
#include <string.h>
#include "inside_flash.h"
#include "hardware/hardware.h"
#include "firmware.h"
#include "m35.h"
#include "m35at.h"


void runApp(unsigned int addr);
void resetBoard(void);
void IncreaseUpdateTimes(void);
extern void uart5Configure(void);
extern void usart3Configure(int baud);

int main(void)
{
	rccConfigure();
	nvicConfigure();
	gpioConfigure();
	uart5Configure();
	usart3Configure(9600);
	sysTickConfiguration();	
	
	IWDG_ReloadCounter();

	if (isNeedDownloadFirmware()) {
		
		m35Startup();
		
		if (!downloadFirmwareToRam()) {
			dprintf("downloadFirmware return false\n");
			resetBoard();
		}

	  programFromRam();
	}
		
	dprintf("runApp ......\n");
	runApp(Program_Run_Addr);
	while (1);
}


void resetBoard(void)
{
	IncreaseUpdateTimes();
	while (true) {
		NVIC_GenerateSystemReset();
	}
}

void runApp(unsigned int addr)
{
	unsigned int *p = (unsigned int *)addr;
	unsigned int sp = *p++;
	void (*func)(void) = (void (*)(void))*p;
	RCC_DeInit();
	NVIC_DeInit();	
	{
		__MSR_MSP(sp);
		func();
		while(1);
	}
}
