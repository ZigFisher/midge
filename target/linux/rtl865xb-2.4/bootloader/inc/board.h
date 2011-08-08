/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/inc/board.h,v 1.26 2005/01/28 02:03:48 yjlou Exp $
*
* Abstract: Board specific definitions.
*
* $Author: yjlou $
*
* $Log: board.h,v $
* Revision 1.26  2005/01/28 02:03:48  yjlou
* *: loader version migrates to "00.00.19".
* +: support Hub mode
* +: Ping mode support input IP address
* *: clear WDTIND always.
*
* Revision 1.25  2005/01/24 01:32:42  danwu
* + add 8186 support
*
* Revision 1.24  2005/01/19 11:46:33  yjlou
* +: support _SUPPORT_UPDATE_ROM_
* *: fixed th bug of program bdinfo: we should use FLASH_MAP_BOARD_INFO_ADDR, instead of flash_chip_info[0].BoardInfo.
*
* Revision 1.23  2005/01/18 06:57:08  yjlou
* +: add _TEST_SDRAM_
*
* Revision 1.22  2004/11/18 14:42:09  yjlou
* *: change DRAM_MAP_LOADER_ADDR to CONFIG_RTL865X_DRAM_MAP_LOADER_ADDR.
* *: chanhe DRAM_MAP_DLOAD_BUF_ADDR to CONFIG_RTL865X_DRAM_MAP_DLOAD_BUF_ADDR.
*
* Revision 1.21  2004/11/10 13:02:21  yjlou
* *: version migrates to "00.00.15".
*    +: Integration with Linux menuconfig.
*    +: add c_data.c
*    *: change the structure of vectors.s for c_data.c
*    *: SDRAM map is dynamicallly generated according to menuconfig.
*
* Revision 1.20  2004/10/11 10:23:08  yjlou
* *: restrict only 865xB revB code apply patch code.
*
* Revision 1.19  2004/09/29 08:03:50  yjlou
* +: support PING mode
* *: change menu style
*
* Revision 1.18  2004/09/02 02:53:15  yjlou
* *: defualt DRAM_MAP_RUN_IMAGE_ADDR has changed from 0x80000400 to 0x80080000. (in order to be compatible with old-fashion runtime code)
*
* Revision 1.17  2004/08/23 08:02:54  yjlou
* *: loader is moved from 0x80500000 to 0x80b00000.
* *: DRAM_MAP_DLOAD_BUF_ADDR is changed from 0x80600000 to 0x80c00000.
*
* Revision 1.16  2004/08/23 07:08:30  yjlou
* *: loader upgrades to "00.00.11"
* *: loader is moved from 0x80000000 to 0x80500000 (to reduce SDRAM consumption).
* *: communication section is cast to "string", not "function pointer".
*
* Revision 1.15  2004/08/04 14:58:27  yjlou
* *: By default, we DO NOT define _SUPPORT_LARGE_FLASH_ !!
*
* Revision 1.14  2004/08/04 14:55:23  yjlou
* +: Loader version upgraded to '00.00.09'
* +: support booting from single 8MB/16MB flash (_SUPPORT_LARGE_FLASH_)
* -: merge rtl_bdinfo.h into flashdrv.h
*
* Revision 1.13  2004/08/03 05:23:10  yjlou
* +: porting _chip_is_shared_pci_mode() to loader. loader supports auto-detect console UART.
* -: remove _50B_PCI_ENABLED_
*
* Revision 1.12  2004/07/26 03:21:58  yjlou
* *: change default configuration: _50B_PCI_ENABLED_ and BICOLOR_LED
*
* Revision 1.11  2004/07/12 11:57:47  yjlou
* +: support BICOLOR_LED
*
* Revision 1.10  2004/05/26 06:51:49  yjlou
* *: use IS_865XB() instead of IS_REV_B()
* *: use IS_865XA() instead of IS_REV_A()
*
* Revision 1.9  2004/05/21 11:43:17  yjlou
* +: support hub mode (still buggy) and erase flash function.
*
* Revision 1.8  2004/05/13 13:27:01  yjlou
* +: loader version is migrated to "00.00.07".
* +: new architecture for INTEL flash (code is NOT verified).
* *: FLASH_BASE is decided by IS_REV_A()
* -: remove flash_map.h (content moved to flashdrv.h)
* -: remove un-necessary calling setIlev()
*
* Revision 1.7  2004/05/12 06:46:49  yjlou
* *: support testing FLASH memory: testFlash()
*
* Revision 1.6  2004/05/06 10:36:30  yjlou
* *: Damn ! The original IS_REV_B() macro is correct!!!!!
*
* Revision 1.5  2004/05/06 03:49:39  yjlou
* +: add IS_REV_A() and IS_REV_B() macro
*
* Revision 1.4  2004/04/20 12:19:01  yjlou
* +: support 50B that UART0 pins share with PCI (_50B_PCI_ENABLED_)
*
* Revision 1.3  2004/04/01 15:41:15  yjlou
* +: //#define _L2_MODE_
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

#ifndef _BOARD_H
#define _BOARD_H

#include <linux/autoconf.h>


//#define _L2_MODE_
//#define _DEBUG_

/* support single 8MB/16MB flash */
/*#undef _SUPPORT_LARGE_FLASH_ use menuconfig setting */ 

/* Enable dump hub */
#define _HUB_MODE_

/* Ping mode */
#define _PING_MODE_

/* Bi-color LED */
#ifdef CONFIG_RTL865X_BICOLOR_LED /* use menuconfig setting */
	#define BICOLOR_LED 
#endif

/* Bi-color LED for BXXX */
//#define BICOLOR_LED_VENDOR_BXXX

/* Compile update ROM function */
#define _SUPPORT_UPDATE_ROM_

/* Compile SDRAM test function */
#define _TEST_SDRAM_

/* Compile flash test function */
#define _TEST_FLASH_

/* Compile erase flash function */
#define _ERASE_FLASH_

/* Console definition 
*/
#define DEFAULT_BAUDRATE 38400



/* Interrupt source definition
*/
#define MAX_ILEV                        8
#define MAX_NUM_OF_CHAINING_INTR        3

#define LBCTMO_ILEV                     6
#define UART1_ILEV                      6
#define TICK_ILEV                       5
#define SW_ILEV                         4
#define UART0_ILEV                      3
#define PABC_ILEV                       3


#ifdef CONFIG_RTL8186
#undef TICK_ILEV
#define TICK_ILEV				2
#undef UART0_ILEV
#define UART0_ILEV				5
#endif



/* Address Map
*/
//#define FLASH_BASE                          0xbfc00000
#define FLASH_BASE                          (IS_865XA()?0xBFC00000:0xBE000000)
#define CRMR_ADDR                           CRMR
#ifdef CONFIG_RTL865X_CUSTOM_LOADER_MAP
	#define DRAM_MAP_DLOAD_BUF_ADDR				CONFIG_RTL865X_DRAM_MAP_DLOAD_BUF_ADDR
#else
	#define DRAM_MAP_DLOAD_BUF_ADDR             0x80C00000
#endif
#define DRAM_MAP_INJECT_BUF_ADDR            0x80100000 /* buffer for inject JUMP instructions */
#ifdef CONFIG_RTL865X_RUNTIME_ADDRESS
	#define DRAM_MAP_RUN_IMAGE_ADDR             0x80080000 /* default */
#else
	#define DRAM_MAP_RUN_IMAGE_ADDR             CONFIG_RTL865X_RUNTIME_ADDRESS /* according to menuconfig */
#endif
#define BSP_STACK_SIZE                      4096        /* size of system stack space */

#define UNCACHED_MALLOC(x)  (void *) (0xa0000000 | (uint32) malloc(x))



/* Tick definition 
*/
#define TICK_FREQ       TICK_10MS_FREQ
#define TICK_10MS_FREQ  100 /* 100 Hz */
#define TICK_100MS_FREQ 1000 /* 1000 Hz */



/* Define flash device 
*/
#define FLASH_AM29LV800BB   /* only use 1MB currently */
#undef FLASH_AM29LV160BB

#define MAX_PORT_NUM    5



/* Nic definitions 
*/
#define NUM_RX_PKTHDR_DESC          16 //80 //16
#define NUM_RX_MBUF_DESC            16
#define NUM_TX_PKTHDR_DESC          16
#define MBUF_LEN                    2048



/* Debug function calls 
*/
#define debug_printf printf



/* Calculation
*/
#define DIV(x,y)        ((x)/(y))
#define MOD(x,y)        ((x)%(y))
#define MUL(x,y)        ((x)*(y))


/* macro to tell 865X_A or 865X_B */
#define IS_865XB() ( ( REG32( CRMR ) & 0xffff ) == 0x5788 )
#define IS_865XA() ( !IS_865XB() )

#endif   /* _BOARD_H */

