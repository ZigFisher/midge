#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/vermagic.h>
#include <linux/kobject.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include "include/sdfe4_lib.h"
#include "include/sg17eoc.h"
#include "sg17lan.h"
#include "sg17oem.h"

// Debug parameters
//#define DEBUG_ON
#define DEFAULT_LEV 1
#include "sg17debug.h"


/* --------------------------------------------------------------------------
 *      Module initialisation/cleanup
 * -------------------------------------------------------------------------- */

#define to_net_dev(class) container_of(class, struct net_device, class_dev)

// Chipset type
static ssize_t show_chipver(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( nl->dev );
	struct sdfe4 *hwdev = &card->hwdev;
	
	switch( hwdev->type ){
	case SDFE4v1:
		return snprintf(buf,PAGE_SIZE,"v1");
	case SDFE4v2:
		return snprintf(buf,PAGE_SIZE,"v2");
	}		
	return 0;
}
static CLASS_DEVICE_ATTR(chipver,0444,show_chipver,NULL);

// Chipset type
static ssize_t show_pwr_source(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( nl->dev );
	
    return snprintf(buf,PAGE_SIZE,"%d",card->pwr_source);
}
static CLASS_DEVICE_ATTR(pwr_source,0444,show_pwr_source,NULL);


// Mode control (master/slave)
static ssize_t show_mode(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	if( nl->shdsl_cfg->mode == STU_C )
		return snprintf(buf,PAGE_SIZE,"master");
	else if( nl->shdsl_cfg->mode == STU_R )
		return snprintf(buf,PAGE_SIZE,"slave");
	else
		return snprintf(buf,PAGE_SIZE,"unknown role");	
}

static ssize_t
store_mode( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;

	if( size > 0 ){
		if( buf[0] == '0' ){
			cfg->mode = STU_R;
			cfg->startup_initialization = STARTUP_FAREND;
			cfg->transaction = GHS_TRNS_00;
			cfg->annex = ANNEX_A_B;
		}else if( buf[0] == '1' ){
			cfg->mode = STU_C;
			cfg->startup_initialization = STARTUP_LOCAL;
			cfg->transaction = GHS_TRNS_10;
		}	    
    }    
    return size;
}
static CLASS_DEVICE_ATTR(mode,0644,show_mode,store_mode);

// Annex control 
static ssize_t show_annex(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;	

	if( !netif_carrier_ok(ndev) && (cfg->mode == STU_R) )
		return 0;
	switch( cfg->annex ){
	case ANNEX_A:
		return snprintf(buf,PAGE_SIZE,"A");
	case ANNEX_B:
		return snprintf(buf,PAGE_SIZE,"B");
	case ANNEX_A_B:
		return snprintf(buf,PAGE_SIZE,"AB");
		/*	case ANNEX_G:
			return snprintf(buf,PAGE_SIZE,"annex G");
			case ANNEX_F:
			return snprintf(buf,PAGE_SIZE,"annex F");
		*/	default:
		return snprintf(buf,PAGE_SIZE,"unknown annex");
	}	
}
static ssize_t
store_annex( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;

	if( !size )	return size;
	
	switch( buf[0] ){
	case '0':
		cfg->annex=ANNEX_A;
		break;
	case '1':
		cfg->annex=ANNEX_B;
		break;
	case '2':
		cfg->annex=ANNEX_A_B;
		break;
	}
	return size;
}
static CLASS_DEVICE_ATTR(annex, 0644 ,show_annex,store_annex);

// Rate control
static ssize_t show_rate(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;
	
	if( !netif_carrier_ok(ndev) && (cfg->mode == STU_R) )
		return 0;
		 
	return snprintf(buf,PAGE_SIZE,"%d",nl->shdsl_cfg->rate);
}

static ssize_t
store_rate( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( nl->dev );
	struct sdfe4 *hwdev = &card->hwdev;
	char *endp;
	u16 tmp;
	
	// check parameters
	if( !size ) return size;

	tmp=simple_strtoul( buf,&endp,0);
	if( !tmp )
		return size;

	switch( hwdev->type ){
	case SDFE4v1:
		if( tmp > SDFE4v1_MAX_RATE )
			tmp = SDFE4v1_MAX_RATE;
		break;
	case SDFE4v2:
		if( tmp > SDFE4v2_MAX_RATE )
			tmp = SDFE4v2_MAX_RATE;
		break;
	}
	// Modulo 64 Kbps!
	tmp/=64;
	cfg->rate=tmp*64;
	return size;
}
static CLASS_DEVICE_ATTR(rate, 0644 ,show_rate,store_rate);

// TCPAM control
static ssize_t show_tcpam(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;	

	if( !netif_carrier_ok(ndev) && (cfg->mode == STU_R) )
		return 0;

	switch( cfg->tc_pam ){
	case TCPAM4:
		return snprintf(buf,PAGE_SIZE,"TCPAM4");
	case TCPAM8:
		return snprintf(buf,PAGE_SIZE,"TCPAM8");
 	case TCPAM16:
 		return snprintf(buf,PAGE_SIZE,"TCPAM16");
 	case TCPAM32:
 		return snprintf(buf,PAGE_SIZE,"TCPAM32");
	case TCPAM64:
		return snprintf(buf,PAGE_SIZE,"TCPAM64");
	case TCPAM128:
		return snprintf(buf,PAGE_SIZE,"TCPAM128");
 	default:
 		return snprintf(buf,PAGE_SIZE,"unknown TCPAM");
	}
}

static ssize_t
store_tcpam( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( nl->dev );
	struct sdfe4 *hwdev = &card->hwdev;
	u8 tmp;

	// if interface is up 
	if( !size )	return size;
	tmp=buf[0]-'0';

	switch( hwdev->type ){
	case SDFE4v1:
		if( (tmp > 4) || (tmp <= 2) ){
			return size;
		}
		break;
	case SDFE4v2:
		if( (tmp > 6) || (tmp <=0) ){
			return size;
		}
		break;
	}
	cfg->tc_pam=tmp;
	return size;
}
static CLASS_DEVICE_ATTR(tcpam, 0644 ,show_tcpam,store_tcpam);

// POWER Backoff control
static ssize_t
show_pbo_mode(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;	

	if( (cfg->mode != STU_C) ){
		return snprintf(buf,PAGE_SIZE,"Normal");
	}
	
	switch( cfg->pbo_mode ){
	case PWRBO_NORMAL:
		return snprintf(buf,PAGE_SIZE,"Normal");
	case PWRBO_FORCED:
		return snprintf(buf,PAGE_SIZE,"Forced");
	}
	return snprintf(buf,PAGE_SIZE,"Unknown");
}

static ssize_t
store_pbo_mode( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;
	u8 tmp;

	// if interface is up 
	if( !size )	return size;
	tmp=buf[0]-'0';

	switch( tmp ){
	case 0:
		cfg->pbo_mode = PWRBO_NORMAL;
		break;
	case 1:
		cfg->pbo_mode = PWRBO_FORCED;
		break;
	}
	return size;
}
static CLASS_DEVICE_ATTR(pbo_mode, 0644 ,show_pbo_mode,store_pbo_mode);

static ssize_t 
show_pbo_val(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;
	int pos = 0;
	int i;
	
	// Output string in format <val1>:<val2>:<val3>:<val4>...
	for(i=0;i<cfg->pbo_vnum;i++){
		pos += snprintf(buf+pos,PAGE_SIZE-pos,"%d",cfg->pbo_vals[i]);
		if( i != cfg->pbo_vnum-1 ){
			pos += snprintf(buf+pos,PAGE_SIZE-pos,":");
		}
	}
	return pos;
}

static ssize_t
store_pbo_val( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;
	char *endp;
	u32 tmp;
	u8 vals[16];
	int vnum = 0,i;
	
	// check parameters
	if( !size ) return size;
	do{
		tmp=simple_strtoul( buf,&endp,0);
		if( buf != endp ){
			vals[vnum] = (tmp > 31) ? 31 : tmp;
			vnum++;
		}
		if( *endp == '\0' ){
			break; // all string is processed
		}
		buf = endp + 1;
	}while( vnum < 16 );
	
	for(i=0;i<vnum;i++){
		cfg->pbo_vals[i] = vals[i];
	}
	cfg->pbo_vnum = vnum;
	return size;
}
static CLASS_DEVICE_ATTR(pbo_val, 0644 ,show_pbo_val,store_pbo_val);


// Clock mode control
static ssize_t show_clkmode(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;
	
	if( !netif_carrier_ok(ndev) && (cfg->mode == STU_R) )
		return 0;
	switch( cfg->clkmode ){
	case 0:
		return snprintf(buf,PAGE_SIZE,"plesio");
	case 1:
		return snprintf(buf,PAGE_SIZE,"sync");
	case 2:
		return snprintf(buf,PAGE_SIZE,"plesio-ref");
	}
	return snprintf(buf,PAGE_SIZE,"unknown");
}

// Clock mode: 0 - Plesiochronuous, 1 - Sinchronuous
static ssize_t
store_clkmode( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;
	
	// check parameters
	switch( buf[0] ){
	case '0':
		cfg->clkmode = 0;
		break;
	case '1':
		cfg->clkmode = 1;
		break;
	case '2':
		cfg->clkmode = 2;
		break;
	}
	return size;
}
static CLASS_DEVICE_ATTR(clkmode, 0644 ,show_clkmode,store_clkmode);

// Apply changes
static ssize_t
store_apply_cfg( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct sdfe4_if_cfg *cfg = (struct sdfe4_if_cfg *)nl->shdsl_cfg;
	// if interface is up 
	if( !size )	return size;
	if( buf[0] == '1' )
		cfg->need_reconf=1;
	return size;
}
static CLASS_DEVICE_ATTR(apply_cfg, 0200 ,NULL,store_apply_cfg);

// ---------------------------- Advanced link ---------------------------------- //
static ssize_t
show_advlink(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);

	if( advlink_enabled(&nl->alink) ){
		return snprintf(buf,PAGE_SIZE,"on");
	}else{
		return snprintf(buf,PAGE_SIZE,"off");
	}
}

static ssize_t
store_advlink(struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	if( !size )
	    return size;
	switch( buf[0] ){
	case '1':
		advlink_enable(&nl->alink);
		break;
	case '0':
		advlink_disable(&nl->alink);
		break;
	default:
		printk(KERN_ERR MR17H_MODNAME ": advlink parameter error");
		break;
	}
	return size;
}
static CLASS_DEVICE_ATTR(advlink, 0644 ,show_advlink,store_advlink);

// ---------------------------- EOC ---------------------------------- //
static ssize_t show_eoc(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( nl->dev );
	struct sg17_sci *s = (struct sg17_sci *)&card->sci;
	struct sdfe4 *hwdev = &card->hwdev;
	struct sdfe4_channel *ch = &hwdev->ch[sg17_sci_if2ch(s,nl->number)];
	char *ptr;
	int size;

    if( !netif_carrier_ok(ndev) ){    
        return 0;
    }

	if( (size = sdfe4_eoc_rx(ch,&ptr)) < 0 )
		return 0;
	memcpy(buf,ptr,size);
	kfree(ptr);
	return size;
}

static ssize_t
store_eoc(struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( nl->dev );
	struct sg17_sci *s = (struct sg17_sci *)&card->sci;
	struct sdfe4 *hwdev = &card->hwdev;

    if( netif_carrier_ok(ndev) ){    
    	sdfe4_eoc_tx(hwdev,sg17_sci_if2ch(s,nl->number),(char*)buf,size);
    }
	return size;
}
static CLASS_DEVICE_ATTR(eoc, 0644 ,show_eoc,store_eoc);


// ------------------------- HDLC 0/1 ---------------------------------------- //

// CRC count attribute 
static ssize_t
show_crc16(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	struct hdlc_config *cfg=&(nl->hdlc_cfg);

	if( cfg->crc16 )
		return snprintf(buf,PAGE_SIZE,"crc16");
	else
		return snprintf(buf,PAGE_SIZE,"crc32");    
}

static ssize_t
store_crc16( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct hdlc_config *cfg=&(nl->hdlc_cfg);
	u8 cfg_bt;

	if( !size )	return 0;
    
	switch(buf[0]){
	case '1':
		if( cfg->crc16 )
			break;
		cfg->crc16=1;
		cfg_bt=ioread8( &(nl->regs->CRA)) | CMOD;
		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
	case '0':
		if( !(cfg->crc16) )
			break;
		cfg->crc16=0;
		cfg_bt=ioread8( &(nl->regs->CRA)) & ~CMOD;
		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
	}	
	return size;	
}
static CLASS_DEVICE_ATTR(crc16, 0644 ,show_crc16,store_crc16);

// fill byte value
static ssize_t
show_fill_7e(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	struct hdlc_config *cfg=&(nl->hdlc_cfg);

	if( cfg->fill_7e )
		return snprintf(buf,PAGE_SIZE,"7E");
	else
		return snprintf(buf,PAGE_SIZE,"FF");    
}

static ssize_t
store_fill_7e( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct hdlc_config *cfg=&(nl->hdlc_cfg);
	u8 cfg_bt;

	if( !size )	return 0;
    
	switch(buf[0]){
	case '1':
		if( cfg->fill_7e )
			break;
		cfg->fill_7e=1;
		cfg_bt=ioread8( &(nl->regs->CRA)) | FMOD;
		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
	case '0':
		if( !(cfg->fill_7e) )
			break;
		cfg->fill_7e=0;
		cfg_bt=ioread8( &(nl->regs->CRA)) & ~FMOD;
		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
	}	
	return size;	
}
static CLASS_DEVICE_ATTR(fill_7e, 0644 ,show_fill_7e,store_fill_7e);
// data inversion
static ssize_t
show_inv(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	struct hdlc_config *cfg=&(nl->hdlc_cfg);

	if( cfg->inv )
		return snprintf(buf,PAGE_SIZE,"on");
	else
		return snprintf(buf,PAGE_SIZE,"off");    
}

static ssize_t
store_inv( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct hdlc_config *cfg=&(nl->hdlc_cfg);
	u8 cfg_bt;

	if( !size )
		return 0;
    
	switch(buf[0]){
	case '1':
		if( cfg->inv )
			break;
		cfg->inv=1;
		cfg_bt=ioread8(&(nl->regs->CRA)) | PMOD;
		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
	case '0':
		if( !(cfg->inv) )
			break;
		cfg->inv=0;
		cfg_bt=ioread8(&(nl->regs->CRA)) & ~PMOD;
		iowrite8( cfg_bt,&(nl->regs->CRA));
		break;
	}	
	return size;	
}
static CLASS_DEVICE_ATTR(inv, 0644 ,show_inv,store_inv);

// PCI read burst on/off
static ssize_t
show_rburst(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	struct hdlc_config *cfg=&(nl->hdlc_cfg);

	if( cfg->rburst )
		return snprintf(buf,PAGE_SIZE,"on");
	else
		return snprintf(buf,PAGE_SIZE,"off");    

}

static ssize_t
store_rburst( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct hdlc_config *cfg=&(nl->hdlc_cfg);
	u8 cfg_bt;

	if( !size )	return 0;
    
	switch(buf[0]){
	case '1':
		if( cfg->rburst )
			break;
		cfg->rburst=1;
		cfg_bt=ioread8(&(nl->regs->CRB)) | RDBE;
		iowrite8( cfg_bt,&(nl->regs->CRB));
		break;
	case '0':
		if( !(cfg->rburst) )
			break;
		cfg->rburst=0;
		cfg_bt=ioread8(&(nl->regs->CRB)) & ~RDBE;
		iowrite8( cfg_bt,&(nl->regs->CRB));
		break;
	}	
	return size;	
}
static CLASS_DEVICE_ATTR(rburst, 0644 ,show_rburst,store_rburst);

// PCI write burst
static ssize_t
show_wburst(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	struct hdlc_config *cfg=&(nl->hdlc_cfg);
	return snprintf(buf,PAGE_SIZE,"%s",cfg->wburst ? "on" : "off");
}

static ssize_t
store_wburst( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	struct hdlc_config *cfg=&(nl->hdlc_cfg);
	u8 cfg_bt;

	if( !size )	return 0;

	switch(buf[0]){
	case '1':
		if( cfg->wburst )
			break;
		cfg->wburst=1;
		cfg_bt=ioread8(&(nl->regs->CRB)) | WTBE;
		iowrite8( cfg_bt,&(nl->regs->CRB));
		break;
	case '0':
		if( !(cfg->wburst) )
			break;
		cfg->wburst=0;
		cfg_bt=ioread8(&(nl->regs->CRB)) & ~WTBE;
		iowrite8( cfg_bt,&(nl->regs->CRB));
		break;
	}	
	return size;	
}
static CLASS_DEVICE_ATTR(wburst, 0644 ,show_wburst,store_wburst);

// MAC address less significant value 
static ssize_t
store_maddr( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	u16 tmp;
	char *endp;

	if( !size ) return 0;

	tmp=simple_strtoul( buf,&endp,16) & 0xfff;
	*(u16 *)ndev->dev_addr = htons( 0x00ff ),
		*(u32 *)(ndev->dev_addr + 2) = htonl( 0x014aa000 | tmp );     

	return size;
}
static CLASS_DEVICE_ATTR(maddr, 0200 ,NULL,store_maddr);

// ------------------------- Statistics ------------------------------- //

static ssize_t
show_statistics(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( nl->dev );
	struct sg17_sci *s = (struct sg17_sci *)&card->sci;
	struct sdfe4_stat statistic, *stat=&statistic;
	
	if( sdfe4_get_statistic(sg17_sci_if2ch(s,nl->number),s->hwdev,stat) )
		return snprintf(buf,PAGE_SIZE,"Error Getting statistic");
	return snprintf(buf,PAGE_SIZE,
			"\tSNR Margin=%d\n\tLoop Attenuation=%d\n"
			"\tCounters: ES=%u SES=%u CRC_Anom=%u LOSWS=%u UAS=%u\n",
			(s8)stat->SNR_Margin_dB,(s8)stat->LoopAttenuation_dB,
			stat->ES_count,stat->SES_count,stat->CRC_Anomaly_count,stat->LOSWS_count,stat->UAS_Count);
}

static ssize_t
store_statistics( struct class_device *cdev,const char *buf, size_t size ) 
{
	//        struct net_device *ndev = to_net_dev(cdev);
	//	struct net_local *nl = netdev_priv(ndev);
	//	struct hdlc_config *cfg=&(nl->hdlc_cfg);

	if( !size )	return 0;

	switch(buf[0] == '1'){
	}	
	return size;	
}
static CLASS_DEVICE_ATTR(statistics,0644,show_statistics,store_statistics);

static ssize_t
show_statistics_row(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	struct sg17_card  *card = (struct sg17_card  *)dev_get_drvdata( nl->dev );
	struct sg17_sci *s = (struct sg17_sci *)&card->sci;
	struct sdfe4_stat statistic, *stat=&statistic;
	
	if( sdfe4_get_statistic(sg17_sci_if2ch(s,nl->number),s->hwdev,stat) )
		return snprintf(buf,PAGE_SIZE,"Error Getting statistic");
	return snprintf(buf,PAGE_SIZE,"%d %d %u %u %u %u %u %u %u %u %u",
			        stat->SNR_Margin_dB,stat->LoopAttenuation_dB,stat->ES_count,stat->SES_count,
					stat->CRC_Anomaly_count,stat->LOSWS_count,stat->UAS_Count,stat->SegmentAnomaly_Count,
			        stat->SegmentDefectS_Count,stat->CounterOverflowInd,stat->CounterResetInd );
												
}
static CLASS_DEVICE_ATTR(statistics_row,0444,show_statistics_row,NULL);


static ssize_t
show_link_state(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);

	if( advlink_get_hwstatus(&nl->alink) == ADVLINK_UP ){
		return snprintf(buf,PAGE_SIZE,"1");
	} else{
		return snprintf(buf,PAGE_SIZE,"0");	
	}
}
static CLASS_DEVICE_ATTR(link_state,0444,show_link_state,NULL);


// ------------------------- Multiplexing ------------------------------- //

// MXRATE rgister
static ssize_t
show_mx_rate(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	u8 reg = nl->regs->MXRATE+1;
	return snprintf(buf,PAGE_SIZE,"%d",reg);
}

static ssize_t
store_mx_rate( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	char *endp;
	u8 tmp;
	
	// check parameters
	if( !size)
		return size;
	tmp=simple_strtoul( buf,&endp,0);
	tmp--;
	nl->regs->MXRATE = tmp;
	return size;
}
static CLASS_DEVICE_ATTR(mxrate,0644,show_mx_rate,store_mx_rate);

// TFS rgister
static ssize_t
show_mx_txstart(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	
	return snprintf(buf,PAGE_SIZE,"%d",nl->regs->TFS);
}

static ssize_t
store_mx_txstart( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	char *endp;
	u16 tmp;

	// check parameters
	if( !size)
		return size;
	tmp=simple_strtoul( buf,&endp,0);
	
	if( tmp>255 )
		return size;		
	nl->regs->TFS = tmp;
	return size;
}
static CLASS_DEVICE_ATTR(mx_txstart,0644,show_mx_txstart,store_mx_txstart);

// RFS rgister
static ssize_t
show_mx_rxstart(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	
	return snprintf(buf,PAGE_SIZE,"%d",nl->regs->RFS);
}

static ssize_t
store_mx_rxstart( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	char *endp;
	u16 tmp;

	// check parameters
	if( !size)
		return size;
	tmp=simple_strtoul( buf,&endp,0);
	if( tmp>255 )
		return size;
	nl->regs->RFS = tmp;
	return size;
}
static CLASS_DEVICE_ATTR(mx_rxstart,0644,show_mx_rxstart,store_mx_rxstart);

// TLINE rgister
static ssize_t
show_mx_tline(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	
	return snprintf(buf,PAGE_SIZE,"%d",nl->regs->TLINE);
}

static ssize_t
store_mx_tline( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	char *endp;
	u16 tmp;

	// check parameters
	if( !size)
		return size;
	tmp=simple_strtoul( buf,&endp,0);
	if( tmp>15 )
		return size;
	nl->regs->TLINE = tmp;
	return size;
}
static CLASS_DEVICE_ATTR(mx_tline,0644,show_mx_tline,store_mx_tline);

// RLINE rgister
static ssize_t
show_mx_rline(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);

	return snprintf(buf,PAGE_SIZE,"%d",nl->regs->RLINE);
}

static ssize_t
store_mx_rline( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	char *endp;
	u16 tmp;

	// check parameters
	if( !size) return size;
	
	tmp=simple_strtoul( buf,&endp,0);
	if( tmp>15 )
		return size;
	nl->regs->RLINE = tmp;
	return size;
}
static CLASS_DEVICE_ATTR(mx_rline,0644,show_mx_rline,store_mx_rline);

//--------------- MXCR rgister

// MX enable
static ssize_t
show_mx_enable(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	
	return snprintf(buf,PAGE_SIZE,"%s",(nl->regs->MXCR & MXEN) ? "1" : "0");
}

static ssize_t
store_mx_enable( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);

	// check parameters
	if( !size) return size;
	
	switch( buf[0] ){
	case '0':
		nl->regs->MXCR &= (~MXEN);
		break;
	case '1':
		nl->regs->MXCR |= MXEN;
		break;
	default:
		break;
	}
	return size;
}
static CLASS_DEVICE_ATTR(mx_enable,0644,show_mx_enable,store_mx_enable);

// CLKM
static ssize_t
show_mx_clkm(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	
	return snprintf(buf,PAGE_SIZE,"%s",(nl->regs->MXCR & CLKM) ? "1" : "0");
}

static ssize_t
store_mx_clkm( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);

	// check parameters
	if( !size) return size;
	
	switch( buf[0] ){
	case '0':
		nl->regs->MXCR &= (~CLKM);
		break;
	case '1':
		nl->regs->MXCR |= CLKM;
		break;
	default:
		break;
	}

	return size;
}
static CLASS_DEVICE_ATTR(mx_clkm,0644,show_mx_clkm,store_mx_clkm);

// CLKAB
static ssize_t
show_mx_clkab(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	
	return snprintf(buf,PAGE_SIZE,"%s",(nl->regs->MXCR & CLKAB) ? "1" : "0");
}

static ssize_t
store_mx_clkab( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);

	// check parameters
	if( !size) return size;
	
	switch( buf[0] ){
	case '0':
		nl->regs->MXCR &= (~CLKAB);
		break;
	case '1':
		nl->regs->MXCR |= CLKAB;
		break;
	default:
		break;
	}
	return size;
}
static CLASS_DEVICE_ATTR(mx_clkab,0644,show_mx_clkab,store_mx_clkab);

// CLKR
static ssize_t
show_mx_clkr(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	
	return snprintf(buf,PAGE_SIZE,"%s",(nl->regs->MXCR & CLKR) ? "1" : "0");
}

static ssize_t
store_mx_clkr( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);    
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);

	// check parameters
	if( !size)
		return size;
	switch( buf[0] ){
	case '0':
		nl->regs->MXCR &= (~CLKR);
		break;
	case '1':
		nl->regs->MXCR |= CLKR;
		break;
	default:
		break;
	}

	return size;
}
static CLASS_DEVICE_ATTR(mx_clkr,0644,show_mx_clkr,store_mx_clkr);


//---------------------------- Power ------------------------------------- //
// PWRR rgister

// PWRON 
static ssize_t
show_pwron(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);

	return snprintf(buf,PAGE_SIZE,"%s",(nl->regs->PWRR & PWRON) ? "1" : "0");
}

static ssize_t
store_pwron(struct class_device *cdev,const char *buf, size_t size )
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);

	// check parameters
	if( !size)
		return size;

	switch( buf[0] ){
	case '0':
		nl->regs->PWRR &= (~PWRON);
		break;
	case '1':
		nl->regs->PWRR |= PWRON;
		break;
	default:
		break;
	}
	return size;
}
static CLASS_DEVICE_ATTR(pwron,0644,show_pwron,store_pwron);

// OVL
static ssize_t
show_pwrovl(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);

	return snprintf(buf,PAGE_SIZE,"%s",(nl->regs->PWRR & OVL) ? "1" : "0");
}
static CLASS_DEVICE_ATTR(pwrovl,0444,show_pwrovl,NULL);


//UNB
static ssize_t
show_pwrunb(struct class_device *cdev, char *buf) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);

	return snprintf(buf,PAGE_SIZE,"%s",(nl->regs->PWRR & UNB) ? "1" : "0");
}
static CLASS_DEVICE_ATTR(pwrunb,0444,show_pwrunb,NULL);

// ------------------------- DEBUG ---------------------------------------- //

#ifdef DEBUG_SYSFS
// debug_verbosity
static ssize_t
store_debug_on( struct class_device *cdev,const char *buf, size_t size ) 
{
	//        struct net_device *ndev = to_net_dev(cdev);
	//	struct net_local *nl = netdev_priv(ndev);
	// if interface is up 
	if( !size )	return size;
	if( buf[0] == '1' ){
		debug_eoc = 0;
		debug_link = 0;
		debug_sdfe4 = 0;
		debug_sci = 0;
	}else{
		debug_eoc = 40;
		debug_sdfe4 = 40;
		debug_sci = 40;
		debug_link = 40;
	}
	return size;
}
static CLASS_DEVICE_ATTR(debug_on, 0200 ,NULL,store_debug_on);


// hdlc registers
static ssize_t
show_sg17_regs(struct class_device *cdev, char *buf) 
{                                                                       
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);

	return snprintf(buf,PAGE_SIZE,  "CRA(%02x),CRB(%02x),SR(%02x),IMR(%02x)\n"
					"CTDR(%02x),LTDR(%02x),CRDR(%02x),LRDR(%02x)\n"
					"RATE(%02x)\n",
					nl->regs->CRA,nl->regs->CRB,nl->regs->SR,nl->regs->IMR,
					*nl->tx.CxDR,*nl->tx.LxDR,*nl->rx.CxDR,*nl->rx.LxDR,
					nl->regs->RATE);
}

static ssize_t
store_sg17_regs( struct class_device *cdev,const char *buf, size_t size )
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = (struct net_local *)netdev_priv(ndev);
	u8 tmp;
	char *endp;
	PDEBUG(0,"buf[0]=%d",buf[0]);
	if( !size ) return 0;
	PDEBUG(0,"buf[0]=%d, %c",buf[0],buf[0]);
	if( buf[0] < '0' || buf[0] > '8' )
		return size;
	tmp=simple_strtoul( buf+2,&endp,16) & 0xff;
	PDEBUG(0,"buf[0]=%d, tmp=%02x, terget=%08x",buf[0],(u32)tmp,(u32)((u8*)nl->regs + buf[0]));	
	*((u8*)nl->regs + buf[0]) = tmp;
	return size;
}

static CLASS_DEVICE_ATTR(regs,0644,show_sg17_regs,store_sg17_regs);


// set|unset loopback
static ssize_t
store_loopback( struct class_device *cdev,const char *buf, size_t size ) 
{
	struct net_device *ndev = to_net_dev(cdev);
	struct net_local *nl = netdev_priv(ndev);
	// if interface is up 
	if( !size )	return size;
	if( buf[0] == '1' )
		nl->regs->CRA |= DLBK;
	else
		nl->regs->CRA &= (~DLBK);
	return size;
}
static CLASS_DEVICE_ATTR(loopback, 0200 ,NULL,store_loopback);

#endif // DEBUG_SYSFS

// ------------------------------------------------------------------------ //
static struct attribute *sg17_attr[] = {
// shdsl
&class_device_attr_chipver.attr,
&class_device_attr_pwr_source.attr,
&class_device_attr_mode.attr,
&class_device_attr_annex.attr,
&class_device_attr_rate.attr,
&class_device_attr_tcpam.attr,
&class_device_attr_pbo_mode.attr,
&class_device_attr_pbo_val.attr,
&class_device_attr_clkmode.attr,
&class_device_attr_apply_cfg.attr,
// EOC
&class_device_attr_eoc.attr,	
// HDLC
&class_device_attr_crc16.attr,
&class_device_attr_fill_7e.attr,
&class_device_attr_inv.attr,
// PCI
&class_device_attr_rburst.attr,
&class_device_attr_wburst.attr,
// net device
&class_device_attr_maddr.attr,
// statistics
&class_device_attr_statistics.attr,
&class_device_attr_statistics_row.attr,
&class_device_attr_link_state.attr,
// power supply
&class_device_attr_pwron.attr,
&class_device_attr_pwrovl.attr,
&class_device_attr_pwrunb.attr,
// advanced link check
&class_device_attr_advlink.attr,
// debug
#ifdef DEBUG_SYSFS
&class_device_attr_debug_on.attr,	
&class_device_attr_regs.attr,
&class_device_attr_loopback.attr,
#endif // DEBUG_SYSFS

NULL
};


static struct attribute *sg17_mx_attr[] = {
// multiplexing
&class_device_attr_mxrate.attr,
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


static struct attribute_group sg17_group = {
.name  = "sg_private",
.attrs  = sg17_attr,
};

static struct attribute_group sg17_mx_group = {
.name  = "sg_multiplexing",
.attrs  = sg17_mx_attr,
};

int
sg17_sysfs_register(struct net_device *ndev)
{
	struct class_device *class_dev = &(ndev->class_dev);	
	struct net_local *nl=(struct net_local *)netdev_priv(ndev);
	static char fname[10] = "device";

	int ret = sysfs_create_group(&class_dev->kobj, &sg17_group);
	ret += sysfs_create_group(&class_dev->kobj, &sg17_mx_group);
    sysfs_create_link( &(class_dev->kobj),&(nl->dev->kobj),fname);
	return ret;
}

void
sg17_sysfs_remove(struct net_device *ndev){
	struct class_device *class_dev = &(ndev->class_dev);	
	sysfs_remove_group(&class_dev->kobj, &sg17_group);
	sysfs_remove_group(&class_dev->kobj, &sg17_mx_group);
    sysfs_remove_link(&(class_dev->kobj),"device");
}
