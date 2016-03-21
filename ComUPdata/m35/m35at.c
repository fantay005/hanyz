#include <string.h>
#include "m35at.h"
#include "m35.h"
#include "hardware.h"

extern void usart3Configure(int baud);

bool m35AtChat(const char *cmd, const char *responsePrefix, char *result, unsigned int timeoutMs)
{
	int n = 0;
	bool cr = false;

	dprintf("m35AtChat-->\n");
	dprintf("command: %s", cmd);

	while(*cmd) 
		m35SerialSendByte(*cmd++);
	if (responsePrefix == NULL || result == NULL) 
		return true;

	//IWDG_ReloadCounter();//Î¹¹·
	setDelayMs(timeoutMs);
	while (true) {
		IWDG_ReloadCounter();//Î¹¹·
		if (isTimeout())
			return false;
		if (m35SerialIsGotByte()) {
			result[n] = m35SerialGetByte();
//			putchar(result[n]);
			if (result[n] != responsePrefix[n]) {
				n = 0;
				continue;
			}
			n++;
			if (n == strlen(responsePrefix)) break;
		}
	}

	while (true) {
		IWDG_ReloadCounter();//Î¹¹·
		if (isTimeout()) return true;
		if (m35SerialIsGotByte()) {
			result[n] = m35SerialGetByte();
//			putchar(result[n]);
			if (cr) {
				if ('\n' == result[n]) {
					result[n] = 0;
					break;
				}
			}
			if (result[n] == '\r') {
				result[n] = 0;
				cr = true;
				continue;
			}
			n++;
		}
	}

	dprintf(" reply: %s\n", result);

	return true;
}

bool m35ReceiveData(char *data, int len, int timeoutMs)
{
	int readLen = 0;

	setDelayMs(timeoutMs);
	while(readLen < len) {
		if (isTimeout()) return false;
		if (m35SerialIsGotByte()) {
			*data++ = m35SerialGetByte();
//			USART_SendData(USART1, *(data-1));
			readLen++;
		}
	}
	return true;
}
