/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/inc/rtl8650/asicregs.h,v 1.11 2005/01/28 02:03:48 yjlou Exp $
*
* Abstract: ASIC specific definitions.
*
* $Author: yjlou $
*
* $Log: asicregs.h,v $
* Revision 1.11  2005/01/28 02:03:48  yjlou
* *: loader version migrates to "00.00.19".
* +: support Hub mode
* +: Ping mode support input IP address
* *: clear WDTIND always.
*
* Revision 1.10  2005/01/24 01:34:01  danwu
* + add 8186 support
*
* Revision 1.9  2004/08/03 05:20:56  yjlou
* +: add HAS_PCI_BONDING
*
* Revision 1.8  2004/07/26 08:12:52  yjlou
* +: add code to check BIST Queue Memory and Packet Buffer
*
* Revision 1.7  2004/05/06 11:32:41  yjlou
* +: add PORT6_PHY_CONTROL series register
*
* Revision 1.6  2004/05/06 03:50:00  yjlou
* +: add CRMR definition
*
* Revision 1.5  2004/05/04 03:56:01  yjlou
* *: BACKOFF_EXPONENTIAL_9 should be BACKOFF_EXPONENTIAL_3
* +: PCI_SRST
*
* Revision 1.4  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.3  2004/03/30 11:34:38  yjlou
* *: commit for 80B IC back
*   +: system clock rate definitions have changed.
*   *: fixed the bug of BIST_READY_PATTERN
*   +: clean all ASIC table when init ASIC.
* -: define _L2_MODE_ to support L2 switch mode.
*
* Revision 1.2  2004/03/29 03:42:25  yjlou
* +: Initializing for 8650 backward compatible mode.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.1.1.1  2003/09/25 08:16:56  tony
*  initial loader tree 
*
* Revision 1.2  2003/05/08 03:30:46  danwu
* change table entry length of vlan table to 5
*
* Revision 1.1.1.1  2003/05/07 08:16:07  danwu
* no message
*
* ---------------------------------------------------------------
*/

#ifndef _ASICREGS_H
#define _ASICREGS_H


#include <linux/autoconf.h>


/* Basic features
*/
#define MAX_PORT_NUMBER                 6
#define PORT_NUM_MASK                   7
#define ALL_PORT_MASK                   0x3F



/* Memory mapping of tables 
*/
#define SWTABLE_BASE_OF_ALL_TABLES 0xbc000000
#define TABLE_ENTRY_DISTANCE (8 * sizeof(uint32))
enum {
    TYPE_L2_SWITCH_TABLE = 0,
    TYPE_ARP_TABLE,
    TYPE_L3_ROUTING_TABLE,
    TYPE_MULTICAST_TABLE,
    TYPE_PROTOCOL_TRAP_TABLE,
    TYPE_VLAN_TABLE,
    TYPE_EXT_INT_IP_TABLE,
    TYPE_ALG_TABLE,
    TYPE_SERVER_PORT_TABLE,
    TYPE_L4_TCP_UDP_TABLE,
    TYPE_L4_ICMP_TABLE,
    TYPE_PPPOE_TABLE,
    TYPE_ACL_RULE_TABLE,
    TYPE_NEXT_HOP_TABLE,
    TYPE_RATE_LIMIT_TABLE,
};
#define table_access_addr_base(type) (SWTABLE_BASE_OF_ALL_TABLES + 0x10000 * (type))
#define table_entry_length(type) ("200005"[type] - '0')
#define NUMBER_OF_VLAN_TABLE_ENTRY             8
#define NUMBER_OF_L2_SWITCH_TABLE_ENTRY        1024
#define NUMBER_OF_ACL_ENTRY        128



/* Register access macro
*/
#define REG32(reg) (*(volatile uint32 *)(reg))
#define REG8(reg) (*(volatile uint8 *)(reg))



#define SWCORE_BASE                 0xBC800000
/* Table access and CPU interface control registers
*/
#define TACI_BASE                   (SWCORE_BASE + 0x00000000)
#define SWTACR                      (0x000 + TACI_BASE)     /* Table Access Control */
#define SWTASR                      (0x004 + TACI_BASE)     /* Table Access Status */
#define SWTAA                       (0x008 + TACI_BASE)     /* Table Access Address */
#define TCR0                        (0x020 + TACI_BASE)     /* Table Access Control 0 */
#define TCR1                        (0x024 + TACI_BASE)     /* Table Access Control 1 */
#define TCR2                        (0x028 + TACI_BASE)     /* Table Access Control 2 */
#define TCR3                        (0x02C + TACI_BASE)     /* Table Access Control 3 */
#define TCR4                        (0x030 + TACI_BASE)     /* Table Access Control 4 */
#define TCR5                        (0x034 + TACI_BASE)     /* Table Access Control 5 */
#define TCR6                        (0x038 + TACI_BASE)     /* Table Access Control 6 */
#define TCR7                        (0x03C + TACI_BASE)     /* Table Access Control 7 */
/* Table access control register field definitions
*/
#define ACTION_MASK                 1
#define ACTION_DONE                 0
#define ACTION_START                1
#define CMD_MASK                    (7 << 1)
#define CMD_ADD                     (1 << 1)
#define CMD_MODIFY                  (2 << 1)
#define CMD_FORCE                   (4 << 1)
/* Table access status register field definitions 
*/
#define TABSTS_MASK                 1
#define TABSTS_SUCCESS              0
#define TABSTS_FAIL                 1
/* Vlan table access definitions 
*/
#define STP_DISABLE                 0
#define STP_BLOCK                   1
#define STP_LEARN                   2
#define STP_FORWARD                 3



/* MIB counter registers
*/
#define MIB_COUNTER_BASE                            (SWCORE_BASE + 0x00001000)
#define ETHER_STATS_OCTETS                          (0x000 + MIB_COUNTER_BASE)
#define ETHER_STATS_DROP_EVENTS                     (0x004 + MIB_COUNTER_BASE)
#define ETHER_STATS_CRC_ALIGN_ERRORS                (0x008 + MIB_COUNTER_BASE)
#define ETHER_STATS_FRAGMENTS                       (0x00C + MIB_COUNTER_BASE)
#define ETHER_STATS_JABBERS                         (0x010 + MIB_COUNTER_BASE)
#define IF_IN_UCAST_PKTS                            (0x014 + MIB_COUNTER_BASE)
#define ETHER_STATS_MULTICAST_PKTS                  (0x018 + MIB_COUNTER_BASE)
#define ETHER_STATS_BROADCAST_PKTS                  (0x01C + MIB_COUNTER_BASE)
#define ETHER_STATS_UNDERSIZE_PKTS                  (0x020 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_64_OCTETS                  (0x024 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_65_TO_127_OCTETS           (0x028 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_128_TO_255_OCTETS          (0x02C + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_256_TO_511_OCTETS          (0x030 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_512_TO_1023_OCTETS         (0x034 + MIB_COUNTER_BASE)
#define ETHER_STATS_PKTS_1024_TO_1518_OCTETS        (0x038 + MIB_COUNTER_BASE)
#define ETHER_STATS_OVERSIZE_PKTS                   (0x03C + MIB_COUNTER_BASE)
#define DOT3_CONTROL_IN_UNKNOWN_OPCODES             (0x040 + MIB_COUNTER_BASE)
#define DOT3_IN_PAUSE_FRAMES                        (0x044 + MIB_COUNTER_BASE)
#define IF_OUT_OCTETS                               (0x048 + MIB_COUNTER_BASE)
#define IF_OUT_UCAST_PKTS                           (0x04C + MIB_COUNTER_BASE)
#define IF_OUT_MULTICASTCAST_PKTS                   (0x050 + MIB_COUNTER_BASE)
#define IF_OUT_BROADCASTCAST_PKTS                   (0x054 + MIB_COUNTER_BASE)
#define DOT3_STATS_LATE_COLLISIONS                  (0x05C + MIB_COUNTER_BASE)
#define DOT3_STATS_DEFERRED_TRANSMISSIONS           (0x05C + MIB_COUNTER_BASE)
#define ETHER_STATS_COLLISIONS                      (0x060 + MIB_COUNTER_BASE)
#define DOT3_STATS_SINGLE_COLLISION_FRAMES          (0x064 + MIB_COUNTER_BASE)
#define DOT3_STATS_MULTIPLE_COLLISION_FRAMES        (0x068 + MIB_COUNTER_BASE)
#define DOT3_OUT_PAUSE_FRAMES                       (0x06C + MIB_COUNTER_BASE)
#define MIB_CONTROL                                 (0x070 + MIB_COUNTER_BASE)
#define SCCR1                                       (0x078 + MIB_COUNTER_BASE)
#define SCCR2                                       (0x07C + MIB_COUNTER_BASE)
/* MIB control register field definitions 
*/
#define IN_COUNTER_RESTART                          (1 << 31)
#define OUT_COUNTER_RESTART                         (1 << 30)
#define PORT_FOR_COUNTING_MASK                      0x3F000000
#define PORT_FOR_COUNTING_OFFSET                    24



/* PHY control registers 
*/
#define PHY_BASE                                    (SWCORE_BASE + 0x00002000)
#define PORT0_PHY_CONTROL                           (0x000 + PHY_BASE)
#define PORT0_PHY_STATUS                            (0x004 + PHY_BASE)
#define PORT0_PHY_IDENTIFIER_1                      (0x008 + PHY_BASE)
#define PORT0_PHY_IDENTIFIER_2                      (0x00C + PHY_BASE)
#define PORT0_PHY_AUTONEGO_ADVERTISEMENT            (0x010 + PHY_BASE)
#define PORT0_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x014 + PHY_BASE)
#define PORT1_PHY_CONTROL                           (0x020 + PHY_BASE)
#define PORT1_PHY_STATUS                            (0x024 + PHY_BASE)
#define PORT1_PHY_IDENTIFIER_1                      (0x028 + PHY_BASE)
#define PORT1_PHY_IDENTIFIER_2                      (0x02C + PHY_BASE)
#define PORT1_PHY_AUTONEGO_ADVERTISEMENT            (0x030 + PHY_BASE)
#define PORT1_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x034 + PHY_BASE)
#define PORT2_PHY_CONTROL                           (0x040 + PHY_BASE)
#define PORT2_PHY_STATUS                            (0x044 + PHY_BASE)
#define PORT2_PHY_IDENTIFIER_1                      (0x048 + PHY_BASE)
#define PORT2_PHY_IDENTIFIER_2                      (0x04C + PHY_BASE)
#define PORT2_PHY_AUTONEGO_ADVERTISEMENT            (0x050 + PHY_BASE)
#define PORT2_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x054 + PHY_BASE)
#define PORT3_PHY_CONTROL                           (0x060 + PHY_BASE)
#define PORT3_PHY_STATUS                            (0x064 + PHY_BASE)
#define PORT3_PHY_IDENTIFIER_1                      (0x068 + PHY_BASE)
#define PORT3_PHY_IDENTIFIER_2                      (0x06C + PHY_BASE)
#define PORT3_PHY_AUTONEGO_ADVERTISEMENT            (0x070 + PHY_BASE)
#define PORT3_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x074 + PHY_BASE)
#define PORT4_PHY_CONTROL                           (0x080 + PHY_BASE)
#define PORT4_PHY_STATUS                            (0x084 + PHY_BASE)
#define PORT4_PHY_IDENTIFIER_1                      (0x088 + PHY_BASE)
#define PORT4_PHY_IDENTIFIER_2                      (0x08C + PHY_BASE)
#define PORT4_PHY_AUTONEGO_ADVERTISEMENT            (0x090 + PHY_BASE)
#define PORT4_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x094 + PHY_BASE)
#define PORT5_PHY_CONTROL                           (0x0A0 + PHY_BASE)
#define PORT5_PHY_STATUS                            (0x0A4 + PHY_BASE)
#define PORT5_PHY_IDENTIFIER_1                      (0x0A8 + PHY_BASE)
#define PORT5_PHY_IDENTIFIER_2                      (0x0AC + PHY_BASE)
#define PORT5_PHY_AUTONEGO_ADVERTISEMENT            (0x0B0 + PHY_BASE)
#define PORT5_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x0B4 + PHY_BASE)
#define PORT6_PHY_CONTROL                           (0x0C0 + PHY_BASE)
#define PORT6_PHY_STATUS                            (0x0C4 + PHY_BASE)
#define PORT6_PHY_IDENTIFIER_1                      (0x0C8 + PHY_BASE)
#define PORT6_PHY_IDENTIFIER_2                      (0x0CC + PHY_BASE)
#define PORT6_PHY_AUTONEGO_ADVERTISEMENT            (0x0D0 + PHY_BASE)
#define PORT6_PHY_AUTONEGO_LINK_PARTNER_ABILITY     (0x0D4 + PHY_BASE)
/* PHY control register field definitions 
*/
#define PHY_RESET                                   (1 << 15)
#define ENABLE_LOOPBACK                             (1 << 14)
#define SPEED_SELECT_100M                           (1 << 13)
#define SPEED_SELECT_10M                            0
#define ENABLE_AUTONEGO                             (1 << 12)
#define POWER_DOWN                                  (1 << 11)
#define ISOLATE_PHY                                 (1 << 10)
#define RESTART_AUTONEGO                            (1 << 9)
#define SELECT_FULL_DUPLEX                          (1 << 8)
#define SELECT_HALF_DUPLEX                          0
/* PHY status register field definitions 
*/
#define STS_CAPABLE_100BASE_T4                      (1 << 15)
#define STS_CAPABLE_100BASE_TX_FD                   (1 << 14)
#define STS_CAPABLE_100BASE_TX_HD                   (1 << 13)
#define STS_CAPABLE_10BASE_TX_FD                    (1 << 12)
#define STS_CAPABLE_10BASE_TX_HD                    (1 << 11)
#define STS_MF_PREAMBLE_SUPPRESSION                 (1 << 6)
#define STS_AUTONEGO_COMPLETE                       (1 << 5)
#define STS_REMOTE_FAULT                            (1 << 4)
#define STS_CAPABLE_NWAY_AUTONEGO                   (1 << 3)
#define STS_LINK_ESTABLISHED                        (1 << 2)
#define STS_JABBER_DETECTED                         (1 << 1)
#define STS_CAPABLE_EXTENDED                        (1 << 0)
/* PHY identifier 1 
*/
#define OUT_3_18_MASK                               (0xFFFF << 16)
#define OUT_3_18_OFFSET                             16
#define OUT_19_24_MASK                              (0x3F << 10)
#define OUT_19_24_OFFSET                            10
#define MODEL_NUMBER_MASK                           (0x3F << 4)
#define MODEL_NUMBER_OFFSET                         4
#define REVISION_NUMBER_MASK                        0x0F
#define REVISION_NUMBER_OFFSET                      0
/* PHY auto-negotiation advertisement and 
link partner ability registers field definitions
*/
#define NEXT_PAGE_ENABLED                           (1 << 15)
#define ACKNOWLEDGE                                 (1 << 14)
#define REMOTE_FAULT                                (1 << 13)
#define CAPABLE_PAUSE                               (1 << 10)
#define CAPABLE_100BASE_T4                          (1 << 9)
#define CAPABLE_100BASE_TX_FD                       (1 << 8)
#define CAPABLE_100BASE_TX_HD                       (1 << 7)
#define CAPABLE_10BASE_TX_FD                        (1 << 6)
#define CAPABLE_10BASE_TX_HD                        (1 << 5)
#define SELECTOR_MASK                               0x1F
#define SELECTOR_OFFSET                             0



/* CPU interface Tx/Rx packet registers 
*/
#define CPU_IFACE_BASE                      (SWCORE_BASE + 0x00004000)
#define CPUICR                              (0x000 + CPU_IFACE_BASE)    /* Interface control */
#define CPURPDCR                            (0x004 + CPU_IFACE_BASE)    /* Rx pkthdr descriptor control */
#define CPURMDCR                            (0x008 + CPU_IFACE_BASE)    /* Rx mbuf descriptor control */
#define CPUTPDCR                            (0x00C + CPU_IFACE_BASE)    /* Tx pkthdr descriptor control */
#define CPUIIMR                             (0x010 + CPU_IFACE_BASE)    /* Interrupt mask control */
#define CPUIISR                             (0x014 + CPU_IFACE_BASE)    /* Interrupt status control */
/* CPU interface control register field definitions 
*/
#define TXCMD                               (1 << 31)       /* Enable Tx */
#define RXCMD                               (1 << 30)       /* Enable Rx */
#define BUSBURST_32WORDS                    0
#define BUSBURST_64WORDS                    (1 << 28)
#define BUSBURST_128WORDS                   (2 << 28)
#define BUSBURST_256WORDS                   (3 << 28)
#define MBUF_128BYTES                       0
#define MBUF_256BYTES                       (1 << 24)
#define MBUF_512BYTES                       (2 << 24)
#define MBUF_1024BYTES                      (3 << 24)
#define MBUF_2048BYTES                      (4 << 24)
#define TXFD                                (1 << 23)       /* Notify Tx descriptor fetch */
#define SOFTRST                             (1 << 22)       /* Re-initialize all descriptors */
#define STOPTX                              (1 << 21)       /* Stop Tx */
#define SWINTSET                            (1 << 20)       /* Set software interrupt */
#define LBMODE                              (1 << 19)       /* Loopback mode */
#define LB10MHZ                             (1 << 18)       /* LB 10MHz */
#define LB100MHZ                            (1 << 18)       /* LB 100MHz */
/* CPU interface descriptor field defintions 
*/
#define DESC_OWNED_BIT                      1
#define DESC_RISC_OWNED                     0
#define DESC_SWCORE_OWNED                   1
#define DESC_WRAP                           (1 << 1)
/* CPU interface interrupt mask register field definitions 
*/
#define LINK_CHANG_IE                       (1 << 31)    /* Link change interrupt enable */
#define RX_ERR_IE                           (1 << 30)    /* Rx error interrupt enable */
#define TX_ERR_IE                           (1 << 29)    /* Tx error interrupt enable */
#define SW_INT_IE                           (1 << 28)    /* Software interrupt enable */
#define PKTHDR_DESC_RUNOUT_IE               (1 << 23)    /* Run out pkthdr descriptor interrupt enable */
#define MBUF_DESC_RUNOUT_IE                 (1 << 22)    /* Run out mbuf descriptor interrupt enable */
#define TX_DONE_IE                          (1 << 21)    /* Tx one packet done interrupt enable */
#define RX_DONE_IE                          (1 << 20)    /* Rx one packet done interrupt enable */
#define TX_ALL_DONE_IE                      (1 << 19)    /* Tx all packets done interrupt enable */
/* CPU interface interrupt status register field definitions 
*/
#define LINK_CHANG_IP                       (1 << 31)   /* Link change interrupt pending */
#define RX_ERR_IP                           (1 << 30)   /* Rx error interrupt pending */
#define TX_ERR_IP                           (1 << 29)   /* Tx error interrupt pending */
#define SW_INT_IP                           (1 << 28)   /* Software interrupt pending */
#define L4_COL_REMOVAL_IP                   (1 << 27)   /* L4 collision removal interrupt pending */
#define PKTHDR_DESC_RUNOUT_IP               (1 << 23)   /* Run out pkthdr descriptor interrupt pending */
#define MBUF_DESC_RUNOUT_IP                 (1 << 22)   /* Run out mbuf descriptor interrupt pending */
#define TX_DONE_IP                          (1 << 21)   /* Tx one packet done interrupt pending */
#define RX_DONE_IP                          (1 << 20)   /* Rx one packet done interrupt pending */
#define TX_ALL_DONE_IP                      (1 << 19)   /* Tx all packets done interrupt pending */
#define INTPENDING_NIC_MASK     (RX_ERR_IP | TX_ERR_IP | PKTHDR_DESC_RUNOUT_IP | \
                                    MBUF_DESC_RUNOUT_IP | TX_ALL_DONE_IP | RX_DONE_IP)



/* System control registers 
*/
#define SYSTEM_BASE                         (SWCORE_BASE + 0x00003000)
#define MACCR                               (0x000 + SYSTEM_BASE)   /* MAC control */
#define MACMR                               (0x004 + SYSTEM_BASE)   /* MAC monitor */
#define VLANTCR                             (0x008 + SYSTEM_BASE)   /* Vlan tag control */
#define DSCR0                               (0x00C + SYSTEM_BASE)   /* Qos by DS control */
#define DSCR1                               (0x010 + SYSTEM_BASE)   /* Qos by DS control */
#define QOSCR                               (0x014 + SYSTEM_BASE)   /* Qos control */
#define MISCCR                              (0x018 + SYSTEM_BASE)   /* Switch core misc control */
#define SWTMCR                              (0x01C + SYSTEM_BASE)   /* Switch table misc control */
#define TMRMR                               (0x020 + SYSTEM_BASE)   /* Test mode Rx mii-like */
#define TMTMR                               (0x024 + SYSTEM_BASE)   /* Test mode Tx mii-like */
#define TMCR                                (0x028 + SYSTEM_BASE)   /* Test mode control */
#define MDCIOCR                             (0x02C + SYSTEM_BASE)   /* MDC/MDIO Command */
#define MDCIOSR                             (0x030 + SYSTEM_BASE)   /* MDC/MDIO Status */
/* MAC control register field definitions 
*/
#define DIS_IPG                             (1 << 31)   /* Set IFG */
#define EN_INT_CAM                          (1 << 30)   /* Enable internal CAM */
#define NORMAL_BACKOFF                      (1 << 29)   /* Normal back off slot timer */
#define BACKOFF_EXPONENTIAL_3	            (1 << 28)   /* Set back off exponential parameter 3 */
#define DIS_BACKOFF_BIST                    (1 << 27)   /* Disable back off BIST */
#define ACPT_MAXLEN_1552                    (1 << 26)   /* Set acceptable max length of packet 1552 */
#define FULL_RST                            (1 << 25)   /* Reset all tables & queues */
#define SEMI_RST                            (1 << 24)   /* Reset queues */
#define LONG_TXE                            (1 << 23)   /* Back pressure, carrier based */
#define BYPASS_TCRC                         (1 << 22)   /* Not recalculate CRC error */
#define INFINITE_PAUSE_FRAMES               (1 << 21)   /* Infinite pause frames */
#define IPG_SEL                             (1 << 20)   /* Fixed IPG */
#define EN_48_DROP                          (1 << 19)
#define DIS_MASK_CGST                       (1 << 18)
#define EN_BACK_PRESSURE                    (1 << 17)   /* Enable back pressure */
#define PCI_SRST                            (1 << 15)   /* PCI software reset */
#define EN_PHY_P4                           (1 << 9)
#define EN_PHY_P3                           (1 << 8)
#define EN_PHY_P2                           (1 << 7)
#define EN_PHY_P1                           (1 << 6)
#define EN_PHY_P0                           (1 << 5)
#define EN_FX_P4                            (1 << 4)
#define EN_FX_P3                            (1 << 3)
#define EN_FX_P2                            (1 << 2)
#define EN_FX_P1                            (1 << 1)
#define EN_FX_P0                            (1 << 0)
/* MAC monitor register field definitions 
*/
#define SYS_CLK_MASK                        (0x7 << 16)
#define SYS_CLK_100M                        (0 << 16)
#define SYS_CLK_90M                         (1 << 16)
#define SYS_CLK_85M                         (2 << 16)
#define SYS_CLK_96M                         (3 << 16)
#define SYS_CLK_80M                         (4 << 16)
#define SYS_CLK_75M                         (5 << 16)
#define SYS_CLK_70M                         (6 << 16)
#define SYS_CLK_50M                         (7 << 16)
/* VLAN tag control register field definitions 
*/
#define VLAN_TAG_ONLY                       (1 << 19)   /* Only accept tagged packets */
/* Qos by DS control register 
*/
/* Qos control register 
*/
#define QWEIGHT_MASK                        (3 << 30)
#define QWEIGHT_ALWAYS_H                    (3 << 30)   /* Weighted round robin of priority always high first */
#define QWEIGHT_16TO1                       (2 << 30)   /* Weighted round robin of priority queue 16:1 */
#define QWEIGHT_8O1                         (1 << 30)   /* Weighted round robin of priority queue 8:1 */
#define QWEIGHT_4TO1                        0           /* Weighted round robin of priority queue 4:1 */
#define EN_FCA_AUTOOFF                      (1 << 29)   /* Enable flow control auto off */
#define DIS_DS_PRI                          (1 << 28)   /* Disable DS priority */
#define DIS_VLAN_PRI                        (1 << 27)   /* Disable 802.1p priority */
#define PORT5_H_PRI                         (1 << 26)   /* Port 5 high priority */
#define PORT4_H_PRI                         (1 << 25)   /* Port 4 high priority */
#define PORT3_H_PRI                         (1 << 24)   /* Port 3 high priority */
#define PORT2_H_PRI                         (1 << 23)   /* Port 2 high priority */
#define PORT1_H_PRI                         (1 << 22)   /* Port 1 high priority */
#define PORT0_H_PRI                         (1 << 21)   /* Port 0 high priority */
#define EN_QOS                              (1 << 20)   /* Enable QoS */
/* Switch core misc control register field definitions 
*/
#define DIS_P5_LOOPBACK                     (1 << 30)   /* Disable port 5 loopback */
/* Switch table misc control register field definitions 
*/
#define EN_VLAN_INGRESS_FILTER              (1 << 3)    /* Enable Vlan ingress filtering */
#define EN_BROADCAST_TO_CPU       	        (1 << 28)   /* Broadcast to CPU */
#define EN_BROADCAST              			(1 << 30)   /* Enable Broadcast */
/* Test mode Rx MII-like register field definitions 
*/
/* Test mode Tx MII-like register field definitions 
*/
/* Test mode enable register 
*/
#define TX_TEST_PORT_OFFSET                 26          /* Tx test mode enable port offset */
#define RX_TEST_PORT_OFFSET                 18          /* Rx test mode enable port offset */



/* Miscellaneous control registers 
*/
#define MISC_BASE                           (SWCORE_BASE + 0x00005000)
#define LEDCR                               (0x000 + MISC_BASE)     /* LED control */
#define BISTCR                              (0x004 + MISC_BASE)     /* BIST control */
#define BCR0                                (0x008 + MISC_BASE)     /* Input bandwidth control */
#define BCR1                                (0x00C + MISC_BASE)     /* Ouput bandwidth control */
#define CSCR                                (0x010 + MISC_BASE)     /* Checksum control */
#define FCREN                               (0x014 + MISC_BASE)     /* Flow control enable control */
#define FCRTH                               (0x018 + MISC_BASE)     /* Flow control threshold */
#define FCPTR                               (0x028 + MISC_BASE)     /* Flow control prime threshold register */
#define PTCR                                (0x01C + MISC_BASE)     /* Port trunk control */
#define SWTECR                              (0x020 + MISC_BASE)     /* Switch Table Extended Control Register */
#define PTRAPCR                             (0x024 + MISC_BASE)     /* Protocol trapping control */
#define TTLCR                               (0x02C + MISC_BASE)     /* TTL control register */
#define MSCR                                (0x030 + MISC_BASE)     /* Module switch control */
#define BSCR                                (0x038 + MISC_BASE)     /* Broadcast storm control */
#define TEATCR                              (0x03C + MISC_BASE)     /* Table entry aging time control */
#define PMCR                                (0x040 + MISC_BASE)     /* Port mirror control */
#define PPMAR                               (0x044 + MISC_BASE)     /* Per port matching action */
#define PATP0                               (0x048 + MISC_BASE)     /* Pattern for port 0 */
#define PATP1                               (0x04C + MISC_BASE)     /* Pattern for port 1 */
#define PATP2                               (0x050 + MISC_BASE)     /* Pattern for port 2 */
#define PATP3                               (0x054 + MISC_BASE)     /* Pattern for port 3 */
#define PATP4                               (0x058 + MISC_BASE)     /* Pattern for port 4 */
#define PATP5                               (0x05C + MISC_BASE)     /* Pattern for port 5 */
#define MASKP0                              (0x060 + MISC_BASE)     /* Mask for port 0 */
#define MASKP1                              (0x064 + MISC_BASE)     /* Mask for port 1 */
#define MASKP2                              (0x068 + MISC_BASE)     /* Mask for port 2 */
#define MASKP3                              (0x06C + MISC_BASE)     /* Mask for port 3 */
#define MASKP4                              (0x070 + MISC_BASE)     /* Mask for port 4 */
#define MASKP5                              (0x074 + MISC_BASE)     /* Mask for port 5 */
#define PVCR                                (0x078 + MISC_BASE)     /* Port based vlan config */
#define OCR                                 (0x080 + MISC_BASE)     /* Offset Control Register */
#define CVIDR                               (0x100 + MISC_BASE)     /* Chip version ID */
#define CRMR                                (0x104 + MISC_BASE)     /* Chip Revision Management Register */
/* LED control register field definitions 
*/
#define LED_P0_SPEED                        (1 << 0)    /* LED port 0 collision */
#define LED_P0_ACT                          (1 << 1)    /* LED port 0 active */
#define LED_P0_COL                          (1 << 2)    /* LED port 0 speed 100M */
#define LED_P1_SPEED                        (1 << 3)    /* LED port 1 collision */
#define LED_P1_ACT                          (1 << 4)    /* LED port 1 active */
#define LED_P1_COL                          (1 << 5)    /* LED port 1 speed 100M */
#define LED_P2_SPEED                        (1 << 6)    /* LED port 2 collision */
#define LED_P2_ACT                          (1 << 7)    /* LED port 2 active */
#define LED_P2_COL                          (1 << 8)    /* LED port 2 speed 100M */
#define LED_P3_SPEED                        (1 << 9)    /* LED port 3 collision */
#define LED_P3_ACT                          (1 << 10)   /* LED port 3 active */
#define LED_P3_COL                          (1 << 11)   /* LED port 3 speed 100M */
#define LED_P4_SPEED                        (1 << 12)   /* LED port 4 collision */
#define LED_P4_ACT                          (1 << 13)   /* LED port 4 active */
#define LED_P4_COL                          (1 << 14)   /* LED port 4 speed 100M */
#define LED_P5_SPEED                        (1 << 15)   /* LED port 5 collision */
#define LED_P5_ACT                          (1 << 16)   /* LED port 5 active */
#define LED_P5_COL                          (1 << 17)   /* LED port 5 speed 100M */
#define EN_LED_CPU_CTRL                     (1 << 18)   /* Enable CPU control LED */
/* BIST control register field definitions 
*/
#define BIST_READY_PATTERN                  0x018F0000
#define BIST_QUEUE_MEMORY_FAIL_PATTERN      0x00700000
#define BIST_PACKET_BUFFER_FAIL_PATTERN     0x0E000000
#define TRXRDY                              (1 << 1)    /* Start normal TX and RX */
/* Bandwidth control register field definitions 
*/
#define OUT_BC_P0_OFFSET                    0           /* Output bandwidth control port 0 offset */
#define IN_BC_P0_OFFSET                     4           /* Input bandwidth control port 0 offset */
#define OUT_BC_P1_OFFSET                    8           /* Output bandwidth control port 1 offset */
#define IN_BC_P1_OFFSET                     12          /* Input bandwidth control port 1 offset */
#define OUT_BC_P2_OFFSET                    16          /* Output bandwidth control port 2 offset */
#define IN_BC_P2_OFFSET                     20          /* Input bandwidth control port 2 offset */
#define OUT_BC_P3_OFFSET                    24          /* Output bandwidth control port 3 offset */
#define IN_BC_P3_OFFSET                     48          /* Input bandwidth control port 3 offset */
#define OUT_BC_P4_OFFSET                    0           /* Output bandwidth control port 4 offset */
#define IN_BC_P4_OFFSET                     4           /* Input bandwidth control port 4 offset */
#define OUT_BC_P5_OFFSET                    8           /* Output bandwidth control port 5 offset */
#define IN_BC_P5_OFFSET                     12          /* Input bandwidth control port 5 offset */
#define BW_FULL_RATE                        0
#define BW_128K                             1
#define BW_256K                             2
#define BW_512K                             3
#define BW_1M                               4
#define BW_2M                               5
#define BW_4M                               6
#define BW_8M                               7
/* Checksum control register field definitions 
*/
#define ALLOW_L2_CHKSUM_ERR                 (1 << 0)    /* Allow L2 checksum error */
#define ALLOW_L3_CHKSUM_ERR                 (1 << 1)    /* Allow L3 checksum error */
#define ALLOW_L4_CHKSUM_ERR                 (1 << 2)    /* Allow L4 checksum error */
#define EN_ETHER_L3_CHKSUM_REC              (1 << 3)    /* Enable L3 checksum recalculation*/
#define EN_ETHER_L4_CHKSUM_REC              (1 << 4)    /* Enable L4 checksum recalculation*/
/* Flow control enable register field defintions 
*/
#define EN_INQ_FC_CPU                       (1 << 31)   /* Enable Flow Control on CPU Port */
#define EN_INQ_FC_5                         (1 << 30)   /* Enable Flow Control on Port 5 */
#define EN_INQ_FC_4                         (1 << 29)   /* Enable Flow Control on Port 4 */
#define EN_INQ_FC_3                         (1 << 28)   /* Enable Flow Control on Port 3 */
#define EN_INQ_FC_2                         (1 << 27)   /* Enable Flow Control on Port 2 */
#define EN_INQ_FC_1                         (1 << 26)   /* Enable Flow Control on Port 1 */
#define EN_INQ_FC_0                         (1 << 25)   /* Enable Flow Control on Port 0 */
#define EN_OUTQ_FC_CPU                      (1 << 24)   /* Enable Flow Control on CPU Port */
#define EN_OUTQ_FC_5                        (1 << 23)   /* Enable Flow Control on Port 5 */
#define EN_OUTQ_FC_4                        (1 << 22)   /* Enable Flow Control on Port 4 */
#define EN_OUTQ_FC_3                        (1 << 21)   /* Enable Flow Control on Port 3 */
#define EN_OUTQ_FC_2                        (1 << 20)   /* Enable Flow Control on Port 2 */
#define EN_OUTQ_FC_1                        (1 << 19)   /* Enable Flow Control on Port 1 */
#define EN_OUTQ_FC_0                        (1 << 18)   /* Enable Flow Control on Port 0 */
#define CPU_LAUNCH_FC_P5                    (1 << 17)   /* CPU launch flow control of Port 5 */
#define CPU_LAUNCH_FC_P4                    (1 << 16)   /* CPU launch flow control of Port 4 */
#define CPU_LAUNCH_FC_P3                    (1 << 15)   /* CPU launch flow control of Port 3 */
#define CPU_LAUNCH_FC_P2                    (1 << 14)   /* CPU launch flow control of Port 2 */
#define CPU_LAUNCH_FC_P1                    (1 << 13)   /* CPU launch flow control of Port 1 */
#define CPU_LAUNCH_FC_P0                    (1 << 12)   /* CPU launch flow control of Port 0 */
#define EN_MDC_MDIO_FC                      (1 << 10)   /* Enable MDC/MDIO Flow Control */
#define DSC_TH_OFFSET                       0           /* Descriptor Initial threshold */
/* Flow control threshold register field defintions 
*/
#define IN_Q_PER_PORT_BUF_FC_THH_OFFSET     24          /* InQ per port buffer page flow control high threshold offset */
#define IN_Q_PER_PORT_BUF_FC_THL_OFFSET     16          /* InQ per port buffer page flow control low threshold offset */
#define OUT_Q_PER_PORT_BUF_FC_THH_OFFSET    8           /* OutQ per port buffer page flow control high threshold offset */
#define OUT_Q_PER_PORT_BUF_FC_THL_OFFSET    0           /* OutQ per port buffer page flow control low threshold offset */
/* Flow control prime threshold register field defintions 
*/
#define IN_Q_PTH_OFFSET                     16          /* InQ Prime flow control threshold */
#define OUT_Q_PTH_OFFSET                    0           /* OutQ Prime flow control threshold */
/* Port trunking control register field definitions 
*/
#define LMPR7_OFFSET                        27          /* Physical port index for logical port 7 */
#define LMPR6_OFFSET                        24          /* Physical port index for logical port 6 */
#define LMPR5_OFFSET                        21          /* Physical port index for logical port 5 */
#define LMPR4_OFFSET                        18          /* Physical port index for logical port 4 */
#define LMPR3_OFFSET                        15          /* Physical port index for logical port 3 */
#define LMPR2_OFFSET                        12          /* Physical port index for logical port 2 */
#define LMPR1_OFFSET                        9           /* Physical port index for logical port 1 */
#define LMPR0_OFFSET                        6           /* Physical port index for logical port 0 */
#define TRUNK1_PORT_MASK_OFFSET             0           /* Physical port mask of trunk 1 */
/* Protocol trapping control register field definitions 
*/
#define EN_ARP_TRAP                         (1 << 24)   /* Enable trapping ARP packets */
#define EN_RARP_TRAP                        (1 << 25)   /* Enable trapping RARP packets */
#define EN_PPPOE_TRAP                       (1 << 26)   /* Enable trapping PPPoE packets */
#define EN_IGMP_TRAP                        (1 << 27)   /* Enable trapping IGMP packets */
#define EN_DHCP_TRAP1                       (1 << 28)   /* Enable trapping DHCP 67 packets */
#define EN_DHCP_TRAP2                       (1 << 29)   /* Enable trapping DHCP 68 packets */
#define EN_OSPF_TRAP                        (1 << 30)   /* Enable trapping OSPF packets */
#define EN_RIP_TRAP                         (1 << 31)   /* Enable trapping RIP packets */
/* Spanning tree control register field definitions 
*/
#define EN_ESTP_S_DROP                      (1 << 31)   /* Enable egress spanning tree forward S_Drop */
/* Module switch control register field definitions 
*/
#define EN_L2                               (1 << 0)    /* Enable L2 module */
#define EN_STP                              (1 << 7)    /* Enable spanning tree */
#define HAS_PCI_BONDING						(1 << 31)	/* ASIC has PCI bonding */
/* Broadcast storm control register field definitions 
*/
#define EN_BCAST_STORM                      (1 << 0)    /* Enable broadcast storm control */
/* Table entry aging time control register field definitions 
*/
#define EN_L2_AGING                         (1 << 0)    /* Enable L2 aging */
/* Port mirror control register field definitions 
*/
#define MIRROR_TO_PORT_OFFSET               26          /* Port receiving the mirrored traffic offset */
#define MIRROR_FROM_PORT_RX_OFFSET          20          /* Rx port to be mirrored offset */
#define MIRROR_FROM_PORT_TX_OFFSET          14          /* Tx port to be mirrored offset */
/* Per port matching action register field definitions 
*/
#define EN_PMATCH_PORT_LIST_OFFSET          26          /* Enable pattern match port list offset */
#define MATCH_OP_P5_OFFSET                  24          /* Offset of operation if matched on port 5 */
#define MATCH_OP_P4_OFFSET                  22          /* Offset of operation if matched on port 4 */
#define MATCH_OP_P3_OFFSET                  20          /* Offset of operation if matched on port 3 */
#define MATCH_OP_P2_OFFSET                  18          /* Offset of operation if matched on port 2 */
#define MATCH_OP_P1_OFFSET                  16          /* Offset of operation if matched on port 1 */
#define MATCH_OP_P0_OFFSET                  14          /* Offset of operation if matched on port 0 */
#define MATCH_DROP                          0           /* Drop if matched */
#define MATCH_MIRROR_TO_CPU                 1           /* Mirror to CPU if matched */
#define MATCH_FORWARD_TO_CPU                2           /* Forward to CPU if matched */
#define MATCH_TO_MIRROR_PORT                3           /* To mirror port if matched */
/* Port based vlan config register field definitions 
*/
#define PVID_MASK                           7           /* MASK for PVID */
#define VIDP0_OFFSET                        0           /* Vlan table index for port 0 */
#define VIDP1_OFFSET                        3           /* Vlan table index for port 1 */
#define VIDP2_OFFSET                        6           /* Vlan table index for port 2 */
#define VIDP3_OFFSET                        9           /* Vlan table index for port 3 */
#define VIDP4_OFFSET                        12          /* Vlan table index for port 4 */
#define VIDP5_OFFSET                        15          /* Vlan table index for port 5 */
/* Chip version ID register field definitions 
*/
#define RTL8650_CVID                        0x86500000



/* UART registers 
*/
#define UART0_BASE                          0xBD011100
#define UART1_BASE                          0xBD011000



/* Global interrupt control registers 
*/
#define GICR_BASE                           0xBD012000
#define GIMR                                (0x000 + GICR_BASE)       /* Global interrupt mask */
#define GISR                                (0x004 + GICR_BASE)       /* Global interrupt status */
#define IRR                                 (0x008 + GICR_BASE)       /* Interrupt routing */
/* Global interrupt mask register field definitions 
*/
#define TCIE                                (1 << 31)       /* Timer/Counter interrupt enable */
#define PCMCIAIE                            (1 << 29)       /* PCMCIA interrupt enable */
#define UART1IE                             (1 << 28)       /* UART 1 interrupt enable */
#define UART0IE                             (1 << 27)       /* UART 0 interrupt enable */
#define SWIE                                (1 << 25)       /* Switch core interrupt enable */
#define PABCIE                              (1 << 24)       /* GPIO port ABC interrupt enable */
#define IREQ0IE                             (1 << 23)       /* External interrupt 0 enable */
#define LBCTMOIE                            (1 << 21)       /* LBC time-out interrupt enable */
/* Global interrupt status register field definitions 
*/
#define TCIP                                (1 << 31)       /* Timer/Counter interrupt pending */
#define PCMCIAIP                            (1 << 29)       /* PCMCIA interrupt pending */
#define UART1IP                             (1 << 28)       /* UART 1 interrupt pending */
#define UART0IP                             (1 << 27)       /* UART 0 interrupt pending */
#define SWIP                                (1 << 25)       /* Switch core interrupt pending */
#define PABCIP                              (1 << 24)       /* GPIO port ABC interrupt pending */
#define IREQ0IP                             (1 << 23)       /* External interrupt 0 pending */
#define LBCTMOIP                            (1 << 21)       /* LBC time-out interrupt pending */
/* Interrupt routing register field definitions 
*/
#define TCIRS_OFFSET                        30              /* Timer/Counter interrupt routing select offset */
#define PCMCIAIRS_OFFSET                    26              /* PCMCIA interrupt routing select offset */
#define UART1IRS_OFFSET                     24              /* UART 1 interrupt routing select offset */
#define UART0IRS_OFFSET                     22              /* UART 0 interrupt routing select offset */
#define SWIRS_OFFSET                        18              /* Switch core interrupt routing select offset */
#define PABCIRS_OFFSET                      16              /* GPIO port B interrupt routing select offset */
#define IREQ0RS_OFFSET                      14              /* External interrupt 0 routing select offset */
#define LBCTMOIRS_OFFSET                    10              /* LBC time-out interrupt routing select offset */



/* Timer control registers 
*/
#define TC0DATA                             (0x020 + GICR_BASE)       /* Timer/Counter 0 data */
#define TC1DATA                             (0x024 + GICR_BASE)       /* Timer/Counter 1 data */
#define TC0CNT                              (0x028 + GICR_BASE)       /* Timer/Counter 0 count */
#define TC1CNT                              (0x02C + GICR_BASE)       /* Timer/Counter 1 count */
#define TCCNR                               (0x030 + GICR_BASE)       /* Timer/Counter control */
#define TCIR                                (0x034 + GICR_BASE)       /* Timer/Counter intertupt */
#define CDBR                                (0x038 + GICR_BASE)       /* Clock division base */
#define WDTCNR                              (0x03C + GICR_BASE)       /* Watchdog timer control */
/* Timer/Counter data register field definitions 
*/
#define TCD_OFFSET                          8
/* Timer/Counter control register field defintions 
*/
#define TC0EN                               (1 << 31)       /* Timer/Counter 0 enable */
#define TC0MODE_COUNTER                     0               /* Timer/Counter 0 counter mode */
#define TC0MODE_TIMER                       (1 << 30)       /* Timer/Counter 0 timer mode */
#define TC1EN                               (1 << 29)       /* Timer/Counter 1 enable */
#define TC1MODE_COUNTER                     0               /* Timer/Counter 1 counter mode */
#define TC1MODE_TIMER                       (1 << 28)       /* Timer/Counter 1 timer mode */
/* Timer/Counter interrupt register field definitions 
*/
#define TC0IE                               (1 << 31)       /* Timer/Counter 0 interrupt enable */
#define TC1IE                               (1 << 30)       /* Timer/Counter 1 interrupt enable */
#define TC0IP                               (1 << 29)       /* Timer/Counter 0 interrupt pending */
#define TC1IP                               (1 << 28)       /* Timer/Counter 1 interrupt pending */
/* Clock division base register field definitions 
*/
#define DIVF_OFFSET                         16
/* Watchdog control register field definitions 
*/
#define WDTE_OFFSET                         24              /* Watchdog enable */
#define WDSTOP_PATTERN                      0xA5            /* Watchdog stop pattern */
#define WDTCLR                              (1 << 23)       /* Watchdog timer clear */
#define OVSEL_15                            0               /* Overflow select count 2^15 */
#define OVSEL_16                            (1 << 21)       /* Overflow select count 2^16 */
#define OVSEL_17                            (2 << 21)       /* Overflow select count 2^17 */
#define OVSEL_18                            (3 << 21)       /* Overflow select count 2^18 */
#define WDTIND                              (1 << 20)       /* Indicate whether watchdog ever occurs */



/* GPIO control registers 
*/
#define PABCNR                              (0x00C + GICR_BASE)     /* Port AB control */
#define PABDIR                              (0x010 + GICR_BASE)     /* Port AB direction */
#define PABDAT                              (0x014 + GICR_BASE)     /* Port AB data */
#define PABISR                              (0x018 + GICR_BASE)     /* Port AB interrupt status */
#define PABIMR                              (0x01C + GICR_BASE)     /* Port AB interrupt mask */
#define PCIMR                               (0x050 + GICR_BASE)     /* Port C interrupt mask */
/* Port ABC data register field definitions 
*/
#define PDA_OFFSET                          24              /* Port A data offset */
#define PDB_OFFSET                          16              /* Port B data offset */
#define PDC_OFFSET                          8               /* Port C data offset */
/* Port ABC interrupt status register field definitions 
*/
#define PAIP_OFFSET                         24              /* Port A pending status offset */
#define PBIP_OFFSET                         16              /* Port B pending status offset */
#define PCIP_OFFSET                         8               /* Port C pending status offset */


/* System Clock Control Register */
#define SCLKCR                              (0x04C + GICR_BASE)     /* System Clock Control Register */

/* Peripheral Lexra timing control registers 
*/
#define PLTCR                               (0x060 + GICR_BASE)     /* Peripheral Lexra Timing Control Register */
#define LTOC                                (0x064 + GICR_BASE)     /* Peripheral Lexra timeout control */
/* Peripheral Lexra timeout control register field definitions
*/
#define TOEN                                (1 << 31)
#define TOLIMIT_2_7                         (0 << 28)
#define TOLIMIT_2_8                         (1 << 28)
#define TOLIMIT_2_9                         (2 << 28)
#define TOLIMIT_2_10                        (3 << 28)
#define TOLIMIT_2_11                        (4 << 28)
#define TOLIMIT_2_12                        (5 << 28)
#define TOLIMIT_2_13                        (6 << 28)
#define TOLIMIT_2_14                        (7 << 28)


/*
 * Memory Controll Registers
 */
#define MCR_BASE                            0xBD013000

#define MCR                                 (0x00 + MCR_BASE)    /* Memory Configuration Register */
#define MTCR0                               (0x04 + MCR_BASE)    /* Memory Timing Configuration Register 0 */
#define MTCR1                               (0x08 + MCR_BASE)    /* Memory Timing Configuration Register 1 */

/* rtl8651_clearAsicAllTable */
#define RTL8651_L2TBL_ROW					256
#define RTL8651_L2TBL_COLUMN				4

// ASIC specification part
#define RTL8651_PPPOE_NUMBER				8
#define RTL8651_ROUTINGTBL_SIZE			8
#define RTL8651_ARPTBL_SIZE				512
#define RTL8651_PPPOETBL_SIZE				8
#define RTL8651_TCPUDPTBL_SIZE			1024
#define RTL8651_TCPUDPTBL_BITS				10
#define RTL8651_ICMPTBL_SIZE				32
#define RTL8651_ICMPTBL_BITS				5
#ifdef CONFIG_RTL8650BBASIC
#define RTL8651_IPTABLE_SIZE				16
#define RTL8651_SERVERPORTTBL_SIZE			16
#define RTL8651_NXTHOPTBL_SIZE			32
#define RTL8651_RATELIMITTBL_SIZE		32
#else
#define RTL8651_IPTABLE_SIZE				8
#define RTL8651_SERVERPORTTBL_SIZE			8
#endif /* CONFIG_RTL8650BBASIC */
#define RTL8651_ALGTBL_SIZE				128
#define RTL8651_MULTICASTTBL_SIZE			64
#define RTL8651_IPMULTICASTTBL_SIZE		64
#define RTL8651_NEXTHOPTBL_SIZE			32
#define RTL8651_RATELIMITTBL_SIZE			32
#define RTL8651_MACTBL_SIZE			1024
#define RTL8651_PROTOTRAPTBL_SIZE		8
#define RTL8651_VLANTBL_SIZE			8
#define RTL8651_ACLTBL_SIZE			128

#define RTL8651_PROTOCOLTRAP_SIZE			8
#define RTL8651_VLAN_NUMBER				8



#ifdef CONFIG_RTL8186
#undef GIMR
#define GIMR								0xbd010000UL
#undef TCIE
#define TCIE								(1 << 0)
#undef UART0IE
#define UART0IE							(1 << 3)
#undef GISR
#define GISR								0xbd010004UL
#undef IRR
#define IRR								0xbd010008UL	/* Actually 8186 does not have this register */
#undef TCCNR
#define TCCNR							0xbd010050UL
#undef TC0EN
#define TC0EN							(1 << 0)
#undef TC0MODE_TIMER
#define TC0MODE_TIMER					(1 << 1)
#define TC0SRC_BASICTIMER				(1 << 8)
#undef TCIR
#define TCIR								0xbd010054
#undef TC0IE
#define TC0IE							(1 << 0)
#undef TC0IP
#define TC0IP							(1 << 4)
#undef CDBR
#define CDBR								0xbd010058UL
#undef TC0DATA
#define TC0DATA							0xbd010060UL
#undef TCD_OFFSET
#define TCD_OFFSET						0
#undef UART0_BASE
#define UART0_BASE						0xbd0100c3
#define UART_BASECLOCK					153600000
#endif



#endif   /* _ASICREGS_H */

