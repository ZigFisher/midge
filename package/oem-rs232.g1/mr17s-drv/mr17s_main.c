/* mr17s_main.c
 *  Sigrand MR17S: RS232 PCI adapter driver for linux (kernel 2.6.x)
 *
 *	Written 2008 by Artem Y. Polyakov (artpol84@gmail.com)
 *
 *	This driver presents MR17S module interfaces as serial ports for OS
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 *
 *	18.11.08 ver 1.1 - Fix transceiver cleanup while wtartup
 *
 */

#include "mr17s_version.h"
#include "mr17s.h"
#include "mr17s_main.h"
#include "mr17s_sysfs.h"
#include <linux/serial_core.h>
#include <linux/init.h>


// DEBUG
//#define DEBUG_ON 
#define DEBUG_LEV 10
#include "mr17s_debug.h"

MODULE_DESCRIPTION( "RS232 PCI adapter driver Version "MR17S_VER"\n" );
MODULE_AUTHOR( "Maintainer: Polyakov Artem <artpol84@gmail.com>\n" );
MODULE_LICENSE( "GPL" );
MODULE_VERSION(MR17S_VER);

// Register module init/deinit routines 
static unsigned int cur_card_number = 0;
static unsigned int cur_port_number = 0;
module_init(mr17s_init);
module_exit(mr17s_exit);



/*----------------------------------------------------------
 * Driver initialisation 
 *----------------------------------------------------------*/
static struct pci_device_id  mr17s_pci_tbl[] __devinitdata = {
{ PCI_DEVICE(MR17S_PCI_VEN,MR17S_PCI_DEV) },
{ 0 }
};

MODULE_DEVICE_TABLE( pci, mr17s_pci_tbl );
	
static struct pci_driver  mr17s_driver = {
 name:           MR17S_DRVNAME,
 probe:          mr17s_init_one,
 remove:         mr17s_remove_one,
 id_table:       mr17s_pci_tbl
};


static struct uart_driver mr17s_uartdrv = {
	.owner = THIS_MODULE,
	.driver_name = MR17S_MODNAME,
    .dev_name   = MR17S_SERIAL_NAME,
	.devfs_name = MR17S_SERIAL_NAME,	
	.major		= MR17S_SERIAL_MAJOR,
	.minor		= MR17S_SERIAL_MINORS,
    .nr         = MR17S_UART_NR,
};

static struct uart_ops mr17s_uart_ops = {
	.tx_empty	= mr17s_tx_empty,
	.set_mctrl	= mr17s_set_mctrl,
	.get_mctrl	= mr17s_get_mctrl,
	.stop_tx	= mr17s_stop_tx,
	.start_tx	= mr17s_start_tx,
	.stop_rx	= mr17s_stop_rx,
   	.enable_ms	= mr17s_enable_ms,
	.break_ctl	= mr17s_break_ctl,
	.startup	= mr17s_startup,
	.shutdown	= mr17s_shutdown,
	.set_termios	= mr17s_set_termios,
	.type		= mr17s_type,
	.release_port	= mr17s_release_port,
	.request_port	= mr17s_request_port,
	.config_port	= mr17s_config_port,
	.verify_port	= mr17s_verify_port,
    .ioctl          = mr17s_ioctl,
};


static int  __devinit
mr17s_init( void )
{
	int result;

	printk(KERN_NOTICE"Load "MR17S_MODNAME" RS232 driver. Version "MR17S_VER"\n");

	result = uart_register_driver(&mr17s_uartdrv);

	if (result){
    	printk(KERN_NOTICE MR17S_MODNAME": Error registering UART driver\n");
		return result;
    }    
	if( (result = pci_module_init(&mr17s_driver)) ){
    	uart_unregister_driver(&mr17s_uartdrv);
        return result;
	}
	PDEBUG(0,"DEBUG is ON. Test #1");
    return 0;
}

static void  __devexit
mr17s_exit( void ){
	printk(KERN_NOTICE"Unload "MR17S_MODNAME" RS232 driver\n");
	pci_unregister_driver( &mr17s_driver );
	uart_unregister_driver(&mr17s_uartdrv);
}




/*----------------------------------------------------------
 * PCI related functions 
 *----------------------------------------------------------*/

static int __devinit
mr17s_init_one(struct pci_dev *pdev,const struct pci_device_id *ent)
{
	struct device *dev = (struct device*)&(pdev->dev);
	struct device_driver *drv = (struct device_driver*)(dev->driver);
    struct mr17s_device *rsdev = NULL;
    int i,j, err = -1;
    u32 len;

	PDEBUG(debug_init,"start");

    // Setup PCI device 
	if( pci_enable_device( pdev ) )
		return -EIO;
	pci_set_master(pdev);
	
    // Create device structure & initialize 
	if( !(rsdev = kmalloc( sizeof(struct mr17s_device), GFP_KERNEL ) ) ){
		printk(KERN_ERR"%s: error allocating rsdev, PCI device=%02x:%02x.%d\n",MR17S_MODNAME,
                pdev->bus->number,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));
        err = -ENOMEM;
        goto pcifree;
    }
	memset((void*)rsdev,0,sizeof(struct mr17s_device));
	rsdev->pdev = pdev;
    rsdev->number = cur_card_number++;
    snprintf(rsdev->name,31,"%s%d",MR17S_MODNAME,rsdev->number);
	rsdev->iomem_start = pci_resource_start(rsdev->pdev,1);
	rsdev->iomem_end = pci_resource_end(rsdev->pdev,1);
	len =  pci_resource_len(pdev, 1);
    // Detect type of the card
    switch(pdev->subsystem_device){
    case MR17S_DTE2CH:
        rsdev->type = DTE;
        rsdev->port_quan = 2;
        break;
    case MR17S_DCE2CH:
        rsdev->type = DCE;
        rsdev->port_quan = 2;
        break;
    case MR17S_DTE4CH:
        rsdev->type = DTE;
        rsdev->port_quan = 4;
        break;
    case MR17S_DCE4CH:
        rsdev->type = DCE;
        rsdev->port_quan = 4;
        break;
    default:
		printk(KERN_ERR"%s: error MR17S subtype=%d, PCI device=%02x:%02x.%d\n",MR17S_MODNAME,
                pdev->subsystem_device,
                pdev->bus->number,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));
        err = -ENODEV;
        goto rsfree;
    }
    PDEBUG(debug_hw,"Found %s(%d channels)",(rsdev->type == DTE) ? "DTE" : "DCE",rsdev->port_quan);
    // Check I/O Memory window
    if( len != MR17S_IOMEM_SIZE ){
		printk(KERN_ERR"%s: wrong size of I/O memory window: %d != %d, PCI device=%02x:%02x.%d\n",
                MR17S_MODNAME,len, MR17S_IOMEM_SIZE,
                pdev->bus->number,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));
        err = -EINVAL;
        goto rsfree;
    }    
    // Request & remap memory region
   	if( !request_mem_region(rsdev->iomem_start,MR17S_IOMEM_SIZE,MR17S_MODNAME) ){
		printk(KERN_ERR"%s: error requesting io memory region, PCI device=%02x:%02x.%d\n",
                MR17S_MODNAME,
                pdev->bus->number,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));
        goto rsfree;
    }
    rsdev->iomem = (void*)ioremap(rsdev->iomem_start,MR17S_IOMEM_SIZE);

    // Create serial ports
    rsdev->ports = (struct mr17s_uart_port*)
            kmalloc( rsdev->port_quan*sizeof(struct mr17s_uart_port), GFP_KERNEL );
    if( !rsdev->ports ){
		printk(KERN_ERR"%s: error allocating memory for ports, PCI device=%02x:%02x.%d\n",
                MR17S_MODNAME,
                pdev->bus->number,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));
        err = -ENOMEM;
        goto memfree;
    }

    // Setup port structures
    for(i=0;i<rsdev->port_quan;i++){
        struct mr17s_uart_port *hw_port = rsdev->ports + i;
        struct uart_port *port = &hw_port->port;
		char port_name[32];

        memset(hw_port,0,sizeof(*hw_port));
        hw_port->type = rsdev->type;
		atomic_set(&hw_port->inuse_cntr,0);
        // port I/O setup
        port->iotype = UPIO_MEM;
        port->iobase = 0;
        port->mapbase = rsdev->iomem_start + i*MR17S_CHANMEM_SIZE;
        port->membase = rsdev->iomem + i*MR17S_CHANMEM_SIZE;
        port->irq = pdev->irq;
        // FIFO size
        port->fifosize = 16;
        port->uartclk = MR17S_UARTCLK;
        // UART operations
        port->ops = &mr17s_uart_ops;
        // port index
        port->line = cur_port_number++;
        // port parent device
        port->dev = &pdev->dev;
        // PORT flags
        port->flags = ASYNC_BOOT_AUTOCONF; //UPF_SKIP_TEST | UPF_BOOT_AUTOCONF | UPF_SHARE_IRQ;
    	if( (err = uart_add_one_port(&mr17s_uartdrv,port)) ){
    		printk(KERN_ERR"%s: error, registering %s%d node, PCI device=%02x:%02x.%d\n",
                   MR17S_MODNAME,MR17S_SERIAL_NAME,port->line,
                   pdev->bus->number,PCI_SLOT(pdev->devfn),PCI_FUNC(pdev->devfn));
            goto porterr;
        }
		PDEBUG(debug_tty,"Add port "MR17S_SERIAL_NAME"%d, uart_port addr = %p, info = %p",port->line,port,port->info);
    	// Symlink to device in sysfs
		snprintf(port_name,32,MR17S_SERIAL_NAME"%d",port->line);
    	if( (err = sysfs_create_link( &(drv->kobj),&(dev->kobj),port_name )) ){
	    	printk(KERN_NOTICE"%s: error in sysfs_create_link\n",__FUNCTION__);	
		    goto porterr;
    	}
		
    }

    // Save MR17S internal structure in PCI device struct
	pci_set_drvdata(pdev,rsdev);

    // Request IRQ line
	if( (err = request_irq(pdev->irq,mr17s_interrupt,SA_SHIRQ,rsdev->name,rsdev)) ){
        goto porterr;
    }

    if( (err = mr17s_sysfs_register(&pdev->dev)) ){
        goto freeirq;
    }

	PDEBUG(debug_init,"end, rsdev = %p",rsdev);

	return 0;

freeirq:
    free_irq(pdev->irq,rsdev);
porterr:
    for(j=0;j<i;j++){
        struct mr17s_uart_port *hw_port = rsdev->ports + j;
        struct uart_port *port = &hw_port->port;
		char port_name[32];
		snprintf(port_name,32,MR17S_SERIAL_NAME"%d",port->line);
		sysfs_remove_link(&(drv->kobj),port_name);
        uart_remove_one_port(&mr17s_uartdrv,(struct uart_port*)&hw_port->port);
    }
    kfree(rsdev->ports);
memfree:
    iounmap(rsdev->iomem);
   	release_mem_region(rsdev->iomem_start,MR17S_IOMEM_SIZE);
rsfree:
    kfree(rsdev);
pcifree:	
	pci_disable_device(pdev);
	PDEBUG(debug_init,"(!)fail");
	return err;
}
				
				
static void __devexit 
mr17s_remove_one( struct pci_dev *pdev )
{
	struct mr17s_device *rsdev = (struct mr17s_device*)pci_get_drvdata(pdev);
	struct device *dev = (struct device*)&(pdev->dev);
	struct device_driver *drv = (struct device_driver*)(dev->driver);
    int i;
    
    mr17s_sysfs_free(&pdev->dev);
    free_irq(pdev->irq,rsdev);
    for(i=0;i<rsdev->port_quan;i++){
		struct uart_port *port = (struct uart_port*)(rsdev->ports + i);
		char port_name[32];
		snprintf(port_name,32,MR17S_SERIAL_NAME"%d",port->line);
		sysfs_remove_link(&(drv->kobj),port_name);
        uart_remove_one_port(&mr17s_uartdrv,port);
    }
    kfree(rsdev->ports);
    iounmap(rsdev->iomem);
   	release_mem_region(rsdev->iomem_start,MR17S_IOMEM_SIZE);
    kfree(rsdev);
	pci_disable_device(pdev);
	PDEBUG(debug_init,"end");
}

static irqreturn_t
mr17s_interrupt(int irq,void *dev_id,struct pt_regs *regs )
{
	struct mr17s_device *rsdev = (struct mr17s_device*)dev_id;
    int i,ret = 0;
    
    PDEBUG(debug_irq,"start, rsdev=%p",rsdev);

    // Poll all ports
    for(i=0;i<rsdev->port_quan;i++){
        PDEBUG(debug_irq,"poll %d channel",i);
        ret += mr17s_port_interrupt((struct uart_port*)(rsdev->ports + i),regs);
    }
    if( ret )
    	return IRQ_HANDLED;
    return IRQ_NONE;
}

//----------------- UART operations -----------------------------------//

static void
mr17s_stop_tx(struct uart_port *port)
{
    // Nothing to do. Transmitter stops automaticaly
}

static void
mr17s_start_tx(struct uart_port *port)
{
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;

    PDEBUG(debug_xmit,"start");
    if( ioread8(&regs->MXCR) & MXEN ){
        // May be there is better way 
        // But now we just drop all outgoing bytes in 
        // Multiplexingmode
        mr17s_drop_bytes(port);
        return;
    }
    mr17s_xmit_bytes(port);
}


// ?? Stop receiver when shutting down ??
static void 
mr17s_stop_rx(struct uart_port *port)
{
    // Nothing to do at this moment
}

static void mr17s_enable_ms(struct uart_port *port)
{
    // Nothing to do at this moment
}

static unsigned int mr17s_get_mctrl(struct uart_port *port)
{
    struct mr17s_uart_port *hw_port = (struct mr17s_uart_port*)port;
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
    unsigned int ret = 0;
    u8 cur = ioread8(&regs->SR);

    PDEBUG(debug_tty,"start");

    switch(hw_port->type){
    case DTE:
        if( cur & DSR ){
            ret |= TIOCM_DSR;
            PDEBUG(debug_tty,"DSR");
        }
        if( cur & CTS ){
            ret |= TIOCM_CTS;
            PDEBUG(debug_tty,"CTS");
        }
        if( cur & CD ){
            ret |= TIOCM_CAR;
            PDEBUG(debug_tty,"CD");
        }

        if( cur & RI ){
            ret |= TIOCM_RNG;
            PDEBUG(debug_tty,"RI");
        }

        break;
    case DCE:
        if( cur & DTR ){
            ret |= TIOCM_DTR;
            PDEBUG(debug_tty,"DTR");
        }

        if( cur & RTS ){
            ret |= TIOCM_RTS;
            PDEBUG(debug_tty,"RTS");
        }

        break;
    }
	
	return ret;
}

static void
mr17s_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
    // Nothing to do at this moment
}
static void 
mr17s_break_ctl(struct uart_port *port, int break_state)
{
    // Nothing to do at this moment
}

static void mr17s_set_termios(struct uart_port *port,
			     struct termios *new, struct termios *old)
{
    struct mr17s_uart_port *hw_port = (struct mr17s_uart_port *)port;
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
    u8 cur;
	u32 baud, quot;
    int div1,div2;
	unsigned long flags;

    PDEBUG(debug_hw,"");

	baud = uart_get_baud_rate(port,new,old, 0, port->uartclk);
	quot = uart_get_divisor(port, baud);
    PDEBUG(debug_hw,"BAUD=%u, QUOTE=%u",baud,quot);

    if( !count_divs(baud,&div1,&div2 ) ){
        PDEBUG(debug_hw,"DIV1=%d, DIV2=%d\n",div1,div2);
        cur = div1 | div2 << 3;
    }else{
        printk(KERN_ERR"%s: error baud rate %d, cannot count divisor\n",MR17S_MODNAME,baud);
    }


	// Locked area - configure hardware
	spin_lock_irqsave(&port->lock, flags);

	switch (new->c_cflag & CSIZE) {
	case CS7:
		cur |= DATA7;
		break;
	default:
        cur &= (~DATA7);
		break;
	}
    
	if (new->c_cflag & CSTOPB){
		cur |= STOP2;
    }else{
        cur &= (~STOP2);
    }

	if (new->c_cflag & PARENB){
        cur |= PAR1;
		if (new->c_cflag & PARODD){
            cur |= PAR0;
        }
	}else{
        cur &= ~(PAR0|PAR1);
    }

    hw_port->baud = baud;
	uart_update_timeout(port, new->c_cflag, baud);

    iowrite8(cur,&regs->CRB);
	spin_unlock_irqrestore(&port->lock, flags);

}

inline static void
mr17s_transceiver_up(struct uart_port *port)
{
    struct mr17s_uart_port *hw_port = (struct mr17s_uart_port *)port;
	struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
	u8 reg;

	// Clean state
	hw_port->old_status = 0;
    // Enable interrupts
    iowrite8(0xff,&regs->SR);
	reg = ioread8(&regs->CRA) | (DTR|RTS|CD|TXEN|RXEN);
    iowrite8(reg,&regs->CRA);
    iowrite8(TXS|RXS|RXE|MCC,&regs->IMR);
	// Cleanup Transceiver
	reg = ioread8(&regs->LRR);
	iowrite8(reg,&regs->CRR);
    reg = ioread8(&regs->CTR);
	iowrite8(reg,&regs->LTR);
}

inline static void
mr17s_transceiver_down(struct uart_port *port)
{
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
	u8 cra;
    // Disable interrupts
    iowrite8(0,&regs->IMR);
	cra = ioread8(&regs->CRA) & (FCEN|MCFW|TXEN|RXEN|DTR);
    iowrite8(cra,&regs->CRA);
}


static int mr17s_startup(struct uart_port *port)
{
    struct mr17s_uart_port *hw_port = (struct mr17s_uart_port *)port;
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
	unsigned long flags;
	u8 cra;

    PDEBUG(debug_hw,"");
	atomic_inc(&hw_port->inuse_cntr);
	if( ioread8(&regs->MXCR) & MXEN ){
		return 0;
	}
	spin_lock_irqsave(&port->lock, flags);
	mr17s_transceiver_up(port);
	spin_unlock_irqrestore(&port->lock, flags);

	// Update modem ststus
	mr17s_modem_status(port);
    return 0;
}

static void mr17s_shutdown(struct uart_port *port)
{
    struct mr17s_uart_port *hw_port = (struct mr17s_uart_port *)port;
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
	unsigned long flags;
	u8 cra;
    PDEBUG(debug_hw,"");

	if(atomic_dec_and_test(&hw_port->inuse_cntr) ){
		// shutdown port only if it is last shutdown
		spin_lock_irqsave(&port->lock, flags);
		mr17s_transceiver_down(port);
		spin_unlock_irqrestore(&port->lock, flags);
	}
}

static const char *mr17s_type(struct uart_port *port)
{
	return MR17S_MODNAME;
}

static void mr17s_release_port(struct uart_port *port)
{
    // Nothing to do at this moment
}

static int mr17s_request_port(struct uart_port *port)
{
    // Nothing to do at this moment
    return 0;
}

static void mr17s_config_port(struct uart_port *port, int flags)
{
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;

    port->type = MR17S_PORT;

    // Reset HDLC regs
    iowrite8(0,&regs->SR);
    iowrite8(0,&regs->IMR);
    iowrite8(0,&regs->CRA);
    iowrite8(0,&regs->CRB);

    // Transceiver reset
    iowrite8(0,&regs->CTR);
    iowrite8(0,&regs->LTR);
    iowrite8(0,&regs->CRR);
    iowrite8(0,&regs->LRR);
    // Mux reset
    iowrite8(0,&regs->MXRATE);
    iowrite8(0,&regs->RLINE);
    iowrite8(0,&regs->TLINE);
    iowrite8(0,&regs->TFS);
    iowrite8(0,&regs->RFS);
    iowrite8(0,&regs->MXCR);
	
	// Enable Transceiver
	iowrite8(TXEN|RXEN|DTR,&regs->CRA);
    iowrite8(0,&regs->SR);
}

static int mr17s_verify_port(struct uart_port *port, struct serial_struct *ser)
{
    // Nothing to do at this moment
	return 0;
}

static unsigned int 
mr17s_tx_empty(struct uart_port *port)
{
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
    
    if( ioread8(&regs->MXCR) & MXEN){
        // In mux mode - allways empty
        return TIOCSER_TEMT;
    }
    if( ioread8(&regs->CTR) == ioread8(&regs->LTR) )
        return TIOCSER_TEMT;
    return 0;
}

static int
mr17s_ioctl(struct uart_port *port, unsigned int cmd, unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
    struct mxsettings setting;
    struct hwsettings hwsetting;
    u16 magic;
    int ret = -ENOIOCTLCMD;

   	if(copy_from_user(&magic,uarg, sizeof(magic)))
    	return ret;
    
    PDEBUG(debug_hw,"MAGIC=%04x",magic);

    if( magic != MXMAGIC )
        return ret;

    PDEBUG(debug_hw,"start");
	switch (cmd) {
	case TIOCGMX:
        PDEBUG(debug_hw,"SIOCMXGET");
		ret = mr17s_mux_get(port,&setting);
    	if(copy_to_user(uarg,&setting,sizeof(setting)))
	    	return -EFAULT;
		break;
	case TIOCSMX:
        PDEBUG(debug_hw,"SIOCMXSET");
    	if(copy_from_user(&setting,uarg, sizeof(setting)))
	    	return -EFAULT;
		ret = mr17s_mux_set(port,&setting);
		break;
	case TIOCGHW:
        PDEBUG(debug_hw,"SIOCHWGET");
		ret = mr17s_hw_get(port,&hwsetting);
        PDEBUG(debug_hw,"SIOCHWGET - end");
    	if(copy_to_user(uarg,&hwsetting,sizeof(hwsetting)))
	    	return -EFAULT;
		break;
	case TIOCSHW:
        PDEBUG(debug_hw,"SIOCHWSET");
    	if(copy_from_user(&hwsetting,uarg, sizeof(hwsetting)))
	    	return -EFAULT;
		ret = mr17s_hw_set(port,&hwsetting);
        PDEBUG(debug_hw,"SIOCHWSET - end");
		break;
	}
    return ret;
}

static int
mr17s_mux_get(struct uart_port *port,struct mxsettings *set)
{
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
    u8 mxcr = ioread8(&regs->MXCR);
    u8 rate;

    rate = ioread8(&regs->MXRATE) + 1;
    set->mxrate = rate;
    set->rline = ioread8(&regs->RLINE);
    set->tline = ioread8(&regs->TLINE);
    set->rfs = ioread8(&regs->RFS);
    set->tfs = ioread8(&regs->TFS);
    
    set->clkm = set->clkab = set->clkr = set->mxen = 0;
    if( mxcr & CLKM ){
        set->clkm = 1;
    }
    if( mxcr & CLKAB ){
        set->clkab = 1;
    }
    if( mxcr & MXEN ){
        set->mxen = 1;
    }
    return 0;
}


static int
mr17s_mux_set(struct uart_port *port,struct mxsettings *set)
{
    struct mr17s_uart_port *hw_port = (struct mr17s_uart_port *)port;
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
    unsigned long flags;
    u8 mxcr = ioread8(&regs->MXCR);
	u8 mxen_change = ((mxcr & MXEN) && !(set->mxen)) || ( !(mxcr&MXEN) && set->mxen);

    switch(set->clkm){
    case 0:
        mxcr &= (~CLKM);
        break;
    case 1:
        mxcr |= CLKM;
        break;
    }

    switch(set->clkab){
    case 0:
        mxcr &= (~CLKAB);
        break;
    case 1:
        mxcr |= CLKAB;
        break;
    }

    switch(set->clkr){
    case 0:
        mxcr &= (~CLKR);
        break;
    case 1:
        mxcr |= CLKR;
        break;
    }

    switch(set->mxen){
    case 0:
        mxcr &= (~MXEN);
        break;
    case 1:
        mxcr |= MXEN;
        break;
    }
    
    spin_lock_irqsave(&port->lock,flags);

	// Setup MUX control register
    iowrite8(mxcr,&regs->MXCR);
	// Setup MUX rate
	iowrite8(--set->mxrate,&regs->MXRATE);

    if( (mxcr&MXEN) && mxen_change ){
		// setup transceiver
		atomic_inc(&hw_port->inuse_cntr);
		iowrite8(0,&regs->IMR);
    }else if( !(mxcr&MXEN) && mxen_change ) {
		atomic_dec(&hw_port->inuse_cntr);
		iowrite8(TXS|RXS|RXE,&regs->IMR);
	}        
    
	if( set->rline >=0 ){
        iowrite8(set->rline,&regs->RLINE);
    }
    if( set->tline >=0 ){
        iowrite8(set->tline,&regs->TLINE);
    }
    if( set->rfs >=0 ){
        iowrite8(set->rfs,&regs->RFS);
    }
    if( set->tfs >=0 ){
        iowrite8(set->tfs,&regs->TFS);
    }
    spin_unlock_irqrestore(&port->lock,flags);

    return 0;
}

static int
mr17s_hw_get(struct uart_port *port,struct hwsettings *set)
{
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
    u8 cra = ioread8(&regs->CRA);

	memset(set,0,sizeof(*set));
	if( cra & FCEN ){
		set->flow_ctrl = 1;
	}
	if( cra & MCFW ){
		set->fwd_sig = 1;
	}
    return 0;
}


static int
mr17s_hw_set(struct uart_port *port,struct hwsettings *set)
{
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
    u8 cra = ioread8(&regs->CRA);
	unsigned long flags;

    PDEBUG(debug_hw,"start");

	if( set->flow_ctrl ){
		cra |= FCEN;
	}else{
		cra &= (~FCEN);
	}

	if( set->fwd_sig ){
		cra |= MCFW;
	}else{
		cra &= (~MCFW);
	}
    
    PDEBUG(debug_hw,"CRA=%02x",cra);

    spin_lock_irqsave(&port->lock,flags);
    iowrite8(cra,&regs->CRA);
    spin_unlock_irqrestore(&port->lock,flags);

    return 0;
}

// Hardware related functions

static int
mr17s_port_interrupt(struct uart_port *port, struct pt_regs *ptregs)
{
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
    // Read mask & status
    u8 mask = ioread8(&regs->IMR);
    u8 status = ioread8(&regs->SR) & mask;
    // Ack all interrupts
    iowrite8(status,&regs->SR);
    // If no interrupt - return immediately
    if( !status )
        return 0;
   
    if( status & TXS ){
        PDEBUG(debug_xmit,"TXS");
        mr17s_xmit_bytes(port);
    }

    if( status & RXS ){
        PDEBUG(debug_irq,"RXS");
        mr17s_recv_bytes(port,ptregs);
    }

    if( status & RXE ){
        PDEBUG(debug_irq,"RXS");
    }
	
	if( status & MCC ){
		PDEBUG(debug_hw,"MCC: status=%02x",status);
		mr17s_modem_status(port);
	}
    return 1;
}


static inline void mr17s_modem_status(struct uart_port *port)
{
    struct mr17s_uart_port *hw_port = (struct mr17s_uart_port*)port;
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
	u8 old_status = 0;
	unsigned long flags;
    // Read mask & status
    u8 status = ioread8(&regs->SR);
	
	
	spin_lock_irqsave(&port->lock,flags);
	old_status = hw_port->old_status;
	PDEBUG(/*debug_hw*/0,"ttyRS%d: Check modem status: old=%02x new=%02x",port->line,old_status,status);

	// Carrier Detected bit 
	if( (status & CD) && !(old_status & CD) ){
		PDEBUG(/*debug_hw*/0,"ttyRS%d: Carrier detected",port->line);
		if( port->info )
			uart_handle_dcd_change(port,1);
	}else if( !(status & CD) && (old_status & CD) ){
		PDEBUG(/*debug_hw*/0,"ttyRS%d: Carrier lost",port->line);
		if( port->info )
			uart_handle_dcd_change(port,0);
	}else{
		PDEBUG(debug_hw,"ttyRS%d: No carrier change",port->line);
	}
	if( (status & RI) && !(old_status & RI) ){
		PDEBUG(/*debug_hw*/0,"ttyRS%d: Ring",port->line);
		port->icount.rng++;
	}

	if( (status & DSR) && !(old_status & DSR) ){
		PDEBUG(/*debug_hw*/0,"ttyRS%d: Device on cable",port->line);
		port->icount.dsr++;
	}

	if( (status & CTS) && !(old_status & CTS) ){
		PDEBUG(/*debug_hw*/0,"ttyRS%d: Device can receive",port->line);
		if( port->info )
			uart_handle_cts_change(port,1);
	}else if( !(status & CTS) && (old_status & CTS) ){
		PDEBUG(/*debug_hw*/0,"ttyRS%d: Device receive stop",port->line);
		if( port->info )
			uart_handle_cts_change(port,0);
	}else{
		PDEBUG(debug_hw,"ttyRS%d: No Dev can recv change",port->line);
	}
	
	if( port->info )
		wake_up_interruptible(&port->info->delta_msr_wait);
	hw_port->old_status = status;
	spin_unlock_irqrestore(&port->lock,flags);
}


void
mr17s_recv_bytes(struct uart_port *port,struct pt_regs *ptregs)
{
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
	struct tty_struct *tty = port->info->tty;
    u8 cur, last;
    unsigned int ch,flg = TTY_NORMAL;
    int i;
    unsigned long flags;

    // Block port. This function can run both 
    // in process and interrupt context
    spin_lock_irqsave(&port->lock,flags);

    cur = ioread8(&regs->CRR);
    last = ioread8(&regs->LRR);

    PDEBUGL(debug_irq," ");
    for(i=cur;i!=last;i=(i+1)&RING_MASK){
        ch = ioread8(mem->rx_buf+i);
        PDEBUGL(debug_irq,"%c",ch);
		port->icount.rx++;
		if( uart_handle_sysrq_char(port,ch,ptregs) )
			continue;
		tty_insert_flip_char(tty,ch,flg);
    }
    PDEBUGL(debug_irq,"\n");
	tty_flip_buffer_push(tty);
    iowrite8(i,&regs->CRR);

    spin_unlock_irqrestore(&port->lock,flags);
	return;
}


static void
mr17s_xmit_bytes(struct uart_port *port)
{
    struct mr17s_chan_iomem *mem = (struct mr17s_chan_iomem *)port->membase;
    struct mr17s_hw_regs *regs = &mem->regs;
	struct circ_buf *xmit = &port->info->xmit;
    u8 last,cur;
    u32 count;
    unsigned long flags;


    // Block port. This function can run both 
    // in process and interrupt context
    spin_lock_irqsave(&port->lock,flags);

    last = ioread8(&regs->LTR);
    cur = ioread8(&regs->CTR);
    PDEBUG(debug_xmit,"CTR=%02x, LTR=%02x",cur,last);

    // Hardware ring is full
    if( ((last+1)&RING_MASK) == cur ){
        PDEBUG(debug_xmit,"No more space in ring: LTR=%d, CTR=%d",last,cur);
        goto unlock;
    }
    
	if (port->x_char){
        PDEBUG(debug_xmit,"Xmit X CHAR: LTR=%d, CTR=%d",last,cur);
        iowrite8(port->x_char,mem->tx_buf + last);
        last = (last+1) & RING_MASK;
		port->icount.tx++;
		port->x_char = 0;
		goto exit;
	}

    // Nothing to xmit OR xmit stopped
    if( uart_circ_empty(xmit) || uart_tx_stopped(port) ) {
        PDEBUG(debug_xmit,"CIRC_EMPTY=%d, TX_STOPPED=%d",uart_circ_empty(xmit),uart_tx_stopped(port));
		goto unlock;
	}


    PDEBUGL(debug_xmit,"Send buf (start=%d): ",last);
    count = port->fifosize >> 2;
	while( !uart_circ_empty(xmit) && ( ((last+1)&RING_MASK) != cur) && count-- ){
        iowrite8(xmit->buf[xmit->tail],mem->tx_buf + last);
        PDEBUGL(debug_xmit,"%c",xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
        last = (last+1)&RING_MASK;
		port->icount.tx++;
	}
    PDEBUGL(debug_xmit," (end=%d)\n",last);

    // Ask for new bytes
	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS){
        PDEBUG(debug_xmit,"Ack for new characters");
		uart_write_wakeup(port);
    }

exit:
    // Fix Circ Ring changes
    iowrite8(last,&regs->LTR);
unlock:
    spin_unlock_irqrestore(&port->lock,flags);
}


static void
mr17s_drop_bytes(struct uart_port *port)
{
	struct circ_buf *xmit = &port->info->xmit;
    unsigned long flags;

    PDEBUG(debug_hw,"start");

    // Block port. This function can run both 
    // in process and interrupt context
    spin_lock_irqsave(&port->lock,flags);

	if (port->x_char){
		port->x_char = 0;
	}

    // Nothing to xmit OR xmit stopped
    if( uart_circ_empty(xmit) || uart_tx_stopped(port) ) {
        PDEBUG(debug_xmit,"CIRC_EMPTY=%d, TX_STOPPED=%d",uart_circ_empty(xmit),uart_tx_stopped(port));
		goto unlock;
	}

	while( !uart_circ_empty(xmit) ){
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
	}

    // Ask for new bytes
	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS){
        PDEBUG(debug_xmit,"Ack for new characters");
		uart_write_wakeup(port);
    }

unlock:
    spin_unlock_irqrestore(&port->lock,flags);
    PDEBUG(debug_hw,"end");
}


static int
count_divs(int baud,int *div1,int *div2)
{
    int d;
    
    if( !baud || (MR17S_MAXBAUD % baud) ){
        PDEBUG(debug_hw,"ERROR: baud=%d,(MAX div baud)=%d",(!baud),(MR17S_MAXBAUD%baud));
        return -1;
    }
    *div1 = 0;
    *div2 = 0;
    d = MR17S_MAXBAUD/baud;
 
    if( !(d%6) ){
        d /= 6;
        *div2 = 1;
    }
    while(d && d!= 1){
        d /= 2;
        (*div1)++;
    }

    return 0;
}
