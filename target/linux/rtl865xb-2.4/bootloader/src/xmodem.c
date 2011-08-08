/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/xmodem.c,v 1.2 2004/03/31 01:49:20 yjlou Exp $
*
* Abstract: Xmodem download source code.
*
* $Author: yjlou $
*
* $Log: xmodem.c,v $
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

#ifndef         TRUE
#define         TRUE            1
#endif

#ifndef         FALSE
#define         FALSE           0
#endif

#define ERROR           -1
#define NO_ERROR        0

#define	DEV_IN_PRISM

#ifdef DEV_IN_PRISM
    #define LONG64  long    long
#else
    #define asm
    #define LONG64  long
#endif

/* --------------------------------------------------------- */
/* Interested ASCII characters macro definition              */
/* --------------------------------------------------------- */
#define         SOH                     0x01            /* Start Of Header       */
#define         EOT                     0x04            /* End Of Transmission   */
#define         ACK                     0x06            /* Acnowledge (positive) */
#define         NAK                     0x15            /* Negative Acnowledge   */
#define         CAN                     0x18            /* Cancel                */

#define         ERROR_MAX       15                      /* max errors tolerated  */
#define         XMOD_SIZE       128                     /* size of xmodem buffer */

#define         TIMEOUT         -1                      /* no char received, must not be 0 ~ 0xff */

extern  uint64           tick_GetSysUpTime(uint64 *);

/* ----------------------------------------------------------------- */
/*                         LOCAL ROUTINES                            */
/* ----------------------------------------------------------------- */
int16    xmodem_ReadPkt(uint8*);
int16    xmodem_ReadByte(int16);
void    xmodem_SendByte(uint8);


/********************************************************************/
/* xmodem_Receive()                                             */
/*                                                                  */
/* Abstraction : Receive Xmodem Transmission                        */
/********************************************************************/
uint16 xmodem_Receive(uint32 *image_len, uint8 *downloadBuf)
{
        int16    c;                  /* 1st byte of packet */
        int16    errors;             /* Running count of errors */
        int16    error_flag;         /* True when error found while reading a packet */
        int16    fatal_error;        /* True when fatal error found while reading packet */
        uint8     pkt_num;            /* 2nd byte of packet: packet # (mod 128) */
        uint8     pkt_chk;            /* 3rd byte of packet: should be complement of pkt_num */
        int16    pkt_cnt;            /* total # of packets received so far */
        uint8     xmdm_buff[XMOD_SIZE+10]; /* buffer for data              */
        int16    initial_tries;
        uint16    err_no, ret_val;    /* error number, return value */
        uint8     *rx_ptr;            /* pointer to download receive area */

        rx_ptr = downloadBuf; 
        fatal_error = FALSE;
        errors = pkt_cnt = 0;
        *image_len = 0;
        /*
         * Start communications by providing NAK to the sending	end. The number
		 * and time over which these are provided is 10 seconds at once per second,
		 * followed by about one minute at 4 second intervals.  the maximum tolerable
         * inter-character time is 3 seconds, and if that occurs, we will wait for 1
		 * second of silence from the host and then NAK the packet.
        */
        initial_tries = 60;

//        uart_Init_Queue();
        do
        {
            xmodem_SendByte(NAK);
            initial_tries--;
        } while(initial_tries && ((c = xmodem_ReadByte(3)) == TIMEOUT));
        /*
         * Check if CTRL_C break key (only for B3 project), works only when
         * remote xmodem not started.
        */
        if (c == 0x03)
        {
            return 1;   /* action canceled by user */
        }
        goto PreLoaded;
        /* ------------------------------------------------------- */
        /* Main Do-While loop to read packets                      */
        /* ------------------------------------------------------- */
        do
        {
            /* start by reading first byte in packet */
            c = xmodem_ReadByte(3);
PreLoaded:
            error_flag = FALSE;
            switch (c)
            {
                default:
                    continue;
                case ACK:                       /* transmitter confirms reception */
                    if (pkt_cnt > 0)
                    {
                        continue;
                    }
                    break;
                case EOT:                       /* transmitter signals finish */
                    /* check for REAL EOT */
                    if ((c = xmodem_ReadByte(1)) == TIMEOUT)
                    {
                        c = EOT;
                    }
                    /* was EOT from data portion of a (delayed) packet? */
                    else if (c != EOT)
                    {
                        c = TIMEOUT;
                        error_flag = TRUE;
                    }
                    break;
                case TIMEOUT:
                    error_flag = TRUE;
                    break;
                case CAN:       /* bailing out? */
                    if ((xmodem_ReadByte(3) & 0x7f) != CAN)
                    {
                        error_flag = TRUE;
                    }
                    else
                    {
                        err_no = 2;     /* remote cancel */
                        fatal_error = TRUE;
                    }
                    break;
                case SOH:
                    /* start reading packet */
                    pkt_num = xmodem_ReadByte(3);
                    pkt_chk = xmodem_ReadByte(3);
                    if (pkt_num != (uint8)~pkt_chk)
                    {
                        /* MISREAD PACKET # */
                        error_flag = TRUE;
                        break;
                    }
                    if (pkt_num == (pkt_cnt & 0xff))
                    {
                        /* DUPLICATE PACKET -- DISCARD */
                        while (xmodem_ReadByte(3) != TIMEOUT){;}
                        xmodem_SendByte(ACK);
                        break;
                    }
                    if (pkt_num != ((pkt_cnt+1) & 0xff))
                    {
                            /* PHASE ERROR */
                            err_no = 2;     /* remote cancel */
                            fatal_error = TRUE;
                            break;
                    }
                    /* Read and calculate checksum for a packet of data */
                    if (xmodem_ReadPkt(xmdm_buff) == ERROR)
                    {
                            error_flag = TRUE;
                            break;
                    }
                    errors = 0;
                    pkt_cnt++;
                    /* Copy data to receive area */
                    /* Always copy XMOD_SIZE, even the last block which may
                     * have paddings. But it's ok, since there is a 'imageEnd'
                     * parameter in the image header which will tell the
                     * true ending point of the image and is referenced by
                     * later processing when need it (checksum calculation,
                     * flashrom write, etc.)
                     */
                    /* KNL_BlockCopy(xmdm_buff, rx_ptr, XMOD_SIZE); */
                    /* check end bound ????? */
                    memcpy(rx_ptr, xmdm_buff, XMOD_SIZE);
                    rx_ptr += XMOD_SIZE;
                    /* ui_imageSize += XMOD_SIZE;  */ /* compute image size, 11/15/1997 */
                    *image_len += XMOD_SIZE;
                    /* ACK the packet. (Must be here!) */
                    xmodem_SendByte(ACK);
                    break;
            } /* END OF 'xmodem_ReadByte' switch */
            /* check on errors or batch transfers */
            if (error_flag || pkt_cnt == 0)
            {
                if (error_flag)
                {
                    if ((++errors) >= ERROR_MAX)
                    {
                        err_no = 3;     /* too many errors */
                        fatal_error = TRUE;
                    }
                }
                if (!fatal_error)
                {
                        /* wait for line to settle */
                        while (xmodem_ReadByte(1) != TIMEOUT){;}
                        xmodem_SendByte(NAK);
                }
            }
        } while ((c != EOT) && (!fatal_error));                 /* end of MAIN Do-While */
        if (c == EOT)
        {
            /* normal exit */
            xmodem_SendByte(ACK);
            ret_val = 0;   /* successfully, 11/15/1997 */
        }
        else
        {
            /* error exit */
            if (pkt_cnt != 0)
            {
                xmodem_SendByte(CAN);
                xmodem_SendByte(CAN);
            }
            ret_val = err_no;
        }
        return (ret_val);
}


/*********************************************************/
/* xmodem_ReadPkt()                                    */
/*                                                       */
/* Get a packet (XMOD_SIZE bytes) from uart.             */
/* Timeout if 3 "seconds" elapse.                        */
/*                                                       */
/* Return NO_ERROR on success, ERROR on TIMEOUT or bad   */
/* checksum.                                             */
/*********************************************************/
int16 xmodem_ReadPkt(uint8 *buf)
{
        int16    count;              /* Number of characters read */
        int16    c;                  /* Next character read */
        uint8     chksum;             /* Accumulate checksum here */

        chksum = 0;

        for (count=0; count<XMOD_SIZE; count++)
        {
//                buf[count] = c = xmodem_ReadByte(300);
                buf[count] = c = xmodem_ReadByte(3);
                if (c == TIMEOUT)
                {
                        return (ERROR);       /* abort operation */
                }
                chksum += c;
        }

        /* Read and confirm checksum */
        c = xmodem_ReadByte(3);

        if ((c == TIMEOUT) || ((c & 0xff) != chksum))
        {
                return (ERROR);
        }

        return (NO_ERROR);
}


/*****************************************************/
/* xmodem_ReadByte()                               */
/*                                                   */
/* This routine waits until a character is           */
/* available at the console input. If the char       */
/* is available before the timeout on the serial     */
/* channel, it returns the character. Otherwise,     */
/* return TIMEOUT error.                             */
/*****************************************************/
int16 xmodem_ReadByte(int16 seconds)
{
        uint64  c_time, s_time, e_time;
        uint8     ch;
#if 1
        tick_GetSysUpTime(&s_time);
#else
   		ULONG   imask;
		s_time = (uint64)0L;
#endif        
        e_time = seconds * 1000;

        while (getch(&ch) == 0)
        {
#if 1
        		tick_GetSysUpTime(&c_time);
                if ((c_time - s_time) >= e_time)
                {
                return (TIMEOUT);
            }
#else
			imask = splx(0);
			if (imask == 1)
			{
				ch='0';
			}
			micro_delay(10);
			s_time++;
			if (s_time >= e_time)
			{
                return (TIMEOUT);				
			}
#endif			
        }
        return ((int16)ch);
}

/************************************************/
/* xmodem_SendByte()                            */
/*                                              */
/* Send character to console output.            */
/************************************************/
void xmodem_SendByte(uint8 ch)
{
    putchar(ch);  /* error: 0, success: 1 */
}


