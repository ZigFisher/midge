/* dslan_gpio.c:  Sigrand DSLAM control driver for linux (kernel 2.6.x)
 *
 *	Written 2009 by Artem Y. Polyakov <artpol84@gmail.com>
 *
 *	This driver pprovides control of DSLAM swiches for Sigrand SG-17 routers 
 *
 *	This software may be used and distributed according to the terms
 *	of the GNU General Public License.
 *
 *
 *	03.09.09	Version 1.0 - Artem Y. Polyakov <art@sigrand.ru>
 */
 
#include <linux/module.h>
#include <linux/config.h>
#include <linux/init.h>
#include <asm/am5120/adm5120.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>


#define DEBUG_ON
#define DEFAULT_LEV 10
#include "sg_debug.h"


/* --------------------------------------------------------------------------
 *      IC+ entry point
 * -------------------------------------------------------------------------- */


struct dslam_env{
    unsigned long io_outen;
    unsigned long io_imask;
    unsigned long io_omask;
    unsigned long c_outen;
    unsigned long c_imask;
    unsigned long c_omask;
	unsigned long regnum;
};


struct dslam_env chips[] = {
    { GPIO6_OUTPUT_EN,GPIO6_INPUT_MASK,GPIO6_OUTPUT_HI,GPIO7_OUTPUT_EN,GPIO7_INPUT_MODE,GPIO7_OUTPUT_HI, 0xf2},
    { GPIO1_OUTPUT_EN,GPIO1_INPUT_MASK,GPIO1_OUTPUT_HI,GPIO0_OUTPUT_EN,GPIO0_INPUT_MODE,GPIO0_OUTPUT_HI, 0xf2}
};

#define chips_num (sizeof(chips)/sizeof(struct dslam_env))

unsigned long ic_readpreamble = 0x58;
unsigned char ic_readpreamble_sz = 7; // 7 bit wide
unsigned long ic_writepreamble = 0x54;
unsigned char ic_writepreamble_sz = 7; // 7 bit wide
unsigned long ic_pause = 0x2;
unsigned char ic_pause_sz = 2; // 2 bit wide

static inline void
enable_bits(unsigned long bits){
	ADM5120_SW_REG(GPIO_conf0_REG) |= bits;
}

static inline void
disable_bits(unsigned long bits){
	ADM5120_SW_REG(GPIO_conf0_REG) &= (~bits);
}

static inline unsigned char
read_bit(unsigned long bit){
	if( ADM5120_SW_REG(GPIO_conf0_REG) & bit )
		return 1;
	else
		return 0;
}


static inline void
dslam_write_mode(struct dslam_env *env)
{
    // enable write on GPIO: MDIO=1(write mode) MDC=1 (write mode)
    enable_bits(env->io_outen | env-> io_omask | env->c_outen | env->c_omask);
}

static inline void 
dslam_read_mode(struct dslam_env *env)
{
    // enable read on GPIO: MDIO (read mode)
    disable_bits(env->io_outen);
    enable_bits(env->c_outen | env->c_omask);
}

static inline void
dslam_empty_clock(struct dslam_env *env)
{
    // Empty clock ticks between read & write
	disable_bits(env->c_omask);
	udelay(1);
	enable_bits(env-> c_omask);
	udelay(1);
	disable_bits(env->c_omask);
	udelay(1);
	enable_bits(env-> c_omask);
	udelay(1);
}



static void
dslam_write_bits(struct dslam_env *env, unsigned short outdata,unsigned char outbits)
{
	int i;
//	PDEBUG(0,"start");
	udelay(1);
	for(i=outbits-1;i>=0;i--){
		if( (outdata>>i) & 0x1 ){
//			PDEBUG(0,"write 1 to IC+");
			enable_bits(env->io_omask);
		}else{
//			PDEBUG(0,"write 0 to IC+");
			disable_bits(env->io_omask);
		}
		disable_bits(env->c_omask);
		udelay(1);
		enable_bits(env-> c_omask);
		udelay(1);
	}
//	PDEBUG(0,"end");
}


static void
dslam_read_bits(struct dslam_env *env, unsigned long *indata,unsigned char inbits)
{
	int i;
	unsigned long in = 0;
//	PDEBUG(0,"start");
	udelay(1);
	printk(KERN_ERR"read_bits: ");
	for(i=0;i<inbits;i++){
		disable_bits(env->c_omask);
		udelay(1);
		in |= read_bit(env->io_imask);
		printk("%d",in&0x1);
		enable_bits(env-> c_omask);
//		PDEBUG(0,"Read bit=%d from IC+",in & 0x1);
		in <<= 1;
		udelay(1);
	}
	printk("\n");
	// remove last shift
	in >>= 1;
	*indata = in;
//	PDEBUG(0,"end");
}

static int 
read_reg_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	int sw_num = *(short*)data;
	struct dslam_env *env;
	unsigned long reg_val = 0;
	
	env = &chips[0];

	if( sw_num > chips_num ){
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	
	env = &chips[sw_num];
	

	PDEBUG(0,"write mode\n");
	dslam_write_mode(env);
	PDEBUG(0,"write preamble bits\n");
	dslam_write_bits(env,ic_readpreamble,ic_readpreamble_sz);	
	PDEBUG(0,"write reg addr\n");
	dslam_write_bits(env,env->regnum,8);	
	PDEBUG(0,"read mode\n");
	dslam_read_mode(env);
	PDEBUG(0,"read wait after write\n");
	dslam_empty_clock(env);
	PDEBUG(0,"read bits\n");
	dslam_read_bits(env,&reg_val,16);
	PDEBUG(0,"return to write mode\n");
	dslam_write_mode(env);
	
	return snprintf(buf,count,"0x%02lx 0x%04lx",env->regnum,reg_val);
}

static int
store_reg_read(struct file *file,const char *buffer,unsigned long count,void *data)
{
	char *ptr=(char*)buffer;
	int sw_num = *(short*)data;
	struct dslam_env *env;
	char *endp;
	unsigned long reg_num;

	if( sw_num > chips_num ){
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	env = &chips[sw_num];
	
	ptr[count-1] = '\0';
	reg_num = simple_strtoul(ptr,&endp,16);
	PDEBUG(0,"save # of register to read, str=%s, reg#=0x%x",buffer,reg_num);
	env->regnum = reg_num;
	return count;
}

static int 
store_reg_write(struct file *file,const char *buffer,unsigned long count,void *data)
{
	int sw_num = *(short*)data;
	struct dslam_env *env;
	char *endp;
	unsigned long reg_num,reg_val;
	
	if( sw_num > chips_num ){
		printk(KERN_ERR"dslam: ERROR sw_num=%d, chips_num=%d\n",sw_num,chips_num);
		return count;
	}
	env = &chips[sw_num];

	reg_num = simple_strtoul(buffer,&endp,16);
	PDEBUG(40,"buf=%p, endp=%p,*endp=%c",buffer,endp,*endp);
	while( *endp == ' '){
		endp++;
	}
	reg_val = simple_strtoul(endp,&endp,16);
	PDEBUG(40,"reg_val=%d",reg_val);	


	PDEBUG(0,"write mode\n");
	dslam_write_mode(env);
	PDEBUG(0,"write preamble bits\n");
	dslam_write_bits(env,ic_writepreamble,ic_writepreamble_sz);	
	PDEBUG(0,"write reg addr\n");
	dslam_write_bits(env,reg_num,8);
	PDEBUG(0,"write ic pause bits: 10b\n");
	dslam_write_bits(env,ic_pause,ic_pause_sz);
	PDEBUG(0,"write reg val\n");
	dslam_write_bits(env,reg_val,16);
	PDEBUG(0,"reset write mode\n");
	dslam_write_mode(env);
	
	return count;
	
}

/* --------------------------------------------------------------------------
 *      PROCFS initialisation/cleanup
 * -------------------------------------------------------------------------- */

#define MAX_PROC_STR 256

#define PFS_SW0 0
#define PFS_SW1 1

char dslam_procdir[]="sys/net/dslam_gpio";
struct proc_dir_entry *dslam_entry;

struct dev_entrie{
	char *name;
	int mark;
	struct proc_dir_entry *pent;
	mode_t mode;
	read_proc_t *fread;
	write_proc_t *fwrite;
};

static int read_reg_read(char *buf, char **start, off_t offset, int count, int *eof, void *data);
static int store_reg_read(struct file *file,const char *buffer,unsigned long count,void *data);

//static int read_reg_write(char *buf, char **start, off_t offset, int count, int *eof, void *data);
static int store_reg_write(struct file *file,const char *buffer,unsigned long count,void *data);

static struct dev_entrie entries[]={
	{ "sw0_regread", PFS_SW0, NULL, 0600, read_reg_read, store_reg_read },
	{ "sw0_regwrite", PFS_SW0, NULL, 0200, NULL , store_reg_write },
	{ "sw1_regread", PFS_SW1, NULL, 0600, read_reg_read, store_reg_read },
	{ "sw1_regwrite", PFS_SW1, NULL, 0200, NULL, store_reg_write },
};

#define PFS_ENTS  (sizeof(entries)/sizeof(struct dev_entrie))

static int set_entry(struct proc_dir_entry *ent,read_proc_t *fread, write_proc_t *fwrite,int mark)
{
	short *mk;
	if( !( mk=(short *)kmalloc( sizeof( short ),GFP_KERNEL ) ) )
		return -1;
	*mk=mark;
	ent->data=(void *)mk;
	ent->owner=THIS_MODULE;
	ent->read_proc=fread;
	ent->write_proc=fwrite;
	return 0;
}


static int dslam_procfs_init(void)
{
	int i,j;
	
	ADM5120_SW_REG(GPIO_conf0_REG) = 0;
	
	dslam_entry=proc_mkdir(dslam_procdir,NULL);
	if ( dslam_entry==NULL )
		return -ENOMEM;
	PDEBUG(0,"creating entries");
	for ( i=0; i<PFS_ENTS; i++ ) {
		if ( !(entries[i].pent = create_proc_entry(entries[i].name,entries[i].mode,dslam_entry) ) )
			goto err1;
		PDEBUG(0,"file \"%s\" created successfuly",entries[i].name);			
		if ( set_entry(	entries[i].pent,entries[i].fread, entries[i].fwrite,entries[i].mark) )
			goto err1;
		PDEBUG(0,"parameters of \"%s\" setted",entries[i].name);
	}
	dslam_write_mode(&chips[0]);
	dslam_write_mode(&chips[1]);
	PDEBUG(0,"conf0=%x",ADM5120_SW_REG(GPIO_conf0_REG));

	return 0;

err1:
	PDEBUG(0,"eror creating \"%s\", abort",entries[i].name);
	for ( j=0; j<=i; j++ )
		if( entries[j].pent->data )
			kfree(entries[j].pent->data);
	for ( j=0; j<=i; j++ )
		remove_proc_entry(entries[j].name,dslam_entry);
	remove_proc_entry("",dslam_entry);
	return -1;
}        

static void dslam_procfs_remove(void)
{
	int j;
	PDEBUG(0,"start");
	for ( j=0; j<PFS_ENTS; j++ )
		if( entries[j].pent->data )
			kfree(entries[j].pent->data);

	PDEBUG(0,"1:");			
	for ( j=0; j<PFS_ENTS; j++ ){
		remove_proc_entry(entries[j].name,dslam_entry);
		PDEBUG(0,"delete %s",entries[j].name);
	}
	PDEBUG(0,"2:");
	remove_proc_entry(dslam_procdir,NULL);
	PDEBUG(0,"end");	
}        


/* --------------------------------------------------------------------------
 *      Module initialisation/cleanup
 * -------------------------------------------------------------------------- */

int __devinit
dslam_gpio_init( void ){
	printk(KERN_NOTICE"Load DSLAM control driver\n");
	return dslam_procfs_init();
}

void __devexit
dslam_gpio_exit( void ){
	printk(KERN_NOTICE"Unload DSLAM control driver\n");
	dslam_procfs_remove();
}

module_init(dslam_gpio_init);
module_exit(dslam_gpio_exit);
