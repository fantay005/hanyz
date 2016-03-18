#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "m35ftp.h"
#include "m35at.h"
#include "inside_flash.h"
#include "libupdater.h"

#define WRITEIN_SIZE   1024

typedef struct {
	unsigned char header;
	unsigned char addr[10];
	unsigned char contr[2];
	unsigned char lenth[2];
} ProtocolHead;

static char RecBuffer[1024];

static char Frame_Buf[32];

extern void delayMs(unsigned int ms);

char *ProtocolMessage(unsigned char address[10], unsigned char  type[2], const char *msg) {
	unsigned char i;
	unsigned int verify = 0;
	unsigned char *p, *ret;
	unsigned char hexTable[] = "0123456789ABCDEF";
	unsigned char len = ((msg == NULL) ? 0 : strlen(msg));
	
	if(type[1] > '9'){
		i = (unsigned char)(type[0] << 4) | (type[1] - 'A' + 10);
	} else{
		i = (unsigned char)(type[0] << 4) | (type[1] & 0x0f);
	}
	
	ret = (unsigned char *)Frame_Buf;
	
	{
		ProtocolHead *h = (ProtocolHead *)ret;
		h->header = 0x02;	
		strcpy((char *)h->addr, (const char *)address);
		h->contr[0] = hexTable[i >> 4];
		h->contr[1] = hexTable[i & 0x0F];
		h->lenth[0] = hexTable[(len >> 4) & 0x0F];
		h->lenth[1] = hexTable[len & 0x0F];
	}

	if (msg != NULL) {
		strcpy((char *)(ret + 15), msg);
	}
	
	p = ret;
	
	for (i = 0; i < (len + 15); ++i) {
		verify ^= *p++;
	}
	
	*p++ = hexTable[(verify >> 4) & 0x0F];
	*p++ = hexTable[verify & 0x0F];
	*p++ = 0x03;
	*p = 0;
	
	return Frame_Buf;
}

bool m35FtpDownload(void)
{
	char buf[64];
	int i = 0, leftLen, writein = 0;

	leftLen = __mark->SizeOfPAK;
	
	while(1){	
		writein = (leftLen > WRITEIN_SIZE) ? WRITEIN_SIZE : leftLen;		
		sprintf(buf, "%02d%04d", i + 1, writein);		
		ProtocolMessage("9999999999", "40", buf);

		if (!m35AtChat(Frame_Buf, NULL, buf, 500)) return false;		
		
		m35ReceiveData(RecBuffer, writein, 8000);
		STMFLASH_Write(Download_Store_Addr + i * 1024, (uint16_t *)RecBuffer, writein / 2);
		i++;
		leftLen -= writein;
		if (writein < 1024)
			break;
		delayMs(300);		
	}
	return true;
}
