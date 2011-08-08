/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /home/cvsroot/uClinux-dist/loader_srcroot/src/rtl8650/swNic_poll.c,v 1.7 2005/02/23 01:39:23 yjlou Exp $
*
* Abstract: Switch core polling mode NIC driver source code.
*
* $Author: yjlou $
*
* $Log: swNic_poll.c,v $
* Revision 1.7  2005/02/23 01:39:23  yjlou
* -: remove swNic_txRxSwitch(), because it is no need. DMA area is allocated in heap. Therefore, before the heap is broken, the loader has been overwritten already.
*
* Revision 1.6  2005/01/12 13:50:44  yjlou
* *: version migrates to 00.00.18
* *: always disable TX/RX DMA before execute runtime code.
*
* Revision 1.5  2004/10/05 06:39:28  yjlou
* *: remove Mii port from the port list in swNic_sendswNic_send().
*
* Revision 1.4  2004/07/14 02:16:09  yjlou
* +: add '#ifdef FAT_CODE' to remove un-used functions
*
* Revision 1.3  2004/05/14 09:39:42  orlando
* check in CONFIG_RTL865X_BICOLOR_LED/DIAG_LED/INIT_BUTTON related code
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
* Revision 1.2  2003/05/16 06:53:26  danwu
* remove interrupt handling
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
#include <rtl8650/swNic_poll.h>



/* STATIC VARIABLE DECLARATIONS
 */
/* descriptor rings */
static uint32  num_of_rx_pkthdr_desc;
static uint32  num_of_rx_mbuf_desc;
static uint32  num_of_tx_pkthdr_desc;
static uint32  *RxPkthdrDesc;
static uint32  *RxMbufDesc;
static uint32  *TxPkthdrDesc;
static uint32  size_of_cluster;

/* descriptor ring tracing pointers */
static int32   currRxPkthdrDescIndex;      /* Rx pkthdr descriptor to be handled by CPU */
static int32   currRxMbufDescIndex;        /* Rx mbuf descriptor to be handled by CPU */
static int32   currTxPkthdrDescIndex;      /* Tx pkthdr descriptor to be handled by CPU */

/* debug counters */
static int32   rxPktCounter;
static int32   txPktCounter;

#define ARPTAB_SIZ 16
struct arptab_s
    {
    uint8   arp_mac_addr[6];  /* hardware address */
    uint8   valid;
    uint32  port_list;
    };
static struct arptab_s arptab[ARPTAB_SIZ];
static uint32 arptab_next_available;

#define     BUF_FREE            0x00   /* Buffer is Free  */
#define     BUF_USED            0x80   /* Buffer is occupied */
#define     BUF_ASICHOLD        0x80   /* Buffer is hold by ASIC */
#define     BUF_DRIVERHOLD      0xc0   /* Buffer is hold by driver */
#define     BUF_INETHOLD        0xe0   /* Buffer is hold by INET protocol stack */

/* mbuf header associated with each cluster 
*/
struct mBuf
{
    struct mBuf *m_next;
    struct pktHdr *m_pkthdr;            /* Points to the pkthdr structure */
    uint16    m_len;                    /* data bytes used in this cluster */
    int8      m_flags;                  /* mbuf flags; see below */
#define MBUF_FREE            BUF_FREE   /* Free. Not occupied. should be on free list   */
#define MBUF_USED            BUF_USED   /* Buffer is occupied */
#define MBUF_EXT             0x10       /* has associated with an external cluster, this is always set. */
#define MBUF_PKTHDR          0x08       /* is the 1st mbuf of this packet */
#define MBUF_EOR             0x04       /* is the last mbuf of this packet. Set only by ASIC*/
    int8      m_type;                   /*  CHANGED: Type of data of this mbuf. Shrinks to 1 byte.  */
#define MBUFTYPE_FREE        0x00       /*Free, unused buffer */
#define MBUFTYPE_DATA        0x01       /*dynamic (data) allocation */
#define MBUFTYPES            0x02       /*Total number of mbuf types for mbtypes[] */
    uint8     *m_data;                  /*  location of data in the cluster */
    uint8     *m_extbuf;                /* start of buffer*/
    void      (*m_extfree) (uint8 *, uint32);    /*  cluster free routine */
    void      (*m_extref) (uint8 *, uint32);    /*  cluster reference counting function */
    uint16    m_extsize;                /* sizeof the cluster */
    uint8     m_process_flags;
    int8      m_reserved[1];
};

/* pkthdr records packet specific information. Each pkthdr is exactly 32 bytes.
 first 20 bytes are for ASIC, the rest 12 bytes are for driver and software usage.
*/
struct pktHdr
{
    union
    {
        struct pktHdr *pkthdr_next;     /*  next pkthdr in free list */
        struct mBuf *mbuf_first;        /*  1st mbuf of this pkt */
    }PKTHDRNXT;
#define ph_nextfree         PKTHDRNXT.pkthdr_next
#define ph_mbuf             PKTHDRNXT.mbuf_first
    uint16    ph_len;                   /*   total packet length */
    uint16    ph_flags;                 /*  NEW:Packet header status bits */
#define PKTHDR_FREE          (BUF_FREE << 8)        /* Free. Not occupied. should be on free list   */
#define PKTHDR_USED          (BUF_USED << 8)
#define PKTHDR_ASICHOLD      (BUF_ASICHOLD<<8)      /* Hold by ASIC */
#define PKTHDR_DRIVERHOLD    (BUF_DRIVERHOLD<<8)    /* Hold by driver */
#define PKTHDR_CPU_OWNED     0x4000
#define PKT_INCOMING         0x1000     /* Incoming: packet is incoming */
#define PKT_OUTGOING         0x0800     /*  Outgoing: packet is outgoing */
#define PKT_BCAST            0x0100     /*send/received as link-level broadcast  */
#define PKT_MCAST            0x0080     /*send/received as link-level multicast   */
#define PKTHDR_HPRIORITY     0x0010     /* High priority */
#define PKTHDR_PPPOE_AUTOADD    0x0008  /* PPPoE header auto-add */
#define PKTHDR_VLAN_AUTOADD     0x0004  /* VLAN tag auto-add */
#define CSUM_TCPUDP_OK       0x0001     /*Incoming:TCP or UDP cksum checked */
#define CSUM_IP_OK           0x0002     /* Incoming: IP header cksum has checked */
#define CSUM_TCPUDP          0x0001     /*Outgoing:TCP or UDP cksum offload to ASIC*/
#define CSUM_IP              0x0002     /* Outgoing: IP header cksum offload to ASIC*/
    uint16    ph_proto;                 /*  NEW:protocol type recognized by ASIC or filled in by CPU */
#define PKTHDR_ETHERNET      0
#define PKTHDR_IP            (2 << 13)
#define PKTHDR_ICMP          (3 << 13)
#define PKTHDR_IGMP          (4 << 13)
#define PKTHDR_TCP           (5 << 13)
#define PKTHDR_UDP           (6 << 13)
#define PKTHDR_PROTO_MASK    (7 << 13)
#define PKTHDR_VLAN_TAGGED   (1 << 12)
#define PKTHDR_LLC_TAGGED    (1 << 11)
#define PKTHDR_PPPOE_TAGGED  (1 << 10)
#define PKTHDR_VLAN_IDX_OFFSET 4
#define PKTHDR_PPPOE_IDX_OFFSET 7
    uint16    ph_reason;                /*   NEW: (TBD) indicates why the packet is received by CPU */
    uint32    ph_portlist;              /*  Physical Incoming port id / Physical Outgoing Port list */
    uint32    ph_reserved;
    union{
        struct {
            void    (*callback)(void *, uint32);        /* for driver call back function */
            struct pktHdr *nextHdr;     /*  next packet in queue/record */
            uint32  mbufCount;          /* for mbuf flow control */
        } driver;
    }u;
#define ph_callback     u.driver.callback
#define ph_mbufcount    u.driver.mbufCount
#define ph_nextHdr      u.driver.nextHdr
};



/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static void arpInput(uint8*,uint32);
static int32 arpResolve(uint8*,uint32*);



#pragma ghs section text=".iram"
/*************************************************************************
*   FUNCTION                                                              
*       swNic_intHandler                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function is the handler of NIC interrupts
*                                                                         
*   INPUTS                                                                
*       intPending      Pending interrupt sources.
*                                                                         
*   OUTPUTS                                                               
*       None
*************************************************************************/
void swNic_intHandler(uint32 intPending) {return;}

/*************************************************************************
*   FUNCTION                                                              
*       swNic_receive                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function reads one packet from rx descriptors, and return the 
*       previous read one to the switch core. This mechanism is based on 
*       the assumption that packets are read only when the handling 
*       previous read one is done.
*                                                                         
*   INPUTS                                                                
*       None
*                                                                         
*   OUTPUTS                                                               
*       None
*************************************************************************/
int32 swNic_receive(void** input, uint32* pLen)
{
    struct pktHdr * pPkthdr;
    int32 pkthdr_index;
    int32 mbuf_index;
    static int32 firstTime = 1;
    
    /* Check OWN bit of descriptors */
    if ( (RxPkthdrDesc[currRxPkthdrDescIndex] & DESC_OWNED_BIT) == DESC_RISC_OWNED )
    {
        ASSERT_ISR(currRxPkthdrDescIndex < num_of_rx_pkthdr_desc);
        
        /* Fetch pkthdr */
        pPkthdr = (struct pktHdr *) (RxPkthdrDesc[currRxPkthdrDescIndex] & 
                                            ~(DESC_OWNED_BIT | DESC_WRAP));
    
        ASSERT_ISR(pPkthdr->ph_len); /* Not allow zero packet length */
        ASSERT_ISR(pPkthdr->ph_len >= 64);
        ASSERT_ISR(pPkthdr->ph_len <= 1522);

        /* Increment counter */
        rxPktCounter++;

        /* Output packet */
        *input = pPkthdr->ph_mbuf->m_data;
        *pLen = pPkthdr->ph_len - 4;
        
        /* Peek ARP response packets to obtain port information */
        if ( (*(uint16*)(pPkthdr->ph_mbuf->m_data + 12) == 0x0806) && 
                (*(uint16*)(pPkthdr->ph_mbuf->m_data + 20) == 0x0002) )
            arpInput(pPkthdr->ph_mbuf->m_data, 1 << pPkthdr->ph_portlist);
        
        if ( !firstTime )
        {
            /* Calculate previous pkthdr and mbuf index */
            pkthdr_index = currRxPkthdrDescIndex;
            if ( --pkthdr_index < 0 )
                pkthdr_index = num_of_rx_pkthdr_desc - 1;
            mbuf_index = currRxMbufDescIndex;
            if ( --mbuf_index < 0 )
                mbuf_index = num_of_rx_mbuf_desc - 1;
        
            /* Reset OWN bit */
            RxPkthdrDesc[pkthdr_index] |= DESC_SWCORE_OWNED;
            RxMbufDesc[mbuf_index] |= DESC_SWCORE_OWNED;
        }
        else
            firstTime = 0;
        
        /* Increment index */
        if ( ++currRxPkthdrDescIndex == num_of_rx_pkthdr_desc )
            currRxPkthdrDescIndex = 0;
        if ( ++currRxMbufDescIndex == num_of_rx_mbuf_desc )
            currRxMbufDescIndex = 0;
    
        if ( REG32(CPUIISR) & PKTHDR_DESC_RUNOUT_IP )
        {
            /* Enable and clear interrupt for continue reception */
            REG32(CPUIIMR) |= PKTHDR_DESC_RUNOUT_IE;
            REG32(CPUIISR) = PKTHDR_DESC_RUNOUT_IP;
        }
        
        return 0;
    }
    else
        return -1;
}

/*************************************************************************
*   FUNCTION                                                              
*       swNic_send                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function writes one packet to tx descriptors, and waits until 
*       the packet is successfully sent.
*                                                                         
*   INPUTS                                                                
*       None
*                                                                         
*   OUTPUTS                                                               
*       None
*************************************************************************/
int32 swNic_send(void * output, uint32 len)
{
    struct pktHdr * pPkthdr;
    
    ASSERT_CSP( ((int32) TxPkthdrDesc[currTxPkthdrDescIndex] & DESC_OWNED_BIT) == DESC_RISC_OWNED );
    
    /* Fetch packet header from Tx ring */
    pPkthdr = (struct pktHdr *) ((int32) TxPkthdrDesc[currTxPkthdrDescIndex] 
                                                & ~(DESC_OWNED_BIT | DESC_WRAP));
    
    /* Pad small packets and add CRC */
    if ( len < 60 )
        pPkthdr->ph_len = 64;
    else
        pPkthdr->ph_len = len + 4;
    pPkthdr->ph_mbuf->m_len = pPkthdr->ph_len;
        
    /* Set cluster pointer to buffer */
    pPkthdr->ph_mbuf->m_data = (uint8 *) output;
    
    /* Set destination port */
    if ( (pPkthdr->ph_mbuf->m_data[0] & 1) || 
            (arpResolve(pPkthdr->ph_mbuf->m_data, &pPkthdr->ph_portlist) != 0) )
        /* Broadcast to all member ports */
        pPkthdr->ph_portlist = 0x1f;
    
    /* Give descriptor to switch core */
    TxPkthdrDesc[currTxPkthdrDescIndex] |= DESC_SWCORE_OWNED;
    
    /* Set TXFD bit to start send */
    REG32(CPUICR) |= TXFD;
    
    /* Wait until packet is successfully sent */
    while ( (*(volatile uint32 *)&TxPkthdrDesc[currTxPkthdrDescIndex] 
                    & DESC_OWNED_BIT) == DESC_SWCORE_OWNED );
    
    txPktCounter++;
    
    if ( ++currTxPkthdrDescIndex == num_of_tx_pkthdr_desc )
        currTxPkthdrDescIndex = 0;
    
    return 0;
}
#pragma ghs section text=default

/*************************************************************************
*   FUNCTION                                                              
*       swNic_init                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function initializes descriptors and data structures.
*                                                                         
*   INPUTS                                                                
*       nRxPkthdrDesc   Number of Rx pkthdr descriptors.
*       nRxMbufDesc     Number of Tx mbuf descriptors.
*       nTxPkthdrDesc   Number of Tx pkthdr descriptors.
*       clusterSize     Size of cluster.
*                                                                         
*   OUTPUTS                                                               
*       Status.
*************************************************************************/
int32 swNic_init(uint32 nRxPkthdrDesc, uint32 nRxMbufDesc, uint32 nTxPkthdrDesc, 
                        uint32 clusterSize)
{
    uint32  icr_mbufsize;
    uint32  i;
    struct pktHdr *pPkthdrList;
    struct mBuf *pMbufList;
    uint8 * pClusterList;
    struct pktHdr * pPkthdr;
    struct mBuf * pMbuf;
    
    if ( (nRxPkthdrDesc == 0) || (nRxMbufDesc == 0) || 
            (nTxPkthdrDesc == 0) )
        return EINVAL;

    /* Cluster size is always 2048 */
    size_of_cluster = 2048;
    icr_mbufsize = MBUF_2048BYTES;
    
    num_of_rx_pkthdr_desc = num_of_rx_mbuf_desc = 
                (nRxPkthdrDesc < nRxMbufDesc) ? nRxPkthdrDesc : nRxMbufDesc;
    num_of_tx_pkthdr_desc = nTxPkthdrDesc;

    /* Allocate descriptors */
    RxPkthdrDesc = (uint32 *) UNCACHED_MALLOC(num_of_rx_pkthdr_desc * sizeof(uint32));
    ASSERT_CSP( (uint32) RxPkthdrDesc & 0x0fffffff );
    RxMbufDesc = (uint32 *) UNCACHED_MALLOC(num_of_rx_mbuf_desc * sizeof(uint32));
    ASSERT_CSP( (uint32) RxMbufDesc & 0x0fffffff );
    TxPkthdrDesc = (uint32 *) UNCACHED_MALLOC(num_of_tx_pkthdr_desc * sizeof(uint32));
    ASSERT_CSP( (uint32) TxPkthdrDesc & 0x0fffffff );
    
    /* Allocate pkthdr */
    pPkthdrList = (struct pktHdr *) UNCACHED_MALLOC(
                    (num_of_rx_pkthdr_desc + num_of_tx_pkthdr_desc) * sizeof(struct pktHdr));
    ASSERT_CSP( (uint32) pPkthdrList & 0x0fffffff );
                    
    /* Allocate mbufs */
    pMbufList = (struct mBuf *) UNCACHED_MALLOC(
                    (num_of_rx_mbuf_desc + num_of_tx_pkthdr_desc) * sizeof(struct mBuf));
    ASSERT_CSP( (uint32) pMbufList & 0x0fffffff );
                    
    /* Allocate clusters */
    pClusterList = (uint8 *) UNCACHED_MALLOC(num_of_rx_mbuf_desc * size_of_cluster + 8 - 1);
    ASSERT_CSP( (uint32) pClusterList & 0x0fffffff );
    pClusterList = (uint8*)(((uint32) pClusterList + 8 - 1) & ~(8 - 1));
    
    /* Initialize interrupt statistics counter */
    rxPktCounter = txPktCounter = 0;
    
    /* Initialize index of Tx pkthdr descriptor */
    currTxPkthdrDescIndex = 0;
    /* Initialize Tx packet header descriptors */
    for (i=0; i<num_of_tx_pkthdr_desc; i++)
    {
        /* Dequeue pkthdr and mbuf */
        pPkthdr = pPkthdrList++;
        pMbuf = pMbufList++;
        
        pPkthdr->ph_mbuf = pMbuf;
        pPkthdr->ph_len = 0;
        pPkthdr->ph_flags = PKTHDR_USED | PKT_OUTGOING;
        pPkthdr->ph_proto = PKTHDR_ETHERNET;
        pPkthdr->ph_portlist = 0;
        pMbuf->m_next = NULL;
        pMbuf->m_pkthdr = pPkthdr;
        pMbuf->m_flags = MBUF_USED | MBUF_EXT | MBUF_PKTHDR | MBUF_EOR;
        pMbuf->m_type = MBUFTYPE_DATA;
        pMbuf->m_data = NULL;
        pMbuf->m_extbuf = NULL;
        pMbuf->m_extsize = 0;
        
        TxPkthdrDesc[i] = (int32) pPkthdr | DESC_RISC_OWNED;
    }
    /* Set wrap bit of the last descriptor */
    TxPkthdrDesc[num_of_tx_pkthdr_desc - 1] |= DESC_WRAP;
    /* Fill Tx packet header FDP */
    REG32(CPUTPDCR) = (uint32) TxPkthdrDesc;

    /* Initialize index of current Rx pkthdr descriptor */
    currRxPkthdrDescIndex = 0;
    /* Initialize index of current Rx pkthdr descriptor */
    currRxMbufDescIndex = 0;
    /* Initialize Rx packet header descriptors */
    for (i=0; i<num_of_rx_pkthdr_desc; i++)
    {
        /* Dequeue pkthdr and mbuf */
        pPkthdr = pPkthdrList++;
        pMbuf = pMbufList++;
        /* Setup pkthdr and mbuf */
        pPkthdr->ph_mbuf = pMbuf;
        pPkthdr->ph_len = 0;
        pPkthdr->ph_flags = PKTHDR_USED | PKT_INCOMING;
        pPkthdr->ph_proto = PKTHDR_ETHERNET;
        pPkthdr->ph_portlist = 0;
        pMbuf->m_next = NULL;
        pMbuf->m_pkthdr = pPkthdr;
        pMbuf->m_len = 0;
        pMbuf->m_flags = MBUF_USED | MBUF_EXT | MBUF_PKTHDR | MBUF_EOR;
        pMbuf->m_type = MBUFTYPE_DATA;
        pMbuf->m_data = NULL;
        pMbuf->m_extbuf = NULL;
        pMbuf->m_extsize = size_of_cluster;
        pMbuf->m_data = pMbuf->m_extbuf = pClusterList;
        pClusterList += size_of_cluster;
        
        /* Setup descriptors */
        RxPkthdrDesc[i] = (int32) pPkthdr | DESC_SWCORE_OWNED;
        RxMbufDesc[i] = (int32) pMbuf | DESC_SWCORE_OWNED;
    }
    /* Set wrap bit of the last descriptor */
    RxPkthdrDesc[num_of_rx_pkthdr_desc - 1] |= DESC_WRAP;
    RxMbufDesc[num_of_rx_mbuf_desc - 1] |= DESC_WRAP;
    /* Fill Rx packet header FDP */
    REG32(CPURPDCR) = (uint32) RxPkthdrDesc;
    REG32(CPURMDCR) = (uint32) RxMbufDesc;
    
    /* Initialize ARP table */
    bzero((void *) arptab, ARPTAB_SIZ * sizeof(struct arptab_s));
    arptab_next_available = 0;

    /* Enable runout interrupts */
    REG32(CPUIIMR) |= RX_ERR_IE | TX_ERR_IE | PKTHDR_DESC_RUNOUT_IE;

    /* Enable Rx & Tx. Config bus burst size and mbuf size. */
    //REG32(CPUICR) = TXCMD | RXCMD | BUSBURST_256WORDS | icr_mbufsize;
    REG32(CPUICR) = TXCMD | RXCMD | BUSBURST_32WORDS | icr_mbufsize;

    return SUCCESS;
}

#ifdef FAT_CODE
/*************************************************************************
*   FUNCTION                                                              
*       swNic_resetDescriptors                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function resets descriptors.
*                                                                         
*   INPUTS                                                                
*       None.
*                                                                         
*   OUTPUTS                                                               
*       None.
*************************************************************************/
void swNic_resetDescriptors(void)
{
    /* Disable Tx/Rx and reset all descriptors */
    REG32(CPUICR) &= ~(TXCMD | RXCMD);
    return;
}
#endif//FAT_CODE

static void arpInput(uint8* buf, uint32 port_list)
{
    int i;
    
    /* Search the ARP table and update the entry if already exists. 
        If not found, fill into the next available slot */
    for (i=0;i<ARPTAB_SIZ;i++)
        if (arptab[i].valid)
        {
            if (memcmp(arptab[i].arp_mac_addr, &buf[6], 6) == 0)
                break;
        }
        else
            break;
    if (i == ARPTAB_SIZ)
    {
        i = arptab_next_available;
        if (++arptab_next_available == ARPTAB_SIZ)
            arptab_next_available = 0;
    }
        
    memcpy(arptab[i].arp_mac_addr, &buf[6], 6);
    arptab[i].port_list = port_list;
    arptab[i].valid = 1;
}

static int32 arpResolve(uint8* buf, uint32* port_list_ptr)
{
    int32 i;
    
    /* search the ARP table */
    for (i=0;i<ARPTAB_SIZ;i++)
        if (arptab[i].valid)
        {
            if (memcmp(arptab[i].arp_mac_addr, &buf[0], 6) == 0)
            {
                *port_list_ptr = arptab[i].port_list;
                return 0;
            }
        }
        else
            break;
    
    return -1;
}


