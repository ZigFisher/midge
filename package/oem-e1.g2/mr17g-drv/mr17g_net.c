/* mr17g_net.c
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

#include "mr17g.h"
#include "mr17g_net.h"
#include "sg17ring.h"
#include "sg17ring_funcs.h"
#include "pef22554.h"
#include "mr17g_sysfs.h"

// Debug settings
#define DEBUG_ON
#define DEFAULT_LEV 10
#include "mr17g_debug.h"

static int __init mr17g_probe( struct net_device  *ndev );
static void __exit mr17g_uninit(struct net_device *ndev);
static irqreturn_t mr17g_interrupt( int  irq,  void  *dev_id,  struct pt_regs  *regs );
static int mr17g_open( struct net_device  *ndev );
static int mr17g_close(struct net_device  *ndev);
static struct net_device_stats *mr17g_get_stats(struct net_device *ndev);
static void mr17g_tx_timeout( struct net_device  *ndev );
static int mr17g_attach(struct net_device *dev, unsigned short encoding, unsigned short parity);
static int mr17g_ioctl(struct net_device *ndev, struct ifreq *ifr, int cmd);
static int mr17g_ioctl_get_iface(struct net_device *ndev,struct ifreq *ifr);
static u32 mr17g_get_rate(struct net_device *ndev);
static u32 mr17g_get_slotmap(struct net_device *ndev);
static u32 mr17g_get_clock(struct net_device *ndev);
static void mr17g_transceiver_shutdown(struct mr17g_channel *ch);
static void xmit_free_buffs( struct net_device *ndev );
static void xmit_drop_buffs(struct net_device *ndev);
static void recv_init_frames( struct net_device *ndev );
static int recv_alloc_buffs( struct net_device *ndev );
static void recv_free_buffs( struct net_device *ndev);


static int
slot_cnt(u32 smap)
{
    int cnt = 0;
    while(smap){
		if( smap & 0x1 )
			cnt++;
		smap = (smap>>1) & 0x7fffffff;
    }
    return cnt;
}


// Register PEF22554 chipset channels as HDLC interfaces in OS
int __devinit
mr17g_net_init(struct mr17g_chip *chip)
{
	struct device *dev = (struct device*)&(chip->pdev->dev);
	struct device_driver *drv = (struct device_driver*)(dev->driver);
    struct mr17g_channel *ch;
    struct net_device *ndev;
    int i,j,err;
   	void *base;

	PDEBUG(debug_net,"start");
    
    for(i=0;i<chip->if_quan;i++){
		PDEBUG(debug_net,"Chip#%d",i);
        if( !(ch = (struct mr17g_channel*)kmalloc(sizeof(struct mr17g_channel),GFP_KERNEL)) ){ 
	    	err = -ENOMEM;
		    goto error;
	    }
        // Setup iface private data
    	memset(ch,0,sizeof(struct mr17g_channel));
		ch->chip = chip;
        ch->num = i;
        if( !(ndev=alloc_hdlcdev(ch)) ){
	    	err = -ENOMEM;
		    goto error;
	    }
PDEBUG(debug_net,"Mark1");
    	ndev->init = mr17g_probe;
    	ndev->uninit = mr17g_uninit;
    	ndev->irq = chip->pdev->irq;
        // Initialize iomemory
        switch(chip->type){
        case MR17G_STANDARD:
        	PDEBUG(debug_net,"Register standard netdev");
        	base = chip->iomem + ch->num*MR17G_CHAN1_SIZE;
        	ch->iomem.regs = (struct mr17g_hw_regs*)((u8*)base + CHAN1_REGS_OFFS);
        	ch->iomem.tx_buf = (struct sg_hw_descr*)((u8*)base + CHAN1_HDLCTX_OFFS);
        	ch->iomem.rx_buf = (struct sg_hw_descr*)((u8*)base + CHAN1_HDLCRX_OFFS);
	    	ndev->mem_start = chip->iomem_start + ch->num*MR17G_CHAN1_SIZE;
	    	ndev->mem_end = ndev->mem_start + MR17G_CHAN1_SIZE;
			break;
		case MR17G_MUXONLY:
        	PDEBUG(debug_net,"Register muxonly netdev");
        	base = chip->iomem + ch->num*MR17G_CHAN2_SIZE;
        	ch->iomem.regs = (struct mr17g_hw_regs*)((u8*)base + CHAN2_REGS_OFFS);
        	ch->iomem.tx_buf = NULL;
        	ch->iomem.rx_buf = NULL;
	    	ndev->mem_start = chip->iomem_start + ch->num*MR17G_CHAN2_SIZE;
	    	ndev->mem_end = ndev->mem_start + MR17G_CHAN2_SIZE;
        	break; 
        }	
PDEBUG(debug_net,"Mark2");
		
        // register net device
    	if( (err = register_hdlc_device(ndev)) < 0){
            printk(KERN_ERR"%s: cannot register hdlc interface, err=%d\n",MR17G_MODNAME,err);
	    	goto ndevfree;
	    }

PDEBUG(0,"\t!%s TX=%x,RX=%x,REGs=%x",ndev->name,
		(u32)ch->iomem.tx_buf - (u32)chip->iomem,
		(u32)ch->iomem.rx_buf - (u32)chip->iomem,
		(u32)ch->iomem.regs - (u32)chip->iomem);
		
PDEBUG(debug_net,"Mark3");
    	// Init control through sysfs
    	if( (err = sysfs_create_link( &(drv->kobj),&(dev->kobj),ndev->name )) ){
	    	printk(KERN_NOTICE"%s: error in sysfs_create_link\n",__FUNCTION__);	
		    goto ndevunreg;
    	}
PDEBUG(debug_net,"Mark4");
        if( mr17g_netsysfs_register(ndev) ){
            printk(KERN_ERR"%s: cannot register parameters in sysfs for %s\n",MR17G_MODNAME,ndev->name);
	    	goto rmlink;
        }
PDEBUG(debug_net,"Mark5");
        chip->ifs[i] = ndev;
    }

	PDEBUG(debug_net,"end");
	return 0;

rmlink:
	sysfs_remove_link(&(drv->kobj),ndev->name);
ndevunreg:
    unregister_netdev(ndev);
ndevfree:
	free_netdev(ndev);	
error:	
    // Free memory for current iface local data
	kfree(ch);
    // Free all previous ifaces
    for(j=0;j<i;j++){
        mr17g_netsysfs_remove(chip->ifs[j]);
       	unregister_netdev( chip->ifs[j] );
    	free_netdev(chip->ifs[j]);	
    }
	PDEBUG(debug_error,"(!)fail");
	return err;
}

int __devexit
mr17g_net_uninit(struct mr17g_chip *chip)
{
	struct device *dev = (struct device*)&(chip->pdev->dev);
	struct device_driver *drv = (struct device_driver*)(dev->driver);
    int i;

	PDEBUG(debug_net,"start, chip = %p",chip);
    for(i=0;i<chip->if_quan;i++){
        PDEBUG(debug_net,"Unregister %s",chip->ifs[i]->name);
        mr17g_netsysfs_remove(chip->ifs[i]);
    	sysfs_remove_link(&(drv->kobj),chip->ifs[i]->name);
       	unregister_netdev( chip->ifs[i] );
        PDEBUG(debug_net,"Free %s",chip->ifs[i]->name);
    	free_netdev(chip->ifs[i]);	
    }
	PDEBUG(debug_net,"end");
	return 0;
}


    
// Common interface initialisation
static int __init
mr17g_probe( struct net_device  *ndev )
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
	int err=-ENODEV;

	PDEBUG(debug_net,"start");
	// Carrier off
	netif_carrier_off( ndev );
	netif_stop_queue(ndev);

	// generate 'unique' MAC address
	*(u16 *)ndev->dev_addr = htons( 0x00ff );
	*(u32 *)(ndev->dev_addr + 2) = htonl( 0x01a39000 | ((u32)ndev->priv & 0x00000fff) );

	// Init net device handler functions 
	ndev->open = mr17g_open;
	ndev->stop = mr17g_close;
	ndev->get_stats = mr17g_get_stats;
	ndev->do_ioctl	= &mr17g_ioctl;
	ndev->tx_timeout = mr17g_tx_timeout;
	ndev->do_ioctl = mr17g_ioctl;
	ndev->watchdog_timeo = TX_TIMEOUT;
	ndev->tx_queue_len  = 50;

	// Set hdlc device fields	
	hdlc->attach = &mr17g_attach;
	hdlc->xmit   = &mr17g_start_xmit;

PDEBUG(debug_net,"Mark1");

	// No interrupts for multiplexing only devices
    if( ch->chip->type != MR17G_MUXONLY ){
		// shut down transceiver 
		mr17g_transceiver_shutdown(ch);
		// setup transmit and receive rings
		ch->tx.hw_ring = (struct sg_hw_descr *)ch->iomem.tx_buf;
		ch->rx.hw_ring = (struct sg_hw_descr *)ch->iomem.rx_buf;
		ch->tx.hw_mask=ch->rx.hw_mask=HW_RING_MASK;
		ch->tx.sw_mask=ch->rx.sw_mask=SW_RING_MASK;
		ch->tx.CxDR=(u8*)&(ch->iomem.regs->CTDR);
		ch->rx.CxDR=(u8*)&(ch->iomem.regs->CRDR);
		ch->tx.LxDR=(u8*)&(ch->iomem.regs->LTDR);
		ch->rx.LxDR=(u8*)&(ch->iomem.regs->LRDR);
		ch->tx.type=TX_RING;
		ch->rx.type=RX_RING;
		ch->tx.dev = ch->rx.dev = &ch->chip->pdev->dev;

	    // Init locking
		spin_lock_init( &ch->tx.lock );
		spin_lock_init( &ch->rx.lock );
	
		// net device interrupt register
		if( (err = request_irq(ndev->irq, mr17g_interrupt, SA_SHIRQ, ndev->name, ndev)) ){
			printk( KERN_ERR "%s(%s): unable to get IRQ %d, error= %08x\n",MR17G_MODNAME,
				ndev->name, ndev->irq, err );
			goto error;
		}
    }

PDEBUG(debug_net,"Mark1");

	//default HDLC X config
	ch->cfg.rburst = ch->cfg.wburst = 0;	
    // Default setup of channel
    pef22554_defcfg(ch);

PDEBUG(debug_net,"Mark2");

    if( pef22554_basic_channel(ch) ){
		printk( KERN_ERR "%s(%s): error while basic setup\n",MR17G_MODNAME,ndev->name);
        err = -1;
		goto irqfree;
	}
PDEBUG(debug_net,"Mark3");

    if( pef22554_channel(ch) ){
		printk( KERN_ERR "%s(%s): error while default setup\n",MR17G_MODNAME,ndev->name);
        err = -1;
		goto irqfree;
	}
PDEBUG(debug_net,"Mark4");
    if( ch->chip->type != MR17G_MUXONLY ){
	    mr17g_transceiver_setup(ch);
    }        
	printk( KERN_NOTICE "%s: " MR17G_MODNAME " E1 module (irq %d, mem %#lx)\n",
			ndev->name, ndev->irq, ndev->mem_start );

PDEBUG(debug_net,"Mark5");

	SET_MODULE_OWNER( ndev );
	return  0;
irqfree:
    if( ch->chip->type != MR17G_MUXONLY ){    
		free_irq(ndev->irq, ndev);
    }
error:
    return err;
}

static void __devexit
mr17g_uninit(struct net_device *ndev)
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    
    PDEBUG(debug_net,"iface = %s",ndev->name);
    if( ch->chip->type != MR17G_MUXONLY ){    
		free_irq( ndev->irq, ndev );
		mr17g_transceiver_shutdown(ch);
    }
    kfree(ch);
}

static irqreturn_t
mr17g_interrupt( int  irq,  void  *dev_id,  struct pt_regs  *pregs )
{
	struct net_device *ndev = (struct net_device *) dev_id;
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs; // = ch->iomem.regs;
	u8 status;
	u8 mask; // = ioread8(&(regs->IMR));

	//PDEBUG(debug_irq,"start");
	regs = ch->iomem.regs;
	mask = ioread8(&(regs->IMR));


	// No interrupts for multiplexing only devices
    if( ch->chip->type == MR17G_MUXONLY )
		return IRQ_NONE;
	
	if( (ioread8(&(regs->SR)) & mask ) == 0 )
		return IRQ_NONE;

	status = ioread8(&(regs->SR));
	iowrite8(status,&(regs->SR));	
	iowrite8( 0, &(regs->IMR));
	PDEBUG(debug_irq,"%s: status = %02x, mask = %02x",ndev->name,status,mask);

	if( status & RXS ){
		PDEBUG(debug_irq,"%s: RXS",ndev->name);	
		recv_init_frames( ndev );
		recv_alloc_buffs( ndev );
	}
	if( status & TXS ){
		PDEBUG(debug_irq,"%s: TXS",ndev->name);
		xmit_free_buffs( ndev );
	}
	if( status & CRC ){
	    PDEBUG(debug_irq,"%s: CRC",ndev->name);	
	    ++ch->stats.rx_errors;
		++ch->stats.rx_crc_errors;
	}
	if( status & OFL ){
	    PDEBUG(debug_irq,"%s: OFL",ndev->name);
	    ++ch->stats.rx_errors;
	    ++ch->stats.rx_over_errors;
	}
	if( status & UFL ){
		//  Whether transmit error is occured, we have to re-enable the
		//  transmitter. That's enough, because linux doesn't fragment
	    //  packets.
	    PDEBUG(debug_irq,"%s: UFL",ndev->name);
	    iowrite8(ioread8(&(regs->CRA))|TXEN,&(regs->CRA) );
	    ++ch->stats.tx_errors;
	    ++ch->stats.tx_fifo_errors;
	}

	iowrite8( mask,&(regs->IMR));	
	
	return IRQ_HANDLED;
}


static int
mr17g_open( struct net_device  *ndev )
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
    
    if( ch->chip->type == MR17G_MUXONLY ){
		return -EBUSY;
	}
    
	// init descripts, allocate receiving buffers 
	ch->tx.head = ch->tx.tail = ch->rx.head = ch->rx.tail = 0;
	ch->tx.FxDR = ch->rx.FxDR = 0;
	iowrite8( 0, (ch->tx.CxDR));
	iowrite8( 0, (ch->tx.LxDR));	
	iowrite8( 0, (ch->rx.CxDR));	
	iowrite8( 0, (ch->rx.LxDR));	
	recv_alloc_buffs( ndev );
	// enable receive/transmit functions 
	iowrite8(ioread8(&regs->CRA)|RXEN|TXEN,&regs->CRA);
	netif_wake_queue( ndev );

	return 0;
}

static int
mr17g_close(struct net_device  *ndev)
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;

	if( ch->chip->type == MR17G_MUXONLY ){
		return 0;
	}
	// disable receive/transmit functions
	iowrite8(ioread8(&regs->CRA)&(~(RXEN|TXEN)),&regs->CRA);
	netif_tx_disable(ndev);
	
	// drop receive/transmit queries 
	PDEBUG(debug_xmit,"RX: head=%d,tail=%d\nTX: head=%d, tail=%d",
		   ch->rx.head,ch->rx.tail, ch->tx.head,ch->tx.tail );
	recv_free_buffs( ndev );
	xmit_drop_buffs( ndev );

	return 0;
}

static struct net_device_stats *
mr17g_get_stats(struct net_device *ndev){
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	return  &((struct mr17g_channel*)hdlc->priv)->stats;
}

static void
mr17g_tx_timeout( struct net_device  *ndev )
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
    
    printk(KERN_ERR"%s: transmit timeout\n", ndev->name );
    // Enable transmitter 
	if( netif_carrier_ok(ndev) )
		iowrite8((ioread8(&regs->CRA)|TXEN),&regs->CRA);
    xmit_drop_buffs( ndev );
}

static int
mr17g_attach(struct net_device *dev, unsigned short encoding, unsigned short parity)
{
	PDEBUG(debug_net,"");
	return 0;
}

static int
mr17g_ioctl(struct net_device *ndev, struct ifreq *ifr, int cmd)
{

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	switch (cmd) {
	case SIOCWANDEV:
		switch (ifr->ifr_settings.type) {
		case IF_GET_IFACE:
			return mr17g_ioctl_get_iface(ndev,ifr);		

		case IF_IFACE_E1:
			return 0;
			//			return mr16g_ioctl_set_iface(ndev,ifr);					

		case IF_GET_PROTO:
		default:
			return hdlc_ioctl(ndev, ifr, cmd);
		}
	case SIOCGLRATE:{
		int *rate = (int*)ifr->ifr_data;
		*rate = mr17g_get_rate(ndev)/1000;
		return 0;
	}

	default:
		/* Not one of ours. Pass through to HDLC package */
		return hdlc_ioctl(ndev, ifr, cmd);
	}
}

static int
mr17g_ioctl_get_iface(struct net_device *ndev,struct ifreq *ifr)
{
	te1_settings sets;
	
	// Setup interface type. For us - only E1
	ifr->ifr_settings.type = IF_IFACE_E1;
	
	if (ifr->ifr_settings.size == 0){
		return 0;       /* only type requested */
	}
	if (ifr->ifr_settings.size < sizeof (sets)){
		return -ENOMEM;
	}

	sets.clock_rate = mr17g_get_rate(ndev);
	sets.clock_type = mr17g_get_clock(ndev);
	sets.loopback = 0;
	sets.slot_map = mr17g_get_slotmap(ndev);
						    
	if (copy_to_user(ifr->ifr_settings.ifs_ifsu.sync, &sets, sizeof (sets)))
		return -EFAULT;
										    
	ifr->ifr_settings.size = sizeof (sets);
	return 0;
}

static u32
mr17g_get_rate(struct net_device *ndev)
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
	struct mr17g_chan_config *cfg= &ch->cfg;
	u32 rate=0, storage=cfg->slotmap;
	
	// if framed mode enabled
    if( !cfg->framed ){
		if( regs->MXCR & MXEN )
			return 0;
		else
	  		return 64000*32;
	}

    // in unframed mode bit0 allways unmapped
    storage &= ~1;
    // check if bit16 is mapped
	if( !cfg->ts16 ){
	    storage &= ~(1<<16);
	}
	    
    // Count rate
	while(storage){
		if( storage & 0x1 )
			rate+=64000;
		storage= (storage>>1)&0x7fffffff;
	}
	return rate;
}

static u32
mr17g_get_slotmap(struct net_device *ndev)
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
	struct mr17g_chan_config *cfg= &ch->cfg;
	u32 storage=cfg->slotmap;

    // form slotmap for framed mode
	if( cfg->framed ){
        storage &= ~1;
		if( !cfg->ts16 ){
	    		storage &= ~(1<<16);
		}
		return storage;
	}

	// in unframed mode all slots mapped to mx
	if( regs->MXCR & MXEN ) 
		return 0;

	return 0xffffffff;
}

static u32
mr17g_get_clock(struct net_device *ndev)
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
	struct mr17g_chan_config *cfg= &ch->cfg;
	
	if( cfg->ext_clck )
		return CLOCK_EXT;
	else
		return CLOCK_INT;
}

//----------------- Transmitter control ----------------------//
static void
mr17g_transceiver_shutdown(struct mr17g_channel *ch)
{
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;

    PDEBUG(debug_net,"TRVR regs = %p",(void*)regs);
    
	iowrite8( 0, &(regs->CRA));    
	iowrite8( RXDE , &(regs->CRB));
	iowrite8( 0, &(regs->IMR));
	iowrite8( 0xff, &(regs->SR));
	
	// Mux shutdown
	iowrite8(0,&regs->MAP0);
	iowrite8(0,&regs->MAP1);	
	iowrite8(0,&regs->MAP2);	
	iowrite8(0,&regs->MAP3);	
	iowrite8(0,&regs->MXMAP0);
	iowrite8(0,&regs->MXMAP1);	
	iowrite8(0,&regs->MXMAP2);	
	iowrite8(0,&regs->MXMAP3);	

	iowrite8(0,&regs->TFS);
	iowrite8(0,&regs->RFS);
	iowrite8(0,&regs->TLINE);
	iowrite8(0,&regs->RLINE);
	iowrite8(0,&regs->MXCR);
}

void
mr17g_transceiver_setup(struct mr17g_channel *ch)
{
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
    struct mr17g_chan_config *cfg = &ch->cfg;
	u8 cfg_byte;
	u32 tmpmap;
	u8 tmp,*smap;

    // 1. Setup CRA register
	cfg_byte = ioread8(&regs->CRA) | XRST0;
    // Setup CRC16/32 check
	if( cfg->crc16 )
		cfg_byte |= CMOD;
    else
        cfg_byte &= ~CMOD;
    // Setup fill byte value
	if( cfg->fill_7e )
		cfg_byte |= FMOD;
    else
        cfg_byte &= ~FMOD;
    // Setup inversion
	if( cfg->inv )
		cfg_byte |= PMOD;
    else
        cfg_byte &= ~PMOD;
	iowrite8(cfg_byte,&(regs->CRA));

    // 2. Setup CRB register
	cfg_byte= ioread8(&regs->CRB) | RODD;
    // PCI Read burst
	if( cfg->rburst )
		cfg_byte |= RDBE;
    else
		cfg_byte &= (~RDBE);
    // PCI write burst
	if( cfg->wburst )
		cfg_byte |= WTBE;
    else
		cfg_byte &= (~WTBE);
    // Framed mode
    if( cfg->framed )
        cfg_byte |= FRM;
    else
        cfg_byte &= (~FRM);
    // External clock
    if( cfg->ext_clck )
        cfg_byte |= EXTC;
    else
        cfg_byte &= (~EXTC);
	iowrite8(cfg_byte,&(regs->CRB));

	// 3. Set slotmap	
	tmpmap = cfg->slotmap;
	if( !cfg->framed ){
		tmpmap = 0;
	}else{
	    tmpmap &= ~(1);
		if( !cfg->ts16 )
		    tmpmap &= ~(1<<16);
	}
	smap=(u8*)&tmpmap;
	iowrite8(smap[0],&regs->MAP0);
	iowrite8(smap[1],&regs->MAP1);	
	iowrite8(smap[2],&regs->MAP2);	
	iowrite8(smap[3],&regs->MAP3);	

	// 4. Set mx slotmap	
	tmpmap = cfg->mxslotmap;
	if( cfg->framed ){
	    tmpmap &= ~(1);
		if( !cfg->ts16 )
	  		tmpmap &= ~(1<<16);
	}else{
		tmpmap = 0xffffffff;
	}
	smap=(u8*)&tmpmap;
	iowrite8(smap[0],&regs->MXMAP0);
	iowrite8(smap[1],&regs->MXMAP1);	
	iowrite8(smap[2],&regs->MXMAP2);	
	iowrite8(smap[3],&regs->MXMAP3);	
	tmp = slot_cnt(tmpmap);
	tmp--;
	iowrite8(tmp,&regs->MXRATE);	
}

//--------------------------- Link state -------------------------------------------//
void mr17g_net_link(struct net_device *ndev)
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
    int lstat = pef22554_linkstate(ch->chip,ch->num,ch->cfg.framed);
	struct mr17g_chan_config *cfg = &ch->cfg;

    PDEBUG(debug_link,"%s: lstate = %d, car_ok=%d",ndev->name,lstat,netif_carrier_ok(ndev));
    if(  lstat > 0 || cfg->llpb ){
	    if( !netif_carrier_ok(ndev) ){
            printk(KERN_NOTICE"%s: link UP\n",ndev->name);
       		netif_carrier_on(ndev);
		    if( ch->chip->type != MR17G_MUXONLY ){
		    	// Setup HDLC controller
	        	iowrite8( 0xff, &regs->SR );
    	    	iowrite8(ioread8(&regs->CRB)&(~RXDE),&regs->CRB );
        		iowrite8((UFL|CRC|OFL|RXS|TXS),&regs->IMR );
		    }
        }
    }else if(  lstat==0 ){ 
	    if( netif_carrier_ok(ndev) ){
            printk(KERN_NOTICE"%s: link DOWN\n",ndev->name);
			netif_carrier_off(ndev);
		    if( ch->chip->type != MR17G_MUXONLY ){
		    	// Setup HDLC controller
	        	iowrite8(ioread8(&regs->CRB)|RXDE,&regs->CRB);
    	    	iowrite8(0,&regs->IMR );
        		iowrite8(0xff,&regs->SR );
		    }
        }
    }else{
        printk(KERN_ERR"%s: error reading link status for %s\n",MR17G_MODNAME,ndev->name);
    }
}

// -------------------------- Package transmit/receive -----------------------------//

/*static*/ int
mr17g_start_xmit( struct sk_buff *skb, struct net_device *ndev )
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
	unsigned long flags;

	// No xmit from MUX only ports
    if( ch->chip->type == MR17G_MUXONLY )
    	return 0;

	PDEBUG(debug_xmit,"start, skb->len=%d",skb->len );
	if ( !netif_carrier_ok(ndev) ){
        PDEBUG(debug_xmit,"Carrier is off - abort");
		dev_kfree_skb_any( skb );
		return 0;
	}
	
/*	TODO: check max length
    if( skb->len > ETHER_MAX_LEN ){
		PDEBUG(0,"too big packet!!!");	
		dev_kfree_skb_any( skb );		
		return 0;
	}
*/

	spin_lock_irqsave(&ch->tx.lock,flags);
	if( sg_ring_add_skb(&ch->tx,skb) == -ERFULL ){
		PDEBUG(debug_xmit,"error: cannot add skb - full queue");
		spin_unlock_irqrestore(&ch->tx.lock,flags);
		netif_stop_queue( ndev );
		return 1; // do not free skb, just return 1
	}
	ch->stats.tx_packets++;
	ch->stats.tx_bytes += skb->len;
	ndev->trans_start = jiffies;
	spin_unlock_irqrestore(&ch->tx.lock,flags);
    PDEBUG(debug_xmit,"end");
	return  0;
}

// xmit_free_buffs may also be used to drop the queue - just turn
// the transmitter off, and set CTDR == LTDR
static void
xmit_free_buffs( struct net_device *ndev )
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
	struct sk_buff *skb;
	int len;

	PDEBUG(debug_xmit,"start");	
	while( (skb=sg_ring_del_skb(&ch->tx,&len)) != NULL ){
		dev_kfree_skb_any( skb );
	}
	if( netif_queue_stopped( ndev )  &&  sg_ring_have_space(&ch->tx) ){
		//		PDEBUG(debug_xmit,"enable xmit queue");		
		netif_wake_queue( ndev );
	}
	PDEBUG(debug_xmit,"end");			
}

static void 
xmit_drop_buffs(struct net_device *ndev)
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;

    iowrite8(ioread8(&regs->LTDR),&regs->CTDR);
    xmit_free_buffs( ndev );
}
//---------------receive-----------------------------------------

static void
recv_init_frames( struct net_device *ndev )
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
	struct sk_buff  *skb;
	unsigned  len=0;

	PDEBUG(debug_recv,"start");		
	while( (skb = sg_ring_del_skb(&ch->rx,&len)) != NULL ) {
		// setup skb & give it to OS
		skb_put( skb, len );
		skb->protocol = hdlc_type_trans(skb, ndev);
		netif_rx( skb );
		++ch->stats.rx_packets;
		ch->stats.rx_bytes += len;
		PDEBUG(debug_recv,"len = %d",skb->len);
	}
	return;
}

static int
recv_alloc_buffs( struct net_device *ndev )
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
	struct sk_buff  *skb;

	PDEBUG(debug_recv,"start");		    
	while( sg_ring_have_space(&ch->rx) ){
		if( !(skb = dev_alloc_skb(PKG_MAX_LEN)) )
			return -ENOMEM;
		skb->dev = ndev;
		skb_reserve( skb, 2 );	// align ip on longword boundaries
		// get dma able address & save skb
		if( sg_ring_add_skb(&ch->rx,skb) ){
			dev_kfree_skb_any( skb );
			return -1;
		}
	}
	PDEBUG(debug_recv,"end");			
	return 0;
}

static void
recv_free_buffs( struct net_device *ndev)
{
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
	struct sk_buff  *skb;
	int len;

	PDEBUG(debug_recv,"start");		    
	while( (skb = sg_ring_del_skb(&ch->rx,&len)) != NULL ) {	
		dev_kfree_skb_any( skb );
	}
	PDEBUG(debug_recv,"end");			
	return;
}
