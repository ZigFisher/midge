
/*
	Copyright 2001, 2002 Georges Menie (www.menie.org)

	This file is part of Tftpnaive.

    Tftpnaive is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Tftpnaive is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Tftpnaive; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _NET_H_
#define _NET_H_

#define MAXPACKETBUF 16
#define MAXPACKETLEN 600
extern unsigned char packetBuf[MAXPACKETBUF][MAXPACKETLEN];
extern unsigned short packetBufLen[MAXPACKETBUF];
extern volatile int idxiPBuf, idxoPBuf;

#define BOOT_RQ_ID 0xbebe2001
#define BOOT_RQ_SEC 100
#define BOOTP_SRV_PORT 67
#define BOOTP_CLI_PORT 68
#define TFTP_SRV_PORT 69
#define TFTP_LOCAL_PORT TFTP_SRV_PORT

#define FRAME_ARP 0x806
#define FRAME_IP 0x800
#define IPV4 0x4500
#define PROTO_UDP 17
#define PROTO_ICMP 1
#define ICMP_ECHO_REQUEST 0x0800
#define ICMP_ECHO_REPLY 0
#define ICMP_TIMESTAMP_REQUEST 0x0d00
#define ICMP_TIMESTAMP_REPLY 0x0e00
#define ARP_ETH 1
#define ARP_REQUEST 1
#define ARP_REPLY 2
#define BOOTP_ETH_REQUEST 0x101
#define BOOTP_ETH_REPLY 0x201
#define DONT_FRAGMENT_MASK 0x4000

typedef struct {
	unsigned char dst_hwadr[6];
	unsigned char src_hwadr[6];
	unsigned short frametype;
} EthHeader;

#ifdef GHS
#pragma pack(1)
#endif

typedef struct {
	EthHeader ehdr;
	unsigned short hardtype;
	unsigned short prottype;
	unsigned char hardsize;
	unsigned char protsize;
	unsigned short op;
	unsigned char sender_hwaddr[6] ;
	unsigned int sender_ipaddr;
	unsigned char target_hwaddr[6];
	unsigned int target_ipaddr __attribute__ ((packed));
} ARPpkt;

typedef struct {
	EthHeader ehdr;
	unsigned short vlt;
	unsigned short length;
	unsigned short id;
	unsigned short frag;
	unsigned char ttl;
	unsigned char prot;
	unsigned short cks;
	unsigned int src_ip __attribute__ ((packed));
	unsigned int dst_ip __attribute__ ((packed));
} IPpkt;

typedef struct {
	IPpkt iphdr;
	unsigned short op;
	unsigned short cks;
	unsigned short id;
	unsigned short seq;
} ICMPpkt;

typedef struct {
	ICMPpkt icmphdr;
	unsigned int org __attribute__ ((packed));
	unsigned int rec __attribute__ ((packed));
	unsigned int tra __attribute__ ((packed));
} ICMPTSpkt;

typedef struct {
	IPpkt iphdr;
	unsigned short src_port;
	unsigned short dst_port;
	unsigned short length;
	unsigned short cks;
} UDPpkt;

typedef struct {
	UDPpkt udphdr;
	unsigned short op_type;
	unsigned short adr_hop;
	unsigned int id __attribute__ ((packed));
	unsigned short sec;
	unsigned short unused;
	unsigned int cli_ip __attribute__ ((packed));
	unsigned int your_ip __attribute__ ((packed));
	unsigned int srv_ip __attribute__ ((packed));
	unsigned int gw_ip __attribute__ ((packed));
	unsigned char hw_addr[16];
	unsigned char srv_name[64];
	unsigned char boot_filename[128];
	unsigned char options[64];
} BOOTPpkt;

#ifdef GHS
#pragma pack()
#endif

typedef struct {
	UDPpkt udphdr;
	unsigned short opcode;
	char data[138];
} TFTPRpkt;

typedef struct {
	UDPpkt udphdr;
	unsigned short opcode;
	unsigned short num;
	unsigned char data[512];
} TFTPDpkt;

extern void processPacket (void);
extern void processARP (unsigned char *, unsigned short);
extern void processIP (unsigned char *, unsigned short);
extern void processUDP (unsigned char *, unsigned short);
extern void processICMP (unsigned char *, unsigned short);
extern void processBOOTP (BOOTPpkt *);
extern void processTFTP (TFTPDpkt *);

extern int sendARPRequest (unsigned int, unsigned int);
extern int sendIP (IPpkt *, unsigned short, unsigned char, unsigned int);
extern int sendUDP (UDPpkt *, unsigned short, unsigned int, unsigned short,
					unsigned short);
extern int sendICMPTimestampRequest (unsigned int);
extern int sendBOOTPRequest (void);
extern int sendTFTPRequest (void);

extern unsigned char *lookupARP (unsigned int);
extern unsigned short cks_partial (void *, unsigned short, long);
extern unsigned short cks (void *, unsigned short);

#endif
