/* mr17s_main.h
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
#ifndef MR17S_MAIN_H
#define MR17S_MAIN_H

#include "mr17s_version.h"
#include "mr17s.h"

// Module initialization
static int  __devinit mr17s_init( void );
static void  __devexit mr17s_exit( void );
// PCI initialization
static int __devinit mr17s_init_one(struct pci_dev *pdev,const struct pci_device_id *ent);
static void __devexit mr17s_remove_one( struct pci_dev *pdev );

// UART functions
static void mr17s_stop_tx(struct uart_port *port);
static void mr17s_start_tx(struct uart_port *port);
static void mr17s_stop_rx(struct uart_port *port);
static void mr17s_enable_ms(struct uart_port *port);
static unsigned int mr17s_get_mctrl(struct uart_port *port);
static void mr17s_set_mctrl(struct uart_port *port, unsigned int mctrl);
static void mr17s_break_ctl(struct uart_port *port, int break_state);
static void mr17s_set_termios(struct uart_port *port,struct termios *new,struct termios *old);
static int mr17s_startup(struct uart_port *port);
static void mr17s_shutdown(struct uart_port *port);
static const char *mr17s_type(struct uart_port *port);
static void mr17s_release_port(struct uart_port *port);
static int mr17s_request_port(struct uart_port *port);
static void mr17s_config_port(struct uart_port *port, int flags);
static int mr17s_verify_port(struct uart_port *port, struct serial_struct *ser);
static unsigned int mr17s_tx_empty(struct uart_port *port);
static int mr17s_ioctl(struct uart_port *port, unsigned int cmd, unsigned long arg);


static int mr17s_mux_get(struct uart_port *port,struct mxsettings *set);
static int mr17s_mux_set(struct uart_port *port,struct mxsettings *set);
static int mr17s_hw_get(struct uart_port *port,struct hwsettings *set);
static int mr17s_hw_set(struct uart_port *port,struct hwsettings *set);
static irqreturn_t mr17s_interrupt(int irq,void *dev_id,struct pt_regs *regs);
static int mr17s_port_interrupt(struct uart_port *port, struct pt_regs *regs);
static inline void mr17s_modem_status(struct uart_port *port);
static void mr17s_recv_bytes(struct uart_port *port,struct pt_regs *regs);
static void mr17s_xmit_bytes(struct uart_port *port);
static void mr17s_drop_bytes(struct uart_port *port);

static int count_divs(int baud,int *div1,int *div2);


/*
struct tty_driver *mr17s_console_device(struct console *co, int *index);
static int __init mr17s_console_init(void);
static int __init mr17s_console_setup(struct console *con, char *options);
static void mr17s_console_write(struct console *con, const char *s,unsigned int count);
*/

#endif
