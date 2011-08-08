/* sg17main.c:  Sigrand SG-17PCI SHDSL modem driver for linux (kernel 2.6.x)
 *
 *	Written 2006-2007 by Artem Y. Polyakov <artpol84@gmail.com>
 *
 *	This driver presents SG-17PCI modem 
 *	to system as common ethernet-like netcard.
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 *
 *
 *	25.01.2007	Version 1.0 - Artem Y. Polyakov <art@sigrand.ru>
 *	23.01.2009	Version 1.1 - Artem Y. Polyakov <art@sigrand.ru>
 *		Fix AdvLink memory leack
 */
 
#define DRIVER_VERSION "1.1"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/vermagic.h>
#include <linux/version.h>

#include <asm/types.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/semaphore.h>
#include <asm/atomic.h>

#include <linux/ptrace.h>
#include <linux/fcntl.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/random.h>
#include <linux/firmware.h>

#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/dma-mapping.h>
#include <linux/workqueue.h>


//---- Local includes ----//


#include "sg17sci.h"
#include "include/sg17hw.h"
#include "include/sdfe4_lib.h"
#include "include/sg17device.h"
#include "include/sg17eoc.h"
#include "sg17main.h"
#include "sg17ring_funcs.h"

#include "advlink.h"

//---- OEM include -------//
#include "sg17oem.h"

// Debug parameters
#define DEBUG_ON
#define DEFAULT_LEV 10
#include "sg17debug.h"

MODULE_DESCRIPTION( "Infineon SHDSL driver Version "DRIVER_VERSION"\n" );
MODULE_AUTHOR( "Maintainer: Artem U. Polyakov artpol84@gmail.com\n" );
MODULE_LICENSE( "GPL" );
MODULE_VERSION(DRIVER_VERSION);


// DEBUG //
struct sg17_sci *SCI;
// DEBUG //


static int error_output = 40;

module_param(error_output, int, 40);
MODULE_PARM_DESC(error_output, "Debug output option - print messages about errors");

/* --------------------------------------------------------------------------
 *      SG17 network interfaces
 * -------------------------------------------------------------------------- */

void
sg17_dsl_init( struct net_device *ndev)
{
	PDEBUG(debug_netcard,"");
	ether_setup(ndev);
	ndev->init = sg17_probe;
	ndev->uninit = sg17_uninit;    
}
	
static int __init
sg17_probe( struct net_device  *ndev )
{
	struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);
	int err=-ENODEV;

	PDEBUG(debug_netcard,"start");
	// Carrier off
	netif_carrier_off( ndev );
	netif_stop_queue(ndev);
	PDEBUG(debug_netcard,"m1");
	// generate 'unique' MAC address
	*(u16 *)ndev->dev_addr = htons( 0x00ff );
	*(u32 *)(ndev->dev_addr + 2) = htonl( 0x01a39000 | ((u32)ndev->priv & 0x00000fff) );
	PDEBUG(debug_netcard,"m2");
	// Init net device handler functions 
	ndev->open = sg17_open;
	ndev->stop = sg17_close;
	ndev->hard_start_xmit = sg17_start_xmit;
	ndev->get_stats = sg17_get_stats;
	ndev->change_mtu = sg17_change_mtu;	
	ndev->set_multicast_list = sg17_set_mcast_list;
	ndev->tx_timeout = sg17_tx_timeout;
	ndev->do_ioctl = sg17_ioctl;
	ndev->watchdog_timeo = TX_TIMEOUT;
	PDEBUG(debug_netcard,"m3");
	// set network device private data 
	nl->regs = (struct sg17_hw_regs *) ((u8 *)ndev->mem_start + HDLC_REGS);
	sg17_tranceiver_down(nl);
	PDEBUG(debug_netcard,"m4");	
	// setup transmit and receive rings
	nl->tx.hw_ring = (struct sg_hw_descr *) ((u8 *)ndev->mem_start + HDLC_TXBUFF);
	nl->rx.hw_ring = (struct sg_hw_descr *) ((u8 *)ndev->mem_start + HDLC_RXBUFF);
	nl->tx.hw_mask=nl->rx.hw_mask=HW_RING_MASK;
	nl->tx.sw_mask=nl->rx.sw_mask=SW_RING_MASK;
	nl->tx.CxDR=(u8*)&(nl->regs->CTDR);
	nl->rx.CxDR=(u8*)&(nl->regs->CRDR);
	nl->tx.LxDR=(u8*)&(nl->regs->LTDR);
	nl->rx.LxDR=(u8*)&(nl->regs->LRDR);
	nl->tx.type=TX_RING;
	nl->rx.type=RX_RING;
	nl->tx.dev = nl->rx.dev = nl->dev;
	//DEBUG//
	nl->nsg_comp = 0;
	//DEBUG//
	spin_lock_init( &nl->tx.lock );
	spin_lock_init( &nl->rx.lock );
	spin_lock_init( &nl->lock );
	PDEBUG(debug_netcard,"m5");
	
	// enable iface
	iowrite8(XRST, &nl->regs->CRA);
	//default HDLC X config
	nl->hdlc_cfg.rburst = 0;
	nl->hdlc_cfg.wburst = 0;	
	
	// net device interrupt register
	PDEBUG(debug_netcard,"start registering irq");
	if( (err = request_irq(ndev->irq, sg17_interrupt, SA_SHIRQ, ndev->name, ndev)) ){
		printk( KERN_ERR "%s: unable to get IRQ %d, error= %08x\n",
				ndev->name, ndev->irq, err );
		return err;
	}
	PDEBUG(debug_netcard,"request_irq - ok");
	
	printk( KERN_NOTICE "%s: "MR17H_MODNAME" SHDSL module (irq %d, mem %#lx)\n",
			ndev->name, ndev->irq, ndev->mem_start );
	SET_MODULE_OWNER( ndev );

	return  0;
}

static void __devexit
sg17_uninit(struct net_device *ndev)
{
	struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);

	free_irq( ndev->irq, ndev );
	sg17_tranceiver_down(nl);
}

static irqreturn_t
sg17_interrupt( int  irq,  void  *dev_id,  struct pt_regs  *regs )
{
	struct net_device *ndev = (struct net_device *) dev_id;
	struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);
	u8 status;
	u8 mask = ioread8(&(nl->regs->IMR));	

	PDEBUG(debug_irq,"%s: mask=%02x",ndev->name,mask);
	if( (ioread8(&(nl->regs->SR)) & mask ) == 0 )
		return IRQ_NONE;

	status = ioread8(&(nl->regs->SR));
	iowrite8(status,&(nl->regs->SR));	
	iowrite8( 0, &(nl->regs->IMR));
	PDEBUG(debug_irq,"%s: status = %02x",ndev->name,status);

	if( status & RXS ){
		PDEBUG(debug_irq,"%s: RXS, CRA=%02x\n",ndev->name,nl->regs->CRA);	
		recv_init_frames( ndev );
		recv_alloc_buffs( ndev );
	}
	if( status & TXS ){
		PDEBUG(debug_irq,"%s: TXS, CRA=%02x\n",ndev->name,nl->regs->CRA);
		xmit_free_buffs( ndev );
	}
	if( status & CRC ){
	    PDEBUG(debug_irq,"%s: CRC, CRA=%02x\n",ndev->name,nl->regs->CRA);	
	    ++nl->stats.rx_errors;
		++nl->stats.rx_crc_errors;
	}
	if( status & OFL ){
	    PDEBUG(debug_irq,"%s: OFL, CRA=%02x\n",ndev->name,nl->regs->CRA);
	    ++nl->stats.rx_errors;
	    ++nl->stats.rx_over_errors;
	}
	if( status & UFL ){
		//  Whether transmit error is occured, we have to re-enable the
		//  transmitter. That's enough, because linux doesn't fragment
	    //  packets.
	    PDEBUG(debug_irq,"%s: UFL, CRA=%02x\n",ndev->name,nl->regs->CRA);
	    iowrite8( ioread8(&(nl->regs->CRA)) | TXEN,
				  &(nl->regs->CRA) );
	    ++nl->stats.tx_errors;
	    ++nl->stats.tx_fifo_errors;
	}

	iowrite8( mask,&(nl->regs->IMR));	
	
	return IRQ_HANDLED;
}


static int
sg17_open( struct net_device  *ndev )
{
	struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);		
    
	// init descripts, allocate receiving buffers 
	nl->tx.head = nl->tx.tail = nl->rx.head = nl->rx.tail = 0;
	nl->tx.FxDR = nl->rx.FxDR = 0;
	iowrite8( 0, (nl->tx.CxDR));
	iowrite8( 0, (nl->tx.LxDR));	
	iowrite8( 0, (nl->rx.CxDR));	
	iowrite8( 0, (nl->rx.LxDR));	
	recv_alloc_buffs( ndev );
	// enable receive/transmit functions 
	sg17_tranceiver_up(nl);
	netif_wake_queue( ndev );
	return 0;
}

static int
sg17_close(struct net_device  *ndev)
{
	struct net_local *nl  = (struct net_local *)netdev_priv(ndev);

	// disable receive/transmit functions
	iowrite8( XRST ,&(nl->regs->CRA));
	netif_tx_disable(ndev);
	
	// drop receive/transmit queries 
	PDEBUG(debug_xmit,"RX: head=%d,tail=%d\nTX: head=%d, tail=%d",
		   nl->rx.head,nl->rx.tail, nl->tx.head,nl->tx.tail );
	recv_free_buffs( ndev );
	xmit_free_buffs( ndev );

	return 0;
}


static struct net_device_stats *
sg17_get_stats(struct net_device *ndev)
{
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	return  &(nl)->stats;
}

static int
sg17_change_mtu(struct net_device *ndev, int new_mtu)
{
	unsigned long flags;
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);

	/* check ranges */
	if ((new_mtu < 68) || (new_mtu > 1536))
		return -EINVAL;

	spin_lock_irqsave(&nl->lock, flags);
	ndev->mtu = new_mtu;
	spin_unlock_irqrestore(&nl->lock, flags);
	return 0;
}

static void
sg17_set_mcast_list( struct net_device  *ndev )
{
	return;		// SG-17PCI always operate in promiscuos mode 
}

static int
sg17_ioctl(struct net_device *ndev, struct ifreq *ifr, int cmd)
{
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;

	switch (cmd) {
	case SIOCGLRATE:{
		int *rate = (int*)ifr->ifr_data;
		if( !netif_carrier_ok(ndev) && (cfg->mode == STU_R) ){
			*rate = -1;
		}else{
			*rate = nl->shdsl_cfg->rate;
			if( nl->regs->MXCR & MXEN ){
			    *rate -= (nl->regs->MXRATE+1)*64;
			}
		}
		return 0;
	}
	default:
		return -EOPNOTSUPP;
	}
	return -EOPNOTSUPP;
}


void
sg17_link_up(struct sg17_sci *s, int if_num)
{
	struct sg17_card *card = container_of(s,struct sg17_card,sci);
	struct net_device *ndev = card->ndevs[if_num];
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);

	// Link UP handlers
	advlink_hwstatus(&nl->alink,ADVLINK_UP);
	usermode_link_event(ndev,1);
	// Cnfigure hardware
	nl->regs->RATE = (nl->shdsl_cfg->rate/64)-1;
	PDEBUG(debug_link,"rate = %d, RATE=%d",nl->shdsl_cfg->rate,nl->regs->RATE);	
	iowrite8( 0xff, &nl->regs->SR );
	iowrite8( (ioread8( &nl->regs->CRB )&(~(RXDE|LED1|LED2)) ), &nl->regs->CRB );
	iowrite8( (UFL|CRC|OFL|RXS|TXS), &nl->regs->IMR );
}

void 
sg17_link_down(struct sg17_sci *s, int if_num)
{
	struct sg17_card *card = container_of(s,struct sg17_card,sci);
	struct net_device *ndev = card->ndevs[if_num];
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	u8 tmp = ioread8(&nl->regs->CRB) | RXDE;

	PDEBUG(debug_link,"");
	// Configure hardware
	tmp &= ~(LED1 | LED2); 
	iowrite8(tmp,&(nl->regs->CRB) );
	iowrite8( 0, &nl->regs->IMR );
	// Link DOWN handlers
	advlink_hwstatus(&nl->alink,ADVLINK_DOWN);
	usermode_link_event(ndev,0);
	PDEBUG(debug_link,"end");	
}

void
sg17_led_blink(struct sg17_sci *s, int if_num)
{
	struct sg17_card *card = container_of(s,struct sg17_card,sci);
	struct net_device *ndev = card->ndevs[if_num];
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	u8 tmp = ioread8(&nl->regs->CRB);

	PDEBUG(debug_link,"");
	tmp &= ~LED2;
	tmp |= LED1;
	iowrite8( tmp ,&(nl->regs->CRB) );
}

void
sg17_led_fblink(struct sg17_sci *s, int if_num)
{
	struct sg17_card *card = container_of(s,struct sg17_card,sci);
	struct net_device *ndev = card->ndevs[if_num];
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	u8 tmp = (ioread8(&nl->regs->CRB) & (~LED1)) | LED2;
	PDEBUG(debug_link,"");
	iowrite8(tmp,&(nl->regs->CRB) );
}

void
sg17_clock_setup(struct sg17_sci *s, int if_num)
{
	struct sg17_card *card = container_of(s,struct sg17_card,sci);
	struct net_device *ndev = card->ndevs[if_num];
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;

// EXTC=1 for Slave in ALL clock modes independent of link state
// REFE=1 for Master in "Plesiochronous with Timing Reference" clock mode
	if( cfg->mode == STU_R ){ // Slave
		iowrite8( ((ioread8( &nl->regs->CRB ) & ~REFE) | EXTC ), &nl->regs->CRB );			
	}else{ // Master
		iowrite8( (ioread8( &nl->regs->CRB ) & ~EXTC ), &nl->regs->CRB );
		if( cfg->clkmode == 2){
			iowrite8( (ioread8( &nl->regs->CRB ) | REFE ), &nl->regs->CRB );
		}else{	
			iowrite8( (ioread8( &nl->regs->CRB ) & ~REFE ), &nl->regs->CRB );
 		}	
	}
}

void
sg17_link_support(struct sg17_sci *s)
{
	struct sg17_card *card = container_of(s,struct sg17_card,sci);
	struct net_device *ndev;
	struct net_local *nl;	
	struct sk_buff *skb;
	u32 alink_msg[ADVLINK_MSIZE32];
	int lstate,i,k;
	
	PDEBUG(debug_link,"start");
	for( i=0;i<card->if_num;i++){
		ndev = card->ndevs[i];
		nl = (struct net_local *)netdev_priv(ndev);
		lstate = advlink_status(&nl->alink);
		switch( lstate ){
		case ADVLINK_UP:
		    if( !netif_carrier_ok(ndev) ){
    			netif_carrier_on(ndev);
                printk(KERN_NOTICE"%s: link is UP\n",ndev->name);
            }
		    break;
		case ADVLINK_DOWN:
		    if( netif_carrier_ok(ndev) ){
			    netif_carrier_off(ndev);
                printk(KERN_NOTICE"%s: link is DOWN\n",ndev->name);
            }
		    break;
		}
		if( advlink_enabled(&nl->alink) &&
		        advlink_get_hwstatus(&nl->alink) == ADVLINK_UP &&
		        (ndev->flags & IFF_UP) ){
    		advlink_send(&nl->alink,alink_msg);
			PDEBUG(debug_link,"Send checking message, scntr=%d, rcntr=%d",
			    alink_msg[1],alink_msg[2]);
			skb = dev_alloc_skb(ADVLINK_MSIZE8);
			if( !skb ){
			    printk(KERN_ERR"%s: sbk ENOMEM!",__FUNCTION__);
    			return;
	        }
		    skb_put( skb,ADVLINK_MSIZE8);
		    for(k=0;k<ADVLINK_MSIZE8;k++){
			    skb->data[k] = *((char*)alink_msg + k);
    		}
			if( sg17_start_xmit_link(skb,ndev) ){
			    advlink_send_error(&nl->alink);
    			dev_kfree_skb_any( skb );
	    	}
	    }
	}
}

#ifdef MR17H_UEVENTS
static int
usermode_link_event(struct net_device *ndev,int link_up)
{
	char *argv[4] = { USERMODE_HELPER, NULL, NULL,NULL };
	char *envp[3] = { NULL, NULL, NULL };
	char ifname[256];
	char lstate[256];
	sprintf(ifname,"%s",ndev->name);
	sprintf(lstate,"%d",link_up);
	argv[1] = ifname;
	argv[2] = lstate;
	envp[0] = "HOME=/";
	envp[1] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";
	//	printk("Start usermode helper\n");		
	return call_usermodehelper(argv[0],argv,envp,1);
}

#endif


/* --------------------------------------------------------------------------
 *   Functions, serving transmit-receive process   
 * -------------------------------------------------------------------------- */

// --------------------- DEBUG ---------------------------- //
struct sg17_statistic{
	int usec;
	int sec;
	int avg;
} xmit_start = {0,0,0},
	xmit_free = {0,0,0},
	recv_init = {0,0,0},
	recv_alloc = {0,0,0};

inline void
time_stamp1(struct timeval *tv){
	do_gettimeofday( tv );
}

inline void
time_stamp2(struct timeval *tv, struct sg17_statistic *s){
	struct timeval tv1;
	do_gettimeofday( &tv1 );
	if ( tv1.tv_usec - tv->tv_usec > 0 ){
	    s->usec += tv1.tv_usec - tv->tv_usec;
	    s->avg = ( (tv1.tv_usec - tv->tv_usec) + s->avg ) / 2;
	}
	else if( tv1.tv_sec - tv->tv_sec > 0 )
	    s->sec += tv1.tv_sec - tv->tv_sec;
}


int
check_skb_free( struct sk_buff *skb, struct net_device *ndev )
{
	struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);		
	int i;
	
	for( i=0; i< SW_RING_LEN;i++){
		if( skb == nl->tx.sw_ring[i] )
			return 1;
		if( skb == nl->rx.sw_ring[i] )
			return 2;
	}
	return 0;
}

static int
sg17_start_xmit( struct sk_buff *skb, struct net_device *ndev )
{
	struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);	
	unsigned pad;
	unsigned long flags;

	PDEBUG(debug_xmit,"start, skb->len=%d, sci:CRA=%02x, ndev:CRA=%02x",
		   skb->len,SCI->regs->CRA,nl->regs->CRA );
	if ( !netif_carrier_ok(ndev) ){
		dev_kfree_skb_any( skb );
		return 0;
	}
	
	if( skb->len < ETH_ZLEN ){
		pad = ETH_ZLEN - skb->len;
		skb = skb_pad(skb,pad);		
		if( !skb ){
			printk(KERN_NOTICE"%s: no mem for skb",__FUNCTION__);
			return 0;
		}
		skb->len = ETH_ZLEN;
	}else if( skb->len > ETHER_MAX_LEN ){
		PDEBUG(0,"too big packet!!!");	
		dev_kfree_skb_any( skb );		
		return 0;
	}

	spin_lock_irqsave(&nl->tx.lock,flags);
	if( sg_ring_add_skb(&nl->tx,skb) == -ERFULL ){
		PDEBUG(debug_xmit,"error: cannot add skb - full queue");
		spin_unlock_irqrestore(&nl->tx.lock,flags);
		netif_stop_queue( ndev );
		return 1; // don't free skb, just return 1;
	}
	nl->stats.tx_packets++;
	nl->stats.tx_bytes += skb->len;
	ndev->trans_start = jiffies;
	spin_unlock_irqrestore(&nl->tx.lock,flags);
	return  0;
}

static int
sg17_start_xmit_link( struct sk_buff *skb, struct net_device *ndev )
{
	struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);	
	unsigned long flags;

	PDEBUG(debug_link,"start, skb->len=%d, sci:CRA=%02x, ndev:CRA=%02x",
		   skb->len,SCI->regs->CRA,nl->regs->CRA );

	spin_lock_irqsave(&nl->tx.lock,flags);
	if( sg_ring_add_skb(&nl->tx,skb) == -ERFULL ){
		PDEBUG(debug_xmit,"error: cannot add skb - full queue");
		spin_unlock_irqrestore(&nl->tx.lock,flags);
		netif_stop_queue( ndev );
		return 1; // don't free skb, just return 1;
	}
	spin_unlock_irqrestore(&nl->tx.lock,flags);
	return  0;
}


// xmit_free_buffs may also be used to drop the queue - just turn
// the transmitter off, and set CTDR == LTDR
static void
xmit_free_buffs( struct net_device *ndev )
{
	struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);
	struct sk_buff *skb;
	int len;

	//	PDEBUG(debug_xmit,"start");	
	while( (skb=sg_ring_del_skb(&nl->tx,&len)) != NULL ){
		dev_kfree_skb_any( skb );
	}
	if( netif_queue_stopped( ndev )  &&  sg_ring_have_space(&nl->tx) ){
		//		PDEBUG(debug_xmit,"enable xmit queue");		
		netif_wake_queue( ndev );
	}
	//	PDEBUG(debug_xmit,"end");			
	
}

//---------------receive-----------------------------------------

static void
recv_init_frames( struct net_device *ndev )
{
	struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);		
	struct sk_buff  *skb;
	unsigned  len=0;

	PDEBUG(debug_recv,"start");		
	while( (skb = sg_ring_del_skb(&nl->rx,&len)) != NULL ) {
		if( advlink_enabled(&nl->alink) &&
		    (len == ADVLINK_MSIZE8) ){
			skb_put( skb, len );
			advlink_recv(&nl->alink,skb->data);
			dev_kfree_skb_any( skb );
			continue;
		}else if( len < ETH_ZLEN ){
			len = ETH_ZLEN;
		}
		// setup skb & give it to OS
		skb_put( skb, len );
		skb->protocol = eth_type_trans( skb, ndev );
		netif_rx( skb );
		++nl->stats.rx_packets;
		nl->stats.rx_bytes += len;
		PDEBUG(debug_recv,"len = %d",skb->len);
	}
	return;
}

static int
recv_alloc_buffs( struct net_device *ndev )
{
	struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);		
	struct sk_buff  *skb;

	//	PDEBUG(debug_recv,"start");		    
	while( sg_ring_have_space(&nl->rx) ){
		//		PDEBUG(debug_recv,"alloc new skb");		
		skb = dev_alloc_skb(ETHER_MAX_LEN + IP_ALIGN);
		if( !skb )
			return -ENOMEM;
		skb->dev = ndev;
		skb_reserve( skb, 2 );	// align ip on longword boundaries
		// get dma able address & save skb
		if( sg_ring_add_skb(&nl->rx,skb) ){
			//			PDEBUG(0,"dev_kfree_skb_any(%p)",skb);			
			dev_kfree_skb_any( skb );
			return -1;
		}
	}
	//	PDEBUG(debug_recv,"end");			
	return 0;
}

static void
recv_free_buffs( struct net_device *ndev)
{
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);		
	struct sk_buff  *skb;
	int len;
	PDEBUG(debug_recv,"start");		    
	while( (skb = sg_ring_del_skb(&nl->rx,&len)) != NULL ) {	
		dev_kfree_skb_any( skb );
	}
	PDEBUG(debug_recv,"end");			
	return;
}

static void
sg17_tx_timeout( struct net_device  *ndev )
{
	struct net_local  *nl  = (struct net_local *)netdev_priv(ndev);		
	u8 tmp;

	tmp=ioread8(&(nl->regs->IMR));
	iowrite8( 0,&(nl->regs->IMR));	    
	udelay(1);
	if( netif_carrier_ok(ndev) )
		iowrite8((ioread8(&nl->regs->CRA)|TXEN),&nl->regs->CRA);
	iowrite8( tmp,&(nl->regs->IMR));		
	PDEBUG(0,"%s: transmit timeout\n", ndev->name );
	if( ioread8( &(nl->regs->SR)) & TXS ){
		PDEBUG(0,"%s: interrupt posted but not delivered\n",
			   ndev->name );
	}
	xmit_free_buffs( ndev );
}

static void
sg17_tranceiver_down(struct net_local *nl)
{
	iowrite8( 0, &( nl->regs->CRA));    
	iowrite8( RXDE , &( nl->regs->CRB));
	iowrite8( 0, &( nl->regs->IMR));
	iowrite8( 0xff, &( nl->regs->SR));
}

static void
sg17_tranceiver_up( struct net_local *nl)
{
	u8 cfg_byte;

	cfg_byte = (XRST | RXEN | TXEN);
	if( nl->hdlc_cfg.crc16 )
		cfg_byte|=CMOD;
	if( nl->hdlc_cfg.fill_7e )
		cfg_byte|=FMOD;
	if( nl->hdlc_cfg.inv )
		cfg_byte|=PMOD;
	iowrite8(cfg_byte,&(nl->regs->CRA));

	cfg_byte=ioread8( &(nl->regs->CRB)) | RODD;
	if( nl->hdlc_cfg.rburst )
		cfg_byte|=RDBE;
	if( nl->hdlc_cfg.wburst )
		cfg_byte|=WTBE;
	iowrite8(cfg_byte,&(nl->regs->CRB));
}

 
/* --------------------------------------------------------------------------
 *      Card related functions
 * -------------------------------------------------------------------------- */
 
//#define DEBUG_SYSFS
#ifdef DEBUG_SYSFS

//-----------   DEBUG ------------------//

#define ADDIT_ATTR
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12)
#undef ADDIT_ATTR
#define ADDIT_ATTR struct device_attribute *attr,
#endif


static ssize_t
show_sci_regs( struct device *dev, ADDIT_ATTR char *buf )
{                                                                       
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( dev );
	struct sg17_sci *s = (struct sg17_sci *)&card->sci;

	return snprintf(buf,PAGE_SIZE,"CRA(%02x),CRB(%02x),SR(%02x),IMR(%02x)\n",
					s->regs->CRA,s->regs->CRB,s->regs->SR,s->regs->IMR);
}

static ssize_t
store_sci_regs( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( dev );
	struct sg17_sci *s = (struct sg17_sci *)&card->sci;
	u8 tmp;
	char *endp;
	PDEBUG(0,"buf[0]=%d",buf[0]);
	if( !size ) return 0;
	PDEBUG(0,"buf[0]=%d, %c",buf[0],buf[0]);
	if( buf[0] < '0' || buf[0] > '3' )
		return size;
	tmp=simple_strtoul( buf+2,&endp,16) & 0xff;
	*((u8*)s->regs + buf[0]) = tmp;
	return size;
}
static DEVICE_ATTR(regs,0644,show_sci_regs,store_sci_regs);


static ssize_t
show_trvr_stat( struct device *dev, ADDIT_ATTR char *buf )
{                                                                       
	return snprintf(buf,PAGE_SIZE,"xmit_start(sec.%u usec.%u avg.%u)\n"
					"xmit_free(sec.%u usec.%u avg.%u)\n"
					"recv_init(sec.%u usec.%u avg.%u)\n"
					"recv_alloc(sec.%u usec.%u avg.%u)\n",
					xmit_start.sec,xmit_start.usec,xmit_start.avg,
					xmit_free.sec,xmit_free.usec,xmit_free.avg,					
					recv_init.sec,recv_init.usec,recv_init.avg,
					recv_alloc.sec,recv_alloc.usec,recv_alloc.avg);
}
static ssize_t
store_trvr_stat( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	if( buf[0] == '1' ){
		memset(&xmit_start,0,sizeof(struct sg17_statistic));
		memset(&xmit_free,0,sizeof(struct sg17_statistic));
		memset(&recv_init,0,sizeof(struct sg17_statistic));
		memset(&recv_alloc,0,sizeof(struct sg17_statistic));
	}	
	return size;
}
static DEVICE_ATTR(trvr_stat,0644,show_trvr_stat,store_trvr_stat);

//-------------- Memory window debug -----------------------------//
static u32 win_start=0,win_count=0;
static ssize_t
show_winread( struct device *dev, ADDIT_ATTR char *buf )
{                               
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( dev );
	char *win = (char*)card->mem_base + win_start;
	int len = 0,i;

	for(i=0;i<win_count && (len < PAGE_SIZE-3);i++){
		len += sprintf(buf+len,"%02x ",(win[i]&0xff));
	}
	len += sprintf(buf+len,"\n");
	return len;
}

static ssize_t
store_winread( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	char *endp;
	if( !size ) return 0;
	win_start = simple_strtoul(buf,&endp,16);
	PDEBUG(40,"buf=%p, endp=%p,*endp=%c",buf,endp,*endp);	
	while( *endp == ' '){
		endp++;
	}
	win_count = simple_strtoul(endp,&endp,16);
	PDEBUG(40,"buf=%p, endp=%p",buf,endp);		
	PDEBUG(40,"Set start=%d,count=%d",win_start,win_count);
	if( !win_count )
		win_count = 1;
	if( (win_start + win_count) > SG17_OIMEM_SIZE ){
		if( win_start >= (SG17_OIMEM_SIZE-1) ){
			win_start = 0;
			win_count = 1;
		} else {
			win_count = 1;
		}
	}
	PDEBUG(40,"Set start=%d,count=%d",win_start,win_count);	
	return size;
}
static DEVICE_ATTR(winread,0644,show_winread,store_winread);

static u32 win_written = 0;
static ssize_t
show_winwrite( struct device *dev, ADDIT_ATTR char *buf )
{                               
	return sprintf(buf,"Byte %x is written",win_written);
}

static ssize_t
store_winwrite( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( dev );
	char *win = (char*)card->mem_base;
	int start, val;
	char *endp;
	if( !size ) return 0;
	start = simple_strtoul(buf,&endp,16);
	PDEBUG(40,"buf=%p, endp=%p,*endp=%c",buf,endp,*endp);	
	while( *endp == ' '){
		endp++;
	}
	val = simple_strtoul(endp,&endp,16);
	PDEBUG(40,"buf=%p, endp=%p",buf,endp);		
	PDEBUG(40,"Set start=%d,val=%d",start,val);
	if( start > SG17_OIMEM_SIZE ){
		start = 0;
	}
	win_written = start;
	win[start] = (val & 0xff );
	return size;
}
static DEVICE_ATTR(winwrite,0644,show_winwrite,store_winwrite);

//-------------- EOC debug -----------------------------//

static ssize_t
store_eocdbg( struct device *dev, ADDIT_ATTR const char *buf, size_t size )
{
	if( !size ) return 0;
	if( buf[0] == '1' ){
		debug_sci = 0;
		debug_eoc = 0;
	} else {
		debug_sci = 40;
		debug_eoc = 40;
	}	
	return size;
}
static DEVICE_ATTR(eocdbg,0600,NULL,store_eocdbg);

//-------------- Register handlers -------------------------------//
void
sg17_sci_sysfs_register(struct device *dev)
{
	device_create_file(dev,&dev_attr_regs);
	device_create_file(dev,&dev_attr_trvr_stat);
	device_create_file(dev,&dev_attr_winread);
	device_create_file(dev,&dev_attr_winwrite);
	device_create_file(dev,&dev_attr_eocdbg);
}

void
sg17_sci_sysfs_remove(struct device *dev){
	device_remove_file(dev,&dev_attr_regs);
	device_remove_file(dev,&dev_attr_trvr_stat);
	device_remove_file(dev,&dev_attr_winread);
	device_remove_file(dev,&dev_attr_winwrite);
	device_remove_file(dev,&dev_attr_eocdbg);	
}

#else
inline void
sg17_sci_sysfs_register(struct device *dev) { }

inline void
sg17_sci_sysfs_remove(struct device *dev){ }

#endif // DEBUG_SYSFS
 
 
 
static int
sg17_def_config(struct sg17_card *card)
{
	struct sdfe4 *hwdev = &(card->hwdev);
	struct sdfe4_if_cfg *cfg_ch0=&(hwdev->cfg[0]);
	struct sdfe4_if_cfg *cfg_ch3=&(hwdev->cfg[3]); 	

	// save module type
	sdfe4_chipset_type type= hwdev->type;
	memset(hwdev,0,sizeof(struct sdfe4));
	hwdev->type = type;
	hwdev->data = (void*)&card->sci;

	hwdev->msg_cntr = 0;		
	PDEBUG(debug_init,"hwdev->data = %08x",(u32)hwdev->data);

	//---- config SDFE channel 0 ------------------
    
    // If this is 2 channel device
   	hwdev->ch[0].enabled = 1;
    // ( STU_C | STU_R )
   	cfg_ch0->mode = STU_R;
    // ( REPEATER | TERMINATOR )
   	cfg_ch0->repeater = TERMINATOR;
    // ( STARTUP_FAREND | STARTUP_LOCAL )
	cfg_ch3->startup_initialization = STARTUP_FAREND;
	// ( GHS_TRNS_00 | GHS_TRNS_01 | GHS_TRNS_11 | GHS_TRNS_10 )
	cfg_ch3->transaction = GHS_TRNS_00;
    // ( ANNEX_A_B | ANNEX_A | ANNEX_B | ANNEX_G | ANNEX_F )
   	cfg_ch0->annex = ANNEX_A_B;
    ///  TC-PAM: TCPAM16  TCPAM32
   	cfg_ch0->tc_pam = TCPAM32;
    // rate (speed)
       cfg_ch0->rate = 5696;
   	// ( SDI_TDMCLK_TDMMSP | SDI_DSL3 )
    cfg_ch0->input_mode = SDI_TDMCLK_TDMSP;
    // ( Terminal=>16384 | Repeater=>12288 )
	cfg_ch0->frequency = 16384;
    // ( Terminal=>5696 | Repeater=>2048 )
	cfg_ch0->payload_bits = 5696;
    // ( SDI_NO_LOOP | SDI_REMOTE_LOOP )
	cfg_ch0->loop = SDI_NO_LOOP;
    // Multiplexing related
 
    if( card->if_num == 1 ){
        // We have only one channel, cancel def config
        return 0;
    }

				
	//---- config SDFE channel 3 -----------------
	hwdev->ch[3].enabled = 1;
	// (STU_C | STU_R)
	cfg_ch3->mode = STU_R;
	// ( REPEATER | TERMINATOR )
	cfg_ch3->repeater = TERMINATOR;
	// ( STARTUP_FAREND | STARTUP_LOCAL )
	cfg_ch3->startup_initialization = STARTUP_FAREND;
	// ( GHS_TRNS_00 | GHS_TRNS_01 | GHS_TRNS_11 | GHS_TRNS_10 )
	//	cfg_ch3->transaction = GHS_TRNS_10;
	cfg_ch3->transaction = GHS_TRNS_00;
	// ( ANNEX_A_B | ANNEX_A | ANNEX_B | ANNEX_G | ANNEX_F )
	cfg_ch3->annex = ANNEX_A_B;
	///  TC-PAM: TCPAM16  TCPAM32
	cfg_ch3->tc_pam = TCPAM32;
	// rate (speed)
	// ( SDI_TDMCLK_TDMMSP | SDI_DSL3 )
	cfg_ch3->input_mode = SDI_TDMCLK_TDMSP ;
	// ( Terminal=>16384 | Repeater=>12288 )
	cfg_ch3->frequency = 16384;
	// ( Terminal=>5696 | Repeater=>2048 )
	cfg_ch3->payload_bits = 5696;
	// ( SDI_NO_LOOP | SDI_REMOTE_LOOP )
	cfg_ch3->loop = SDI_NO_LOOP;
	// Multiplexing related
		
	return 0;
}

static int __devinit
sg17_init_card( struct sg17_card *card )
{
	unsigned long iomem_start = pci_resource_start( card->pdev, 1 );
	unsigned long iomem_end = pci_resource_end( card->pdev, 1 );
	struct sdfe4 *hwdev = &(card->hwdev);
	struct sg17_sci *sci = &card->sci;
	int error = 0;	
	
	// DEBUG //
	SCI =  &card->sci;
	// DEBUG //	

	PDEBUG(debug_init,"");
	// Determine version of device
	if( card->pdev->device == SG17_PCI_DEVICE ){
		hwdev->type = SDFE4v1;
	}else{
		hwdev->type = SDFE4v2;
	}

	// set card name
	sprintf(card->name,"sg17card%d",card->number);
	// IOmem
	PDEBUG(debug_init,"IOmem, size=%x, ideal=%x",(u32)(iomem_end-iomem_start),(u32)SG17_OIMEM_SIZE);	
	if( (iomem_end - iomem_start) != (SG17_OIMEM_SIZE - 1) )
		return -ENODEV;
	PDEBUG(debug_init,"strt request_mem_region ");
	if( !request_mem_region( iomem_start,SG17_OIMEM_SIZE, card->name ) )
		return  -ENODEV;
	PDEBUG(debug_init,"request_mem_region - ok");
	card->mem_base = (void *) ioremap( iomem_start, SG17_OIMEM_SIZE );	
	// determine if number & power feeding presence
//    prinkt(KERN_INFO,"SUBSYS_ID=%04x\n",card->pdev->subsystem_device);
    switch(card->pdev->subsystem_device){
    case SUBSYS_ID_1CH:
        card->if_num = 1;
        card->pwr_source = 0;
        break;
    case SUBSYS_ID_1CH_PWR:
        card->if_num = 1;
        card->pwr_source = 1;
        break;
    case SUBSYS_ID_2CH:
        card->if_num = 2;
        card->pwr_source = 0;
        break;
    case SUBSYS_ID_2CH_PWR:
        card->if_num = 2;
        card->pwr_source = 1;
        break;
    default:
        printk("%s: error hardware PCI Subsystem ID for module\n",MR17H_MODNAME);
        error = -1;
        goto err_release_mem;
    }

	// setup SCI
	sci->mem_base = card->mem_base + SG17_SCI_MEMOFFS;
	sci->irq = card->pdev->irq;
	PDEBUG(debug_init,"sg17_sci_init");	
	if ( sg17_sci_init( sci,card->name,hwdev) ){
		error = -ENODEV;
		goto err_release_mem;
	}
	sg17_sci_sysfs_register(&(card->pdev->dev));
	return 0;
 err_release_mem:
	release_mem_region( iomem_start, SG17_OIMEM_SIZE );	
	PDEBUG(debug_init,"err_release_mem");
	return error;
}

static int __devinit
sg17_enable_card( struct sg17_card *card )
{
	struct sdfe4 *hwdev = &(card->hwdev);
	struct sg17_sci *sci = &card->sci;
	struct firmware *fw;
	char fwname[256];
	int ret = 0;
	int i;

	// load firmware
	PDEBUG(debug_error,"request_firmware");
	switch( hwdev->type ){
	case SDFE4v1:
		strncpy(fwname,MR17H_DRVNAME".bin",256);
		break;
	case SDFE4v2:
		strncpy(fwname,MR17H_DRVNAME"_v2.bin",256);
		break;
	}
		
	PDEBUG(debug_error,"Request %s file,type=%d",fwname,hwdev->type);
	if( (ret = request_firmware((const struct firmware **)&fw,fwname,&(card->pdev->dev))) ){
		printk(KERN_NOTICE"firmware file not found\n");
		return -ENOENT;
	}

	PDEBUG(debug_init,"sg17_sci_enable");
	sg17_sci_enable(sci);
	sg17_def_config(card);
	mdelay(2);
	// Allocate EOC data structures
	PDEBUG(debug_eoc,"EOC: init sctructures\n");
	hwdev->ch[3].eoc = eoc_init();
	hwdev->ch[0].eoc = eoc_init();
	i=0;
	PDEBUG(debug_error,"2 type=%d",hwdev->type);
	while( (ret = sdfe4_download_fw(hwdev,fw->data,fw->size)) && i<3 ){
		printk(KERN_NOTICE"Error loading fw. Try once again (#%d)\n",i+1);
		sg17_sci_disable(sci);
		udelay(100);
		sg17_sci_enable(sci);
		mdelay(2);
		i++;
	}
	release_firmware(fw);

	// Check for successfull firmware load	
	if( i ==3 ){
		printk(KERN_NOTICE MR17H_MODNAME ": Cannot download firmware for PCI device=%02x:%02x.%d\n",
				card->pdev->bus->number,PCI_SLOT(card->pdev->devfn),PCI_FUNC(card->pdev->devfn));
		sg17_sci_disable(sci);
		return -ENODEV;	
	}
	PDEBUG(debug_init,"success");		
	return 0;
}


static void __devexit
sg17_disable_card( struct sg17_card *card )
{
	struct sdfe4 *hwdev = &(card->hwdev);
	PDEBUG(debug_init,"");
	sg17_sci_disable(&card->sci);
	// Free EOC data structures
	PDEBUG(debug_eoc,"EIC: remove sctructures\n");	
	eoc_free(hwdev->ch[3].eoc);
	eoc_free(hwdev->ch[0].eoc);
	PDEBUG(debug_init,"success");		
}


static void __devexit
sg17_remove_card( struct sg17_card *card )
{
	unsigned long iomem_start = pci_resource_start( card->pdev, 1 );
	PDEBUG(debug_init,"");
	sg17_sci_sysfs_remove(&(card->pdev->dev));
	sg17_sci_remove( &card->sci );
	release_mem_region( iomem_start, SG17_OIMEM_SIZE );	
	PDEBUG(debug_init,"success");			
}


/*
 * SG-17PCI PCI device structure & functions
 */
int card_number = 0; 
 
static struct pci_device_id  sg17_pci_tbl[] __devinitdata = {
{ PCI_DEVICE(SG17_PCI_VENDOR,SG17_PCI_DEVICE) },
{ PCI_DEVICE(SG17v2_PCI_VENDOR,SG17v2_PCI_DEVICE) },
{ 0 }
};
MODULE_DEVICE_TABLE( pci, sg17_pci_tbl );

static struct pci_driver  sg17_driver = {
 name:           MR17H_DRVNAME,
 probe:          sg17_probe_one,
 remove:         sg17_remove_one,
 id_table:       sg17_pci_tbl
};
				
static int __devinit
sg17_probe_one(struct pci_dev *pdev, const struct pci_device_id *dev_id)
{
	struct device *dev_dev=(struct device*)&(pdev->dev);
	struct device_driver *dev_drv=(struct device_driver*)(dev_dev->driver);
	struct net_device *ndev = NULL;
	struct sg17_card *card;
	struct net_local *nl;
	int if_processed,i,ch_num;
	int ret;
	
	
	PDEBUG(debug_init,"New device");
	// Setup PCI card configuration
	if( pci_enable_device( pdev ) )
		return  -EIO;
	pci_set_master( pdev );
	// Save PCI card info
	card = kmalloc( sizeof(struct sg17_card), GFP_KERNEL );
	memset((void*)card,0,sizeof(struct sg17_card));
	pci_set_drvdata(pdev, card);
	card->number = card_number++;
	card->pdev = pdev;
	
	// setup SCI HDLC controller
	PDEBUG(debug_init,"sg17_init_card");
	if( (ret = sg17_init_card(card)) ){
		PDEBUG(debug_error,"error registering SG-17PCI card");
		return -ENODEV;
	}

	// setup network interfaces
	PDEBUG(debug_netcard,"network ifs init");
	for(if_processed=0; if_processed < card->if_num; if_processed++){
		// allocate network device 
		if( !( ndev = alloc_netdev( sizeof(struct net_local),"dsl%d",sg17_dsl_init)) ){
			printk(KERN_NOTICE"error while alloc_netdev #%d\n",if_processed);
			goto exit_unreg_ifs;
		}
		PDEBUG(debug_netcard,"alloc_netdev - %s",ndev->name);
		// set some net device fields
		ndev->mem_start = (unsigned long)((u8*)card->mem_base +
										  SG17_HDLC_CH0_MEMOFFS + if_processed*SG17_HDLC_MEMSIZE);
		ndev->mem_end = (unsigned long)((u8*)ndev->mem_start + SG17_HDLC_MEMSIZE);
		ndev->irq = pdev->irq;
		// device private data initialisation
		nl=(struct net_local *)netdev_priv(ndev);
		memset( nl, 0, sizeof(struct net_local) );
		nl->dev=&(pdev->dev);
		nl->number = if_processed;
		if( (ch_num = sg17_sci_if2ch(&card->sci,if_processed)) < 0 ){
			PDEBUG(debug_error,"error(%d) in sg17_sci_if2ch",ch_num);
			free_netdev( ndev );			
			goto exit_unreg_ifs;			
		}
		nl->shdsl_cfg = &(card->hwdev.cfg[ch_num]);
		// network interface registration
		if( (ret = register_netdev(ndev)) ) {
			printk(KERN_NOTICE"sg17lan: error(%d) while register device %s\n",ret,ndev->name);
			free_netdev( ndev );
			goto exit_unreg_ifs;
		}
		PDEBUG(debug_netcard,"success");
		card->ndevs[if_processed] = ndev;

		PDEBUG(debug_netcard,"sg17_sysfs_register");
		if( sg17_sysfs_register( ndev ) ){
			printk( KERN_ERR "%s: unable to create sysfs entires\n",ndev->name);
			goto exit_unreg_ifs;
		}
		// Create symlink to device in /sys/bus/pci/drivers/mr17h/
		sysfs_create_link( &(dev_drv->kobj),&(dev_dev->kobj),ndev->name );
		PDEBUG(debug_netcard,"sg17_sysfs_register - success");
	}

	PDEBUG(debug_init,"sg17_enable_card");
	if( (ret = sg17_enable_card(card)) ){
		PDEBUG(debug_error,"error(%d) registering SG-17PCI card",ret);
		goto exit_unreg_ifs;
	}

	return 0;

 exit_unreg_ifs:
	PDEBUG(debug_error,"Error, if_pocessed = %d",if_processed);
	for(i=0;i<if_processed;i++){
		sysfs_remove_link(&(dev_drv->kobj),ndev->name);	
		unregister_netdev(card->ndevs[i]);
		free_netdev(card->ndevs[i]);
	}
	sg17_remove_card(card);	
	PDEBUG(debug_error,"kfree CARD");
	kfree( card );
	pci_disable_device( pdev );
	pci_set_drvdata(pdev, NULL);
	return -ENODEV;
}

static void __devexit
sg17_remove_one(struct pci_dev *pdev)
{
	struct device *dev_dev=(struct device*)&(pdev->dev);
	struct device_driver *dev_drv=(struct device_driver*)(dev_dev->driver);
	struct sg17_card  *card = pci_get_drvdata( pdev );
	int i;

	PDEBUG(debug_netcard,"Card = %08x",(u32)card);
	PDEBUG(debug_init,"goodbye!");
	if( card ){
		for(i=0;i<card->if_num;i++){
			PDEBUG(debug_init,"unreg %s",card->ndevs[i]->name);

			// Remove symlink on device from driver /sys/bus/pci/drivers/mr17h dir in sysfs 
			sysfs_remove_link(&(dev_drv->kobj),card->ndevs[i]->name);
			// Remove device options driver /sys/class/net dir in sysfs 
			sg17_sysfs_remove(card->ndevs[i]);
			unregister_netdev(card->ndevs[i]);
			free_netdev(card->ndevs[i]);
			PDEBUG(debug_init,"unreg %s: OK",card->ndevs[i]->name);			
		}
		sg17_disable_card( card );		
		sg17_remove_card( card );
		kfree( card );
	}
	pci_disable_device(pdev);
	pci_set_drvdata(pdev, NULL);
}



/* --------------------------------------------------------------------------
 *      Module initialisation/cleanup
 * -------------------------------------------------------------------------- */

int __devinit
sg17_init( void ){
#ifdef DEBUG_ON
//	debug_error = error_output;
#endif
	int i = pci_register_driver( &sg17_driver );
	printk(KERN_NOTICE"Load "MR17H_MODNAME" driver\n");	
	PDEBUG(10,"return = %d",i);
	return 0;
}

void __devexit
sg17_exit( void ){
	printk(KERN_NOTICE"Unload "MR17H_MODNAME" driver\n");
	pci_unregister_driver( &sg17_driver );
}

module_init(sg17_init);
module_exit(sg17_exit);


