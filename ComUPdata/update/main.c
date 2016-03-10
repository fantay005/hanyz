//说明：升级的一些标志和升级的参数都放在内部flash的最后一页里首地址是0x0807F800
//倒数第二页的用于存放升级失败的次数，次数大于3次时此次升级失败，擦除标志，地址是0x0807F000
//内部flash每一页的大小为2K字节
//0-64K存放bootloader程序65-508K存放应用程序

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
