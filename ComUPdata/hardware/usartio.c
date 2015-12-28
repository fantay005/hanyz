#include <stdio.h>
#include "stm32f10x_usart.h"

int fputc (int ch, FILE *fp)
{
	if (ch == '\n') fputc('\r', fp);
	
	USART_SendData(UART5, ch);
	while(USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
	return (ch);
}

int getc(FILE *fp)
{
	while (1) {
		if (USART_GetFlagStatus(UART5, USART_FLAG_RXNE) != RESET) {
			int ch = USART_ReceiveData(UART5);
			fputc((ch == '\r') ? '\n' : ch, fp);
			return ch;
		}

		if (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) != RESET)
			USART_SendData(UART5, USART_ReceiveData(USART3));
	}
}

char *gets(char *s)
{
	int ch;
	char *p = s;
	
	while(1) {
		ch = getc((FILE *)1);
		
		if (ch == '\r') {
			*p = 0;
			return s;
		}
		
		*p++ = ch;
	}
}
