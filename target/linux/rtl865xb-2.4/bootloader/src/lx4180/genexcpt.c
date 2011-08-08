/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/lx4180/genexcpt.c,v 1.2 2004/03/31 01:49:20 yjlou Exp $
*
* Abstract: General exception handler.
*
* $Author: yjlou $
*
* $Log: genexcpt.c,v $
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


/* current interrupt priority level */
int32   _currIlev = 8;


#ifdef GNU
void genexcpt_handler(void) __attribute__ ((section (".iram")));
#else
#pragma ghs section text=".iram"
#endif
void genexcpt_handler(void)
{
    uint32      status_reg;
    uint32      cause_reg;
    uint32      exc_code;
    int32       intr_index;
    
    /* Load cause register */
    cause_reg = lx4180_ReadCause();
    
    /* Determine if this is an exception */
    if ( exc_code = ((cause_reg & 0xBC) >> 2) )
    {
        if ( exc_code != 9 )
        {
            printfByPolling("\n\nException %d!!", exc_code);
            printfByPolling("\n    EPC: 0x%08x", lx4180_ReadEPC());
        }
        gdb_exception(exc_code);
        
        /* Flush I-cache to ensure correct behavior of breakpoint and stepping, which 
        change instructions by data path (through D-cache) and thus might result in I-cache 
        content mismatch */
        lx4180_WriteCCTL(0);
        lx4180_WriteCCTL(2);
        lx4180_WriteCCTL(0);
        return;
    }
    
    /* Load status and cause registers.
        The later is to acknowledge interrupts since last load */    
    status_reg = lx4180_ReadStatus();
    cause_reg = lx4180_ReadCause();
    
    /* Extract only unmasked interrupts */
    cause_reg &= status_reg & 0xFF00;
    
    /* Determine interrupt source and call dispatcher */
    for (intr_index = 7; intr_index >= 0; intr_index--)
    {
        if ( cause_reg & 0x8000 )
        {
            int_Dispatch(intr_index);
        }
            
        cause_reg <<= 1;
    }
    
    return;
}
#ifndef GNU
#pragma ghs section text=default
#endif
