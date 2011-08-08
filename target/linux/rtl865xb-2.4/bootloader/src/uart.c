/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/uart.c,v 1.8 2005/01/24 01:35:07 danwu Exp $
*
* Abstract: UART driver.
*
* $Author: danwu $
*
* $Log: uart.c,v $
* Revision 1.8  2005/01/24 01:35:07  danwu
* + add 8186 support
*
* Revision 1.7  2005/01/21 08:03:02  yjlou
* *: fixed _chip_is_shared_pci_mode() function. Always use HAS_PCI_BONDING bit of MSCR to consider UART.
*
* Revision 1.6  2004/11/10 12:52:13  yjlou
* *: UART console can be selected by menuconfig.
*
* Revision 1.5  2004/08/26 13:53:27  yjlou
* -: remove all warning messages!
* +: add compile flags "-Wno-implicit -Werror" in Makefile to treat warning as error!
*
* Revision 1.4  2004/08/03 05:23:10  yjlou
* +: porting _chip_is_shared_pci_mode() to loader. loader supports auto-detect console UART.
* -: remove _50B_PCI_ENABLED_
*
* Revision 1.3  2004/04/20 12:19:01  yjlou
* +: support 50B that UART0 pins share with PCI (_50B_PCI_ENABLED_)
*
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.2  2004/03/09 00:37:51  danwu
* remove unused code and dev interface to shrink loader image size under 0xc000 and only flash block 0 & 3 occupied
*
* Revision 1.1.1.1  2003/09/25 08:16:55  tony
*  initial loader tree 
*
* Revision 1.1.1.1  2003/05/07 08:16:06  danwu
* no message
*
* ---------------------------------------------------------------
*/

#include <rtl_types.h>
#include <rtl_errno.h>
#include <stdarg.h>
#include <board.h>
#include <rtl8650/asicregs.h>
#include <uart.h>
#include <ns16550.h>
#include <rtl_dev.h>
#include <linux/autoconf.h>



typedef enum RESULT_E
{
   RESULT_FAIL = 0, /* for post diagnosis return value, orlando 4/27/199 */
   RESULT_PASS      /* for post diagnosis return value, orlando 4/27/199 */
}RESULT_T;


/* DATA STRUCTURE DECLARATIONS
*/
typedef struct {
    uint32          producerIndex;
    uint32          consumerIndex;
    uint32          boundryIndex;
    uint8           *sBuf;
    uint32          nQueueLen;
} UARTQUEUE;
    
typedef struct {
    ns16550_chan_T  device;
    UARTQUEUE       uartTxQueue;
    UARTQUEUE       uartRxQueue;
} UART_CHAN;



/* STATIC VARIABLE DECLARATIONS
 */
static uint8            uart_printf_buf[MAX_UART_PRINTF_SIZE];
static UART_CHAN        uartChan0;
static UART_CHAN        uartChan1;
static uint8 *          uartTxBuf;
static uint8 *          uartRxBuf;



/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static int32    uart_read(void *);
static int32    uart_Loopback_Test(UART_CHAN *);
static int32    uart_WriteString(UART_CHAN *, int32, uint8 *);
static int32    uart_GetByte(UART_CHAN *, uint8 *);
//static int32    _visprintf(int8 *, const int8 *, va_list);
static void     uart_Init_Queue(UART_CHAN *);
static int32    uart_Insert_Queue(UARTQUEUE *, uint8);
static int32    uart_Remove_Queue(UARTQUEUE *, uint8 *);
static int32    uart_Is_Queue_Empty(UARTQUEUE *);
static int32    uart_Is_Queue_Full(UARTQUEUE *);
static void     uart_intHandler();
static void     uart_CallbackInstall(UART_CHAN *, STATUS uart_RxChar(uint8), 
                                                    STATUS uart_TxChar(uint8 *));
static void     uart_StartTransmit(UART_CHAN *);
static int32    uart_RxChar(uint8);
static int32    uart_TxChar(uint8 *);



/* Extern Routines for stdio or ease of use 
*/
int printf(const int8 *fmt, ...)
{
    va_list     args;
    
    va_start(args,fmt);
    _visprintf(uart_printf_buf, fmt, args);
    
    /* transmit out to the console */
    uart_WriteString(&uartChan0, strlen((const int8 *) uart_printf_buf), (uint8 *) uart_printf_buf);
    uart_printf_buf[0] = '\0';
}

void putchar(uint8 ch)
{
    uint8   buffer[2];
    
    buffer[0] = ch;
    buffer[1] = '\0';
    
    uart_WriteString(&uartChan0, strlen((const int8 *) buffer), (uint8 *) buffer);
}

int32 getchar(uint8 *ch)
{
	while (uart_read((void*) ch));
    return 1;
}

int32 getch(uint8 *ch)
{
    return (!uart_read((void *) ch));
}

int32 getline(uint32 max_char, uint8 *buffer, uint32 msec)
{
    uint16      input_size = 0;
    uint8       endline = FALSE;
    uint8       inch;
    uint64      time_old = 0;
    uint64      time_new = 0;
    
    buffer[0] = 0;
    input_size = 0;
    tick_GetSysUpTime(&time_old);
    while (TRUE)
    {
        if (msec != 0)
        {
            tick_GetSysUpTime(&time_new);
            if (time_new >= (time_old + (uint64)msec))
                return input_size;
        }
            
        if ( uart_read((void *) &inch) )
            continue;
            
        switch (inch)
        {
          case '\n':
          case '\r':
                buffer[ input_size++] = 0;
                endline = TRUE;
                break;
          case '\b':
                /* back space */
                if (input_size > 0)
                {
                    /* erase the last input byte */
                    input_size -= 1;
                    printf("\b \b");
                }
                break;
          case ' ':
                /* tab/space */
                if (input_size) 
                {
                    buffer[ input_size++] = ' ';
                    printf("%c", inch);
                }
                break;
          case 0x1b:
                /* escape */
          case '\t':
                /* enter */
                input_size = 0;
                buffer[input_size] = 0;
                endline = TRUE;
                break;
          default:
                buffer[input_size++] = inch;
                printf("%c", inch);  
                break;
        }
        
        if (endline)
        {
            /* end of input line */
            return input_size;
        }
        
        if (input_size > max_char)
        {
            printf("\b \b");
            input_size = max_char;
        }
    }
}

void printfByPolling(const int8 *fmt, ...)
{
    va_list     args;
    uint8 *     ch;
    int32       len;
    int32       i;
    
    va_start(args,fmt);
    _visprintf(uart_printf_buf, fmt, args);
    
    /* transmit out to the console */
    ch = uart_printf_buf;
    len = strlen((const int8 *) uart_printf_buf);
    while (len > 0)
    {
        i = 0;
        while ( (uartChan0.device.getTxArg = *ch) && 
                (ns16550PollOutput(&(uartChan0.device)) != NS16550_OK) )
        {
            /* buffer is not availble */
            if (i >= 3000)
                return;
            i++;
        }

        ch++;
        len--;
    }
    uart_printf_buf[0] = '\0';
}


/*
 *  This function is used to return console is UART0 or UART1.
 *	0 is UART0.
 *	1 is UART1.
 *
 *	This function is copied from linux-2.4-x/init/main.c.
 */
int _chip_is_shared_pci_mode()
{
#ifdef CONFIG_RTL865X_UART_UART0
	return 0;
#elif defined(CONFIG_RTL865X_UART_UART1)
	return 1;
#else/*CONFIG_RTL865X_UART_AUTO*/

#if 1
	/*
	 * The HAS_PCI_BONDING bit decides whether UART0 is shared.
	 * If set, the UART0 is shared. Therefore, we use UART1 as console.
	 * If not set, we use UART0 as console.
	 */
	if ((*((volatile unsigned int *)MSCR))&HAS_PCI_BONDING)
		return 1;
	else
		return 0;
#else
	if(0==(*((volatile unsigned int *)MSCR)&HAS_PCI_BONDING))
	{
		/* ASIC does NOT have PCI bonding, so that we must use UART0. */
		return 0;
	}
	if(0x05005788==(*((volatile unsigned int *)CRMR)&0x07f0ffff))
	{
		/* chip is 8650B 208 package with PCI bonding and PCI kernel option is enabled. */
		return 1;
	}
	return 0;
#endif/*1*/

#endif
}


/*****************************************************************************/
/* initialization function                                                   */
/*****************************************************************************/
int32 uart_InitTestConsl(uint32 enable_channel)
{
    int32   result = 0;
    uint32 consoleNo; /* 0 for UART0, 1 for UART1 */

#ifdef CONFIG_RTL865X
    consoleNo = _chip_is_shared_pci_mode();
#endif

#ifdef CONFIG_RTL8186
	consoleNo = 0;
#endif
    
//    if ( enable_channel & ENABLE_UART0_INTERRUPT_MODE )
    {
        /* Install interrupt handler */
		if ( consoleNo == 0 )
	        int_Register(UART0_ILEV, UART0IE, UART0IRS_OFFSET, uart_intHandler);
		else
	        int_Register(UART1_ILEV, UART1IE, UART1IRS_OFFSET, uart_intHandler);
        
        uart_printf_buf[MAX_UART_PRINTF_SIZE - 1] = '\0';
        
        /* Initialize UART device */
		if ( consoleNo == 0 )
	        ns16550DevInit(&(uartChan0.device), 
                            UART0_BASE, 
                            1, /* interrupt mode */
#ifdef CONFIG_RTL865X
                            tick_getSysClkRate(), 
#endif
#ifdef CONFIG_RTL8186
                            UART_BASECLOCK,
#endif
                            DEFAULT_BAUDRATE);
		else
    	    ns16550DevInit(&(uartChan0.device), 
                            UART1_BASE, 
                            1, /* interrupt mode */
                            tick_getSysClkRate(), 
                            DEFAULT_BAUDRATE);

    
        /* Install callback routines */
        uart_CallbackInstall(&uartChan0, uart_RxChar, uart_TxChar);
    
        /* Initailize Tx, Rx queues */
        uart_Init_Queue(&uartChan0);
    
        /* Enable UART interrupt */
		if ( consoleNo == 0 )
			if ( UART0_ILEV < getIlev() )
	    	    setIlev(UART0_ILEV);
		else
			if ( UART1_ILEV < getIlev() )
	    	    setIlev(UART1_ILEV);
        
        if ( uart_Loopback_Test(&uartChan0) != RESULT_PASS )
            result = -1;
    }
#if 0
    else if ( enable_channel & ENABLE_UART0_POLLING_MODE )
    {
        uart_printf_buf[MAX_UART_PRINTF_SIZE - 1] = '\0';
        
        /* Initialize UART device */
        ns16550DevInit(&(uartChan0.device), 
#ifndef _50B_PCI_ENABLED_
                            UART0_BASE, 
#else //_50B_PCI_ENABLED_
                            UART1_BASE, 
#endif//_50B_PCI_ENABLED_
                            0, /* polling mode */
                            tick_getSysClkRate(), 
                            DEFAULT_BAUDRATE);
        
        if ( uart_Loopback_Test(&uartChan0) != RESULT_PASS )
            result = -1;
    }
    
    if ( enable_channel & ENABLE_UART1_INTERRUPT_MODE )
    {
        /* Install interrupt handler */
        int_Register(UART1_ILEV, UART1IE, UART1IRS_OFFSET, uart_gdbHandler);
        
        ns16550DevInit(&(uartChan1.device), 
                            UART1_BASE, 
                            0, /* polling mode */
                            tick_getSysClkRate(), 
                            115200 /*DEFAULT_BAUDRATE*/);
    
        if ( uart_Loopback_Test(&uartChan1) != RESULT_PASS )
            result = -1;
            
        /* Workaround: enable rx interrupt to support gdb stop */
        *(volatile uint8 *)(uartChan1.device.regs + IER) = (uint8)(IER_ERBI);
    
        /* Enable UART interrupt */
	      if ( UART1_ILEV < getIlev() )
	    	    setIlev(UART1_ILEV);
    }
    else if ( enable_channel & ENABLE_UART1_POLLING_MODE )
    {
        ns16550DevInit(&(uartChan1.device), 
                            UART1_BASE, 
                            0, /* polling mode */
                            tick_getSysClkRate(), 
                            115200 /*DEFAULT_BAUDRATE*/);
    
        if ( uart_Loopback_Test(&uartChan1) != RESULT_PASS )
            result = -1;
    }
#endif
    
    uart_printf_buf[0] = '\0';
            
    return result;
}
#if 0
/*****************************************************************************/
/* set baudrate function                                                     */
/*****************************************************************************/
static int32 uart_SetBaudrate(UART_CHAN *chan, uint32 baudrate)
{
    switch (baudrate)
    {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 115200:
            break;

        default:
            return RESULT_FAIL;
    }        

    if ( (baudrate == chan->device.baudRate) ||
            (ns16550Ioctl(&(chan->device), NS16550_BAUD_SET, baudrate) == NS16550_OK) )
        return RESULT_PASS;
    else
        return RESULT_FAIL;
}

int32 console_setBaudrate(uint32 baudrate)
{
    return uart_SetBaudrate(&uartChan0, baudrate);
}
#endif

/*
 * LOCAL SUBPROGRAM BODIES
 */
static int32 uart_read(void * input)
{
    int32       retval = FALSE;
    uint32         imask;

    if ( uartChan0.device.channelMode == NS16550_MODE_INT )
    {
        imask = setIlev(UART0_ILEV + 1);
        
        retval = uart_Remove_Queue(&(uartChan0.uartRxQueue), (uint8 *) input);
        
        setIlev(imask);
    }
    else
    {
        uint8 * ptr = (uint8 *) input;
        
        /* Console channel is in polling mode */ 
        if ( ns16550PollInput(&(uartChan0.device)) == NS16550_OK ) 
        { 
            *ptr = uartChan0.device.putRcvArg; 
            retval = TRUE; 
        } 
        else 
            retval = FALSE; 
    }
 
    if (retval == TRUE)
	    return 0;
    else
	    return EAGAIN;
}

static int32 uart_Loopback_Test(UART_CHAN *chan)
{
    uint8  i, ch, retval = RESULT_PASS;
//    uint8   txString[] = "!@#$%^&*()-+|abcdefghijklmnopqrstuvwxyz.";
    uint8  txString[] = "!@#$^&*()-+|abcdefghijklmnopqrstuvwxyz.";
    uint32  length;
    uint32    count;
    
    while (uart_GetByte(chan, &ch) == RESULT_PASS);

    /* set loopback mode */
    if (ns16550Ioctl(&(chan->device), NS16550_LOOPBACK_SET, NS16550_ENABLE_LOOPBACK) != NS16550_OK)
        return RESULT_FAIL;

    length = strlen((const int8 *) txString);
    count = 0;
    
    if ( chan->device.channelMode == NS16550_MODE_INT )
    {
        /* This uart channel is in interrupt mode */
        uart_WriteString(chan, strlen((const int8 *) txString), (uint8 *) txString);
        
        for (i = 0; i < length; i++)
        {
            while (uart_GetByte(chan, &ch) != RESULT_PASS)
            {
                   count++;
                   if (count >=300)
                   {
                    retval = RESULT_FAIL;
                       break;
                   }
             }
             if ((retval == RESULT_FAIL) || (ch != txString[i]))
            {
                retval = RESULT_FAIL;
                break;
            }
            count = 0;
        }
    }
    else
    {
        /* This uart channel is in polling mode */
        for (i = 0; i < length; i++)
        {
            uart_WriteString(chan, 1, (uint8 *) (txString + i));
            
            while (uart_GetByte(chan, &ch) != RESULT_PASS)
            {
                   count++;
                   if (count >=300)
                   {
                    retval = RESULT_FAIL;
                       break;
                   }
             }
             if ((retval == RESULT_FAIL) || (ch != txString[i]))
            {
                retval = RESULT_FAIL;
                break;
            }
            count = 0;
        }
    }

    /* set normal operation mode */
    if (ns16550Ioctl(&(chan->device), NS16550_LOOPBACK_SET, NS16550_DISABLE_LOOPBACK) != NS16550_OK)
        return RESULT_FAIL;

    return (retval);
}

static int32 uart_WriteString(UART_CHAN *chan, int32 length, uint8 *byte)
{
    uint8     *ch;
    int32     i;
    int32     j;
    int32     len;

    if (length == 0)
        return EINVAL;

    ch = byte;
    len = length;

    if ( chan->device.channelMode == NS16550_MODE_INT )
    {
        /* This uart channel is in interrupt mode */
        uint32 imask = setIlev(UART0_ILEV + 1);
        
        while (len > 0)
        {
            i = 0;
            while (uart_Insert_Queue(&(chan->uartTxQueue), *ch) == FALSE)
            {
                /* turn on interrupt for a while */
                setIlev(imask);
                for (j=0;j<300;j++);
                imask = setIlev(UART0_ILEV + 1);
                /* buffer is not availble */
                if (i >= 3000)
                {
                    setIlev(imask);
                    return EAGAIN;
                }
                i++;
            }
    
            ch++;
            len--;
        }
        
        uart_StartTransmit(chan);
        
        setIlev(imask);
    }
    else
    {
        /* This uart channel is in polling mode */
        while (len > 0)
        {
            i = 0;
            while ( (chan->device.getTxArg = *ch) && 
                    (ns16550PollOutput(&(chan->device)) != NS16550_OK) )
            {
                /* buffer is not availble */
                if (i >= 3000)
                    return EAGAIN;
                i++;
            }
    
            ch++;
            len--;
        }
    }

    return 0;
}

static int32 uart_GetByte(UART_CHAN *chan, uint8 *byte)
{
    int32   retval = FALSE;

    *byte = 0x0;
    
    if ( chan->device.channelMode == NS16550_MODE_INT )
    {
        /* This uart channel is in interrupt mode */
        uint32 imask = setIlev(UART0_ILEV + 1);
        
        retval = uart_Remove_Queue(&(chan->uartRxQueue), byte);
        
        setIlev(imask);
    }
    else
    {
        /* This uart channel is in polling mode */
        if ( ns16550PollInput(&(chan->device)) == NS16550_OK )
        {
            *byte = chan->device.putRcvArg;
            retval = TRUE;
        }
        else
            retval = FALSE;
    }
 
    return retval;
}

static void uart_Init_Queue(UART_CHAN *chan)
{
    uartTxBuf = (uint8 *) malloc(UART_TX_QUEUE_SIZE * sizeof(uint8));
    chan->uartTxQueue.producerIndex = 1;
    chan->uartTxQueue.consumerIndex = 1;
    chan->uartTxQueue.boundryIndex = 0;
    chan->uartTxQueue.sBuf = uartTxBuf;
    chan->uartTxQueue.nQueueLen = UART_TX_QUEUE_SIZE;

    uartRxBuf = (uint8 *) malloc(UART_RX_QUEUE_SIZE * sizeof(uint8));
    chan->uartRxQueue.producerIndex =  1;
    chan->uartRxQueue.consumerIndex = 1;
    chan->uartRxQueue.boundryIndex = 0;
    chan->uartRxQueue.sBuf = uartRxBuf;
    chan->uartRxQueue.nQueueLen = UART_RX_QUEUE_SIZE;
    
    return;
}

static int32 uart_Insert_Queue(UARTQUEUE *pQueue, uint8 ch)
{
    if (uart_Is_Queue_Full(pQueue))
    {
        return FALSE;
    }

    pQueue->sBuf[pQueue->producerIndex++] = ch;
    if(pQueue->producerIndex == pQueue->nQueueLen)
    {
        pQueue->producerIndex = 0;
    }

    return TRUE;
}

static int32 uart_Is_Queue_Full(UARTQUEUE *pQueue)
{
    if(pQueue->producerIndex == pQueue->boundryIndex)
        return TRUE;
    else
        return FALSE;
}

static int32 uart_Remove_Queue(UARTQUEUE *pQueue, uint8 *ch)
{
    if(uart_Is_Queue_Empty(pQueue))
    {
        return FALSE;
    }

    pQueue->boundryIndex = pQueue->consumerIndex;
    *(ch) = pQueue->sBuf[pQueue->consumerIndex++];
    if(pQueue->consumerIndex >= pQueue->nQueueLen)
    {
        pQueue->consumerIndex = 0;
    }

    return TRUE;
}

static int32 uart_Is_Queue_Empty(UARTQUEUE *pQueue)
{
    if (pQueue->producerIndex == pQueue->consumerIndex) 
        return TRUE;
    else
        return FALSE;
}

static void uart_intHandler()
{
    ns16550Int(&(uartChan0.device));
}

static void uart_CallbackInstall(UART_CHAN *chan, STATUS uart_RxChar(uint8), 
                                                    STATUS uart_TxChar(uint8 *))
{
    ns16550CallbackInstall(&(chan->device), uart_RxChar, uart_TxChar);
}

static void uart_StartTransmit(UART_CHAN *chan)
{
    ns16550TxStartup(&(chan->device));     
}

static STATUS uart_RxChar(uint8 ch)
{
    if ( uart_Insert_Queue(&(uartChan0.uartRxQueue), ch) )
        return NS16550_OK;
    else
        return NS16550_ERROR;
}

static STATUS uart_TxChar(uint8 *ch_ptr)
{
    if ( uart_Remove_Queue(&(uartChan0.uartTxQueue), ch_ptr) )
        return NS16550_OK;
    else
        return NS16550_ERROR;
}

static void uart_reverse(uint8 *buf)
{
    int32     c, i, j;
    for (i = 0, j = strlen((const int8 *) buf)-1; i < j; i++, j--)
    {
        c = buf[i];
        buf[i] = buf[j];
        buf[j] = c;
    }
}

static uint8 uart_BYTEtoStr(uint8 n, uint8 *buf)
{
    int32 i = 0;
    do
    {
        buf[i++] = MOD(n , 10) + '0';
//    } while ((n /= 10) > 0);
    } while ((n = DIV(n, 10)) > 0);
    buf[i] = '\0';
    uart_reverse(buf);
    return i;   /* return value is the length of WORD integer */
}

static void _insert( int8 *s, int8 c)   /* insert c at head of string s */
{
    int8 *tmps;
    
    tmps = s;
    while (*tmps++);
    while( tmps > s )
    {
        tmps--;
        tmps[1] = tmps[0];
    }
    *s = c;
}

static void _append(int8 *s, int8 c)      /* append c to end of string s */
{
    while(*s++)
        ;
    *s-- = 0;
    if (c != '\n')
        *s = c;
    else    /* append one more CR if NEW LINE, by ProChao, 12/20/96 */
    {
        *s++ = '\r';
        *s++ = c;
        *s-- = 0;
    }
}

static int32 s_atoi(const int8 *str)
{
    int32   a = 0;
    
    while ((*str >= '0') && (*str <= '9'))
    {
        a = MUL(a, 10) + (*str - '0');
        str++;
    }
    
    return a;
}

//static int32 _visprintf(int8 *buf, const int8 *fmt, va_list args)
int32 _visprintf(int8 *buf, const int8 *fmt, va_list args)
{
    int32       count = 0;
    int32       pwidth, width, pcnt, base, i;
    uint32   num, ip_addr;
    int8              fch, c;         /* format char */
    int8              *s, *bp;
    int8              ljust, zsup;    /* left justify, zero suppress flags */
    int8              sign;           /* signed number conversion flag */
    int8              letter = 0;     /* hex conversion letter increment */
    int8              islong;         /* long integer flag */
    int8              pflag;
    uint8   ip_byte;
    uint8   *us, ip_str[20];
    
    *buf = 0;
    bp = buf;                     /* point to start of buf */
    while ( ((fch = *fmt++) != 0) && (count < (MAX_UART_PRINTF_SIZE - 1)) )
    {
        while (*bp)                       /* find end of current string */
            bp++;                         /*  where next field starts */
        
        if (fch == '%')
        {
            ljust = 0;                /* reset flags and width */
            pwidth = sizeof(void *)*2;      /* minimum pointer print width */
            pcnt = 0;             /* printed length of current field */
            islong = 0;           /* default int is not long */
            sign = 0;                 /* default unsigned */
            pflag = 0;            /* not %p spec */
            zsup = 1;             /* zero suppression default */
            base = 10;                /* default base */
            
            switch( *fmt++ ){         /* parse flags */
            case '%':
                goto s_copy;
                
            case '-':
                ljust = 1;
                break;
                
            case '*':                             /* dynamic field width */
                width = va_arg(args, int32);
                goto s_gotwidth;
                
            default:
                fmt--;
                break;
            }
            if (*fmt == '0')            /* get width if there */
                zsup = 0;                                 /* no zero suppression */
            width = s_atoi(fmt);                  /* get minimum field width */
s_gotwidth:
            while ((*fmt >= '0') && (*fmt <= '9'))
                fmt++;
s_ismod:
            fch = *fmt++;
            switch( fch ){
            case 'l':
            case 'L':
                islong = 1;
                goto s_ismod;         /* modifier character */
                
            case 'd':
                sign = 1;
                goto s_donumber;
                
            case 'o':             /* octal */
                base = 8;
                goto s_donumber;
                
            case 'u':
                goto s_donumber;
                
            case 'x':             /* hex */
                base = 16;
                letter = 'a'-10;
                goto s_donumber;
                
            case 'X':
                base = 16;
                letter = 'A'-10;
                goto s_donumber;
                
            case 'p':             /* (void *) */
                pflag = 1;
                if ( width < pwidth )
                    width = pwidth;
                base = 16;
                letter = 'A'-10;
                
                num = (int32) va_arg(args, void *);
                goto s_doptr;
                
            case 'c':
                _append(bp++, (int8)(va_arg(args, int32)));
                COUNT();
                    goto s_endarg;
                
            case 's':
                s = va_arg(args,int8 *);
                if (!s)
                    s = "NULL";             /* null pointer passed for %s */
                while (*s)
                {                 /* copy string to buf */
                    _append(bp,*s++);
                    COUNT();
                        pcnt++;
                }
                for ( ; pcnt<width; pcnt++)
                {
                    COUNT();
                        if (ljust)
                            _append(bp, ' ');
                        else
                        {
                            _insert(bp, ' ');
                        }
                }
                goto s_endarg;
                
            case 'I':
                ip_addr = va_arg(args, int32);
                for (i = 0; i < 4; i++)
                {
                    ip_byte = (uint8) ((ip_addr >> (24 - i * 8)) & 0xFF);
                    uart_BYTEtoStr(ip_byte, ip_str);
                    us = ip_str;    /* daniel change here s -> us */
                    if (!us)
                        break;    /* null pointer passed for %s */
                    while (*us)
                    {         /* copy string to buf */
                        _append(bp, *us++);
                        COUNT();
                            pcnt++;
                    }
                    for ( ; pcnt<width; pcnt++)
                    {
                        COUNT();
                            if (ljust)
                                _append(bp, ' ');
                            else
                            {
                                _insert(bp, ' ');
                            }
                    }
                    if (i != 3)
                        _append(bp, '.');
                }
                goto s_endarg;
                
            default:
                goto s_copy;
            }
s_donumber:
            {
                if ( islong )
                    num = va_arg(args, int32);             /* long argument */
                else
                    if(sign)
                        num = (int32)va_arg(args, int32);    /* integer argument */
                    else
                        num = (int32)va_arg(args, uint32); /* integer argument */
                    if( sign && (num & 0x80000000) )
                    {
                        sign = 1;                 /* do sign */
                        num = (uint32)(-(int32)num);
                    }
                    else
                    {
                        sign = 0;
                    }
s_doptr:
                    while( num != 0l )
                    {
                        c = MOD(num, base);
                        /*c = (int8)(num % base);*/
                        num = DIV(num, base);
                        /*num /= base;*/
                        _insert(bp, (int8)(c > 9 ? c + letter : c + '0'));
                        pcnt++;           /* count digits */
                        COUNT();
                    }
                    if(!*bp)
                    {
                        _insert(bp, '0');           /* at least 1 zero */
                        pcnt++;
                        COUNT();
                    }
                    if( pflag )
                    {
                        for(;pcnt < pwidth; pcnt++)
                        {
                            _insert(bp, '0');
                            COUNT();
                        }
                    }
                    c = (int8)(zsup ? ' ' : '0');           /* pad char */
                    for (pcnt += sign ;pcnt < width; pcnt++)
                        if ( ljust )
                        {                 /* left justified ? */
                            _append(bp, ' ');
                            COUNT();
                        }
                        else
                        {
                            _insert(bp, c);
                            COUNT();
                        }
                        if (sign)
                            _insert(bp, '-');
            }
        }
        else
        {
s_copy:     _append(bp++, fch);         /* copy char to output */
            COUNT();
        }
s_endarg:
            continue;
    }
    return count;
}



