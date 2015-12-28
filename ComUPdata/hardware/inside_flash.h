#ifndef __INSIDE_FLASH_H__
#define __INSIDE_FLASH_H__

#include <stdint.h>

#define Download_Store_Addr  (0x08040000)
#define Program_Run_Addr     (0x08010000)

void STMFLASH_Write(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t NumToWrite);
void STMFLASH_Read_Word(uint32_t ReadAddr, uint32_t *pBuffer, uint16_t NumToRead);
void STMFLASH_Read(uint32_t ReadAddr, uint16_t *pBuffer, uint16_t NumToRead);

#endif
