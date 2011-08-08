#ifndef SG_DEBUG_H
#define SG_DEBUG_H

#define DIRQ 10 // Debug interrupt requests
#define DCARR 0 // Debug carrier detection
#define DINIT 10 // Debug driver init
#define DXMIT 10 // Debug transmittion
#define DRECV 10 // Debug receiving
#define DIFACE 10 // Debug hdlc-driver stuff
#define DSYSFS 40 // Debug sysfs interface
#define DSYSDBG 40 // Debug sysfs memory window debugger

#define PDEBUG(lev,fmt,args...)
#ifdef DEBUG_ON
#       undef PDEBUG
#       define PDEBUG(lev,fmt,args...) \
		if( lev<DEFAULT_LEV ) \
			printk(KERN_ERR "mr16g: %s " fmt " \n",__FUNCTION__, ## args  )
#endif

#endif		    
