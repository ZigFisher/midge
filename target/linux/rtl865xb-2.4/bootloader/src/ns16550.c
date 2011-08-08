/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/ns16550.c,v 1.2 2004/03/31 01:49:20 yjlou Exp $
*
* Abstract: NS16550 UART driver.
*
* $Author: yjlou $
*
* $Log: ns16550.c,v $
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
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
#include <board.h>
#include <ns16550.h>



/* LOCAL DEFINITIONS
*/
#define REG(reg,pchan) \
 (*(volatile uint8 *)((uint32)pchan->regs + reg))



/* STATIC VARIABLE DECLARATIONS
 */
uint32 ec_test = 0;



/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
static void ns16550InitChannel (ns16550_chan_T *);
static STATUS ns16550DummyTxCallback (uint8 *);
static STATUS ns16550DummyRxCallback (uint8);



/******************************************************************************
*
* ns16550DummyCallback - dummy callback routine.
*/
static STATUS ns16550DummyTxCallback (uint8 *ch)
    {
    return (NS16550_ERROR);
    }
static STATUS ns16550DummyRxCallback (uint8 ch)
    {
    return (NS16550_ERROR);
    }

/******************************************************************************
*
* ns16550DevInit - intialize an NS16550 channel
*
* This routine initializes some NS16550_CHAN function pointers and then resets
* the chip in a quiescent state.  Before this routine is called, the BSP
* must already have initialized all the device addresses, etc. in the
* NS16550_CHAN structure.
*
* RETURNS: N/A
*/
void ns16550DevInit
    (
    ns16550_chan_T * pChan,
    uint32 regBase,
    uint32 setInterruptMode,
    uint32 clockFreq,
    uint32 baudrate
    )
    {

    /* initialize the driver function pointers in the NS16550_CHAN's */

    /*pChan->pDrvFuncs    = &ns16550NS16550DrvFuncs;*/

    /* set the non BSP-specific constants */
    pChan->getTxChar    = ns16550DummyTxCallback;
    pChan->putRcvChar   = ns16550DummyRxCallback;

    if ( setInterruptMode )
        pChan->channelMode  = NS16550_MODE_INT;
    else
        pChan->channelMode  = NS16550_MODE_POLL;

    /* setup Register Set locations */
    pChan->regs         = (uint8 *) regBase;

    /* setup XTAL clock freq */
    pChan->xtal         = clockFreq;
    
    /* setup baudrate */
    pChan->baudRate     = baudrate;

    ns16550InitChannel(pChan);

    }

/*******************************************************************************
*
* ns16550InitChannel - initialize UART
*/
static void ns16550InitChannel
    (
    ns16550_chan_T *pChan
    )
    {
    uint32  divisor;
    
    /* Configure Port -  Set 8 bits, 1 stop bit, no parity. */
    pChan->lcr = (uint8)((CHAR_LEN_8 | ONE_STOP) & ~LCR_PEN);
    REG(LCR,pChan) = pChan->lcr;

    /* Reset/Enable the FIFOs */
    REG(FCR,pChan) = RxCLEAR | TxCLEAR | FIFO_ENABLE; 
 
    /* Enable access to the divisor latches by setting DLAB in LCR. */
    REG(LCR,pChan) = LCR_DLAB | pChan->lcr;

    /* Set divisor latches. */
    divisor = DIV( pChan->xtal, (16 * pChan->baudRate) ) - 1;
    REG(DLL,pChan) = (uint8) (divisor & 0xff);
    REG(DLM,pChan) = (uint8) ((divisor & 0xff00 ) >> 8);

    /* Restore line control register */
    REG(LCR,pChan) = pChan->lcr;

    /* Make a copy since register is not readable */
    if ( pChan->channelMode == NS16550_MODE_INT )
        pChan->ier = (uint8)(IER_ERBI | IER_ETBEI);
    else
        pChan->ier = 0;

    /* Enable interrupts */
    REG(IER,pChan) = pChan->ier;

    }

/*******************************************************************************
*
* ns16550TxStartup - transmitter startup routine
*
* Call interrupt level character output routine and enable interrupt!
*/
void ns16550TxStartup
    (
    ns16550_chan_T *pChan 
    )
    {
    if ( pChan->channelMode == NS16550_MODE_INT )
        {
        pChan->ier |= IER_ETBEI;
        REG(IER,pChan) = pChan->ier; 
        }
    }

/*******************************************************************************
*
* ns16550Ioctl - special device control
*
* RETURNS: OK on success, EIO on device error, ENOSYS on unsupported
*          request.
*/
STATUS ns16550CallbackInstall
    (
    	ns16550_chan_T *pChan ,
    	STATUS      (*Rxcallback)(uint8),
    	STATUS      (*Txcallback)(uint8 *)
    )
    {
		pChan->getTxChar	= Txcallback;
        pChan->putRcvChar   = Rxcallback;
	    return (NS16550_OK);
    }

/*******************************************************************************
*
* ns16550Ioctl - special device control
*
* RETURNS: OK on success, EIO on device error, ENOSYS on unsupported
*          request.
*/
STATUS ns16550Ioctl
    (
    ns16550_chan_T *pChan,                /* device to control */
    int32        request,         /* request code */
    int32        arg              /* some argument */
    )
    {
    STATUS  status;
    uint32  divisor;

    status = NS16550_OK;

    switch (request)
        {
        case NS16550_BAUD_SET:
            pChan->baudRate = arg;
            
            /* disable interrupts during chip access */


            /* Enable access to the divisor latches by setting DLAB in LCR. */
            REG(LCR,pChan) = LCR_DLAB | pChan->lcr;

            /* Set divisor latches. */
            divisor = DIV( pChan->xtal, (16 * pChan->baudRate) ) - 1;
            REG(DLL,pChan) = (uint8) (divisor & 0xff);
            REG(DLM,pChan) = (uint8) ((divisor & 0xff00 ) >> 8);

            /* Restore line control register */
            REG(LCR,pChan) = pChan->lcr; 

            break;

        case NS16550_BAUD_GET:
            *(uint32 *)arg = pChan->baudRate;
            break; 

        case NS16550_LOOPBACK_SET:
            if (arg == NS16550_ENABLE_LOOPBACK)
                REG(MCR,pChan) = REG(MCR,pChan) | MCR_LOOP;
            else if (arg == NS16550_DISABLE_LOOPBACK)
                REG(MCR,pChan) = REG(MCR,pChan) & ~MCR_LOOP;
            else
                status =  NS16550_EIO;

            break;

        case NS16550_MODE_SET:
            if ((arg != NS16550_MODE_POLL) && (arg != NS16550_MODE_INT))
                {
                status =  NS16550_EIO;
                break;
                }           

            if (arg == NS16550_MODE_INT)
                {
                /* Enable appropriate interrupts */

                REG(IER,pChan) = pChan->ier | IER_ERBI | IER_ETBEI;
                }
            else
                {
                /* Disable the interrupts */ 

                REG(IER,pChan) = 0;   
                }

            pChan->channelMode = arg;

            break;          

        case NS16550_MODE_GET:
            *(int32 *)arg = pChan->channelMode;
            break;

        case NS16550_AVAIL_MODES_GET:
            *(int32 *)arg = NS16550_MODE_INT | NS16550_MODE_POLL;
            break;

        case NS16550_WORD_LEN_SET:
            if ( arg < CHAR_LEN_6 || arg > CHAR_LEN_8 )
            {
                status = NS16550_EIO;
                break;
            }
            pChan->lcr = (pChan->lcr & ~LCR_WLN) | arg;
            REG(LCR,pChan) = pChan->lcr;
            break;
            
        case NS16550_WORD_LEN_GET:
            *(int32 *)arg = pChan->lcr & LCR_WLN;
            break;
            
        case NS16550_STOP_BITS_SET:
            if ( arg < ONE_STOP || arg > TWO_STOP )
            {
                status = NS16550_EIO;
                break;
            }
            pChan->lcr = (pChan->lcr & ~LCR_STB) | arg;
            REG(LCR,pChan) = pChan->lcr;
            break;
            
        case NS16550_STOP_BITS_GET:
            *(int32 *)arg = pChan->lcr & LCR_STB;
            break;
            
        case NS16550_PARITY_SET:
            if ( arg < PARITY_ODD || arg > PARITY_NONE )
            {
                status = NS16550_EIO;
                break;
            }
            if (arg == PARITY_NONE)
                pChan->lcr &= ~LCR_PEN;
            else
                pChan->lcr = (pChan->lcr & ~LCR_EPS) | LCR_PEN | arg;
            REG(LCR,pChan) = pChan->lcr;
            break;
            
        case NS16550_PARITY_GET:
            if (pChan->lcr & LCR_PEN)
                *(int32 *)arg = pChan->lcr & LCR_EPS;
            else
                *(int32 *)arg = PARITY_NONE;
            break;

        case NS16550_HW_OPTS_SET:
        case NS16550_HW_OPTS_GET:
        default:
            status = NS16550_ENOSYS;
        }
    return (status);
    }

/******************************************************************************
*
* ns16550PollOutput - output a character in polled mode.
*
* RETURNS: OK if a character arrived, EIO on device error, EAGAIN
*          if the output buffer if full.
*/
STATUS ns16550PollOutput
    (
    ns16550_chan_T *  pChan
    )
    {
	static uint32   busy_cnt = 0;

    /* is the transmitter ready to accept a character? */
    do {
//        tm_wkafter(1);
        /* prevent hang */
        if (++busy_cnt >= 300000)
        {
            /* reset Tx FIFO */
            REG(FCR,pChan) = TxCLEAR;
	        return (NS16550_EAGAIN);
        }
    } while (!(REG(LSR,pChan) & LSR_THRE));

    /* send character */
    REG(THR,pChan) = pChan->getTxArg;
        
    /* once succeed, clear busy count */
	busy_cnt = 0;

    return (NS16550_OK);
    }

/******************************************************************************
*
* ns16550PollInput - poll the device for input.
*
* RETURNS: OK if a character arrived, EIO on device error, EAGAIN
*          if the input buffer if empty.
*/

STATUS ns16550PollInput
    (
    ns16550_chan_T *  pChan
    )
    {
    uint8 pollStatus;

    /*---------------------------------------------------------------------*/
    /* Execute the loop as long as there is a receive event pending in the */
    /* chip. Break once a valid char got or no pending in RX.              */
    /*---------------------------------------------------------------------*/
    while ( (pollStatus = REG(LSR,pChan)) & RxCHAR_AVAIL )
    {
            /* Received a break or error                                   */
        if ( pollStatus & (LSR_BI | LSR_FERR) )
        {
            /* Received a break or error                                   */
            if (pollStatus & LSR_BI)
            {
                /* Flush the RX FIFO after the break.                  */
                REG(FCR,pChan) = RxCLEAR;
            }
            else
            {
                /* We must have an error. Discard the character associated with */
                /* the error.                                                   */
                if (REG(RBR,pChan));
            }
        }
        else
        {
            /* Received a valid character */
            pChan->putRcvArg = REG(RBR,pChan);
    
            return (NS16550_OK);
        }
    }

    return (NS16550_EAGAIN);
    }



/********************************************************************************
* ns16550Int - interrupt level processing
*
* This routine handles interrupts from the UART.
*
* RETURNS: N/A
*
*/
void ns16550Int 
    (
    ns16550_chan_T *pChan
    )
    {
    register uint8        intStatus;
    uint8       ch;


    /* read the Interrrupt Status Register (Int. Ident.) */
    intStatus = (REG(IIR, pChan)) & 0x0f;
    
    /*
     * This UART chip always produces level active interrupts, and the IIR 
     * only indicates the highest priority interrupt.  
     * In the case that receive and transmit interrupts happened at
     * the same time, we must clear both interrupt pending to prevent
     * edge-triggered interrupt(output from interrupt controller) from locking
     * up. One way doing it is to disable all the interrupts at the beginning
     * of the ISR and enable at the end.
     */
    REG(IER,pChan) = 0;    /* disable interrupt */
    while (intStatus)
    {
    switch (intStatus)
        {
        case IIR_RLS:
            /* overrun,parity error and break interrupt */
            if (REG(RBR, pChan));
            break;

        case IIR_RDA:     /* received data available */
        case IIR_TIMEOUT: /* receiver FIFO interrupt. In some case, IIR_RDA will
                             not be indicated in IIR register when there is more
                             than one char. in FIFO. */

            /* read character from Receive Holding Reg. */
            ch = REG(RBR, pChan);        
            if (ec_test)
                REG(THR,pChan) = ch;
            (*pChan->putRcvChar) (ch);
            break;

        case IIR_THRE:  /* transmitter holding register ready */
            if ((*pChan->getTxChar) (&ch) != NS16550_ERROR)
                REG(THR,pChan) = ch; /* write char. to Transmit Holding Reg. */
            else
                {
                pChan->ier &= (~IER_ETBEI); /* indicates to disable Tx Int */
                REG(IER,pChan) = pChan->ier;
        	}
            break;

        default:
            break;
        }
    intStatus = (REG(IIR, pChan)) & 0x0E;   
    }
        
    REG(IER,pChan) = pChan->ier; /* enable interrupts accordingly */
    

    }
