/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/rtl8650/swCore.c,v 1.27 2005/04/24 03:49:59 yjlou Exp $
*
* Abstract: Switch core driver source code.
*
* $Author: yjlou $
*
* $Log: swCore.c,v $
* Revision 1.27  2005/04/24 03:49:59  yjlou
* *: do not touch other fields, except SYS_CLK_MASK, of MACMR
*
* Revision 1.26  2005/01/28 02:03:48  yjlou
* *: loader version migrates to "00.00.19".
* +: support Hub mode
* +: Ping mode support input IP address
* *: clear WDTIND always.
*
* Revision 1.25  2004/12/28 12:28:53  yjlou
* *: version migrates to 00.00.17
* *: VLAN MTU changes to 1500 (not 1522).
* *: support I28F320J3A flash
*
* Revision 1.24  2004/10/01 01:39:30  yjlou
* -: remove Mii port from VLAN member port
*
* Revision 1.23  2004/08/02 01:12:28  yjlou
* *: fixed the compile error
*
* Revision 1.22  2004/07/30 10:51:22  yjlou
* *: To get the correct link status, register PORT0_PHY_STATUS must be read twice.
*
* Revision 1.21  2004/07/26 08:12:52  yjlou
* +: add code to check BIST Queue Memory and Packet Buffer
*
* Revision 1.20  2004/07/13 02:59:42  yjlou
* +: enable Mii at swCore_init()
*
* Revision 1.19  2004/07/12 11:57:47  yjlou
* +: support BICOLOR_LED
*
* Revision 1.18  2004/06/11 07:12:11  yjlou
* *: change compile flag from 'CONFIG_RTL865X_BICOLOR_LED' to 'BICOLOR_LED_VENDOR_BXXX'
*
* Revision 1.17  2004/05/21 11:43:17  yjlou
* +: support hub mode (still buggy) and erase flash function.
*
* Revision 1.16  2004/05/14 09:39:42  orlando
* check in CONFIG_RTL865X_BICOLOR_LED/DIAG_LED/INIT_BUTTON related code
*
* Revision 1.15  2004/05/06 11:32:41  yjlou
* +: add PORT6_PHY_CONTROL series register
*
* Revision 1.14  2004/05/06 03:52:50  yjlou
* +: code for Buffalo bi-color LED (defualt marked)
*
* Revision 1.13  2004/05/04 06:24:27  yjlou
* *: fixed the FCRTH value (Flow Control Threshold Register)
*
* Revision 1.12  2004/04/23 04:33:08  yjlou
* *: fixed the bug of TEATCR L2 Aging field.
*
* Revision 1.11  2004/04/13 02:11:32  yjlou
* *: clear FULL_RST and SEMI_RST bit to 0 after reset.
*
* Revision 1.10  2004/04/05 09:32:46  yjlou
* *: fixed the bug of TRXRDY always high when reseting.
*
* Revision 1.9  2004/04/01 12:41:26  yjlou
* *: fixed FullAndSemiReset() to be compatible with 8651
*
* Revision 1.8  2004/04/01 12:17:45  yjlou
* *: Patched the bug of Switch Core Full Reset: add FullAndSemiReset()
*
* Revision 1.7  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.6  2004/03/30 11:34:38  yjlou
* *: commit for 80B IC back
*   +: system clock rate definitions have changed.
*   *: fixed the bug of BIST_READY_PATTERN
*   +: clean all ASIC table when init ASIC.
* -: define _L2_MODE_ to support L2 switch mode.
*
* Revision 1.5  2004/03/29 03:42:25  yjlou
* +: Initializing for 8650 backward compatible mode.
*
* Revision 1.4  2004/03/26 09:20:28  yjlou
* +: add code for Buffalo bi-color LED (but commented)
*
* Revision 1.3  2004/03/25 08:59:58  yjlou
* -: remove 'int_Register(SW_ILEV, SWIE, SWIRS_OFFSET, swCore_intHandler);' in swCore_init()
*
* Revision 1.2  2004/03/22 05:54:55  yjlou
* +: support two flash chips.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.4  2004/03/16 06:04:04  yjlou
* +: support pure L2 switch mode (for hardware testing)
*
* Revision 1.3  2004/03/09 00:45:01  danwu
* remove unused code to shrink loader image size under 0xc000 and only flash block 0 & 3 occupied
*
* Revision 1.2  2004/01/27 08:37:05  tony
* small change
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
#include "board.h"
#include <rtl8650/asicregs.h>
#include <rtl8650/swCore.h>
#include <rtl8650/phy.h>
#include <linux/autoconf.h>


/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static void     swCore_intHandler(void);

static void _rtl8651_clearSpecifiedAsicTable(uint32 type, uint32 count) {
	struct { uint32 _content[8]; } entry;
	uint32 idx;
	
	bzero(&entry, sizeof(entry));
	for (idx=0; idx<count; idx++)// Write into hardware
		swTable_addEntry(type, idx, &entry);

}


void FullAndSemiReset( void )
{
	int c;

    /* Perform full-reset for sw-core. */ 
    REG32(MACCR) |=  FULL_RST;
    
	/* Wait BISTing bit become 1. */
	c = 0;
	while( REG32( BISTCR ) & 0x40000000 == 0 )
    {
    	tick_Delay10ms(1);
    	c++;
    	if(c>100) break;
	}

	/* Wait BISTing bit become 0. */
	c = 0;
	while( REG32( BISTCR ) & 0x40000000 )
    {
    	tick_Delay10ms(1);
    	c++;
    	if(c>100) break;
	}

	/* Delay 100ms after BISTing bit became 0. */
   	tick_Delay100ms(1);

	/* Disable TRXRDY */
	REG32( BISTCR ) &= ~TRXRDY;

	/* Semi-Reset */
	REG32( MACCR ) |= SEMI_RST;

	/* Enable TRXRDY */
	REG32( BISTCR ) |= TRXRDY;
	
	/* Wait QOK and COK bit all become 1. */
	c = 0;
	while( ( REG32( BISTCR ) & 0x30000000 ) != 0x30000000 )
	{
    	tick_Delay10ms(1);
    	c++;
    	if(c>100) break;
	}

	/* clear bits */
    REG32(MACCR) &=  ~(FULL_RST|SEMI_RST);
}


int32 swCore_init()
{
    int c;

	/* Full reset and semreset */
	FullAndSemiReset();
    
	/* Initializing for 8650 backward compatible mode */
	REG32(SWTECR) = 0x00000000;

    /* Disable NIC Tx/Rx and reset all descriptors */
    REG32(CPUICR) &= ~(TXCMD | RXCMD);
    
    /* Check BIST until all packet buffers are reset */
	c = 0;
    while ( ( REG32(BISTCR) & BIST_READY_PATTERN ) != BIST_READY_PATTERN )
    {
    	tick_Delay10ms(1);
    	c++;
    	if(c>500) break;
	}
	if(c>500)
	{
		printf("swCore_init(): BIST failed, BISTCR=0x%08x\n", REG32(BISTCR) );
		while(1);
	}

	/* Check Queue Memory is OK */
	if ( ( REG32(BISTCR) & BIST_QUEUE_MEMORY_FAIL_PATTERN ) == BIST_QUEUE_MEMORY_FAIL_PATTERN )
	{
		printf("swCore_init(): BIST: Queue Memory failed, BISTCR=0x%08x\n", REG32(BISTCR) );
		while(1);
	}

	/* Check Packet Buffer is OK */
	if ( ( REG32(BISTCR) & BIST_PACKET_BUFFER_FAIL_PATTERN ) == BIST_PACKET_BUFFER_FAIL_PATTERN )
	{
		printf("swCore_init(): BIST: Packet Buffer failed, BISTCR=0x%08x\n", REG32(BISTCR) );
		while(1);
	}

	/* rtl8651_clearAsicAllTable */
	_rtl8651_clearSpecifiedAsicTable(TYPE_L2_SWITCH_TABLE, RTL8651_L2TBL_ROW*RTL8651_L2TBL_COLUMN);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ARP_TABLE, RTL8651_ARPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L3_ROUTING_TABLE, RTL8651_ROUTINGTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_MULTICAST_TABLE, RTL8651_IPMULTICASTTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_PROTOCOL_TRAP_TABLE, RTL8651_PROTOCOLTRAP_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_VLAN_TABLE, RTL8651_VLAN_NUMBER);
	_rtl8651_clearSpecifiedAsicTable(TYPE_EXT_INT_IP_TABLE, RTL8651_IPTABLE_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ALG_TABLE, RTL8651_ALGTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_SERVER_PORT_TABLE, RTL8651_SERVERPORTTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L4_TCP_UDP_TABLE, RTL8651_TCPUDPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L4_ICMP_TABLE, RTL8651_ICMPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_PPPOE_TABLE, RTL8651_PPPOE_NUMBER);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ACL_RULE_TABLE, RTL8651_ACLTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_NEXT_HOP_TABLE, RTL8651_NEXTHOPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_RATE_LIMIT_TABLE, RTL8651_RATELIMITTBL_SIZE);

    
    #if defined(BICOLOR_LED_VENDOR_BXXX)
    /* Install interrupt handler */
    int_Register(SW_ILEV, SWIE, SWIRS_OFFSET, swCore_intHandler);
    #endif /* BICOLOR_LED_VENDOR_BXXX */
    
    /* Enable back pressure and PHY */
    REG32(MACCR) = EN_BACK_PRESSURE | EN_PHY_P4 | EN_PHY_P3 | EN_PHY_P2 | 
                    EN_PHY_P1 | EN_PHY_P0;
    
    /* Enable VLAN ingress filtering */
    REG32(SWTMCR) = EN_VLAN_INGRESS_FILTER;
    
    /* Setup protocol trapping functionality */
    REG32(PTRAPCR) = EN_ARP_TRAP | EN_RARP_TRAP | EN_PPPOE_TRAP | EN_IGMP_TRAP | 
	                    EN_DHCP_TRAP1 | EN_DHCP_TRAP2 | EN_OSPF_TRAP | EN_RIP_TRAP;
    
    /* Enable L2 lookup engine and spanning tree functionality */
    REG32(MSCR) = EN_L2;
    
    /* Initialize MIB counters */
    REG32(MIB_CONTROL) = IN_COUNTER_RESTART | OUT_COUNTER_RESTART;
    
    /* Start normal TX and RX */
    REG32(BISTCR) |= TRXRDY;
    
    /* Setup flow control functionality */
    REG32(FCRTH) = (0x10 << IN_Q_PER_PORT_BUF_FC_THH_OFFSET) | 
                    (0x20 << IN_Q_PER_PORT_BUF_FC_THL_OFFSET) | 
                    (0x10 << OUT_Q_PER_PORT_BUF_FC_THH_OFFSET) | 
                    (0x20 << OUT_Q_PER_PORT_BUF_FC_THL_OFFSET);
    REG32(FCPTR) = (0x006c << IN_Q_PTH_OFFSET) | 
                    (0x006c << OUT_Q_PTH_OFFSET);
    REG32(FCREN) = EN_INQ_FC_CPU | EN_INQ_FC_5 | EN_INQ_FC_4 | EN_INQ_FC_3 | EN_INQ_FC_2 | 
                    EN_INQ_FC_1 | EN_INQ_FC_0 | EN_OUTQ_FC_CPU | EN_OUTQ_FC_5 | EN_OUTQ_FC_4 | 
                    EN_OUTQ_FC_3 | EN_OUTQ_FC_2 | EN_OUTQ_FC_1 | EN_OUTQ_FC_0 | EN_MDC_MDIO_FC | 
                    (0x1f0 << DSC_TH_OFFSET);
    
    /* Initialize PHY */
    REG32(MACCR) |= (EN_PHY_P4 | EN_PHY_P3 | EN_PHY_P2 | EN_PHY_P1 | EN_PHY_P0);
    
    /* Enable interrupt */
    if ( SW_ILEV < getIlev() )
  	    setIlev(SW_ILEV);
    
	/* Init PHY LED style */
#ifdef BICOLOR_LED
	REG32(LEDCR) = 0x01180000; // for bi-color LED
	REG32(TCR0) = 0x000002c2;
	REG32(SWTAA) = PORT5_PHY_CONTROL;
	REG32(SWTACR) = ACTION_START | CMD_FORCE;
	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE ); /* Wait for command done */
#else
	#if defined(BICOLOR_LED_VENDOR_BXXX)
		REG32(LEDCR) |= 0x00080000;

		REG32(PABCNR) &= ~0xc01f0000; /* set port a-7/6 & port b-4/3/2/1/0 to gpio */
		REG32(PABDIR) |=  0x401f0000; /* set port a-6 & port b-4/3/2/1/0 gpio direction-output */
		REG32(PABDIR) &= ~0x80000000; /* set port a-7 gpio direction-input */
	#else /* BICOLOR_LED_VENDOR_BXXX */
		REG32(LEDCR) = 0x00000000;
		REG32(TCR0) = 0x000002c7;
		REG32(SWTAA) = PORT5_PHY_CONTROL;
		REG32(SWTACR) = ACTION_START | CMD_FORCE;
		while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE ); /* Wait for command done */
	#endif /* BICOLOR_LED_VENDOR_BXXX */
#endif

	/* Enable MII */
	REG32(SWTAA) = PORT6_PHY_CONTROL;
	REG32(TCR0) = 0x00000056;
	REG32(SWTACR) = CMD_FORCE | ACTION_START; // force add
	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE ); /* Wait for command done */

	#if defined(CONFIG_RTL865X_DIAG_LED)
	/* diagnosis led (gpio-porta-6) on */
	REG32(PABDAT) |=  0x40000000; /* pull high by set portA-0(bit 30) as gpio-output-1, meaning: diag led OFF */
	#endif /* CONFIG_RTL865X_DIAG_LED */

	
    return 0;
}


#ifdef _L2_MODE_
void DumpAsicRegisters()
{
	uint32 count;
	uint32* adr;
	uint32 len;
	
    /* Display registers */
    
    count = 0;
    adr = (uint32*) 0xbc802000;
    len = 64;
    printf( "\nAddr: %p -----------------------------------", adr );
    while (len--)
    {
        if (count == 0)
            printf("\n0x%p: ", adr);
        if ( ++count == 4 )
            count = 0;
            
        printf("%08x ", *(adr++));
    }
    
    count = 0;
    adr = (uint32*) 0xbc805000;
    len = 65;
    printf( "\nAddr: %p -----------------------------------", adr );
    while (len--)
    {
        if (count == 0)
            printf("\n0x%p: ", adr);
        if ( ++count == 4 )
            count = 0;
            
        printf("%08x ", *(adr++));
    }
    
    count = 0;
    adr = (uint32*) 0xbc803000;
    len = 64;
    printf( "\nAddr: %p -----------------------------------", adr );
    while (len--)
    {
        if (count == 0)
            printf("\n0x%p: ", adr);
        if ( ++count == 4 )
            count = 0;
            
        printf("%08x ", *(adr++));
    }
    
    count = 0;
    adr = (uint32*) 0xbc050000;
    len = 64;
    printf( "\nAddr: %p -----------------------------------", adr );
    while (len--)
    {
        if (count == 0)
            printf("\n0x%p: ", adr);
        if ( ++count == 4 )
            count = 0;
            
        printf("%08x ", *(adr++));
    }
}

/*
 *	Configuration for L2 Switch Mode
 *
 *	From david's letter.
 *
 *  for_RTL8650A = 1 -- 8650A
 *               = 0 -- 8650B
 */
int32 L2_swCore_config( uint8* gmac, uint32 for_RTL8650A )
{
	uint32 sysCLKRate;
	uint32 SCLK;
	uint32 DIV;
	uint32 SCR;
	uint32 initUART1 = 1;    // Is UART1 existed ?

	printf("\n\n");
	if ( for_RTL8650A )
		printf( "For 8650A\n");
	else
		printf( "For 8650B\n");
        
	// Read High-speed Lexra bus
	uint32 scr = (REG32(SCLKCR) & 0xF0000000) >> 28;
	uint32 mcr = (REG32(SCLKCR) & 0x0F000000) >> 24;

	printf( "System Clock Rate: " );
	switch( scr )
	{
		case 0: printf( "200MHz" ); break;
		case 1: printf( "180MHz" ); break;
		case 2: printf( "170MHz" ); break;
		case 3: printf( "190MHz" ); break;
		case 4: printf( "160MHz" ); break;
		case 5: printf( "150MHz" ); break;
		case 6: printf( "140MHz" ); break;
		case 7: printf( "100MHz" ); break;
		default:printf( "unknown" ); break;
	}

	printf( ", Memory Clock Rate: " );
	switch( mcr )
	{
		case 0: printf( " 50MHz" ); break;
		case 1: printf( "100MHz" ); break;
		case 2: printf( "110MHz" ); break;
		case 3: printf( "120MHz" ); break;
		case 4: printf( "130MHz" ); break;
		case 5: printf( "140MHz" ); break;
		case 6: printf( "150MHz" ); break;
		case 7: printf( "160MHz" ); break;
		default:printf( "unknown" ); break;
	}
	printf( " %08x\n",REG32(SCLKCR));

	// dv "\n [Set](3) : Swith Core MAC, PCI 33 Mhz, all others =0 (0xbc803000):\n"
	// dv "##### to Configure PCI clock (33Mhz) clock divide factor \n "
	// ew 0xBC803004 = 0x0     ;//Write 0 before read to update the current clock value of this register. 
	// dw 0xBC803004
	// ew $sysCLKRate = (@0xBC803004 & 0x00070000) >> 0x10 ; //Read system clock rate and set PCI clock setting factor.
	printf( " [Set](3) : Swith Core MAC, PCI 33 Mhz, all others =0 (0xbc803000):\n" );
	printf( "##### to Configure PCI clock (33Mhz) clock divide factor \n " );
    REG32(MACMR) = REG32(MACMR) & ~SYS_CLK_MASK;
	sysCLKRate = ( REG32(MACMR) & 0x00070000 ) >> 0x10;

	switch( sysCLKRate )
	{
		case 0x0: printf( " --> low system clock = 100MHZ \n" ); SCLK= 0x64; break;
		case 0x1: printf( " --> low system clock =  90MHZ \n" ); SCLK= 0x5A; break;
		case 0x2: printf( " --> low system clock =  85MHZ \n" ); SCLK= 0x55; break;
		case 0x3: printf( " --> low system clock =  96MHZ \n" ); SCLK= 0x60; break;
		case 0x4: printf( " --> low system clock =  80MHZ \n" ); SCLK= 0x50; break;
		case 0x5: printf( " --> low system clock =  75MHZ \n" ); SCLK= 0x4B; break;
		case 0x6: printf( " --> low system clock =  70MHZ \n" ); SCLK= 0x46; break;
		case 0x7: printf( " --> low system clock =  50MHZ \n" ); SCLK= 0x32; break;
		default:  printf( " --> low system clock = unknown\n" ); SCLK= 0xFF; break;
	}

	// if ( @$for_RTL8650A == 0x1) {ew $DIV = (@$SCLK+0x10)/0x21 -1 ; dv "RTL8650A mode\n"} {ew $DIV = 2*@$SCLK/0x21 -1 ; dv "RTL8650B mode\n" }
	if ( for_RTL8650A ) 
	{
		DIV = ( SCLK + 0x10 ) / 0x21 - 1;
		printf( "RTL8650A mode\n" );
	} 
	else
	{
		DIV = 2 * SCLK / 0x21 - 1;
		printf( "RTL8650B mode\n" );
	}

	// ew 0xBC803000 = (@0xBC803000 & 0xffff8fff)|(@$DIV << 0xC) ;
	// dv " Now, sysCLK=@$SCLK Mhz ;DIV = %d \n", @$DIV  
	REG32(MACCR) = ( REG32(MACCR) & 0xffff8fff ) | ( DIV << 0xC );
	printf( " Now, sysCLK=%d Mhz ;DIV = %d \n", SCLK, DIV );

	// dv "\n [Set](1) : LED display Mode (0xbc8020a0):\n" (for 15 LED, single color, Link/SPD/DUP)
	// ew 0xbc8020a0 = 0x000002C7 
	printf( " [Set](1) : LED display Mode (0xbc8020a0):\n" );
	REG32(SWTAA) = PORT5_PHY_CONTROL;
	REG32(TCR0) = 0x000002C7;
	REG32(SWTACR) = CMD_FORCE | ACTION_START; // force add
	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE ); /* Wait for command done */

	/* added for david's reguest */
	REG32(SWTAA) = PORT6_PHY_CONTROL;
	REG32(TCR0) = 0x00000056;
	REG32(SWTACR) = CMD_FORCE | ACTION_START; // force add
	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE ); /* Wait for command done */

	// dv "\n [Set](2) : MIB counter (0xbc801070,78,7C):\n"
	// ew 0xbc801070 = 0xff000000 
	// ew 0xbc801078 = 0xff000000 
	// ew 0xbc80107C = 0xff000000 
	// Note : RTL8650A = 0xff000000
	printf( " [Set](2) : MIB counter (0xbc801070,78,7C):\n" );
	REG32(MIB_CONTROL) = 0xff000000;
	if ( for_RTL8650A )
	{
		// SCCR1 and SCCR2 is not available.
	}
	else
	{
		REG32(SCCR1) = 0xff000000;
		REG32(SCCR2) = 0xff000000;
	}

	// dv "\n [Set](4) : init MDC/MDIO (0xbc803018) \n" 
	// ew 0xbc803018 = 0x4ff005f2 
	// RTL8650A = 0x4ff00000 
	printf( " [Set](4) : init MDC/MDIO (0xbc803018) \n" );
	if ( for_RTL8650A )
		REG32(MISCCR) = 0x4ff00000;
	else
		REG32(MISCCR) = 0x4ff005f2;

	// dv "\n [Set](5) : Init SwitchCore MISC, enable L3/L4 Re-calculation, L2 CRC Err allow (0xbc805010)\n"
	// ew 0xbc805010 =  0x00000000 
	// RTL8650A = 0x00000000
	printf( " [Set](5) : Init SwitchCore MISC, enable L3/L4 Re-calculation, L2 CRC Err allow (0xbc805010)\n" );
	if ( for_RTL8650A )
		REG32(CSCR) = 0x00000000;
	else
		REG32(CSCR) = 0x00000000;

	// dv "\n [Set] (6): # Disable flow control, descriptor run out threshold = 496 (0xbc805014)\n"
	// ew 0xbc805014 =  0xfffc05e0 
	// RTL8650A = 0xfffc05e0
	printf( " [Set] (6): # Disable flow control, descriptor run out threshold = 496 (0xbc805014)\n" );
	if ( for_RTL8650A )
		REG32(FCREN) = 0xfffc05e0;
	else
		REG32(FCREN) = 0xfffc05e0;

	// dv "\n [Set] (7): # set flow control per-port reserved threshold FC_On =32, FC_Off =16(0xbc805018)\n"
	// ew 0xbc805018 =  0x20102010 
	printf( " [Set] (7): # set flow control per-port reserved threshold FC_On =32, FC_Off =16(0xbc805018)\n" );
	REG32(FCRTH) = 0x10201020;

	// dv "\n [Set] (8): # set flow control shared threshold = 54 (0xbc805028)\n"
	// ew 0xbc805028 =  0x00360036 
	// RTL8650A = 0x00400040 
	printf( " [Set] (8): # set flow control shared threshold = 54 (0xbc805028)\n" );
	if ( for_RTL8650A )
		REG32(FCPTR) = 0x00400040;
	else
		REG32(FCPTR) = 0x00360036;

	// dv "\n [Set] (9): # enable TTL-1 (0xbc80502c) \n"
	// ew 0xbc80502c =  0x80000000
	printf( " [Set] (9): # enable TTL-1 (0xbc80502c) \n" );
	REG32(TTLCR) = 0x80000000;

	// dv "\n [Set] (10):# enable L2, disable Egress/Ingress ACL ;  disable L3/L4, Spanning tree (0xbc805030) \n"
	// ew 0xbc805030 =  0x00000001
	// RTL8650A =0x0000009F 
	printf( " [Set] (10):# enable L2, disable Egress/Ingress ACL ;  disable L3/L4, Spanning tree (0xbc805030) \n" );
	if ( for_RTL8650A )
		REG32(MSCR) = 0x0000009F;
	else
		REG32(MSCR) = 0x00000001;

	// dv "\n [Set] (10.2):# disable broadcast storm control (0xbc805038) \n"
	// ew 0xbc805038 =  0x00000000
	printf( " [Set] (10.2):# disable broadcast storm control (0xbc805038) \n" );
	REG32(BSCR) = 0x00000000;

	// dv "\n [Set] (11):# enable L2/L4 Aging (0xbc80503c) \n"
	// ew 0xbc80503c =  0xffffffff 
	printf( " [Set] (11):# enable L2/L4 Aging (0xbc80503c) \n" );
	REG32(TEATCR) = 0xfffffffc;
	
	// dv "\n [Set] (12): # enable L4 offset control \n"
	// ew 0xbc805080 =  0x03f00000 
	//printf( "\n [Set] (12): # enable L4 offset control \n" );
	//REG32(OCR) = 0x03f00000;

	// dv "\n [Set] (13): # Init SDRAM timing \n" 
	// ew 0xbd013008 =  0x00000463 
	//printf( " [Set] (13): # Init SDRAM timing \n" );
	//REG32(MTCR1) = 0x00000463;

	// dv "\n [Set] (14): # Init lexra bus timeout \n"
	// ew 0xbd012064 =  0xf0000000 
	printf( " [Set] (14): # Init lexra bus timeout \n" );
	REG32(LTOC) = 0xf0000000;

	// dv "\n [Set] (14): # Init peripheral lexra timing (0xbd012060) \n"
	// ew 0xbd012060 =  0x00000000 
	printf( " [Set] (14): # Init peripheral lexra timing (0xbd012060) \n" );
	REG32(PLTCR) = 0x00000000;

	// dv "\n [Set] (15): # Init TRXRDY \n"
	// ew 0xbc805004 =  0x318f0002 
	printf( " [Set] (15): # Init TRXRDY \n" ); /* BIST ??? */
	REG32(BISTCR) = 0x318f0002;

	// dv "\n [Set] (16):# set Port_based VLAN ID , all PVID=0 (0xbc805078) \n"
	// ew 0xbc805078 =  0x00000000 
	printf( " [Set] (16):# set Port_based VLAN ID , all PVID=0 (0xbc805078) \n" );
	REG32(PVCR) = 0x00000000;

	//dv "\n [set] (17): # Set UART 38400 bps, N,8,1  \n"
	// 1. Cehck system clock value (that is low speed Lexra BUS clock) 
	printf( " [set] (17): # Set UART 38400 bps, N,8,1  \n" );
	switch( ( REG32(MACMR) & 0x00070000 ) >> 0x10 )
	{
		case 0: SCR = 0x5F5E100; printf( " --> system clock = 100MHZ \n" ); break;
		case 1: SCR = 0x55D4A80; printf( " --> system clock =  90MHZ \n" ); break;
		case 2: SCR = 0x510FF40; printf( " --> system clock =  85MHZ \n" ); break;
		case 3: SCR = 0x5B8D800; printf( " --> system clock =  96MHZ \n" ); break;
		case 4: SCR = 0x4C4B400; printf( " --> system clock =  80MHZ \n" ); break;
		case 5: SCR = 0x47868C0; printf( " --> system clock =  75MHZ \n" ); break;
		case 6: SCR = 0x42C1D80; printf( " --> system clock =  70MHZ \n" ); break;
		case 7: SCR = 0x2FAF080; printf( " --> system clock =  50MHZ \n" ); break;
		default:SCR = 0xFFFFFFF; printf( " --> system clock = unknown\n" ); break;
	}
	
	// 2. set Line Control Register : 
	// eb 0xbd01110c = 0x03 ; dv " --> Line Control Parameter = [N,8,1]\n" ; 
	// eb 0xbd01100c = 0x03 ; dv " --> Line Control Parameter = [N,8,1]\n" ; 
	// ew $BR =0x9600  ; // --> Baud Rate = 38400 bps\n  
	// ew $divisor = (@$SCR/(@$BR*0x10))-1 ;// calculate the Divisor Latch value 
	REG8(UART0_BASE+0x0c) = 0x03;
	//printf( " --> Line Control Parameter = [N,8,1]\n" );
	if ( initUART1 )
	{
		REG8(UART1_BASE+0x0c) = 0x03;
		//printf( " --> Line Control Parameter = [N,8,1]\n" );
	}
	uint32 BR = 0x9600; /* 38400 bps */
	uint32 divisor = ( SCR / (BR*0x10) ) - 1;

	// to configure DLL and DLM   
	// ew 0xbd01110c = @0xbd01110c | 0x80000000 ;;  // set DLAB bit =1 
	// ew 0xbd01100c = @0xbd01100c | 0x80000000 ;;  // set DLAB bit =1 
	// eb 0xbd011100 = @$divisor ; 
	// eb 0xbd011000 = @$divisor ; 
	REG32(UART0_BASE+0x0c) |= 0x80000000; // set DLAB bit = 1
	REG8(UART0_BASE+0x00) = divisor;
	if ( initUART1 )
	{
		REG32(UART1_BASE+0x0c) |= 0x80000000; // set DLAB bit = 1
		REG8(UART1_BASE+0x00) = divisor;
	}

	// eb 0xbd011104 = @$divisor >> 8;  
	// eb 0xbd011004 = @$divisor >> 8;  
	// ew 0xbd01110c = @0xbd01110c & 0x7fffffff ;;  // set DLAB =0
	// ew 0xbd01100c = @0xbd01100c & 0x7fffffff ;;  // set DLAB =0
	REG8(UART0_BASE+0x04) = divisor >> 8;
	REG32(UART0_BASE+0x0c) &= 0x7FFFFFFF; // set DLAB bit = 1
	if ( initUART1 )
	{
		REG8(UART1_BASE+0x04) = divisor >> 8;
		REG32(UART1_BASE+0x0c) |= 0x7FFFFFFF; // set DLAB bit = 1
	}

	// ----------------------------------
	// VLAN table Setup :
	// ----------------------------------
	// dv "\n // Write to VLAN table entry 0 : (VLAN 0: without tagging) \n"
	// ew (0xbc800000 + 0x8)   = 0xbc050000 + 0*0x20 ;;   //Note : each entry = 8 Word = 32 byte.
	// ew (0xbc800000 + 0x20)  = 0x11223344 ;; // W0  ,
	// ew (0xbc800000 + 0x24)  = 0x003F0000 ;; // W1  ,MBR= W1:22:17, VID=W1:31:23
	// ew (0xbc800000 + 0x28)  = 0x00000000 ;; // W2
	// ew (0xbc800000 + 0x2C)  = 0xffff3ffd ;; // W3  ,VLANUntag= W3:21:16
	// ew (0xbc800000 + 0x30)  = 0x00000005 ;; // W4
	// ew (0xbc800000 + 0x34)  = 0x00000000 ;; // W5
	// ew (0xbc800000 + 0x38)  = 0x00000000 ;; // W6
	// ew (0xbc800000 + 0x3C)  = 0x00000000 ;; // W7
	// ew (0xbc800000 + 0x0)   = 0x9 ;;    
	printf( " // Write to VLAN table entry 0 : (VLAN 0: without tagging) \n" );
	REG32(SWTAA) = 0xbc050000 + 0*0x20 ;;   //Note : each entry = 8 Word = 32 byte.
	REG32(TCR0)  = 0x11223344 ;; // W0  ,
	REG32(TCR1)  = 0x003F0000 ;; // W1  ,MBR= W1:22:17, VID=W1:31:23
	REG32(TCR2)  = 0x00000000 ;; // W2
	REG32(TCR3)  = 0xffff3ffd ;; // W3  ,VLANUntag= W3:21:16
	REG32(TCR4)  = 0x00000005 ;; // W4
	REG32(TCR5)  = 0x00000000 ;; // W5
	REG32(TCR6)  = 0x00000000 ;; // W6
	REG32(TCR7)  = 0x00000000 ;; // W7
	REG32(SWTACR)= 0x9 ;;        // 
	
	// dv "\n // Write to VLAN table entry 1 : (VLAN 1: with tagging) \n"
	// ew (0xbc800000 + 0x8)   = 0xbc050000 + 1*0x20 ;;   //Note : each entry = 8 Word = 32 byte.
	// ew (0xbc800000 + 0x20)  = 0x11223344 ;; // W0
	// ew (0xbc800000 + 0x24)  = 0x007f0000 ;; // W1  ,MBR= W1:22:17, VID=W1:31:23
	// ew (0xbc800000 + 0x28)  = 0x00000000 ;; // W2
	// ew (0xbc800000 + 0x2C)  = 0xffc03ffd ;; // W3  ,VLANUntag= W3:21:16
	// ew (0xbc800000 + 0x30)  = 0x00000005 ;; // W4
	// ew (0xbc800000 + 0x34)  = 0x00000000 ;; // W5
	// ew (0xbc800000 + 0x38)  = 0x00000000 ;; // W6
	// ew (0xbc800000 + 0x3C)  = 0x00000000 ;; // W7
	// ew (0xbc800000 + 0x0)   = 0x9 ;;        // 
	printf( " // Write to VLAN table entry 1 : (VLAN 1: with tagging) \n" );
	REG32(SWTAA) = 0xbc050000 + 1*0x20 ;;   //Note : each entry = 8 Word = 32 byte.
	REG32(TCR0)  = 0x11223344 ;; // W0  ,
	REG32(TCR1)  = 0x007f0000 ;; // W1  ,MBR= W1:22:17, VID=W1:31:23
	REG32(TCR2)  = 0x00000000 ;; // W2
	REG32(TCR3)  = 0xffc03ffd ;; // W3  ,VLANUntag= W3:21:16
	REG32(TCR4)  = 0x00000005 ;; // W4
	REG32(TCR5)  = 0x00000000 ;; // W5
	REG32(TCR6)  = 0x00000000 ;; // W6
	REG32(TCR7)  = 0x00000000 ;; // W7
	REG32(SWTACR)= 0x9 ;;        // 

	// DumpAsicRegisters();

/*****************************************************************/
#if 0
	uint8 ch;
	uint32 *adr;
	uint32 count, len;
	ether_addr_t mac;
	rtl_vlan_param_t vp;

	/* Simulate RTL8651B default value */
	
    /* Enable VLAN ingress filtering */
    REG32(SWTMCR) = (1<<30); /* EnBrdCst */
    
    /* Setup protocol trapping functionality */
    REG32(PTRAPCR) = 0;
    
    /* Enable L2 lookup engine and spanning tree functionality */
    REG32(MSCR) = EN_L2;
	
	/* Init PHY LED style */
#ifdef BICOLOR_LED
	REG32(LEDCR) = 0x01180000; // for bi-color LED
	REG32(TCR0) = 0x000002c2;
#else
	REG32(LEDCR) = 0x00000000;
	REG32(TCR0) = 0x000002c7;
#endif
	REG32(SWTAA) = PORT5_PHY_CONTROL;
	REG32(SWTACR) = ACTION_START | CMD_FORCE;
	while ( (REG32(SWTACR) & ACTION_MASK) != ACTION_DONE ); /* Wait for command done */

	
	/* Create WAN vlan */
	bzero((void *) &vp, sizeof(rtl_vlan_param_t));
	memcpy(&vp.gMac, gmac, 6);
	vp.egressUntag = 0x1f;
	vp.mtu = 1522;
	vp.memberPort = 0x1f;
	vp.bcastToCPU = 1;
	if ( swCore_vlanCreate(8, &vp) != 0 )
	{
		printf("\nCreating vlan fails!");
		while(1);
	}
	swCore_vlanSetSTPStatusOfAllPorts(8, RTL_STP_FORWARD);

	/* Create LAN vlan */
	bzero((void *) &vp, sizeof(rtl_vlan_param_t));
	memcpy(&vp.gMac, gmac, 6);
	vp.egressUntag = 0x1f;
	vp.mtu = 1522;
	vp.memberPort = 0x1f;
	vp.bcastToCPU = 1;
	if ( swCore_vlanCreate(9, &vp) != 0 )
	{
		printf("\nCreating vlan fails!");
		while(1);
	}
	swCore_vlanSetSTPStatusOfAllPorts(8, RTL_STP_FORWARD);

	/* Init MIB counter to count all ports */

	/* Init SwitchCore MAC, PCI 32 MHz, all others are default all zeros */
	REG32(MACCR) = 0x202623e0;
	
	/* Init MDC/MDIO */
	REG32(MISCCR) = 0x0ff00000;
	
	/* Enable L3/L4 re-calculation, allow L2 CRC error */
	//REG32(CSCR) = EN_ETHER_L3_CHKSUM_REC | EN_ETHER_L4_CHKSUM_REC | ALLOW_L2_CHKSUM_ERR;
	REG32(CSCR) = EN_ETHER_L3_CHKSUM_REC | EN_ETHER_L4_CHKSUM_REC;
	
	/* Enable flow control, descriptor run out threashold = 496 */
	REG32(FCREN) = EN_INQ_FC_CPU | EN_INQ_FC_5 | EN_INQ_FC_4 | EN_INQ_FC_3 | EN_INQ_FC_2 | 
				EN_INQ_FC_1 | EN_INQ_FC_0 | EN_OUTQ_FC_CPU | EN_OUTQ_FC_5 | EN_OUTQ_FC_4 | 
				EN_OUTQ_FC_3 | EN_OUTQ_FC_2 | EN_OUTQ_FC_1 | EN_OUTQ_FC_0 | 0x5e0;
				
	/* Set flow control per-port reserved threshold */
	REG32(FCRTH) = 0x10201020;
	
	/* Set flow control shared threshold */
	//REG32(FCPTR) = 0x00400040;
	REG32(FCPTR) = 0x00360036;
	
	/* Enable TTL-1 */

	/* Enable L2/L3/L4 Egress/Ingress ACL, Spanning tree */
	REG32(MSCR) = EN_L2 | EN_STP;
	
	/* Enable L2 aging */
	REG32(TEATCR) = 0xfffffffe;
	
	/* Enable L4 offset control */
	
	/* Init SDRAM timing */
	REG32(0xbd013008) = 0x00000463;
	
	/* Init Lexra bus timout */
	REG32(0xbd012064) = 0xf0000000;
	
	/* Init TRXDY */
	REG32(BISTCR) |= TRXRDY;
#endif//0

	return 0;
}
#endif //_L2_MODE_


#ifdef _HUB_MODE_

uint32 getLinkStatus( void )
{
	uint32 link = 0;
	uint32 i, dummy;

	for( i = 0; i < 5; i++ )
	{
		dummy = REG32(PHY_BASE+(i<<5) + 0x4); /* To get the correct link status, this register must be read twice. */
		if(REG32(PHY_BASE+(i<<5) + 0x4) & 0x4)
		{
			//link is up
			link |= (1<<i);
		}
	}

	return link;
}

/*
 *  HubMode() --
 *
 *  Ocuppy all L2 table entries to force broadcast.
 *
 */
void HubMode()
{
	int i;
	uint32 entry[8];
	rtl_vlan_param_t vp;
	uint32 oldLinkStatus;
	uint32 newLinkStatus;
	
	printf( "\n\nEnter HubMode():\n" );

	/* Accept broadcast packets, but do not broadcast to CPU. */
	REG32(SWTMCR) |= EN_BROADCAST;
	REG32(SWTMCR) &= ~EN_BROADCAST_TO_CPU;
	
	/* Never Trap ARP/RARP/PPPOR/... to CPU */
	REG32(PTRAPCR) &= ~(EN_ARP_TRAP|EN_RARP_TRAP|EN_PPPOE_TRAP|EN_IGMP_TRAP|
	                    EN_DHCP_TRAP1|EN_DHCP_TRAP2|EN_OSPF_TRAP|EN_RIP_TRAP);
	
	printf( "Config ACL Table ..." );
	// Set ACL to permit
	entry[0] = entry[1] = entry[2] = entry[3] = entry[4] = entry[5] = entry[6] = entry[7] = 0x0;
	for( i = 0; i < NUMBER_OF_ACL_ENTRY; i++ )
	{
		if ( swTable_forceAddEntry( TYPE_ACL_RULE_TABLE, i, entry ) == 0 )
		{
			// OK
		}
		else
		{
			// FAILED
			printf( "swTable_forceAddEntry(i=%d) FAILED. Halted.\n", i );
			goto inloop;
		}
	}
	printf( "OK.\n" );

	printf( "Config L2 Table ..." );
	// Fill L2 table	
	entry[0] = entry[1] = entry[2] = entry[3] = entry[4] = entry[5] = entry[6] = entry[7] = 0x00000000;
	entry[1] |= 0x1f << 8; // MBR
	entry[1] |= 1 << 20; // NHFlag
	//entry[1] &= ~(1 << 15); // Trap to CPU
	//entry[1] |= 1 << 16; // Static

	for( i = 0; i < NUMBER_OF_L2_SWITCH_TABLE_ENTRY; i++ )
	{
		if ( swTable_forceAddEntry( TYPE_L2_SWITCH_TABLE, i, entry ) == 0 )
		{
			// OK
		}
		else
		{
			// FAILED
			printf( "swTable_addEntry(i=%d) FAILED. Halted.\n", i );
			goto inloop;
		}
	}
	printf( "OK.\n" );

recreateVlan:
	printf( "Create VLAN ... " );
	/* Create VLAN */
	bzero((void *) &vp, sizeof(vp));
	memset(&vp.gMac, 0x00, 6);
	vp.egressUntag = 0x1f;
	vp.mtu = 1500;
	vp.memberPort = oldLinkStatus = getLinkStatus();
	printf( "MemberPort=0x%02X ", oldLinkStatus );
	vp.bcastToCPU = 0;
	vp.promiscuous = 0;
	if ( swCore_vlanCreate(8, &vp) != 0 )
	{
		printf("\nCreating vlan fails!");
		while(1);
	}
	swCore_vlanSetSTPStatusOfAllPorts(8, RTL_STP_FORWARD);
	printf( "OK.\n" );


inloop:
	while (1)
	{
	    unsigned char * pPkt;
	    uint32 len;
	    int i;

#if 0
	    if ( swNic_receive(&pPkt, &len) == 0 )
	    {
	        printf("Recv a packet: ethType=0x%04X  len=%d\n", ((unsigned short *)pPkt)[12/2], len );
	        for( i = 0; i < len; i++ )
	        {
	        	if ( ( i & 0xf ) == 0x0 ) printf("%03X ", i );
	        	printf("%02X:", pPkt[i] );
	        	if ( ( i & 0xf ) == 0xf ) printf("\n");
	        }
	        swNic_send(pPkt, len);
	        printf( "\n--------------------------------------------------\n" );

	    }
#endif

	    /* polling link status */
	    newLinkStatus = getLinkStatus();
	    if ( newLinkStatus != oldLinkStatus )
	    {
	    	printf("link changed. re-create VLAN.\n");
	    	swCore_vlanDestroy( 8 );
	    	goto recreateVlan;
	    }
	}
}
#endif //_HUB_MODE_


static void swCore_intHandler(void)
{
    int32  intPending;
    uint32 i;
    
    /* Read interrupt control register */
    intPending = REG32(CPUIISR);
    
    /* Filter masked interrupt */
    intPending &= REG32(CPUIIMR);
    
    /* Check and handle NIC interrupts */
    if (intPending & INTPENDING_NIC_MASK)
        swNic_intHandler(intPending);
        
    /* Check and handle link change interrupt */
    if (intPending & LINK_CHANG_IP){
        /* Handle link change here */        
        REG32(CPUIISR) = LINK_CHANG_IP; /* write to clear pending bit */
		for(i=0; i<5; i++) {  
			//while(!(REG32(PHY_BASE+(i<<5)+0x4) & 0x20));

			uint32 dummy;
			dummy = REG32(PHY_BASE+(i<<5) + 0x4); /* To get the correct link status, this register must be read twice. */
			
			if(REG32((PHY_BASE+(i<<5)+0x4)) & 0x4) { //link is up
				//printf("port %d phyControlRegister 0[%08x] 4[%08x]\n",i,REG32(PHY_BASE+(i<<5)),REG32(PHY_BASE+(i<<5)+4));
				//printf("Port %d Link Change!\n",i);
 				if(REG32((PHY_BASE+(i<<5))) & 0x2000){
					/* link up speed 100Mbps */
					/* set gpio port  with high */
					//printf("port %d speed 100M %08x\n",i,REG32(PHY_BASE+(i<<5)));
					REG32(PABDAT) |= (1<<(16+i));
				}else{
					/* link up speed 10Mbps */
					//printk("linkSpeed10M\n");
					/* set gpio port  with high */
					//printf("port %d speed 10M %08x\n",i,REG32(PHY_BASE+(i<<5)));
					REG32(PABDAT) &= ~(1<<(16+i));
				} /* end if  100M */
			}/* end if  link up */
		}/* end for */        
    }
    
    /* Check and handle software interrupt */
    if (intPending & SW_INT_IP)
        /* Handle software interrupt here */;
}
