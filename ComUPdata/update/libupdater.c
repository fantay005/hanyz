#include "stm32f10x_flash.h"
#include "stdbool.h"
#include "string.h"
#include "ctype.h"

void eraseupdatemark(void)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(0x0800F800);
	FLASH_Lock();
}
