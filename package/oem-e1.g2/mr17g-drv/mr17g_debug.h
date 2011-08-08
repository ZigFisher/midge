/* mr17g_debug.h
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

#ifndef SG_DEBUG_H
#define SG_DEBUG_H

#include "mr17g_oem.h"

#undef DEBUG_ON
//#define DEBUG_ON

#ifndef DEFAULT_LEV 
#	define DEFAULT_LEV 0
#endif

#define PDEBUG(lev,fmt,args...)
#define PDEBUGL(lev,fmt,args...)
#ifdef DEBUG_ON
#       undef PDEBUG
#       define PDEBUG(lev,fmt,args...)									\
	if( lev<=DEFAULT_LEV )												\
		printk(KERN_NOTICE MR17G_DRVNAME"(%s): " fmt " \n",__FUNCTION__, ## args  )

#       undef PDEBUGL
#       define PDEBUGL(lev,fmt,args...)			\
	if( lev<=DEFAULT_LEV )						\
		printk(fmt, ## args  )

#endif

extern int debug_xmit;
extern int debug_recv;
extern int debug_irq;
extern int debug_sci;
extern int debug_init;
extern int debug_pef;
extern int debug_cur;
extern int debug_net;
extern int debug_link;
extern int debug_error;
extern int debug_sysfs;

#endif		    
