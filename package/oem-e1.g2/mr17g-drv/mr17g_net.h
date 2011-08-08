/* mr17g_net.h
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

#include "mr17g.h"

// mr16g_ioctl
#define SIOCGLRATE	(SIOCDEVPRIVATE+14)

int __devinit mr17g_net_init(struct mr17g_chip *chip);
int __devexit mr17g_net_uninit(struct mr17g_chip *chip);
void mr17g_transceiver_setup(struct mr17g_channel *ch);
void mr17g_net_link(struct net_device *ndev);
// TODO: uncomment static
/*static */int mr17g_start_xmit( struct sk_buff *skb, struct net_device *ndev );
