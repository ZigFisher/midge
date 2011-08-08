/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/inc/uart.h,v 1.2 2004/03/31 01:49:20 yjlou Exp $
*
* Abstract: UART driver header file.
*
* $Author: yjlou $
*
* $Log: uart.h,v $
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.1.1.1  2003/09/25 08:16:56  tony
*  initial loader tree 
*
* Revision 1.1.1.1  2003/05/07 08:16:07  danwu
* no message
*
* Revision 1.2  2003/03/12 02:26:15  danwu
* modify arguments of uart_InitTestConsl().
*
* Revision 1.1  2002/10/31 12:26:05  danwu
* no message
*
* Revision 1.5  2002/08/20 01:35:08  danwu
* Trival modification.
*
* ---------------------------------------------------------------
*/

#ifndef _UART_H
#define _UART_H



#define ENABLE_UART0_INTERRUPT_MODE     (1 << 0)
#define ENABLE_UART0_POLLING_MODE       (1 << 1)
#define ENABLE_UART1_INTERRUPT_MODE     (1 << 2)
#define ENABLE_UART1_POLLING_MODE       (1 << 3)
int32 uart_InitTestConsl(uint32 enable_channel);

#define MAX_UART_PRINTF_SIZE    1024
#define COUNT()     count++;

#define UART_TX_QUEUE_SIZE      1024
#define UART_RX_QUEUE_SIZE      1024

#define ESC		0x1b
#define	CTRL_C	0x03
#define	SPACE	0x20
#define	RETURN	'\r'



#endif   /* _UART_H */

