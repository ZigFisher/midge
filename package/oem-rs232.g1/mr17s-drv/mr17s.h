/* mr17s.h
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

#ifndef SIGRAND_MR17S_H
#define SIGRAND_MR17S_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/random.h>
#include<linux/firmware.h>
#include <linux/vermagic.h>
#include <linux/config.h>

#include <asm/io.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>

#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/console.h>
#include <linux/serial.h>
#include <linux/serial_core.h>


// I/O memory read/write functions
#define iowrite8(val,addr)  writeb(val,addr)
#define iowrite16(val,addr)  writew(val,addr)
#define iowrite32(val,addr)  writel(val,addr)
#define ioread8(addr) readb(addr)
#define ioread16(addr) readw(addr)
#define ioread32(addr) readl(addr)

//----------------------- Types of hardware -----------//
#define MR17S_PCI_VEN 0x0055
#define MR17S_PCI_DEV 0x0096
// Differentiation between modules
#define MR17S_DTE2CH    0x00
#define MR17S_DCE2CH    0x01
#define MR17S_DTE4CH    0x02
#define MR17S_DCE4CH    0x03

//----------------------- UART parameters --------------//
#ifndef MR17S_SERIAL_NAME
#	define MR17S_SERIAL_NAME "ttyRS"
#endif
#ifndef MR17S_SERIAL_NAMEDFS
#	define MR17S_SERIAL_NAMEDFS "ttyRS%d"
#endif
#define MR17S_SERIAL_MAJOR  TTY_MAJOR
#define MR17S_SERIAL_MINORS 128
#define MR17S_UART_NR 32
#define MR17S_PORT 72
#define MR17S_UARTCLK 230400
#define MR17S_MAXBAUD MR17S_UARTCLK
#define MUX_TIMESLOT 64000

// ioctl commands
#define MXMAGIC 0xAFAF
#define TIOCGMX 0xA000 /* Get MUX configuration */
#define TIOCSMX	0xA001 /* Set MUX configuration */
#define TIOCGHW 0xA002 /* Get HW configuration */
#define TIOCSHW	0xA003 /* Set HW configuration */

struct mxsettings{
    u16 magic;
    s32 mxrate;
    s8 rline,tline;
    s16 tfs,rfs;
    s8 clkm;
    s8 clkab;
    s8 clkr;
    s8 mxen;
};

struct hwsettings{
    u16 magic;
    u8 flow_ctrl : 1;
    u8 fwd_sig : 1;
};

//---------- General memory map constants --------------//
#define MR17S_IOMEM_SIZE    4*1024 // 16 KB
#define MR17S_CHANMEM_SIZE       1024    // 1KB

//----------- IO memory structure ----------------------//

// Channel registers
struct mr17s_hw_regs{
    u8 CRA,CRB,SR,IMR,CTR,LTR,CRR,LRR;
	u8 RATE,MXRATE,TFS,RFS,TLINE,RLINE,MXCR;
};
// CRA register bits
#define TXEN    0x01 // Transmitter Enable (Always ON)
#define RXEN    0x02 //Receiver Enable (Always ON)
#define FCEN    0x04 // Flow Control Enable (RTS/CTS): Hardware flow control
#define MCFW    0x08 // Modem Control signals Forwarding (for MXEN=1 only): Transmit signals through multiplex channel
#define DTR     0x10 // DTR output (DTE), DSR output (DCE) (DTR DataTerminalReady, DataSetReady) Enable if some app work with port
#define RTS     0x20 // RTS output (DTE), CTS output (DCE) (RTS RequestToSend, CTS ClearToSend) Enable if some app work with port
#define CD      0x40 // CD output (DCE only)
#define RI      0x80 // RI output (DCE only)

// CRB register bits
#define BAUD0   0x01 //BAUD[2..0] - Baud rate: 230400/(2^BAUD[2..0]) 
#define BAUD1   0x02
#define BAUD2   0x04
#define BDIV6   0x08 // Baud rate: 0-divide by 1, 1-divide by 6 (38400/(2^BAUD[2..0]))
#define PAR0    0x10 // PAR[1..0] - Parity: 00-none, 10-even, 11-odd
#define PAR1    0x20
#define STOP2   0x40 // Stop bits: 0 - 1 stop bit, 1 - 2 stop bits
#define DATA7   0x80 // Data bits: 0 - 8 data bits, 1 - 7 data bits

// SR register bits
#define TXS     0x01 // Transmit Success
#define RXS     0x02 // Receive Success
#define RXE     0x04 // Receive Error (Receive byte error, set with RXS, can be omitted)
#define MCC     0x08 // Modem Control Signals Change (DSR, CTS, CD, RI)
#define DSR     0x10 // DSR input (DTE), DTR input (DCE) // Modem (DCE) is on (cable present)
#define CTS     0x20 // CTS input (DTE), RTS input (DCE) // Modem can receive data (Clear To Send)
#define CD      0x40 // CD input (DTE only)    // Modem is connected to remote Modem
#define RI      0x80 // RI input (DTE only)    // Remote modem is calling to local

// IMR register bits
#define TXS     0x01 // Transmit Success
#define RXS     0x02 // Receive Success
#define RXE     0x04 // Receive Error
#define MCC     0x08 // Modem Control Signals Change

// MXCR register
#define MXEN    0x01 // Multiplexer enable: 0-disabled, 1-enabled
#define CLKM    0x02 // Multiplexer Bus Clock usage: 1-Clock master, 0-slave
#define CLKAB   0x04 // Clock domain: 0 - clock A, 1 - clock B.
#define CLKR    0x08 // Not used, should be always 0.

// Channel memory map
#define RING_SIZE 256
#define RING_MASK (RING_SIZE-1)

struct mr17s_chan_iomem{
    char tx_buf[RING_SIZE];
    char rx_buf[RING_SIZE];
    struct mr17s_hw_regs regs;
    char pad[497];
};

//------------- MR17S service structures -------------------//

enum mr17s_type {DTE,DCE};

// MR17S devise data
struct mr17s_device{
    int number; // Module sequence number (depends on PCI slot & other MR17S modules)
    char name[32];
    enum mr17s_type type;
    unsigned long iomem_start,iomem_end;
    void *iomem; // iomemory pointer
    int port_quan;   // port number
    int irq;    // IRQ line
    struct pci_dev *pdev;    // PCI device pointer
    struct mr17s_uart_port *ports;
};


struct mr17s_uart_port{
    struct uart_port port;
    enum mr17s_type type;
    u32 baud;
	atomic_t inuse_cntr;
	u8 old_status;
};

#endif
