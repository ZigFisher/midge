/* mr17s_debug.h
 *  Sigrand MR17S: RS232 PCI adapter driver for linux (kernel 2.6.x)
 *
 *	Written 2008 by Artem Y. Polyakov (artpol84@gmail.com)
 *
 *	This driver presents MR17S module interfaces as serial ports for OS
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 *
 */

#ifndef SG_DEBUG_H
#define SG_DEBUG_H

#include "mr17s_oem.h"

#undef DEBUG_ON

#ifndef DEBUG_LEV 
#	define DEBUG_LEV 0
#endif

#define PDEBUG(lev,fmt,args...)
#define PDEBUGL(lev,fmt,args...)
#ifdef DEBUG_ON
#       undef PDEBUG
#       define PDEBUG(lev,fmt,args...)									\
	if( lev<=DEBUG_LEV )												\
		printk(KERN_NOTICE MR17S_DRVNAME"(%s): " fmt " \n",__FUNCTION__, ## args  )

#       undef PDEBUGL
#       define PDEBUGL(lev,fmt,args...)			\
	if( lev<=DEBUG_LEV )						\
		printk(fmt, ## args  )

#endif

extern int debug_xmit;
extern int debug_recv;
extern int debug_irq;
extern int debug_init;
extern int debug_tty;
extern int debug_hw;
extern int debug_error;
extern int debug_sysfs;

#endif		    
