/*
 * Copyright c                Realtek Semiconductor Corporation, 2002
 * All rights reserved.                                                    
 * 
 * $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/rtl8650/int.c,v 1.5 2004/07/14 02:16:51 yjlou Exp $
 *
 * $Author: yjlou $
 *
 * Abstract:
 *
 *   Interrupt dispatcher source code.
 *
 * $Log: int.c,v $
 * Revision 1.5  2004/07/14 02:16:51  yjlou
 * +: add '#ifdef FAT_CODE' to remove un-used functions
 *
 * Revision 1.4  2004/05/12 09:42:17  yjlou
 * *: int_unRegister() also needs setIlev()'s protection.
 *
 * Revision 1.3  2004/05/12 09:37:20  yjlou
 * *: fixed the bug of int_Register(): setIlev(MAX_ILEV) to protect critical section.
 *
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
 */

#include <rtl_types.h>
#include <rtl_errno.h>
#include <lx4180/eregdef.h>
#include "board.h"
#include <rtl8650/asicregs.h>



/* DATA STRUCTURE DECLARATIONS
*/
struct intr_register_s {
    int32   gimrMaskBit;
    void    (*handler)(void);
};



/* STATIC VARIABLE DECLARATIONS
 */
static struct intr_register_s intr_register[MAX_ILEV][MAX_NUM_OF_CHAINING_INTR];



void int_InitEnable(void)
{
    uint32 sts;
    
    /* Clear interrupt status register */
    REG32(GISR) = 0;
    
    /* Clear interrupt routing register */
    REG32(IRR) = 0;
    
    /* Clear interrupt mask register */
    REG32(GIMR) = 0;

    /* Clear interrupt register table */
    bzero((void *)intr_register, MAX_ILEV * MAX_NUM_OF_CHAINING_INTR * sizeof(struct intr_register_s));
    
    //* (uint8 *) 0x80600000 = 0x12;
    
    /* Modify status register to enable interrupt and mask all sources */
    sts = lx4180_ReadStatus();
    sts = ( sts & ~(SR_IMASK | SR_BEV) ) | SR_IEC;
    lx4180_WriteStatus(sts);
}

int32 int_Register(uint32 ilev, int32 gimrMaskBit, int32 irrOffset, void (*handlerRoutine)())
{
    uint32  chainCount = 0;
    int32 imask;
    int32 ret;
    
    ASSERT_CSP( ilev < MAX_ILEV );
    ASSERT_CSP( handlerRoutine );

	// here is critical section
	imask = setIlev( MAX_ILEV );	
    
    /* Set interrupt mask register */
    REG32(GIMR) |= gimrMaskBit;

    /* Set interrupt routing register */
    REG32(IRR) |= (ilev - 3) << irrOffset;
    
    /* Register handler routine */
    while ( chainCount < MAX_NUM_OF_CHAINING_INTR )
    {
        if ( !intr_register[ilev][chainCount].handler )
        {
            intr_register[ilev][chainCount].gimrMaskBit = gimrMaskBit;
            intr_register[ilev][chainCount].handler = handlerRoutine;
            ret = 0;
            goto out;
        }
        chainCount++;
    }

    /* Resource runout for chaining interrupt */
    ret = ENFILE;

out:
	// leave critical section
    setIlev( imask );

    return ret;
}

#ifdef FAT_CODE
int32 int_unRegister(uint32 ilev, int32 gimrMaskBit, int32 irrOffset, void (*handlerRoutine)())
{
    uint32  chainCount = 0;
	int32 imask;
    int32 ret;
   
    ASSERT_CSP( ilev < MAX_ILEV );
    ASSERT_CSP( handlerRoutine );
    
	// here is critical section
	imask = setIlev( MAX_ILEV );	
	
    /* Clear interrupt mask register */
    REG32(GIMR) &= ~gimrMaskBit;

    /* Clear interrupt routing register */
    REG32(IRR) &= ~(3 << irrOffset);
    
    /* Clear handler routine */
    while ( chainCount < MAX_NUM_OF_CHAINING_INTR )
    {
        if ( intr_register[ilev][chainCount].handler )
        {
            if ( intr_register[ilev][chainCount].handler == handlerRoutine )
            {
                intr_register[ilev][chainCount].gimrMaskBit = 0;
                intr_register[ilev][chainCount].handler = NULL;
	            ret = 0;
	            goto out;
            }
        }
        else
            break;
            
        chainCount++;
    }
    
    /* Not found */
    ret = ENOENT;

out:
	// leave critical section
    setIlev( imask );

    return ret;
}
#endif// FAT_CODE
	
void int_Dispatch(uint32 ilev)
{
    uint32  chainCount;
    
    if ( ilev < 2 )
        /* Clear software interrupt */
        lx4180_WriteCause(0);
    
    for (chainCount=0; chainCount<MAX_NUM_OF_CHAINING_INTR; chainCount++)
    {
        /* Dispatch to registered handler */
        if ( intr_register[ilev][chainCount].handler )
        {
            if ( REG32(GISR) & intr_register[ilev][chainCount].gimrMaskBit )
                (*intr_register[ilev][chainCount].handler)();
        }
        else
            break;
    }
}
