/* mr17s_sysfs.h
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
#ifndef MR17S_SYSFS_H
#define MR17S_SYSFS_H

#define ADDIT_ATTR
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12)
#undef ADDIT_ATTR
#define ADDIT_ATTR struct device_attribute *attr,
#endif    

int mr17s_sysfs_register(struct device *dev);
void mr17s_sysfs_free(struct device *dev);

#endif

