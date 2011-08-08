/* pef22554.h
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

#ifndef PEF22554_H
#define PEF22554_H

#include "mr17g.h"

struct pef22554_address{
    u8 ms:1;
    u8 cr:1;
    u8 addr:6;
};

struct pef22554_rsta{
    u8 ta:1;
    u8 cr:1;
    u8 sa0:1;
    u8 sa1:1;
    u8 rab:1;
    u8 crc:1;
    u8 rdo:1;
    u8 vfr:1;
};

// Control bits
#define WR_FALC 0x00
#define RD_FALC 0x01
#define WR_SCI  0x02
#define RD_SCI  0x03

struct pef22554_write_cmd{
    struct pef22554_address src;
    struct pef22554_address dst;
    u16 reg_addr:14;
    u16 ctrl_bits:2;
    u8 data;
};
#define PEF22554_WCMD_SIZE 5

struct pef22554_read_cmd{
    struct pef22554_address src;
    struct pef22554_address dst;
    u16 reg_addr:14;
    u16 ctrl_bits:2;
    u8 rdepth;
};
#define PEF22554_RCMD_SIZE 5

struct pef22554_write_ack{
    struct pef22554_address src;
    struct pef22554_address dst;
    struct pef22554_rsta rsta;
};
#define PEF22554_WACK_SIZE 3

struct pef22554_read_ack{
    struct pef22554_address src;
    struct pef22554_address dst;
    struct pef22554_rsta rsta;
    u8 reg_cont;
};
#define PEF22554_RACK_SIZE 4

// Read Status Byte (RSTA) bits
#define RSTA_VFR 0x80
#define RSTA_CRC 0x20
#define RSTA_RAB 0x10

// SCI Configuration Register bits
#define DUP     0x01
#define ARB     0x02
#define CRC_EN  0x04
#define INT_EN  0x08
#define ACK_EN  0x10
#define CLK_GAT 0x20
#define CLK_POL 0x40
#define PP      0x80

// PEF22554 QuadFALC registers addresses
// Control registers
#define IPC     0x0008
#define GPC1    0x0085
#define GPC2    0x008A
#define GCM1    0x0092
#define GCM2    0x0093
#define GCM3    0x0094
#define GCM4    0x0095
#define GCM5    0x0096
#define GCM6    0x0097
#define GCM7    0x0098
#define GCM8    0x0099
#define GIMR    0x00A7
#define REGFP   0x00BB
#define REGFD   0x00BC
#define GPC3    0x00D3
#define GPC4    0x00D4
#define GPC5    0x00D5
#define GPC6    0x00D6
#define INBLDTR 0x00D7
#define PRBSTS1 0x00DB
#define PRBSTS2 0x00DC
#define PRBSTS3 0x00DD
#define PRBSTS4 0x00DE
#define CMDR    0x0002
#define CCR1  0x0009
#define CCR2  0x000A
#define RDICR 0x000B
#define RTR1  0x000C
#define RTR2    0x000D
#define RTR3    0x000E
#define RTR4    0x000F
#define TTR1    0x0010
#define TTR2    0x0011
#define TTR3    0x0012
#define TTR4    0x0013
#define IMR0    0x0014
#define IMR1    0x0015
#define IMR2    0x0016
#define IMR3    0x0017
#define IMR4    0x0018
#define IMR5    0x0019
#define IMR6    0x001A
#define IERR    0x001B
#define FMR0    0x001C
#define FMR1    0x001D
#define FMR2    0x001E
#define LOOP    0x001F
#define XSW     0x0020
#define XSP     0x0021
#define XC0     0x0022
#define XC1     0x0023
#define RC0     0x0024
#define RC1     0x0025
#define XPM0    0x0026
#define XPM1    0x0027
#define XPM2    0x0028
#define TSWM    0x0029
#define SIC4    0x002A
#define IDLE    0x002B
#define XSA4    0x002C
#define XSA5    0x002D
#define XSA6    0x002E
#define XSA7    0x002F
#define XSA8    0x0030
#define FMR3    0x0031
#define ICB1    0x0032
#define ICB2    0x0033
#define ICB3    0x0034
#define ICB4    0x0035
#define LIM0    0x0036
#define LIM1    0x0037
#define PCD     0x0038
#define PCR     0x0039
#define LIM2    0x003A
#define LCR1    0x003B
#   define      FLLB 0x02
#define LCR2    0x003C
#define LCR3    0x003D
#define SIC1    0x003E
#define SIC2    0x003F
#define SIC3    0x0040
#define CMR4    0x0041
#define CMR5    0x0042
#define CMR6    0x0043
#define CMR1    0x0044
#define CMR2    0x0045
#define GCR     0x0046
#define ESM     0x0047
#define CMR3    0x0048
#define DEC     0x0060
#define XS1     0x0070
#define XS2     0x0071
#define XS3     0x0072
#define XS4     0x0073
#define XS5     0x0074
#define XS6     0x0075
#define XS7     0x0076
#define XS8     0x0077
#define XS9     0x0078
#define XS10    0x0079
#define XS11    0x007A
#define XS12    0x007B
#define XS13    0x007C
#define XS14    0x007D
#define XS15    0x007E
#define XS16    0x007F
#define PC1     0x0080
#define PC2     0x0081
#define PC3     0x0082
#define PC4     0x0083
#define PC5     0x0084
#define PC6     0x0085
#define BFR     0x00BD
#define ALS     0x00D9
#define IMR7    0x00DF
#define WCON    0x00E8

// Status registers
#define VSTR 0x004A
#define CIS 0x006F
#define GIS2 0x00AD
#define DSTR 0x00E7
#define RFIFO1L 0x0000
#define RFIFO1H 0x0001
#define RBD   0x0049
#define RES 0x004B
#define FRS0 0x004C
#   define LOS  0x80
#   define AIS  0x40
#   define LFA  0x20
#   define RRA  0x10
#   define NMF  0x04
#   define LMFA 0x02
#define FRS1 0x004D
#define RSW 0x004E
#define RSP 0x004F
#define FECL 0x0050
#define FECH 0x0051
#define CVCL 0x0052
#define CVCH 0x0053
#define CEC1L 0x0054
#define CEC1H 0x0055
#define EBCL 0x0056
#define EBCH 0x0057
#define CEC2L 0x0058
#define CEC2H 0x0059
#define CEC3L 0x005A
#define CEC3H 0x005B
#define RSA4 0x005C
#define RSA5 0x005D
#define RSA6 0x005E
#define RSA7 0x005F
#define RSA8 0x0060
#define RSA6S 0x0061
#define RSP1 0x0062
#define RSP2 0x0063
#define SIS 0x0064
#define RSIS 0x0065
#define RBCL 0x0066
#define RBCH 0x0067
#define ISR0 0x0068
#define ISR1 0x0069
#define ISR2 0x006A
#define ISR3 0x006B
#define ISR4 0x006C
#define ISR5 0x006D
#define GIS 0x006E
#define RS1 0x0070
#define RS2 0x0071
#define RS3 0x0072
#define RS4 0x0073
#define RS5 0x0074
#define RS6 0x0075
#define RS7 0x0076
#define RS8 0x0077
#define RS9 0x0078
#define RS10 0x0079
#define RS11 0x007A
#define RS12 0x007B
#define RS13 0x007C
#define RS14 0x007D
#define RS15 0x007E
#define RS16 0x007F
#define RBC2 0x0090
#define RBC3 0x0091
#define SIS3 0x009A
#define RSIS3 0x009B
#define RFIFO2L 0x009C
#define RFIFO2H 0x009D
#define RFIFO3L 0x009E
#define RFIFO3H 0x009F
#define SIS2 0x00A9
#define RSIS2 0x00AA
#define MFPI 0x00AB
#define ISR6 0x00AC
#define ISR7 0x00D8
#define PRBSSTA 0x00DA
#define CLKSTAT 0x00FE

#define PEF22554_IS_GENERAL(reg_addr) ( \
    (reg_addr == IPC ) || (reg_addr == GPC1) || (reg_addr == GPC2) || \
    ((reg_addr >= GCM1) && (reg_addr <= GCM8)) || \
    (reg_addr == GIMR) || (reg_addr == REGFP ) || (reg_addr == REGFD) || \
    ((reg_addr >=GPC3) && (reg_addr <=PRBSTS3)) || \
    (reg_addr == VSTR) || (reg_addr == CIS) || (reg_addr == GIS2) || \
    (reg_addr == DSTR) \
    )

int pef22554_setup_sci(struct mr17g_card *card);
void pef22554_defcfg(struct mr17g_channel *chan);
int pef22554_writereg(struct mr17g_chip *chip,u8 chan,u16 addr,u8 val);
int pef22554_readreg(struct mr17g_chip *chip,u8 chan,u16 addr,u8 *val);
int pef22554_basic_chip(struct mr17g_chip *chip);
int pef22554_basic_channel(struct mr17g_channel *chan);
int pef22554_channel(struct mr17g_channel *chan);
int pef22554_linkstate(struct mr17g_chip *chip,int chnum,u8 framed);



#endif
