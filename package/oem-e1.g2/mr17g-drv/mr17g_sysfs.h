/* mr17g_sysfs.h
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
#ifndef MR17G_SYSFS_H
#define MR17G_SYSFS_H

int mr17g_netsysfs_register(struct net_device *ndev);
void mr17g_netsysfs_remove(struct net_device *ndev);


#endif
