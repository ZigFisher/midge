/* mr17s_debug.c
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

int debug_xmit = 40;
int debug_recv = 40;
int debug_irq = 40;
int debug_init = 40;
int debug_tty = 0;
int debug_hw = 0;
int debug_error = 40;
int debug_sysfs = 40;
