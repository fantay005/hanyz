#include "stm32f10x_usart.h"

void uart5Configure(void)
{
	USART_InitTypeDef USART_InitStructure;
	
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART5, &USART_InitStructure);       // Configure USART1
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);  // Enable USART1 Receive
	USART_Cmd(UART5, ENABLE);                    // Enable the USART1
	USART_SendData(UART5, 'X');
	while(USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
}

void usart3Configure(int baud){
  USART_InitTypeDef USART_InitStructure;
	 	
	USART_InitStructure.USART_BaudRate = baud;
	//USART_InitStructure.USART_BaudRate = 9600;      //和手机模块通讯
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);       // Configure USART
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);  // Enable USART1 Receive
	USART_Cmd(USART3, ENABLE);

}
