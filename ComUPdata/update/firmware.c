#include <string.h>
#include <ctype.h>
#include "firmware.h"
#include "m35at.h"
#include "m35.h"
#include "hardware.h"
#include "m35fs.h"
#include "m35ftp.h"
#include "m35sendsms.h"
#include "inside_flash.h"
#include "libupdater.h"

typedef struct {
	unsigned int headerCrc;
	unsigned int headerLength;
	unsigned int fileLength;
	unsigned int startAddr;
	unsigned int resolved[4];
	unsigned int crc[256];
}Header;

#define BLOCK_SIZE 2048
#define HEADER_SIZE 32

unsigned short header_Buf[512];
unsigned int block_Buf[BLOCK_SIZE/sizeof(unsigned int)];

#include "libupdater.h"

int updateTimes() {
	int ret = 0;
	int i;
	for (i=0; i<sizeof(__mark->timesFlag)/sizeof(__mark->timesFlag[0]); ++i) {
		if (__mark->timesFlag[i] != 0xFFFFFFFF) {
			++ret;
		}
	}
	return ret;
}

void IncreaseUpdateTimes(void) {
	int i;
	for (i=0; i < sizeof(__mark->timesFlag)/sizeof(__mark->timesFlag[0]); i++) {
		if (__mark->timesFlag[i] == 0xFFFFFFFF) {
			FLASH_Unlock();
			FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);  
			FLASH_ProgramWord((u32)&__mark->timesFlag[i],0x00);
			FLASH_Lock();
			return;
		}
	}
}

bool isNeedDownloadFirmware(void)
{	
	if (__mark->RequiredFlag != __RequiredFlag) {
		dprintf("Don't need to upgrade -->\n");
		return false;
	}
	
	if (updateTimes() >= 5) {
		return false;
	}
	
	return true;
}

bool downloadFirmwareToRam(void)
{
	bool rc = false;
	
	if (!m35FtpDownload()) {
		eprintf("S1 FTP DOWNLOAD ERROR\n");
	} else {
		rc = true;
	}
	return rc;
}

static unsigned int calculateCrc32Stm32(void *buf, int len)
{
	unsigned int xbit, data;
	unsigned int polynomial = 0x04c11db7;
	unsigned int crc = 0xFFFFFFFF;    // init
	unsigned int *ptr = (unsigned int *)buf;

	len = len / 4;
	
	while (len--) {
			data = *ptr++;
			for (xbit = 0x80000000; xbit != 0; xbit = xbit>>1) {
					if (crc & 0x80000000) {
						crc <<= 1;
						crc ^= polynomial;
					} else {
							crc <<= 1;
					}

					if (data & xbit) {
							crc ^= polynomial;
					}
			}
	}
  return crc;
}

#define EraseTimeout             ((u32)0x00000FFF)
#define ProgramTimeout           ((u32)0x0000000F)

// 把一个BLOCK写入到内部FLASH并校验写入是否正确
static bool writeBlockDataToFlash(unsigned int *buf, int addr, int len)
{
	int t = 100;
	unsigned int *p = buf;
	unsigned int paddr = addr;
	int leftLen = len;
	int status;
	
	status = FLASH_ErasePage(addr);

	while ((status == FLASH_BUSY || status == FLASH_TIMEOUT) && t > 0) {
		status = FLASH_WaitForLastOperation(EraseTimeout);
	}

	if (status !=  FLASH_COMPLETE) return false;

	for (; leftLen > 0; leftLen -= 4, paddr += 4) {
		t = 100;
		status = FLASH_ProgramWord(paddr, *p++);
		while ((status == FLASH_BUSY || status == FLASH_TIMEOUT) && t > 0) {
			status = FLASH_WaitForLastOperation(ProgramTimeout);
		}
		if (status != FLASH_COMPLETE) return false;
	}

	if (0 != memcmp(buf, (void *)addr, len)) return false;

	return true;
}

extern char *ProtocolMessage(unsigned char address[10], unsigned char  type[2], const char *msg);

// 校验下载文件，并写入代码区
bool programFromRam(void)
{
	int readLen;
	int blockNum;
	int fileLeftLen;
	int block;
	unsigned int addr;
	bool rc = true;
	unsigned int crc;
	Header *m_header;
	char *ret;
	
	IWDG_ReloadCounter();//喂狗
	dprintf("Read header ...\n");
	
	STMFLASH_Read(Download_Store_Addr, header_Buf, HEADER_SIZE * 8);
	
	m_header = (Header *)header_Buf;
	dprintf("headerCrc: %08X\n", m_header->headerCrc);
	dprintf("headerLength: %08X\n", m_header->headerLength);
	dprintf("fileLength: %08X\n", m_header->fileLength);
	dprintf("startAddr: %08X\n", m_header->startAddr);
	
	if (m_header->startAddr != Program_Run_Addr) {
		goto __exit;
	}
	
	if (m_header->fileLength > 1024*(512-64-4)) {
		goto __exit;
	}

	blockNum = (m_header->fileLength + BLOCK_SIZE - 1)/BLOCK_SIZE;
	addr = m_header->startAddr;

	if ((addr & (BLOCK_SIZE - 1)) != 0) {
		eprintf("S2 START ADDR ERROR\n");
		goto __exit;
	}

	if (m_header->headerLength != HEADER_SIZE + blockNum * 4) {
		eprintf("S2 HEADER SIZE ERROR\n");
		goto __exit;
	}
	
	dprintf("Calc header ...\n");	
	if ((crc = calculateCrc32Stm32(&(m_header->headerLength), m_header->headerLength - 4)) != m_header->headerCrc) {
		eprintf("Header CRC error expect %08X but %08X\n", m_header->headerCrc, crc);
		goto __exit;
	}

	rc = false;

	fileLeftLen = m_header->fileLength;
//	rc = true;
	IWDG_ReloadCounter();//喂狗
	FLASH_Unlock();
	for(block = 0; block < blockNum; block++) {
		readLen = (fileLeftLen > BLOCK_SIZE) ? BLOCK_SIZE : fileLeftLen;
		
		dprintf("Read block %d, len: %d ...\n", block, readLen);

		STMFLASH_Read_Word((Download_Store_Addr + block * 2048 + HEADER_SIZE + blockNum * 4), block_Buf, (readLen + 3) / 4);
		
		if (readLen != BLOCK_SIZE) {
			unsigned char *p = (unsigned char *)block_Buf;
			memset(&p[readLen], 0, BLOCK_SIZE - readLen);
		}

		if((crc = calculateCrc32Stm32(block_Buf, BLOCK_SIZE)) != m_header->crc[block]) {
			char buf[48];
			sprintf(buf, "S2 BLOCK CRC %d ERROR", block);
			eprintf("Block %d CRC error, expect %08X but %08X\n", block, m_header->crc[block], crc);
			
			if (block != 0) {			
			NVIC_GenerateSystemReset();
			}
			goto __exit;
		}

//		STMFLASH_Write(addr, block_Buf, readLen);
		if ((__mark->type == GWPACKET) && (!writeBlockDataToFlash(block_Buf, addr, readLen))) {
			char buf[48];
			sprintf(buf, "S2 WRITE %08X ERROR", addr);
			eprintf("Write data to flash error\n");			
			NVIC_GenerateSystemReset();
			goto __exit;
		}
		fileLeftLen -= readLen;
		addr += readLen;
	}
	FLASH_Lock();
	
	dprintf("programFromRam OK\n");
	if(__mark->type == GWPACKET){
		eraseupdatemark();
		ret = ProtocolMessage((unsigned char *)"9999999999", (unsigned char *)"41", "00");
		
		while(*ret)
			m35SerialSendByte(*ret++);
	}

	rc = true;

__exit:
	return rc;
}

bool updateFirmware(void)
{
	return true;
}

