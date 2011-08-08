/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/inc/ns16550.h,v 1.3 2004/08/26 13:53:27 yjlou Exp $
*
* Abstract: NS16550 specific definitions.
*
* $Author: yjlou $
*
* $Log: ns16550.h,v $
* Revision 1.3  2004/08/26 13:53:27  yjlou
* -: remove all warning messages!
* +: add compile flags "-Wno-implicit -Werror" in Makefile to treat warning as error!
*
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
* ---------------------------------------------------------------
*/

#ifndef _NS16550_H
#define _NS16550_H



/* prototype definitions */
#define STATUS      int32


#define RBR                                 0x000       /* Rx buffer */
#define THR                                 0x000       /* Tx holding */
#define DLL                                 0x000       /* Divisor latch LSB */
#define IER                                 0x004       /* Interrupt enable */
#define DLM                                 0x004       /* Divisor latch MSB */
#define IIR                                 0x008       /* Interrupt identification */
#define FCR                                 0x008       /* FIFO control */
#define LCR                                 0x00C       /* Line control */
#ifndef MCR
#define MCR                                 0x010       /* Modem control */
#endif/*MCR*/
#define LSR                                 0x014       /* Line status */
#define MSR                                 0x018       /* Modem status */
#define SCR                                 0x01C       /* Scratchpad */
/* Line Control Register 
*/
#define LCR_WLN         0x03
#define CHAR_LEN_5      0x00
#define CHAR_LEN_6      0x01
#define CHAR_LEN_7      0x02
#define CHAR_LEN_8      0x03
#define LCR_STB         0x04
#define ONE_STOP        0x00
#define TWO_STOP        0x04
#define LCR_PEN         0x08
#define LCR_EPS         0x30
#define PARITY_ODD      0x00
#define PARITY_EVEN     0x10
#define PARITY_MARK     0x20
#define PARITY_SPACE    0x30
#define PARITY_NONE     0x80
#define LCR_SBRK        0x40
#define LCR_DLAB        0x80
#define DLAB            LCR_DLAB
/* Line Status Register 
*/
#define LSR_DR          0x01
#define RxCHAR_AVAIL    LSR_DR
#define LSR_OE          0x02
#define LSR_PE          0x04
#define LSR_FE          0x08
#define LSR_BI          0x10
#define LSR_THRE        0x20
#define LSR_TEMT        0x40
#define LSR_FERR        0x80
/* Interrupt Identification Register 
*/
#define IIR_IP          0x01
#define IIR_ID          0x0e
#define IIR_RLS         0x06
#define Rx_INT          IIR_RLS
#define IIR_RDA         0x04
#define RxFIFO_INT      IIR_RDA
#define IIR_THRE        0x02
#define TxFIFO_INT      IIR_THRE
#define IIR_MSTAT       0x00
#define IIR_TIMEOUT     0x0c
/* Interrupt Enable Register 
*/
#define IER_ERBI        0x01
#define IER_ETBEI       0x02
#define IER_ELSI        0x04
#define IER_EDSSI       0x08
#define IER_ESLP        0x10
#define IER_ELP         0x20
/* Modem Control Register 
*/
#define MCR_DTR         0x01
#define DTR             MCR_DTR
#define MCR_RTS         0x02
#define MCR_OUT1        0x04
#define MCR_OUT2        0x08
#define MCR_LOOP        0x10
/* Modem Status Register 
*/
#define MSR_DCTS        0x01
#define MSR_DDSR        0x02
#define MSR_TERI        0x04
#define MSR_DDCD        0x08
#define MSR_CTS         0x10
#define MSR_DSR         0x20
#define MSR_RI          0x40
#define MSR_DCD         0x80
/* FIFO Control Register 
*/
#define FCR_EN          0x01
#define FIFO_ENABLE     FCR_EN
#define FCR_RXCLR       0x02
#define RxCLEAR         FCR_RXCLR
#define FCR_TXCLR       0x04
#define TxCLEAR         FCR_TXCLR
#define FCR_DMA         0x08
#define FCR_RXTRIG_L    0x40
#define FCR_RXTRIG_H    0x80


/* NS16550 channel data structure
*/
typedef struct ns16550_chan_S
    {
    /* always goes first */

    /*NS16550_DRV_FUNCS *     pDrvFuncs;*/      /* driver functions */

    /* callbacks */

    STATUS      (*getTxChar) (uint8 *);
    STATUS      (*putRcvChar) (uint8);
    /*void *      getTxArg;
    void *      putRcvArg;*/
    uint8       getTxArg;
    uint8       putRcvArg;
    
    uint16      pad0;

    uint8 *regs;        /* NS16552 registers */
    /*uint8 level;*/        /* 8259a interrupt level for this device */
    uint8 ier;          /* copy of ier */
    uint8 lcr;          /* copy of lcr, not used by ns16552 driver */

    uint16          channelMode;   /* such as INT, POLL modes */
    /*uint16          regDelta;*/      /* register address spacing */
    /*uint16          pad1;*/
    uint32          baudRate;
    uint32          xtal;          /* UART clock frequency     */     
    /*uint32*         uartClearReg ;
    uint32*         uartMasterReg;
    uint32          uartMasterVal;*/

} ns16550_chan_T;

/* Request code for ns16550Ioctl 
*/
enum {
	NS16550_BAUD_SET = 0,
	NS16550_BAUD_GET,	
    NS16550_LOOPBACK_SET,
	NS16550_MODE_SET,
	NS16550_MODE_GET,
	NS16550_AVAIL_MODES_GET,
	NS16550_HW_OPTS_SET,
	NS16550_HW_OPTS_GET,
    NS16550_WORD_LEN_SET,
    NS16550_WORD_LEN_GET,
    NS16550_STOP_BITS_SET,
    NS16550_STOP_BITS_GET,
    NS16550_PARITY_SET,
    NS16550_PARITY_GET
};

/* Return status 
*/
enum {
    NS16550_OK = 0,
    NS16550_ERROR,
    NS16550_EIO,
    NS16550_ENOSYS,
    NS16550_EAGAIN
};

enum {
    NS16550_ENABLE_LOOPBACK = 0,
    NS16550_DISABLE_LOOPBACK
};

enum {
    NS16550_MODE_POLL = 0,              /* Polling mode */
    NS16550_MODE_INT                    /* Interrupt mode */
};



/* EXTERN SUBPROGRAM SPECIFICATIONS
 */
void ns16550DevInit(ns16550_chan_T * pChan, uint32 regBase, uint32 setInterruptMode, 
                        uint32 clockFreq, uint32 baudrate);
STATUS ns16550Ioctl(ns16550_chan_T *pChan, int32 request, int32 arg);
STATUS ns16550PollOutput(ns16550_chan_T * pChan);
STATUS ns16550PollInput(ns16550_chan_T * pChan);
void ns16550Int(ns16550_chan_T * pChan);
STATUS ns16550CallbackInstall(ns16550_chan_T * pChan, STATUS (*Rxcallback)(uint8), 
                                                                STATUS (*Txcallback)(uint8 *));
void ns16550TxStartup(ns16550_chan_T * pChan);



#endif /* _NS16550_H */
