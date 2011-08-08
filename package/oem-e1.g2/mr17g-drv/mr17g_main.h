/* mr17g_main.h
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

#ifndef MR17G_MAIN_H
#define MR17G_MAIN_H

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

#include "mr17g.h"
#include "mr17g_oem.h"
#include "mr17g_version.h"

static int mr17g_init( void );
static void mr17g_exit( void );

/*----------------------------------------------------------
 * PCI related functions 
 *----------------------------------------------------------*/
static int __devinit mr17g_init_one(struct pci_dev *pdev,const struct pci_device_id *ent);
static void __devexit mr17g_remove_one(struct pci_dev *pdev);
struct mr17g_card * __devinit mr17g_init_card(struct pci_dev *pdev);
static void __devexit mr17g_shutdown_card(struct mr17g_card *card);
int __devinit mr17g_net_init(struct mr17g_chip *chip);
int __devexit mr17g_net_uninit(struct mr17g_chip *chip);

#endif
