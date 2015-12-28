#ifndef __LIBUPDATER_H__
#define __LIBUPDATER_H__

#include "stm32f10x_flash.h"

#define __markSavedAddr 0x0800F800

static const unsigned int __RequiredFlag = 0xF8F88F8F;

struct UpdateMark {
	unsigned int RequiredFlag;
	unsigned int SizeOfPAK;
	unsigned int type;
	unsigned int timesFlag[5];
}; 

typedef enum{
	GWPACKET = 2,
	ELECPACKET,
	BALLASTPACKET,	
	STANDBYPACKET,
	EXTRAPACKET,
}UpgrateType;

static struct UpdateMark *const __mark = (struct UpdateMark *)__markSavedAddr;

void eraseupdatemark(void);

#endif
