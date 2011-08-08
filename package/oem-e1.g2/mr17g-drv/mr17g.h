/* mr17g.h
 *  Sigrand MR17G E1 PCI adapter driver for linux (kernel 2.6.x)
 *
 *	Written 2008 by Artem Y. Polyakov (artpol84@gmail.com)
 *
 *	This driver presents MR17G modem to OS as common hdlc interface.
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 *
 */

#ifndef SIGRAND_MR17G_H
#define SIGRAND_MR17G_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/random.h>
#include<linux/firmware.h>
#include <linux/vermagic.h>
#include <linux/config.h>

#include <asm/io.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/semaphore.h>

#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include<linux/dma-mapping.h>

#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/hdlc.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include <net/arp.h>
#include <linux/ioctl.h>

#include "sg17ring.h"

// I/O memory read/write functions

#define iowrite8(val,addr)  writeb(val,addr)
#define iowrite16(val,addr)  writew(val,addr)
#define iowrite32(val,addr)  writel(val,addr)
#define ioread8(addr) readb(addr)
#define ioread16(addr) readw(addr)
#define ioread32(addr) readl(addr)
/*
// DIRECT Read/Write
#define iowrite8(val,addr)  *(addr) = val
#define iowrite16(val,addr) *(addr) = val
#define iowrite32(val,addr) *(addr) = val
#define ioread8(addr) *(addr)
#define ioread16(addr) *(addr)
#define ioread32(addr) *(addr)
*/

//----------------------- Types of hardware -----------
#define MR17G_PCI_VEN 0x0055
#define MR17G_PCI_DEV 0x0098

// MR17G4 (4*E1, PEF22554 chipset)
#define MR17G4_SUBSYS_ID 0x0001
#define MR17G4_IOMEM_SIZE 16*1024
// MR17G84 (8*E1, 2*PEF22554 chipsets, 4*E1 only for MULTIPLEXING)
#define MR17G84_SUBSYS_ID 0x0002
#define MR17G84_IOMEM_SIZE 16*1024
// MR17G8 (8*E1, 2*PEF22554 chipsets)
#define MR17G8_SUBSYS_ID 0x0003

//---------- General memory map constants --------------
#define MR17G_CH_SIZE 2*1024  // 2Kb

// Channel & SCI CRA bits
#define TXEN    0x01            // transmitter enable
#define RXEN    0x02            // receiver  enable
#define NCRC    0x04            // ignore received CRC
#define DLBK    0x08            // digital loopback
#define CSEL    0x10            // In SCI Chip select bit
#define FMOD    0x20            // interframe fill: 0 - all ones, 1 - 0xfe
#define XRST0    0x80            // reset the transceiver

// Channel only CRA bits
#define CMOD    0x10            // 0 - use CRC-32, 1 - CRC-16
#define PMOD    0x40            // data polarity: 0 - normal, 1 - invert

// SCI only CRA bits
#define CSEL    0x10            // In SCI Chip select bit
#define XRST1    0x40            // 


// CRB bits
#define RDBE    0x01            // read burst enable
#define WTBE    0x02            // write burst enable
#define RODD    0x04            // receive 2-byte alignment
#define RXDE    0x08            // receive data enable
#define FRM	    0x10		    // framed mode
#define EXTC	0x20		    // sync from external generator

// SR and IMR bits
#define TXS     0x01            // transmit success
#define RXS     0x02            // receive success
#define CRC     0x04            // CRC error
#define COL     0x40            // transmit collision
// SR only
#define OFL     0x08            // fifo overflow error
#define UFL     0x10            // fifo underflow error
#define EXT     0x20            // interrupt from sk70725
// IMR only
#define TST     0x80            // generate test interrupt

// MXCR bits
#define MXEN	0x01		// Multiplexer enable: 0-disabled (HDLC only), 1-enabled
#define CLKM	0x02		// Multiplexer Bus Clock usage: 1-Clock master, 0-slave
#define CLKAB	0x04		// Clock domain: 0 - clock A, 1 - clock B
#define CLKR	0x08		// 0 - Clock master uses local oscillator, 1 - Clock master uses clock from receiver

// Send timeout
#define TX_TIMEOUT	5*HZ

// E1
#define MAX_TS_BIT 32


//--------------- Memory map ------------------------------ //
#define MR17G_CHAN1_SIZE 2*1024
#	define CHAN1_BUFSIZE	512
#	define CHAN1_HDLCTX_OFFS 0
#	define CHAN1_HDLCRX_OFFS (CHAN1_HDLCTX_OFFS + CHAN1_BUFSIZE)  
#	define CHAN1_REGS_OFFS (CHAN1_HDLCRX_OFFS + CHAN1_BUFSIZE)  
#define MR17G_SCI_SIZE 4*1024
#define MR17G_CHAN2_SIZE 256
#	define CHAN2_REGS_OFFS 0

#define MR17G_CHAN1_START 0x0000
#define MR17G_SCI_START (MR17G_CHAN1_START + 4*MR17G_CHAN1_SIZE)
#define MR17G_CHAN2_START (MR17G_SCI_START + MR17G_SCI_SIZE)



// -------------- Channel memory -------------------------- //

// Channel registers
struct mr17g_hw_regs{
        u8 CRA,CRB,SR,IMR,CTDR,LTDR,CRDR,LRDR;
        u8 MAP0,MAP1,MAP2,MAP3;
	    u8 MXMAP0,MXMAP1,MXMAP2,MXMAP3;
	    u8 RATE,MXRATE,TFS,RFS,TLINE,RLINE,MXCR;
};

// Channel memory map
struct mr17g_chan_iomem{
    volatile struct mr17g_hw_regs *regs;
    volatile struct sg_hw_descr  *tx_buf;
    volatile struct sg_hw_descr  *rx_buf;
};

// ---------------- SCI  ----------------------- //

// SCI registers
struct mr17g_sci_regs{
	u8  CRA,CRB,SR,IMR;
	u16 TXLEN,RXLEN;
};

// SCI memory map
#define SCI_BUF_SIZE (1*1024)
struct mr17g_sci_iomem {
    u8 tx_buf[SCI_BUF_SIZE],rx_buf[SCI_BUF_SIZE];
    struct mr17g_sci_regs regs;
};

// SCI service structure
struct mr17g_sci{
	volatile struct mr17g_sci_iomem *iomem;
	struct semaphore sem;
	wait_queue_head_t  wait_q;
	struct work_struct wqueue;
    u8 rxs:1;
    u8 crc_err:1;
    u32 tx_packets,tx_bytes;
    u32 rx_packets,rx_bytes;
    u32 crc_errors,tx_collisions;
};

//------------------- MR17G card service structures ----------//
// 4 channel  = 1 chip
// 1 or 2 chips = 1 card

struct mr17g_chan_config{
    // Slotmaps
    u32 slotmap;
    u32 mxslotmap;
    // PEF22554 settings
    u8 framed	:1;
    u8 cas   	:1;
    u8 crc4 	:1;
    u8 ts16     :1;
    u8 hdb3 	:1;
    u8 long_haul:1;
    u8 ext_clck :1;
    u8 llpb :1;
    u8 rlpb :1;
    // HDLC settings
    u8  crc16: 1;
    u8  fill_7e: 1;
	u8  inv: 1;
	u8  rburst: 1;
	u8  wburst: 1;
};


// Chipset description
enum mr17g_chiptype {MR17G_STANDARD,MR17G_MUXONLY};
struct mr17g_chip{
	enum mr17g_chiptype type;
	int num;
    unsigned long iomem_start;
    void *iomem;
    struct net_device *ifs[4];
    struct mr17g_sci *sci;
    struct pci_dev *pdev;
    u8 if_quan;
};

struct mr17g_channel{
    struct mr17g_chan_iomem iomem;
    struct mr17g_chan_config cfg;
	struct sg_ring rx,tx;
    struct mr17g_chip *chip;
    u8 num;
	struct net_device_stats	stats;
};

//
enum mr17g_cardtype { MR17G4,MR17G84,MR17G8 };
struct mr17g_card{
    char name[32];
    int number;
	// resources
    unsigned long iomem_start,iomem_end;
    void *iomem;
    struct pci_dev *pdev;
	enum mr17g_cardtype type;
	// devices
	struct mr17g_sci sci;
    struct mr17g_chip *chips;
    u8 chip_quan;
};

#endif
