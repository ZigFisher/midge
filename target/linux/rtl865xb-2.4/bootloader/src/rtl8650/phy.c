/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/rtl8650/phy.c,v 1.3 2004/07/14 02:16:09 yjlou Exp $
*
* Abstract: Phy access driver source code.
*
* $Author: yjlou $
*
* $Log: phy.c,v $
* Revision 1.3  2004/07/14 02:16:09  yjlou
* +: add '#ifdef FAT_CODE' to remove un-used functions
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
* ---------------------------------------------------------------
*/

#include <rtl_types.h>
#include <rtl8650/asicregs.h>
#include <rtl8650/phy.h>



#ifdef FAT_CODE

uint32 phy_readReg(uint32 port, uint32 regnum)
{
    ASSERT_CSP( port < MAX_PORT_NUMBER );
    ASSERT_CSP( regnum <= PHY_ANLP_REG );
    
    return REG32(PHY_BASE + (port << 5) + (regnum << 2));
}

int32 phy_writeReg(uint32 port, uint32 regnum, uint32 value)
{
    ASSERT_CSP( port < MAX_PORT_NUMBER );
    ASSERT_CSP( regnum <= PHY_ANLP_REG );
    
    /* Wait for command ready */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );
    
    REG32(TCR0) = value;
    
    /* Fill address */
    REG32(SWTAA) = PHY_BASE + (port << 5) + (regnum << 2);
        
    /* Activate add command */
    REG32(SWTACR) = ACTION_START | CMD_FORCE;
    
    /* Wait for command done */
    while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE );
    
    /* Check status */
    if ( (REG32(SWTASR) & TABSTS_MASK) == TABSTS_SUCCESS )
        return 0;
        
    /* There might be something wrong */
    ASSERT_CSP( 0 );
}

#endif//FAT_CODE

