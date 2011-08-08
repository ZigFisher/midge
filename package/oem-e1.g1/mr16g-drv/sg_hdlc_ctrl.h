/*
 * sg_hdlc_ctrl.h,v 1.00 2006/09/04
 *
 * Definitions for Sigrand HDLC controller 
 * Copyright (C) 2006, Artem U. Polyakov (art@sigrand.ru)
 */

#ifndef SG_HDLC_H
#define SG_HDLC_H


// CR bits
#define TXEN    0x01            // transmitter enable
#define RXEN    0x02            // receiver  enable
#define NCRC    0x04            // ignore received CRC
#define DLBK    0x08            // digital loopback
#define CMOD    0x10            // 0 - use CRC-32, 1 - CRC-16
#define FMOD    0x20            // interframe fill: 0 - all ones, 1 - 0xfe
#define PMOD    0x40            // data polarity: 0 - normal, 1 - invert
#define XRST    0x80            // reset the transceiver

// CRB bits
#define RDBE    0x01            // read burst enable
#define WTBE    0x02            // write burst enable
#define RODD    0x04            // receive 2-byte alignment
#define RXDE    0x08            // receive data enable
#define FRM     0x10		// framed mode
#define EXTC	0x20		// sync from external generator

// SR and IMR bits
#define TXS     0x01            // transmit success
#define RXS     0x02            // receive success
// SR only
#define CRC     0x04            // CRC error
#define OFL     0x08            // fifo overflow error
#define UFL     0x10            // fifo underflow error
#define EXT     0x20            // interrupt from sk70725
// IMR only
#define TSI     0x80            // generate test interrupt

// MXCR bits
#define MXEN	0x01		// Multiplexer enable: 0-disabled (HDLC only), 1-enabled
#define CLKM	0x02		// Multiplexer Bus Clock usage: 1-Clock master, 0-slave
#define CLKAB	0x04		// Clock domain: 0 - clock A, 1 - clock B
#define CLKR	0x08		// 0 - Clock master uses local oscillator, 1 - Clock master uses clock from receiver

#define LAST_FRAG 0x00008000

#define ETHER_MIN_LEN   64
#define SG16_MAX_FRAME  (1536 + 16)

#define XQLEN   8
#define RQLEN   8

// We don't have official vendor id yet...
#define MR16G_PCI_VENDOR 0x55
#define MR16G_PCI_DEVICE 0x9b

// Portability 
#define iotype void*
//#define IO_READ_WRITE
#ifndef IO_READ_WRITE
#       define iowrite8(val,addr)  writeb(val,addr)
#       define iowrite32(val,addr)  writel(val,addr)
#       define ioread8(addr) readb(addr)
#       define ioread32(addr) readl(addr)
#endif

// Send timeout
#define TX_TIMEOUT	5*HZ

// E1
#define MAX_TS_BIT 32

// macroses
#define mr16g_priv(ndev) ((struct net_local*)(dev_to_hdlc(ndev)->priv))
#define mr16g_e1cfg(ndev) ((struct ds2155_config*)&(mr16g_priv(ndev)->e1_cfg))
#define mr16g_hdlcfg(ndev) ((struct hdlc_config*)&(mr16g_priv(ndev)->hdlc_cfg))
	


struct ds2155_config{
    u32 slotmap;
    u32 mxslotmap;
    u8 framed	:1;
    u8 int_clck :1;
    u8 cas 	:1;
    u8 crc4 	:1;
    u8 ts16     :1;
    u8 hdb3 	:1;
    u8 long_haul:1;
    
};

struct hdlc_config
{
    u8  crc16: 1;
    u8  fill_7e: 1;
	u8  inv: 1;
	u8  rburst: 1;
	u8  wburst: 1;
};
		    

struct mr16g_hw_regs {
    u8  CRA,CRB,SR,IMR,CTDR,LTDR,CRDR,LRDR;
	u8 MAP0,MAP1,MAP2,MAP3;
	u8 MXMAP0,MXMAP1,MXMAP2,MXMAP3;
	u8 RATE,MXRATE,TFS,RFS,TLINE,RLINE,MXCR;
};

struct mr16g_hw_descr{
    u32  address;
	u32  length;
};
																	 

struct net_local{

	// standard net device statictics
	struct net_device_stats     stats;
	
	// device entity
	struct device *dev;
	// configuration	
	struct ds2155_config e1_cfg;
	struct hdlc_config hdlc_cfg;
	   
	// IO memory map
	void *mem_base;
	volatile struct mr16g_hw_regs *hdlc_regs;
	volatile struct mr16g_hw_descr    *tbd;
	volatile struct mr16g_hw_descr    *rbd;
	volatile u8 *ds2155_regs;
	
	//concurent racing 
	spinlock_t rlock,xlock;
	
        // transmit and reception queues 
        struct sk_buff *xq[ XQLEN ], *rq[ RQLEN ];
	unsigned head_xq, tail_xq, head_rq, tail_rq;
	    
	// the descriptors mapped onto the first buffers in xq and rq 
	unsigned head_tdesc, head_rdesc;
		    	
};

// Driver initialisation
static int  mr16g_init( void );
static void mr16g_exit( void );

// PCI related functions
static int __devinit  mr16g_init_one( struct pci_dev *,
                                const struct pci_device_id * );
static void __devexit mr16g_remove_one( struct pci_dev * );
					
// Net device specific functions
static int __init  mr16g_probe( struct net_device * );
static int  mr16g_open( struct net_device * );
static int  mr16g_close( struct net_device * );
static struct net_device_stats  *mr16g_get_stats( struct net_device * );
static irqreturn_t  mr16g_int( int, void *, struct pt_regs * );
static void mr16g_setup_carrier(struct net_device *ndev,u8 *mask);
static int mr16g_ioctl(struct net_device *, struct ifreq *, int );
static void  mr16g_tx_timeout( struct net_device * );
static int mr16g_attach(struct net_device *, unsigned short ,unsigned short );
static u32 mr16g_get_rate(struct net_device *ndev);
static u32 mr16g_get_slotmap(struct net_device *ndev);
static u32 mr16g_get_clock(struct net_device *ndev);

// Functions serving tx/rx
static void mr16g_txrx_up(struct net_device *);
static void mr16g_txrx_down(struct net_device *);
static int mr16g_start_xmit( struct sk_buff*, struct net_device* );
static void xmit_free_buffs( struct net_device * );
static void recv_init_frames( struct net_device * );
static void recv_alloc_buffs( struct net_device * );
static void recv_free_buffs( struct net_device * );

// HDLC controller functions
inline void mr16g_hdlc_down(struct net_local *nl);
inline void mr16g_hdlc_up( struct net_local *nl);
inline void mr16g_hdlc_setup( struct net_local *nl);
inline void mr16g_hdlc_open( struct net_local *nl);
inline void mr16g_hdlc_close( struct net_local *nl);

// DS2155 control/setup 
inline void ds2155_setreg(struct net_local *nl,u8 regname,u8 regval);
inline u8 ds2155_getreg(struct net_local *nl,u8 regname);
static int mr16g_E1_int_setup(struct net_local *nl);
static int mr16g_E1_setup(struct net_device *ndev);
static u8 ds2155_carrier(struct net_local *nl);
static int ds2155_interrupt( struct net_device *ndev, u8 *mask );


// Slotap related
static u32 str2slotmap(char *str,size_t size,int *err);
static int slotmap2str(u32 smap,struct ds2155_config *cfg,char *buf);
static int slot_cnt(u32 smap);

// Sysfs related functions
#define ADDIT_ATTR
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12)
#undef ADDIT_ATTR
#define ADDIT_ATTR struct device_attribute *attr,
#endif
static void mr16g_defcfg(struct net_local *nl);
static int mr16g_sysfs_init( struct device *);
static void mr16g_sysfs_del(struct device *);

#define to_net_dev(class) container_of(class, struct net_device, class_dev)

static ssize_t show_crc16(struct class_device *cdev, char *buf);
static ssize_t store_crc16( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_fill_7e(struct class_device *cdev, char *buf);
static ssize_t store_fill_7e( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_inv(struct class_device *cdev, char *buf);
static ssize_t store_inv( struct class_device *cdev,const char *buf, size_t size);

static ssize_t show_rburst(struct class_device *cdev, char *buf);
static ssize_t store_rburst( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_wburst(struct class_device *cdev, char *buf);
static ssize_t store_wburst( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_slotmap(struct class_device *cdev, char *buf);
static ssize_t store_slotmap( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_map_ts16(struct class_device *cdev, char *buf);
static ssize_t store_map_ts16( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_framed(struct class_device *cdev, char *buf);
static ssize_t store_framed( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_clck(struct class_device *cdev, char *buf);
static ssize_t store_clck( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_lhaul(struct class_device *cdev, char *buf);
static ssize_t store_lhaul( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_hdb3(struct class_device *cdev, char *buf);
static ssize_t store_hdb3( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_crc4(struct class_device *cdev, char *buf);
static ssize_t store_crc4( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_cas(struct class_device *cdev, char *buf);
static ssize_t store_cas( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_rloopback(struct class_device *cdev, char *buf);
static ssize_t store_rloopback( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_lloopback(struct class_device *cdev, char *buf);
static ssize_t store_lloopback( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_flinkdown(struct class_device *cdev, char *buf);
static ssize_t store_flinkdown( struct class_device *cdev,const char *buf, size_t size );

// DEBUG
static ssize_t show_hdlcregs(struct class_device *cdev, char *buf);
//-------------- Memory window debug -----------------------------//
#ifdef SYSFS_DEBUG
static ssize_t show_winread(struct class_device *cdev, char *buf);
static ssize_t store_winread( struct class_device *cdev,const char *buf, size_t size );

static ssize_t show_winwrite(struct class_device *cdev, char *buf);
static ssize_t store_winwrite( struct class_device *cdev,const char *buf, size_t size );
#endif

// ---------------- Multiplexing -------------------------------------//
// MXMAP rgisters
static ssize_t show_mx_slotmap(struct class_device *cdev, char *buf);
static ssize_t store_mx_slotmap(struct class_device *cdev,const char *buf,size_t size);
// TFS rgister
static ssize_t show_mx_txstart(struct class_device *cdev, char *buf);
static ssize_t store_mx_txstart( struct class_device *cdev,const char *buf, size_t size);
// RFS rgister
static ssize_t show_mx_rxstart(struct class_device *cdev, char *buf);
static ssize_t store_mx_rxstart( struct class_device *cdev,const char *buf, size_t size);
// TLINE rgister
static ssize_t show_mx_tline(struct class_device *cdev, char *buf);
static ssize_t store_mx_tline( struct class_device *cdev,const char *buf, size_t size);
// RLINE rgister
static ssize_t show_mx_rline(struct class_device *cdev, char *buf);
static ssize_t store_mx_rline( struct class_device *cdev,const char *buf, size_t size);

// MXCR rgister
// MX enable
static ssize_t show_mx_enable(struct class_device *cdev, char *buf);
static ssize_t store_mx_enable(struct class_device *cdev,const char *buf,size_t size);
// CLKM
static ssize_t show_mx_clkm(struct class_device *cdev, char *buf);
static ssize_t store_mx_clkm(struct class_device *cdev,const char *buf,size_t size);
// CLKAB
static ssize_t show_mx_clkab(struct class_device *cdev,char *buf);
static ssize_t store_mx_clkab(struct class_device *cdev,const char *buf,size_t size);
// CLKR
static ssize_t show_mx_clkr(struct class_device *cdev, char *buf);
static ssize_t store_mx_clkr(struct class_device *cdev,const char *buf,size_t size);

#endif
