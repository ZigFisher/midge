/* mr17g_sysfs.c
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/vermagic.h>
#include <linux/kobject.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include "mr17g.h"
#include "mr17g_net.h"
#include "pef22554.h"

//#define DEBUG_ON
#define DEBUG_LEV 10
#include "mr17g_debug.h"

#define to_net_dev(class) container_of(class, struct net_device, class_dev)

static u32
str2slotmap(char *str,size_t size,int *err)
{
	char *e,*s=str;
	u32 fbit,lbit,ts=0;
	int i;

	PDEBUG(debug_net,"start");	
	for (;;) {
		fbit=lbit=simple_strtoul(s, &e, 10);
		if (s == e)
			break;
		if (*e == '-') {
			s = e+1;
			lbit = simple_strtoul(s, &e, 10);
		}

		if (*e == ','){
			e++;
		}
		if( !(fbit < MAX_TS_BIT && lbit < MAX_TS_BIT) )
			break;
		for (i=fbit; i<=lbit;i++){
			ts |= 1L << i;
		}
		s=e;
	}
	PDEBUG(debug_net,"str=%08x, s=%08x,size=%d",(u32)str,(u32)s,size);
	*err=0;	
	if( s != str+(size-1) && s != str)
		*err=1;
    return ts;
}

static int
slotmap2str(u32 smap,struct mr17g_chan_config *cfg,char *buf)
{
	int start = -1,end, i;
	char *p=buf;
	
	if( cfg->framed ){
	    smap &= ~1;
		if( !cfg->ts16 )
			smap &= ~(1<<16);
	}
	
	for(i=0;i<32;i++){
		if( start<0 ){
			start = ((smap >> i) & 0x1) ? i : -1;
			if( start>0 && i==31 )
			    p += sprintf(p,"%d",start);
		}else if( !((smap >> i) & 0x1) || i == 31){
			end = ((smap >> i) & 0x1) ? i : i-1;
			if( p>buf )
				p += sprintf(p,",");
			p += sprintf(p,"%d",start);
			if( start<end )
				p += sprintf(p,"-%d",end);
			start=-1;
		}
	}
	return strlen(buf);
}


// Template
/*
static ssize_t
show_(struct class_device *cdev, char *buf) 
{                                                                       
}

static ssize_t
store_X( struct class_device *cdev,const char *buf, size_t size ) 
{
}
static CLASS_DEVICE_ATTR(,0644,show_X,store_X);
*/


//----------- Interface type -----------------------//
static ssize_t
show_muxonly(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    
    if( ch->chip->type == MR17G_MUXONLY ){
    	return snprintf(buf,PAGE_SIZE,"1");
    }else{
    	return snprintf(buf,PAGE_SIZE,"0");
    }    	
}
static CLASS_DEVICE_ATTR(muxonly,0444,show_muxonly,NULL);



//----------- HDLC settings ------------------------//

// CRC sum settings
static ssize_t
show_crc16(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( cfg->crc16 )
		return snprintf(buf,PAGE_SIZE,"crc16");
	else
		return snprintf(buf,PAGE_SIZE,"crc32");    
}

static ssize_t
store_crc16(struct class_device *cdev,const char *buf,size_t size) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !size )	return 0;
    
	if(buf[0] == '1' )
		cfg->crc16=1;
	else if( buf[0] == '0' )
		cfg->crc16=0;
	mr17g_transceiver_setup(ch);
	return size;	
}
static CLASS_DEVICE_ATTR(crc16,0644,show_crc16,store_crc16);

// Fill byte value
static ssize_t
show_fill_7e(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( cfg->fill_7e )
		return snprintf(buf,PAGE_SIZE,"fill_7e");
	else
		return snprintf(buf,PAGE_SIZE,"fill_ff");    
}

static ssize_t
store_fill_7e( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !size )	return 0;
	
	if(buf[0] == '1' )
		cfg->fill_7e=1;
	else if( buf[0] == '0' )
		cfg->fill_7e=0;
	mr17g_transceiver_setup(ch);
	return size;	
}
static CLASS_DEVICE_ATTR(fill_7e,0644,show_fill_7e,store_fill_7e);

// Inversion control
static ssize_t
show_inv(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( cfg->inv )
		return snprintf(buf,PAGE_SIZE,"inversion");
	else
		return snprintf(buf,PAGE_SIZE,"normal");    
}

static ssize_t
store_inv( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !size )
		return 0;
    
	if(buf[0] == '1' )
		cfg->inv=1;
	else if( buf[0] == '0' )
		cfg->inv=0;
	mr17g_transceiver_setup(ch);
	return size;	
}
static CLASS_DEVICE_ATTR(inv,0644,show_inv,store_inv);

// Read burst mode
static ssize_t
show_rburst(struct class_device *cdev,char *buf)
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( cfg->rburst )
		return snprintf(buf,PAGE_SIZE,"rbon");
	else
		return snprintf(buf,PAGE_SIZE,"rboff");    
}

static ssize_t
store_rburst(struct class_device *cdev,const char *buf,size_t size)
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !size )	return 0;

	if(buf[0] == '1' )
		cfg->rburst=1;
	else if( buf[0] == '0' )
		cfg->rburst=0;
	mr17g_transceiver_setup(ch);
	return size;	
}
static CLASS_DEVICE_ATTR(rburst,0644,show_rburst,store_rburst);

// Write burst mode
static ssize_t
show_wburst(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( cfg->wburst )
		return snprintf(buf,PAGE_SIZE,"wbon");
	else
		return snprintf(buf,PAGE_SIZE,"wboff");    
}

static ssize_t
store_wburst( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !size )	return 0;

	if(buf[0] == '1' )
		cfg->wburst=1;
	else if( buf[0] == '0' )
		cfg->wburst=0;
	mr17g_transceiver_setup(ch);
	return size;	
}
static CLASS_DEVICE_ATTR(wburst,0644,show_wburst,store_wburst);

//-------------------------- E1 settings ---------------------//

// Slotmap
static ssize_t
show_slotmap(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;

	if( cfg->framed )
		return slotmap2str(cfg->slotmap,cfg,buf);
	// in unframed mode multiplexing is overlap hdlc	
	if( regs->MXCR & MXEN ) 
		return slotmap2str(0,cfg,buf);
	return slotmap2str(0xffffffff,cfg,buf);
}


static ssize_t
store_slotmap( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;
	u32 ts=0;
	int err;
	char *str;

	PDEBUG(debug_sysfs,"start");	

	if( !size )
		return size;

	str=(char *)(buf+(size-1));
	*str=0;
	str=(char *)buf;	
	PDEBUG(debug_sysfs,"call str2slotmap");	
	ts=str2slotmap(str,size,&err);
	PDEBUG(debug_sysfs,"str2slotmap completed");		
	if( err ){
		printk(KERN_ERR MR17G_DRVNAME": error in timeslot string (%s)\n",buf);
		return size;
	}
	cfg->slotmap=ts;
	mr17g_transceiver_setup(ch);		
    pef22554_channel(ch);
	return size;
}
static CLASS_DEVICE_ATTR(slotmap,0644,show_slotmap,store_slotmap);


// framed/unframed mode 
static ssize_t
show_framed(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( cfg->framed )
		return snprintf(buf,PAGE_SIZE,"framed");
	else
		return snprintf(buf,PAGE_SIZE,"unframed"); 
}

static ssize_t
store_framed( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( size > 0 ){
		if( buf[0]=='0' )
			cfg->framed=0;
		else if( buf[0]=='1' )
			cfg->framed=1;
		else 
			return size;
		mr17g_transceiver_setup(ch);		
        pef22554_channel(ch);
	}    
	return size;
}
static CLASS_DEVICE_ATTR(framed,0644,show_framed,store_framed);

static ssize_t
show_map_ts16(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( cfg->ts16 )
		return snprintf(buf,PAGE_SIZE,"mapped");
	else
		return snprintf(buf,PAGE_SIZE,"not mapped"); 
}

static ssize_t
store_map_ts16(struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !(size>0) )
		return size;

	if( buf[0] == '0' ){
		cfg->ts16 = 0;
	}else if( buf[0] == '1' ){
		cfg->ts16 = 1;
	}else
		return size;
	mr17g_transceiver_setup(ch);
	pef22554_channel(ch);
	return size;
}
static CLASS_DEVICE_ATTR(map_ts16,0644,show_map_ts16,store_map_ts16);


// internal/external clock 
static ssize_t
show_clck(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !cfg->ext_clck )
		return snprintf(buf,PAGE_SIZE,"internal");
	else
		return snprintf(buf,PAGE_SIZE,"external"); 
}

static ssize_t
store_clck( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !(size > 0) )
		return size;

	if( buf[0]=='0' )
		cfg->ext_clck=0;
	else if( buf[0]=='1' )
	    cfg->ext_clck=1;
	else
		return size;
	mr17g_transceiver_setup(ch);
    pef22554_channel(ch);
    
	return size;
}
static CLASS_DEVICE_ATTR(clck,0644,show_clck,store_clck);

// Long haul mode
static ssize_t
show_lhaul(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( cfg->long_haul )
		return snprintf(buf,PAGE_SIZE,"on");
	else
		return snprintf(buf,PAGE_SIZE,"off"); 
}

static ssize_t
store_lhaul(struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !(size > 0) )
		return size;
	
	if( buf[0]=='0' )
		cfg->long_haul=0;
	else if( buf[0]=='1' )
		cfg->long_haul=1;
	else
		return size;
	mr17g_transceiver_setup(ch);		
    pef22554_channel(ch);

	return size;
}
static CLASS_DEVICE_ATTR(long_haul,0644,show_lhaul,store_lhaul);

// HDB3 mode
static ssize_t
show_hdb3(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( cfg->hdb3 )
		return snprintf(buf,PAGE_SIZE,"on");
	else
		return snprintf(buf,PAGE_SIZE,"off"); 
}

static ssize_t
store_hdb3( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !(size > 0) )
		return size;
	
	if( buf[0]=='0' )
		cfg->hdb3=0;
	else if( buf[0]=='1' )
        cfg->hdb3=1;
	else
		return size;
	mr17g_transceiver_setup(ch);		
    pef22554_channel(ch);
    
	return size;
}
static CLASS_DEVICE_ATTR(hdb3,0644,show_hdb3,store_hdb3);

// CRC4 mode
static ssize_t
show_crc4(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( cfg->crc4 )
		return snprintf(buf,PAGE_SIZE,"on");
	else
		return snprintf(buf,PAGE_SIZE,"off"); 
}

static ssize_t
store_crc4( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !(size > 0) )
		return size;

	if( buf[0]=='0' )
		cfg->crc4=0;
	else if( buf[0]=='1' )
        cfg->crc4=1;
	else
		return size;
	mr17g_transceiver_setup(ch);		
    pef22554_channel(ch);

	return size;
}
static CLASS_DEVICE_ATTR(crc4,0644,show_crc4,store_crc4);

// CAS mode
static ssize_t
show_cas(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( cfg->cas )
		return snprintf(buf,PAGE_SIZE,"on");
	else
		return snprintf(buf,PAGE_SIZE,"off"); 
}

static ssize_t
store_cas( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !(size > 0) )
		return size;
	
	if( buf[0]=='0' )
		cfg->cas=0;
	else if( buf[0]=='1' )
		cfg->cas=1;
	else
		return size;
	mr17g_transceiver_setup(ch);		
    pef22554_channel(ch);
	    
	return size;
}
static CLASS_DEVICE_ATTR(cas,0644,show_cas,store_cas);

//--------------- Loopback rgisters

// Remote loopback
static ssize_t
show_rloopback(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	return snprintf(buf,PAGE_SIZE,"%s", cfg->rlpb ? "on" : "off");
}

static ssize_t
store_rloopback( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !(size > 0) )
		return size;
	
	if( buf[0]=='0' ){
		printk(KERN_NOTICE"%s: Remote Loopback disabled\n",ndev->name);
		cfg->rlpb=0;
	}else if( buf[0]=='1' ){
		printk(KERN_NOTICE"%s: Remote Loopback enabled\n",ndev->name);
		cfg->rlpb=1;
	}else
		return size;
	mr17g_transceiver_setup(ch);		
    pef22554_channel(ch);
	    
	return size;
}
static CLASS_DEVICE_ATTR(rlpb,0644,show_rloopback,store_rloopback);

// Local loopback
static ssize_t
show_lloopback(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	return snprintf(buf,PAGE_SIZE,"%s", cfg->llpb ? "on" : "off");
}

static ssize_t
store_lloopback( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;

	if( !(size > 0) )
		return size;
	
	if( buf[0]=='0' ){
		cfg->llpb=0;
		printk(KERN_NOTICE"%s: Local Loopback disabled\n",ndev->name);
	}
	else if( buf[0]=='1' ){
		printk(KERN_NOTICE"%s: Local Loopback enabled\n",ndev->name);
		cfg->llpb=1;
	}else
		return size;

	mr17g_transceiver_setup(ch);		
    pef22554_channel(ch);
	    
	return size;
}
static CLASS_DEVICE_ATTR(llpb,0644,show_lloopback,store_lloopback);



// ------------------------- Multiplexing ------------------------------- //

// MXMAP
static ssize_t
show_mx_slotmap(struct class_device *cdev, char *buf)
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;
	if( cfg->framed ){
		return slotmap2str(cfg->mxslotmap,cfg,buf);
	}else{
		return slotmap2str(0xffffffff,cfg,buf);
	}
}


static ssize_t
store_mx_slotmap(struct class_device *cdev,const char *buf,size_t size)
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chan_config *cfg = &ch->cfg;
	u32 ts=0;
	int err;
	char *str;

	PDEBUG(debug_sysfs,"start, buf=%s",buf);	

	if( !size )
		return size;

	str=(char *)(buf+(size-1));
	*str=0;
	str=(char *)buf;
	PDEBUG(debug_sysfs,"call str2slotmap for (%s)",str);
	ts=str2slotmap(str,size,&err);
	PDEBUG(debug_sysfs,"str2slotmap completed, ts=%08x",ts);	
	if( err ){
		printk(KERN_NOTICE MR17G_DRVNAME": error in timeslot string (%s)\n",buf);
		return size;
	}
	
	cfg->mxslotmap = ts;	
	mr17g_transceiver_setup(ch);		
    pef22554_channel(ch);
	
	return size;
}
static CLASS_DEVICE_ATTR(mx_slotmap,0644,show_mx_slotmap,store_mx_slotmap);

// TFS rgister
static ssize_t
show_mx_txstart(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
	
	return snprintf(buf,PAGE_SIZE,"%d",ioread8(&regs->TFS));
}

static ssize_t
store_mx_txstart( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
	char *endp;
	u16 tmp;

	// check parameters
	if( !size)
		return size;
	tmp=simple_strtoul( buf,&endp,0);
	
	if( tmp>255 )
		return size;		
	iowrite8(tmp,&regs->TFS);
	return size;
}
static CLASS_DEVICE_ATTR(mx_txstart,0644,show_mx_txstart,store_mx_txstart);

// RFS rgister
static ssize_t
show_mx_rxstart(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;

	return snprintf(buf,PAGE_SIZE,"%d",ioread8(&regs->RFS));
}

static ssize_t
store_mx_rxstart( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
	char *endp;
	u16 tmp;

	// check parameters
	if( !size)
		return size;
	tmp=simple_strtoul( buf,&endp,0);
	if( tmp>255 )
		return size;
	iowrite8(tmp,&regs->RFS);
	return size;
}
static CLASS_DEVICE_ATTR(mx_rxstart,0644,show_mx_rxstart,store_mx_rxstart);

// TLINE rgister
static ssize_t
show_mx_tline(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
	
	return snprintf(buf,PAGE_SIZE,"%d",ioread8(&regs->TLINE));
}

static ssize_t
store_mx_tline( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
	char *endp;
	u16 tmp;

	// check parameters
	if( !size)
		return size;
	tmp=simple_strtoul( buf,&endp,0);
	if( tmp>15 )
		return size;
	iowrite8(tmp,&regs->TLINE);
	return size;
}
static CLASS_DEVICE_ATTR(mx_tline,0644,show_mx_tline,store_mx_tline);

// RLINE rgister
static ssize_t
show_mx_rline(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;

	return snprintf(buf,PAGE_SIZE,"%d",ioread8(&regs->RLINE));
}

static ssize_t
store_mx_rline( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
	char *endp;
	u16 tmp;

	// check parameters
	if( !size) return size;
	
	tmp=simple_strtoul( buf,&endp,0);
	if( tmp>15 )
		return size;
	iowrite8(tmp,&regs->RLINE);
	return size;
}
static CLASS_DEVICE_ATTR(mx_rline,0644,show_mx_rline,store_mx_rline);

//--------------- MXCR rgister

// MX enable
static ssize_t
show_mx_enable(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
	
	return snprintf(buf,PAGE_SIZE,"%s",(ioread8(&regs->MXCR) & MXEN) ? "1" : "0");
}

static ssize_t
store_mx_enable( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;

	// check parameters
	if( !size) return size;
	
	switch( buf[0] ){
	case '0':
		iowrite8(ioread8(&regs->MXCR)&(~MXEN),&regs->MXCR);
		break;
	case '1':
		iowrite8(ioread8(&regs->MXCR)|MXEN,&regs->MXCR);
		break;
	default:
		break;
	}
	
	mr17g_transceiver_setup(ch);		
    pef22554_channel(ch);

	return size;
}
static CLASS_DEVICE_ATTR(mx_enable,0644,show_mx_enable,store_mx_enable);

// CLKM
static ssize_t
show_mx_clkm(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
	
	return snprintf(buf,PAGE_SIZE,"%s",(ioread8(&regs->MXCR)&CLKM) ? "1" : "0");
}

static ssize_t
store_mx_clkm( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;

	// check parameters
	if( !size) return size;
	
	switch( buf[0] ){
	case '0':
		iowrite8(ioread8(&regs->MXCR)&(~CLKM),&regs->MXCR);
		break;
	case '1':
		iowrite8(ioread8(&regs->MXCR)|CLKM,&regs->MXCR);
		break;
	default:
		break;
	}

	mr17g_transceiver_setup(ch);		
    pef22554_channel(ch);
	return size;
}
static CLASS_DEVICE_ATTR(mx_clkm,0644,show_mx_clkm,store_mx_clkm);

// CLKAB
static ssize_t
show_mx_clkab(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
	
	return snprintf(buf,PAGE_SIZE,"%s",(ioread8(&regs->MXCR)&CLKAB) ? "1" : "0");
}

static ssize_t
store_mx_clkab( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;

	// check parameters
	if( !size) return size;
	
	switch( buf[0] ){
	case '0':
		iowrite8(ioread8(&regs->MXCR)&(~CLKAB),&regs->MXCR);
		break;
	case '1':
		iowrite8(ioread8(&regs->MXCR)|CLKAB,&regs->MXCR);
		break;
	default:
		break;
	}
	mr17g_transceiver_setup(ch);		
    pef22554_channel(ch);
	return size;
}
static CLASS_DEVICE_ATTR(mx_clkab,0644,show_mx_clkab,store_mx_clkab);

// CLKR
static ssize_t
show_mx_clkr(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
	
	return snprintf(buf,PAGE_SIZE,"%s",(ioread8(&regs->MXCR)&CLKR) ? "1" : "0");
}

static ssize_t
store_mx_clkr( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;

	// check parameters
	if( !size)
		return size;
	switch( buf[0] ){
	case '0':
		iowrite8(ioread8(&regs->MXCR)&(~CLKR),&regs->MXCR);
		break;
	case '1':
		iowrite8(ioread8(&regs->MXCR)|CLKR,&regs->MXCR);
		break;
	default:
		break;
	}

	mr17g_transceiver_setup(ch);		
    pef22554_channel(ch);
	return size;
}
static CLASS_DEVICE_ATTR(mx_clkr,0644,show_mx_clkr,store_mx_clkr);

//-------------------------------- DEBUG -----------------------------------------//

#ifdef DEBUG_SYSFS

// HDLC registers
static ssize_t 
show_hdlc_regs(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    volatile struct mr17g_hw_regs *regs = ch->iomem.regs;
    int len;

	len = snprintf(buf,PAGE_SIZE,"CRA(%02x),CRB(%02x),SR(%02x),IMR(%02x)\n"
					"CTDR(%02x),LTDR(%02x),CRDR(%02x),LRDR(%02x)\n",
					ioread8(&regs->CRA),ioread8(&regs->CRB),ioread8(&regs->SR),
                    ioread8(&regs->IMR),
                    ioread8(ch->tx.CxDR),ioread8(ch->tx.LxDR),
                    ioread8(ch->rx.CxDR),ioread8(ch->rx.LxDR) );

	len += snprintf(buf+len,PAGE_SIZE-len,"MAP0=%02x MAP1=%02x MAP2=%02x MAP3=%02x\n",
                    ioread8(&regs->MAP0),ioread8(&regs->MAP1),ioread8(&regs->MAP2),
					ioread8(&regs->MAP3) );

	len += snprintf(buf+len,PAGE_SIZE-len,"MXMAP0=%02x MXMAP1=%02x MXMAP2=%02x MXMAP3=%02x\n",
                    ioread8(&regs->MXMAP0),ioread8(&regs->MXMAP1),ioread8(&regs->MXMAP2),
					ioread8(&regs->MXMAP3) );

	len += snprintf(buf+len,PAGE_SIZE,"RATE(%02x),MXRATE(%02x),TFS(%02x),RFS(%02x)\n"
					"TLINE(%02x),RLINE(%02x),MXCR(%02x)\n",
					ioread8(&regs->RATE),ioread8(&regs->MXRATE),ioread8(&regs->TFS),
                    ioread8(&regs->RFS),ioread8(&regs->TLINE),ioread8(&regs->RLINE),
                    ioread8(&regs->MXCR));
	return len;
}
static CLASS_DEVICE_ATTR(hdlc_regs,0444,show_hdlc_regs,NULL);

// PEF22554 registers
u16 REG_ADDR = 0;
static ssize_t
show_readreg(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chip *chip = ch->chip;
    
    u8 tmp = 0;
    if( pef22554_readreg(chip,ch->num,REG_ADDR,&tmp) ){
        PDEBUG(debug_net,"Error, reading register %04x",REG_ADDR);
        return sprintf(buf,"Error");
    }
	return sprintf(buf,"0x%02x",tmp);
}

static ssize_t
store_readreg(struct class_device *cdev,const char *buf,size_t size)
{
    char *endp;
	if( !size ) return 0;
	REG_ADDR = simple_strtoul(buf,&endp,16);
	return size;
}
static CLASS_DEVICE_ATTR(chip_readreg,0644,show_readreg,store_readreg);


static ssize_t
store_writereg(struct class_device *cdev,const char *buf,size_t size)
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chip *chip = ch->chip;
	int addr, val;

	char *endp;
	if( !size ) return 0;
	addr = simple_strtoul(buf,&endp,16);
	while( *endp == ' '){
		endp++;
	}
	val = simple_strtoul(endp,&endp,16);
    if( pef22554_writereg(chip,ch->num,addr,val) ){
        PDEBUG(debug_net,"Error, writing register %04x",addr);
        return size;
    }
	return size;
}
static CLASS_DEVICE_ATTR(chip_writereg,0200,NULL,store_writereg);


// Memory window debug 
static u32 win_start=0,win_count=0;
static ssize_t
show_winread(struct class_device *cdev, char *buf) 
{                               
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chip *chip = ch->chip;
	char *win = (char*)chip->iomem + win_start;
	int len = 0,i;

	for(i=0;i<win_count && (len < PAGE_SIZE-3);i++){
		len += sprintf(buf+len,"%02x ",ioread8(win+i) & 0xff);
	}
	len += sprintf(buf+len,"\n");
	return len;
}

static ssize_t
store_winread(struct class_device *cdev,const char *buf,size_t size )
{
	char *endp;
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chip *chip = ch->chip;
    int iomem_size;

	if( !size ) return 0;
	
    switch( chip->type ){
    case MR17G_STANDARD:
		iomem_size = 4*MR17G_CHAN1_SIZE;
		if( !chip->num ){
			iomem_size += MR17G_SCI_SIZE;
		}
        break;
    case MR17G_MUXONLY:
		iomem_size = 4*MR17G_CHAN2_SIZE;
        break;
    default:
        printk("%s: error chip type\n",MR17G_MODNAME);
		return size;
	}

	win_start = simple_strtoul(buf,&endp,16);
	PDEBUG(0,"buf=%p, endp=%p,*endp=%c",buf,endp,*endp);	
	while( *endp == ' '){
		endp++;
	}
	win_count = simple_strtoul(endp,&endp,16);
	PDEBUG(0,"buf=%p, endp=%p",buf,endp);		
	PDEBUG(0,"Set start=%d,count=%d",win_start,win_count);
	if( !win_count )
		win_count = 1;
	if( (win_start + win_count) > iomem_size ){
		if( win_start >= (iomem_size-1) ){
			win_start = 0;
			win_count = 1;
		} else {
			win_count = 1;
		}
	}
	PDEBUG(0,"Set start=%d,count=%d",win_start,win_count);	
	return size;
}
static CLASS_DEVICE_ATTR(winread,0644,show_winread,store_winread);


static u32 win_written = 0;
static ssize_t
show_winwrite(struct class_device *cdev, char *buf) 
{                               
	return sprintf(buf,"Byte %x is written",win_written);
}

static ssize_t
store_winwrite(struct class_device *cdev,const char *buf,size_t size)
{
	struct net_device *ndev = to_net_dev(cdev);
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
    struct mr17g_chip *chip = ch->chip;
	char *win = (char*)chip->iomem;
	int start, val;
	char *endp;
	int iomem_size = 0;

	if( !size ) return 0;

    switch( chip->type ){
    case MR17G_STANDARD:
		iomem_size = 4*MR17G_CHAN1_SIZE;
        break;
    case MR17G_MUXONLY:
		iomem_size = 4*MR17G_CHAN2_SIZE;
        break;
    default:
        printk("%s: error chip type\n",MR17G_MODNAME);
		return size;
	}

	start = simple_strtoul(buf,&endp,16);
	PDEBUG(0,"buf=%p, endp=%p,*endp=%c",buf,endp,*endp);	
	while( *endp == ' '){
		endp++;
	}
	val = simple_strtoul(endp,&endp,16);
	PDEBUG(0,"buf=%p, endp=%p",buf,endp);		
	PDEBUG(0,"Set start=%d,val=%d",start,val);
	if( start > (iomem_size - 1) ){
		start = 0;
	}
	win_written = start;
	iowrite8( (val & 0xff ),win + start);
	return size;
}
static CLASS_DEVICE_ATTR(winwrite,0644,show_winwrite,store_winwrite);

static ssize_t
store_testxmit(struct class_device *cdev,const char *buf,size_t size)
{
	struct net_device *ndev = to_net_dev(cdev);
	struct sk_buff *skb = dev_alloc_skb(100);
    int k;

	if( !skb ){
		printk(KERN_ERR"%s: sbk ENOMEM!",__FUNCTION__);
		return size;
	}
    skb_put( skb,100);
	for(k=0;k<100;k++){
        skb->data[k] = k;
	}
	if( mr17g_start_xmit(skb,ndev) ){
        PDEBUG(debug_net,"Error sending skb");
		dev_kfree_skb_any( skb );
	}
    return size;
}
static CLASS_DEVICE_ATTR(testxmit,0200,NULL,store_testxmit);

#endif // DEBUG_SYSFS


// ------------------------------------------------------------------------ //
static struct attribute *mr17g_attr[] = {
// Iface type
&class_device_attr_muxonly.attr,
//E1
&class_device_attr_framed.attr,
&class_device_attr_map_ts16.attr,
&class_device_attr_slotmap.attr,
&class_device_attr_clck.attr,
&class_device_attr_long_haul.attr,
&class_device_attr_hdb3.attr,
&class_device_attr_crc4.attr,
&class_device_attr_cas.attr,
&class_device_attr_rlpb.attr,
&class_device_attr_llpb.attr,
// HDLC
&class_device_attr_crc16.attr,
&class_device_attr_fill_7e.attr,
&class_device_attr_inv.attr,
// PCI
&class_device_attr_rburst.attr,
&class_device_attr_wburst.attr,

// debug
#ifdef DEBUG_SYSFS
&class_device_attr_hdlc_regs.attr,
&class_device_attr_chip_readreg.attr,
&class_device_attr_chip_writereg.attr,
&class_device_attr_winread.attr,
&class_device_attr_winwrite.attr,
&class_device_attr_testxmit.attr,
#endif // DEBUG_SYSFS

NULL
};

static struct attribute *sg17_mx_attr[] = {
// multiplexing
&class_device_attr_mx_slotmap.attr,
&class_device_attr_mx_txstart.attr,
&class_device_attr_mx_rxstart.attr,
&class_device_attr_mx_tline.attr,
&class_device_attr_mx_rline.attr,
&class_device_attr_mx_enable.attr,
&class_device_attr_mx_clkm.attr,
&class_device_attr_mx_clkab.attr,
&class_device_attr_mx_clkr.attr,
NULL
};

static struct attribute_group mr17g_group = {
.name  = "hw_private",
.attrs  = mr17g_attr,
};

static struct attribute_group sg17_mx_group = {
.name  = "hw_multiplexing",
.attrs  = sg17_mx_attr,
};

int
mr17g_netsysfs_register(struct net_device *ndev)
{
	struct class_device *class_dev = &(ndev->class_dev);	
    hdlc_device *hdlc = dev_to_hdlc(ndev);
	struct mr17g_channel *ch  = (struct mr17g_channel*)hdlc->priv;
	static char fname[10] = "device";

	int ret = sysfs_create_group(&class_dev->kobj, &mr17g_group);
	ret += sysfs_create_group(&class_dev->kobj, &sg17_mx_group);
    sysfs_create_link(&(class_dev->kobj),&(ch->chip->pdev->dev.kobj),fname);
	return ret;
}

void
mr17g_netsysfs_remove(struct net_device *ndev){
	struct class_device *class_dev = &(ndev->class_dev);	
	sysfs_remove_group(&class_dev->kobj, &mr17g_group);
	sysfs_remove_group(&class_dev->kobj, &sg17_mx_group);
    sysfs_remove_link(&(class_dev->kobj),"device");
}
