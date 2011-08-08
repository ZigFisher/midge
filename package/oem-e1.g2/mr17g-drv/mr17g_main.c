/* mr17g_main.c
 *  	Sigrand MR17G E1 PCI adapter driver for linux (kernel 2.6.x)
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
#include "mr17g_oem.h"
#include "mr17g_version.h"
#include "mr17g_main.h"
#include "mr17g_sci.h"
#include "pef22554.h"

// Debug settings
#define DEBUG_ON
#define DEFAULT_LEV 10
#include "mr17g_debug.h"


// mr17g_ioctl
#define SIOCGLRATE	(SIOCDEVPRIVATE+14)

MODULE_DESCRIPTION( "E1 PCI adapter driver Version "MR17G_VER"\n" );
MODULE_AUTHOR( "Maintainer: Polyakov Artem <artpol84@gmail.com>\n" );
MODULE_LICENSE( "GPL" );
MODULE_VERSION(MR17G_VER);

// Register module init/deinit routines 
static unsigned int cur_card_number = 0;
module_init(mr17g_init);
module_exit(mr17g_exit);

/*----------------------------------------------------------
 * Driver initialisation 
 *----------------------------------------------------------*/
static struct pci_device_id  mr17g_pci_tbl[] __devinitdata = {
{ PCI_DEVICE(MR17G_PCI_VEN,MR17G_PCI_DEV) },
{ PCI_DEVICE(MR17G_PCI_VEN,MR17G_PCI_DEV) },
{ 0 }
};

MODULE_DEVICE_TABLE( pci, mr17g_pci_tbl );
	
static struct pci_driver  mr17g_driver = {
 name:           MR17G_DRVNAME,
 probe:          mr17g_init_one,
 remove:         mr17g_remove_one,
 id_table:       mr17g_pci_tbl
};


static int  __devinit
mr17g_init( void )
{
	printk(KERN_NOTICE"Load "MR17G_MODNAME" E1 driver. Version "MR17G_VER"\n");
	PDEBUG(debug_error,"debug error test");
	return pci_module_init( &mr17g_driver );
}

static void  __devexit
mr17g_exit( void ){
	printk(KERN_NOTICE"Unload "MR17G_MODNAME" E1 driver\n");
	pci_unregister_driver( &mr17g_driver );
}

/*----------------------------------------------------------
 * PCI related functions 
 *----------------------------------------------------------*/

static int __devinit
mr17g_init_one( struct pci_dev *pdev,const struct pci_device_id *ent )
{
    struct mr17g_card *card = NULL;
	int err = -1;

	PDEBUG(debug_init,"start");

    // Setup PCI device 
	if( pci_enable_device( pdev ) )
		return -EIO;
	pci_set_master(pdev);
	
    // Init MR17G card
    if( !(card = mr17g_init_card(pdev)) ){
        goto pcifree;
    }
	pci_set_drvdata(pdev,card);

	PDEBUG(debug_init,"end, card = %p",card);
	return 0;
pcifree:	
	pci_disable_device(pdev);
	PDEBUG(debug_init,"(!)fail");
	return err;
}
				
				
static void __devexit 
mr17g_remove_one( struct pci_dev *pdev )
{
	struct mr17g_card *card = (struct mr17g_card*)pci_get_drvdata( pdev );

    PDEBUG(debug_init,"TEST!");
    PDEBUG(debug_init,"card = %p, chip_quan=%d",card,card->chip_quan);

	PDEBUG(debug_init,"start");
    mr17g_shutdown_card(card);
    kfree(card);
	pci_disable_device(pdev);
	PDEBUG(debug_init,"end");
}

inline int
mr17g_setup_chip(struct mr17g_card *card,int ind,enum mr17g_chiptype type)
{
	struct mr17g_chip *chip = card->chips + ind;
    PDEBUG(debug_init,"New chip = %p",chip);
    chip->pdev = card->pdev;
	chip->if_quan = 4; // TODO: in future may be cards vith 2 and 1 interfaces
	chip->sci = &card->sci;
	chip->type = type;
	chip->num = ind;
	switch( ind ){
	case 0:
		chip->iomem_start = card->iomem_start + MR17G_CHAN1_START;
		chip->iomem = card->iomem + MR17G_CHAN1_START;
		break;
	case 1:
		chip->iomem_start = card->iomem_start + MR17G_CHAN2_START;
		chip->iomem = card->iomem + MR17G_CHAN2_START;
		break;
	default:
		printk(KERN_ERR"%s: wrong chip number (%d), may be only 0,1\n",MR17G_MODNAME,ind);
		return -1;
	}	
	// Setup PEF22554 basic general registers
	if( pef22554_basic_chip(chip) ){
		return -1;
    }
    // Initialise network interfaces
    if( mr17g_net_init(chip) )
    	return -1;  
	return 0;
} 

// Initialize MR17G hardware 
struct mr17g_card * __devinit
mr17g_init_card(struct pci_dev *pdev)
{
    struct mr17g_card *card = NULL;
    u32 len,iomem_size;

    PDEBUG(debug_init,"start");

    // Setup card structure
	if( !(card = kmalloc( sizeof(struct mr17g_card), GFP_KERNEL ) ) ){
		printk(KERN_ERR"%s: error allocating memory for card ctructure\n",MR17G_MODNAME);
        goto error;
    }

	// Clear card structure
	memset((void*)card,0,sizeof(struct mr17g_card));
	// Setup card resources
	card->pdev = pdev;
	card->number = cur_card_number++;
	card->iomem_start = pci_resource_start(card->pdev,1);
	card->iomem_end = pci_resource_end( card->pdev, 1 );
    len = pci_resource_len(pdev,1);
	sprintf(card->name,MR17G_DRVNAME"_%d",card->number);
	
	// Setup card subtype
    switch(card->pdev->subsystem_device){
    case MR17G4_SUBSYS_ID:
        card->type = MR17G4;
		iomem_size = MR17G4_IOMEM_SIZE;
		card->chip_quan = 1;
        break;
    case MR17G84_SUBSYS_ID:
        card->type = MR17G84;
		iomem_size = MR17G84_IOMEM_SIZE;
		card->chip_quan = 2;
        break;
    case MR17G8_SUBSYS_ID:
        card->type = MR17G8;
		iomem_size = MR17G84_IOMEM_SIZE;
		card->chip_quan = 2;
        break;
    default:
        printk("%s: error hardware PCI Subsystem ID = %04x for module\n",
        		MR17G_MODNAME,card->pdev->subsystem_device);
        goto cardfree;
    }

	// Check & request memory region
    if( (card->iomem_end - card->iomem_start) + 1 != iomem_size ){
		printk(KERN_ERR"%s: wrong size of I/O Memory Window (%ld != %d) for chip %s\n",
				MR17G_MODNAME,(card->iomem_end-card->iomem_start) + 1, iomem_size,card->name);
        goto cardfree;
    }
   	if( !request_mem_region(card->iomem_start,iomem_size,card->name) ){
	    	printk(KERN_ERR"%s: error requesting io memory region for %s\n",MR17G_MODNAME,card->name);
            goto cardfree;
    }
   	card->iomem = (void*)ioremap(card->iomem_start,iomem_size);

   	PDEBUG(debug_init,"request IRQ");
    if( request_irq( pdev->irq, mr17g_sci_intr, SA_SHIRQ, card->name, (void*)&card->sci) ){
	    printk(KERN_ERR"%s: error requesting irq(%d) for %s\n",MR17G_MODNAME,pdev->irq,card->name);
   		goto memfree;
    }
	
	// Initialise SCI controller
	card->sci.iomem = card->iomem + MR17G_SCI_START;
	PDEBUG(0,"SCI=%x",(unsigned long)(card->sci.iomem) - (unsigned long)(card->iomem));
    if( mr17g_sci_enable(card) ){
		goto irqfree;
    }

	// Allocate chip structures
    card->chips = (struct mr17g_chip*)
            kmalloc( card->chip_quan*sizeof(struct mr17g_card), GFP_KERNEL );
    if( !card->chips ){
		printk(KERN_ERR"%s: error allocating memory for chips ctructures\n",MR17G_MODNAME);
        goto irqfree;
    }

	// Setup first chip
	if( mr17g_setup_chip(card,0,MR17G_STANDARD) )
		goto chipsfree;

	switch(card->type){
	case MR17G4:
		// Card has only one chip
		break;
	case MR17G84:
		// Setup second chip (Only with multiplexing)	
		if( mr17g_setup_chip(card,1,MR17G_MUXONLY) )
			goto chip1free;
		break;
	case MR17G8:
		// Setup second chip normally	
		if( mr17g_setup_chip(card,1,MR17G_STANDARD) )
			goto chip1free;
		break;
	}	

	// Start chipsets monitoring
    mr17g_sci_monitor((void*)card);

	PDEBUG(0,"CHAN1=%p, CHAN2=%p, SCI=%p",
		(void*)card->chips->iomem_start,(void*)(card->chips+1)->iomem_start,card->sci.iomem);


    PDEBUG(debug_init,"Return card = %p",card);
    return card;
	
chip1free:
	mr17g_net_uninit(card->chips);
chipsfree:
    kfree(card->chips);
irqfree:
    free_irq(pdev->irq,&card->sci);
memfree:
	iounmap( (void*)card->iomem );
    release_mem_region(card->iomem_start,iomem_size);
cardfree:
    kfree(card);
error:
    PDEBUG(debug_init,"(!)fail");
    return NULL;
}

static void  __devexit
mr17g_shutdown_card(struct mr17g_card *card)
{
    int iomem_size = card->iomem_end - card->iomem_start + 1;
    int i;
    
	PDEBUG(debug_init,"start, card = %p",card);
	mr17g_sci_endmon(card);
	for(i=0;i<card->chip_quan;i++){
		struct mr17g_chip *chip = (card->chips + i);
		PDEBUG(debug_init,"uninit, card = %p, chip = %p",card,chip);
		mr17g_net_uninit(chip);
    }
	mr17g_sci_disable(card);
	mr17g_sci_endmon(card);
	kfree(card->chips);
	free_irq(card->pdev->irq,(void*)&card->sci);
	iounmap((void*)card->iomem);
	release_mem_region(card->iomem_start,iomem_size);
	PDEBUG(debug_init,"end");
}

