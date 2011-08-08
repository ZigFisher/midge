/****************************************************************************
                  Copyright © 2007  SIGRAND 
                 // St. Martin Strasse 53; 81669 Munich, Germany

   THE DELIVERY OF THIS SOFTWARE AS WELL AS THE HEREBY GRANTED NON-EXCLUSIVE,
   WORLDWIDE LICENSE TO USE, COPY, MODIFY, DISTRIBUTE AND SUBLICENSE THIS
   SOFTWARE IS FREE OF CHARGE.

   THE LICENSED SOFTWARE IS PROVIDED "AS IS" AND SIGRAND EXPRESSLY DISCLAIMS
   ALL REPRESENTATIONS AND WARRANTIES, WHETHER EXPRESS OR IMPLIED, INCLUDING
   WITHOUT LIMITATION, WARRANTIES OR REPRESENTATIONS OF WORKMANSHIP,
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, DURABILITY, THAT THE
   OPERATING OF THE LICENSED SOFTWARE WILL BE ERROR FREE OR FREE OF ANY
   THIRD PARTY CLAIMS, INCLUDING WITHOUT LIMITATION CLAIMS OF THIRD PARTY
   INTELLECTUAL PROPERTY INFRINGEMENT.

   EXCEPT FOR ANY LIABILITY DUE TO WILLFUL ACTS OR GROSS NEGLIGENCE AND
   EXCEPT FOR ANY PERSONAL INJURY INFINEON SHALL IN NO EVENT BE LIABLE FOR
   ANY CLAIM OR DAMAGES OF ANY KIND, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ****************************************************************************
   Module      : drv_sgatab.c
   Description : This file contains the implementation of the pci callbacks and
                 init / exit driver functions.
*******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/pci.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>

#include "ab_ioctl.h"
#include "../tapi/include/ifx_types.h"
#include "../vinetic/include/vinetic_io.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

#define PCI_VENDOR_ID_SIGRAND 	0x0055
#define PCI_DEVICE_ID_MR17VOIP8 0x009C

#define MR17VOIP8_ACCESS_MODE 	VIN_ACCESS_PARINTEL_MUX8
#define MR17VOIP8_DEV_MEM_OFFSET 256

#define MIN_BOARD_SLOT 2

#define DEV_TYPE_MASK    0x3
#define DEV_TYPE_LENGTH    2

#define DEV_NAME "sgatab"

MODULE_DESCRIPTION("SGATAB driver");
MODULE_AUTHOR("Vladimir Luchko <vlad.luch@mail.ru>");
MODULE_LICENSE("GPL");

/* ============================= */
/* Global variable definition    */
/* ============================= */

struct ab_board_dev_s {
	unsigned int idx;
	struct pci_dev * pci_dev;
	unsigned char pci_device_enabled;
	unsigned char slot;
	u16 sub_id;
	unsigned long mem_addr;
	unsigned char first_chan_idx;
};

static struct ab_boards_manager_s {
	int free_board_idx; /* -1 - no free places */
	unsigned char boards_count;
	unsigned char is_count_changed;
	struct ab_board_dev_s boards [BOARDS_MAX];
	spinlock_t lock; 
} g_boardsman;

static struct cdev g_cdev;

/* ============================= */
/* Global function declaration   */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* operating system common methods */

/* Local helper functions */
static void set_free_board_idx ( void );
static int  chardev_init  ( void );
static void chardev_undef ( void );
static int  pci_init      ( struct ab_board_dev_s * c_brd );
static void pci_undef     ( struct ab_board_dev_s * c_brd );

static int SGATAB_boards_count( unsigned long user_data );
static int SGATAB_board_params( unsigned long user_data );

int SGATAB_Ioctl(struct inode *inode, struct file *filp,
		unsigned int nCmd, unsigned long nArgument );

/* Driver callbacks */
static struct file_operations sgatab_fops = 
{	.owner = THIS_MODULE,
	.ioctl = SGATAB_Ioctl, 
};

static int  __devinit sgatab_probe (struct pci_dev *pci_dev, 
		const struct pci_device_id *pci_id);
static void __devexit sgatab_remove(struct pci_dev *pci_dev);

/* ============================= */
/* Local variable definition     */
/* ============================= */

static struct pci_device_id sgatab_pci_tbl[] __devinitdata = {
	{PCI_DEVICE (PCI_VENDOR_ID_SIGRAND, PCI_DEVICE_ID_MR17VOIP8)},
	{0,},
};

MODULE_DEVICE_TABLE(pci, sgatab_pci_tbl);

static struct pci_driver sgatab_pci_driver = {
      .name     = "sgatab",
      .id_table = sgatab_pci_tbl,
      .probe    = sgatab_probe,
      .remove   = __devexit_p (sgatab_remove),
};

/* ============================= */
/* Local function definition     */
/* ============================= */

/****************************************************************************
Description:
   Configuration / Control for the device.
Arguments:
   inode - pointer to the inode
   filp  - file pointer to be opened
   nCmd  - IoCtrl
   nArgument - additional argument
Return:
   0 or error code
Remarks:
   None.
****************************************************************************/
int SGATAB_Ioctl(struct inode *inode, struct file *filp, unsigned int nCmd, 
		unsigned long nArgument)
{
	int err;
	switch (nCmd) {
		case SGAB_GET_BOARDS_COUNT:
			err = SGATAB_boards_count (nArgument);
			break;
		case SGAB_GET_BOARD_PARAMS:
			err = SGATAB_board_params (nArgument);
			break;
		default:
			printk(KERN_WARNING "%s: Unknown IOCTL command [%d]\n",
					DEV_NAME, nCmd);
			err = -1;
			break;
	}
	return err;
}

static void 
set_free_board_idx ( void )
{
	if( g_boardsman.boards_count == BOARDS_MAX){
		g_boardsman.free_board_idx = -1;
	} else {
		int i;
		for(i=0; i<BOARDS_MAX; i++){
			if( !g_boardsman.boards[i].pci_dev){
				g_boardsman.free_board_idx = i;
				break;
			}
		}
	}
}

static int 
chardev_init( void )
{
	dev_t dev;
	int err;

	g_cdev.owner = THIS_MODULE;

	err = alloc_chrdev_region( &g_cdev.dev, 0, 1, DEV_NAME );
	dev = g_cdev.dev;
	if ( err ){
		printk(KERN_ERR "%s: ERROR : Could not allocate "
				"character device number. EXIT\n", DEV_NAME);
		goto ___exit;
	}

	cdev_init ( &g_cdev, &sgatab_fops );
	err = cdev_add ( &g_cdev, dev, 1 );
	if ( err ) {
		printk(KERN_ERR "%s: ERROR : cdev_add() fails. EXIT\n", 
				DEV_NAME);
		goto ___region;
	}
	g_cdev.dev = dev;

	return 0;

___region:
	unregister_chrdev_region(dev, 1);
___exit:
	return err;
}

static void 
chardev_undef( void )
{
	dev_t dev = g_cdev.dev;
	cdev_del ( &g_cdev );
	unregister_chrdev_region ( dev, 1 );
}

static int 
pci_init( struct ab_board_dev_s * c_brd )
{
	int err;

	c_brd->slot = PCI_SLOT(c_brd->pci_dev->devfn);
	printk(KERN_INFO "%s:(0x%x:0x%x) board found in slot [%d]\n", 
			DEV_NAME, c_brd->pci_dev->vendor,
			c_brd->pci_dev->device, c_brd->slot);

	c_brd->mem_addr = pci_resource_start(c_brd->pci_dev, 0);

	if (pci_enable_device(c_brd->pci_dev)) {
		printk(KERN_ERR "%s: ERROR: can`t enable PCI device, "
				"0x%x:0x%x on slot %d. EXIT.\n",
				DEV_NAME, c_brd->pci_dev->vendor, 
				c_brd->pci_dev->device, c_brd->slot);
		err = -EIO;
		goto pci_init_exit;
	}
	c_brd->pci_device_enabled = 1;

	pci_read_config_word (c_brd->pci_dev, PCI_SUBSYSTEM_ID, 
			&(c_brd->sub_id));
	/* Becouse slot always starts from [2] (MIN_BOARD_SLOT) we always 
	 * got channels absolute numbers from 17, 
	 * thats not good - we wont it from zero.
	c_brd->first_chan_idx = c_brd->slot * DEVS_PER_BOARD_MAX *
			CHANS_PER_DEV + 1;
	*/
	c_brd->first_chan_idx = (c_brd->slot - MIN_BOARD_SLOT) * 
			DEVS_PER_BOARD_MAX * CHANS_PER_DEV;
	printk(KERN_INFO "%s: id=%x at bus - 0x%02x func - 0x%x\n", 
			DEV_NAME, c_brd->pci_dev->device, 
			c_brd->pci_dev->bus->number,
			PCI_FUNC(c_brd->pci_dev->devfn));
	printk(KERN_INFO "%s: irq %d, subsystem id 0x%x, memory addr 0x%lx\n",
			DEV_NAME, c_brd->pci_dev->irq, c_brd->sub_id, 
			c_brd->mem_addr);

	return 0;
pci_init_exit:
	return err;
}

static void 
pci_undef ( struct ab_board_dev_s * c_brd )
{
	/* disable the PCI device if it was enabled */
	if( c_brd->pci_device_enabled ) {
		pci_disable_device ( c_brd->pci_dev );
		c_brd->pci_device_enabled = 0;
	}
	printk(KERN_INFO "%s: %s()\n", DEV_NAME, __func__);
}


static int 
SGATAB_boards_count( unsigned long user_data )
{
	int bc;

	spin_lock (g_boardsman.lock);
	bc = g_boardsman.boards_count;
	g_boardsman.is_count_changed = 0;
	spin_unlock (g_boardsman.lock);

	if(copy_to_user((void *)user_data, &bc, sizeof(bc))){
		printk(KERN_ERR "%s: ERROR : copy_to_user(...) failed\n", 
				DEV_NAME );
		goto ___exit_fail;
	}
	return 0;
___exit_fail:
	return -1;
}

static int 
SGATAB_board_params (unsigned long user_data)
{
	ab_board_params_t bp;
	struct ab_board_dev_s * c_brd = NULL;
	unsigned long base_addr;
	unsigned char chan_idx;
	int idx_in_drv;
	int cur_idx;
	int i = 0;
	int j = 0;
	int err = 0;

	if(copy_from_user (&bp, (void *)user_data, sizeof(bp))){
		printk(KERN_ERR "%s: ERROR : copy_from_user(...) failed\n", 
				DEV_NAME );
		err = -1;
		goto ___exit;
	}

	spin_lock (g_boardsman.lock);

	for(idx_in_drv=-1,cur_idx=0,i=0; i<BOARDS_MAX; i++){
		if(g_boardsman.boards[i].pci_dev){
			if(cur_idx == bp.board_idx){
				idx_in_drv = i;
				break;
			}
			cur_idx++;
		} 
	}

	if (idx_in_drv==-1){
		bp.is_present = 0;
		err = 0; 
		goto ___copy_and_exit;
	}

	c_brd = &g_boardsman.boards [idx_in_drv];

	/* set bp values */
	bp.is_present = 1;
	bp.is_count_changed = g_boardsman.is_count_changed;
	bp.nIrqNum = c_brd->pci_dev->irq;

	base_addr = c_brd->mem_addr;
	chan_idx = c_brd->first_chan_idx;
	for(i=0; i<DEVS_PER_BOARD_MAX; i++){
		ab_dev_params_t * cd = &bp.devices[i];
		cd->type = (c_brd->sub_id>>(i*DEV_TYPE_LENGTH))&DEV_TYPE_MASK;
		cd->nBaseAddress = base_addr;
		cd->AccessMode = MR17VOIP8_ACCESS_MODE;
		for(j=CHANS_PER_DEV-1; j>=0; j--,chan_idx++){
			cd->chans_idx[j] = chan_idx;
		}
		base_addr += MR17VOIP8_DEV_MEM_OFFSET;
	}

___copy_and_exit:
	if(copy_to_user((void *)user_data, &bp, sizeof(bp))){
		printk(KERN_ERR "%s: ERROR : copy_to_user(...) failed\n", 
				DEV_NAME );
		goto ___exit_locked;
	}
___exit_locked:
	spin_unlock (g_boardsman.lock);
___exit:
	return err;
}

/**
   The proc filesystem: function to read an entry.
   This function provides information of proc files to the user space

   \return
   length
*/
static int 
proc_read_sgatab (char *page, char **start, off_t off,
                          int count, int *eof, void *data)
{
	int len;

	int (*fn)(char *buf);

	/* write data into the page */
	if( !data){
		len = 0;
		goto ___exit;
	}

	fn = data;
	len = fn(page);

	if (len <= off+count){
		*eof = 1;
	}
	*start = page + off;
	len -= off;
	if (len > count){
		len = count;
	}
	if (len < 0){
		len = 0;
	}

___exit:
	/* return the data length  */
	return len;
}

/**
  Read the channels information

  \return
  length

*/
static int 
proc_get_sgatab_channels(char *buf)
{
	int i;
	int j;
	int k;
	int len = 0;
	struct ab_board_dev_s * c_brd = NULL;

	spin_lock(g_boardsman.lock);

	for (i=0; i<BOARDS_MAX; i++){
		c_brd = &g_boardsman.boards[i];
		if( !c_brd->pci_dev ){
			continue;
		}
		for (j=0; j<DEVS_PER_BOARD_MAX; j++){
			dev_type_t dev_type = (c_brd->sub_id>>(j*DEV_TYPE_LENGTH)) & 
					DEV_TYPE_MASK;
			unsigned char chan_idx = c_brd->first_chan_idx + 
					j * CHANS_PER_DEV;
			for (k=0; k<CHANS_PER_DEV; k++){
				chan_idx +=k;
				switch (dev_type){
				case dev_type_ABSENT:
				break;
				case dev_type_FXO:
					len += sprintf(buf+len, "%02d:FXO\n", chan_idx);
				break;
				case dev_type_FXS:
					len += sprintf(buf+len, "%02d:FXS\n", chan_idx);
				break;
				case dev_type_VF:
					len += sprintf(buf+len, "%02d:VF\n", chan_idx);
				break;
				}
			}
		}
	}
	spin_unlock (g_boardsman.lock);
	return len;
}

/**
  Read the major number information

  \return
  length

*/
static int 
proc_get_sgatab_major(char *buf)
{
	int len = 0;
	len += sprintf(buf+len, "%d\n", MAJOR(g_cdev.dev));
	return len;
}

/**
   Initialize and install the proc entry

\return
   -1 or 0 on success
\remark
   Called by the kernel.
*/
static int 
proc_install_sgatab_entries( void )
{
	struct proc_dir_entry *driver_proc_node;

	/* install the proc entry */
	driver_proc_node = proc_mkdir( "driver/" DEV_NAME, NULL);
	if (driver_proc_node != NULL){
		create_proc_read_entry( "channels" , S_IFREG|S_IRUGO,
				     driver_proc_node, proc_read_sgatab,
				     (void *)proc_get_sgatab_channels );
		create_proc_read_entry( "major" , S_IFREG|S_IRUGO,
				     driver_proc_node, proc_read_sgatab,
				     (void *)proc_get_sgatab_major );
	} else {
		printk(KERN_ERR "%s: ERROR : can`t register PROC entries!\n", 
				DEV_NAME);
		return -1;
	}

	return 0;
}

/*******************************************************************************
Description:
   This function calling then PCI device probing.
Arguments:
   pci_dev   - pointer to pci_dev structure of probing device
   pci_id    - pointer to pci_device_id structure of probing device
Return Value:
   0 if ok / -x in error case
Remarks:
   Called by the PCI core.
*******************************************************************************/
static int __devinit 
sgatab_probe (struct pci_dev *pci_dev, const struct pci_device_id *pci_id) 
{
	struct ab_board_dev_s * c_brd = NULL;
	int err = 0;

	/* locks the boards struct for proper use */
	spin_lock (g_boardsman.lock);

	if(g_boardsman.free_board_idx==-1){
		printk(KERN_ERR "%s: ERROR: driver limit is [%d] pci devices, "
				"you try to exceed it\n",
				DEV_NAME, BOARDS_MAX);
		err = -1;
		goto ___unlock_and_exit;
	}
	c_brd = &g_boardsman.boards[g_boardsman.free_board_idx];
	c_brd->idx = g_boardsman.free_board_idx;
	c_brd->pci_dev = pci_dev;
	c_brd->pci_device_enabled = 0;
	g_boardsman.boards_count++;
	g_boardsman.is_count_changed = 1;

	err = pci_init (c_brd);
	if( err ) {
		goto ___exit;
	}

	pci_set_drvdata (pci_dev, c_brd);
	set_free_board_idx();

___unlock_and_exit:
	/* unlocks the boards struct */
	spin_unlock (g_boardsman.lock);
	return err;
___exit:
	pci_undef (c_brd);
	memset(c_brd, 0, sizeof(*c_brd));

	g_boardsman.boards_count--;
	/* unlocks the boards struct */
	spin_unlock (g_boardsman.lock);
	return err;
}

/*******************************************************************************
Description:
   This function clean up all data and memory after removing PCI device.
Arguments:
   pci_dev   - pointer to pci_dev structure of removed device
Return Value:
   none
Remarks:
   Called by the PCI core.
*******************************************************************************/
static void __devexit 
sgatab_remove(struct pci_dev *pci_dev)
{
	struct ab_board_dev_s * c_brd = NULL;
	c_brd = pci_get_drvdata (pci_dev);
	if(c_brd){
		spin_lock (g_boardsman.lock);

		g_boardsman.boards_count--;
		g_boardsman.is_count_changed = 1;
		if(g_boardsman.free_board_idx > c_brd->idx){
			g_boardsman.free_board_idx = c_brd->idx;
		}
		pci_undef (c_brd);
		memset(c_brd, 0, sizeof(*c_brd));
		pci_set_drvdata (pci_dev, NULL);

		spin_unlock (g_boardsman.lock);
	}
}

/****************************************************************************
Description:
   Initialize the module.
Arguments:
   None.
Return Value:
   0 if ok / -x in error case
Remarks:
   Called by the kernel.
   Register the PCI driver.
****************************************************************************/
static int __init 
sgatab_module_init(void)
{
	int err;

	memset(&g_boardsman, 0, sizeof(g_boardsman));
	g_boardsman.lock = SPIN_LOCK_UNLOCKED;
	g_boardsman.is_count_changed = 0;

	/* Register the PCI driver */
	err = pci_register_driver ( &sgatab_pci_driver );
	if ( err < 0 ){
		printk(KERN_ERR "%s: ERROR : Loading module ERROR: can`t "
				"register PCI driver!\n", DEV_NAME);
		goto ___exit;
	}

	err = chardev_init ();
	if (err) {
		goto ___exit;
	}

	err = proc_install_sgatab_entries ();
	if (err){
		goto ___exit;
	}

___exit:
	return err;
}

/****************************************************************************
Description:
   Clean up the module if unloaded.
Arguments:
   None.
Return Value:
   None.
Remarks:
   Called by the kernel.
   Unregister the PCI driver.
****************************************************************************/
static void __exit 
sgatab_module_exit(void)
{
	pci_unregister_driver( &sgatab_pci_driver );
	printk(KERN_INFO "%s: Removed.\n", 
	DEV_NAME);

	remove_proc_entry("driver/" DEV_NAME "/channels" ,0);
	remove_proc_entry("driver/" DEV_NAME "/major" ,0);
	remove_proc_entry("driver/" DEV_NAME, 0);

	chardev_undef ();
}

module_init(sgatab_module_init);
module_exit(sgatab_module_exit);

