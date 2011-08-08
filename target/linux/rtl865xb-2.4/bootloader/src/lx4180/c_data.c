/*
 * ----------------------------------------------------------------
 * Copyright c                  Realtek Semiconductor Corporation, 2002
 * All rights reserved.
 *
 * $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/lx4180/c_data.c,v 1.4 2005/03/08 02:52:57 yjlou Exp $
 *
 * Abstract: To store C data (no code allowed)
 *
 * $Author: yjlou $
 *
 * $Log: c_data.c,v $
 * Revision 1.4  2005/03/08 02:52:57  yjlou
 * *: according to Ryan's suggest, we fixed the default value of RxDelay.
 *
 * Revision 1.3  2005/01/24 07:56:35  yjlou
 * *: do not consider CONFIG_RTL865X_CUSTOM_FLASH_MAP anymore.
 *
 * Revision 1.2  2004/11/15 04:35:28  yjlou
 * -: remove regMcr0_sdram_bus_width (use regMcr0_sdram instead).
 *
 * Revision 1.1  2004/11/10 13:02:22  yjlou
 * *: version migrates to "00.00.15".
 *    +: Integration with Linux menuconfig.
 *    +: add c_data.c
 *    *: change the structure of vectors.s for c_data.c
 *    *: SDRAM map is dynamicallly generated according to menuconfig.
 *
 *	
 */

#include "flashdrv.h"

/*
 *  If you want to store variables in flash area, use __C_DATA__
 */
#define __C_DATA__  __attribute__ ((section (".c_data"))) 



/****************************************************
 * #--- Loader Segment Descriptors Table
 *
 * Move from src/lx4180/vector.s
 *
	.globl	ldrSegDesTable
ldrSegDesTable:
	#       Start addr  End addr
	.word	0x57888651, 0x57888651	# magic number
	.word	0x00000000, 0x00004000	# segment 0
	.word	0x00008000, 0x00010000	# segment 1
	.word	0x00010000, 0x00020000	# segment 2
	.word	0x00000000, 0x00000000	# ending mark

 ****************************************************/

#if 1/*CONFIG_RTL865X_CUSTOM_FLASH_MAP*/

__C_DATA__ struct LDR_SEG_DESC ldrSegDesTable[]=
{
	{ LDR_SEG_DESC_MAGIC, LDR_SEG_DESC_MAGIC },
	{ CONFIG_RTL865X_CUSTOM_LOADER_SEG1_ADDRESS, CONFIG_RTL865X_CUSTOM_LOADER_SEG1_ADDRESS+CONFIG_RTL865X_CUSTOM_LOADER_SEG1_SIZE },
	{ CONFIG_RTL865X_CUSTOM_LOADER_SEG2_ADDRESS, CONFIG_RTL865X_CUSTOM_LOADER_SEG2_ADDRESS+CONFIG_RTL865X_CUSTOM_LOADER_SEG2_SIZE },
	{ CONFIG_RTL865X_CUSTOM_LOADER_SEG3_ADDRESS, CONFIG_RTL865X_CUSTOM_LOADER_SEG3_ADDRESS+CONFIG_RTL865X_CUSTOM_LOADER_SEG3_SIZE },
	{ 0x00000000, 0x00000000 },
};

#else

__C_DATA__ struct LDR_SEG_DESC ldrSegDesTable[]=
{
	{ 0x57888651, 0x57888651 },
	{ 0x00000000, 0x00004000 },
	{ 0x00008000, 0x00020000 },
	{ 0x00000000, 0x00000000 },
};

#endif


/****************************************************
 * MCR - Memory Control Registers
 *
 *  regMcr0 = 0xbd013000
 *  regMcr1 = 0xbd013004
 *  regMcr2 = 0xbd013008
 *  regMcrD = 0xbd01204c
 ****************************************************/
__C_DATA__ uint32 regMcr0 = 0xcaa00000;
__C_DATA__ uint32 regMcr1 = 0x1b1b1b00;
__C_DATA__ uint32 regMcr2 = 0x00000cea;
__C_DATA__ uint32 regMcrD = 0x00000003;

__C_DATA__ uint32 regMcr0_sdram = CONFIG_RTL865X_MCR_SDRAM;

#ifdef CONFIG_BANK1_ROM_TYPE
	__C_DATA__ uint32 regMcr0_bank1type = 0<<15;
#else/*CONFIG_BANK1_IO_TYPE*/
	__C_DATA__ uint32 regMcr0_bank1type = 1<<15;
#endif


