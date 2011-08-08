
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

#include "tftpnaive.h"
#include "net.h"

#define MAXARPENTRIES 16
static struct arp_entry {
	unsigned int ip;
	unsigned char ia[6];
	char ok;
} arpCache[MAXARPENTRIES];
static int arpCacheCnt;

static int updateAddr (ARPpkt * p)
{
	int i, f = 0;

	for (i = 0; i < MAXARPENTRIES; ++i) {
		if (arpCache[i].ip == p->sender_ipaddr) {
			if (f) {
				arpCache[i].ok = 0;
			}
			else if (arpCache[i].ok != 0) {
				memcpy (arpCache[i].ia, p->sender_hwaddr, 6);
				arpCache[i].ok = f = 1;
			}
		}
	}

	return f;
}

static void addAddr (ARPpkt * p)
{
	memcpy (arpCache[arpCacheCnt].ia, p->sender_hwaddr, 6);
	arpCache[arpCacheCnt].ip = p->sender_ipaddr;
	arpCache[arpCacheCnt].ok = 1;
	arpCacheCnt = (arpCacheCnt + 1) % MAXARPENTRIES;
}

int sendARPRequest (unsigned int sender_ip, unsigned int target_ip)
{
	ARPpkt p;
	int i;

	for (i = 0; i < 6; ++i)
		p.ehdr.dst_hwadr[i] = 0xff;
	memcpy (p.ehdr.src_hwadr, netif.IEEEIA, 6);
	p.ehdr.frametype = FRAME_ARP;
	p.hardtype = ARP_ETH;
	p.prottype = FRAME_IP;
	p.hardsize = 6;
	p.protsize = 4;
	p.op = ARP_REQUEST;
	memcpy (p.sender_hwaddr, netif.IEEEIA, 6);
	p.sender_ipaddr = sender_ip;
	memset (p.target_hwaddr, 0, 6);
	p.target_ipaddr = target_ip;
	return netif.send (&p, sizeof (ARPpkt));
}

unsigned char *lookupARP (unsigned int ip)
{
	int i;

	for (i = 0; i < MAXARPENTRIES; ++i) {
		if (arpCache[i].ip == ip) {
			if (arpCache[i].ok == 1)
				return arpCache[i].ia;
			if (arpCache[i].ok == 2)
				return 0;
		}
	}

	arpCache[arpCacheCnt].ip = ip;
	arpCache[arpCacheCnt].ok = 2;
	arpCacheCnt = (arpCacheCnt + 1) % MAXARPENTRIES;

	return 0;
}

#if 0
void listARPCache (void)
{
	int i;

	printf ("ARP Cache entries:\n");
	for (i = 0; i < MAXARPENTRIES; ++i) {
		if (arpCache[i].ok == 1) {
			printf ("%d.%d.%d.%d is-at %02X:%02X:%02X:%02X:%02X:%02X\n",
					(arpCache[i].ip >> 24) & 0xff,
					(arpCache[i].ip >> 16) & 0xff,
					(arpCache[i].ip >> 8) & 0xff, (arpCache[i].ip) & 0xff,
					arpCache[i].ia[0], arpCache[i].ia[1], arpCache[i].ia[2],
					arpCache[i].ia[3], arpCache[i].ia[4], arpCache[i].ia[5]);
		}
	}
}
#endif

void processARP (unsigned char *pkt, unsigned short len)
{
	register ARPpkt *p = (ARPpkt *) pkt;

	if (len >= sizeof (ARPpkt)) {
		if (p->hardtype == ARP_ETH && p->prottype == FRAME_IP && netif.ip) {
			if (p->hardsize == 6 && p->protsize == 4) {
				if (p->op == ARP_REQUEST) {
					if (!updateAddr (p)) {
						addAddr (p);
					}
					if (netif.ip == p->target_ipaddr) {
						p->op = ARP_REPLY;
						memcpy (p->target_hwaddr, p->sender_hwaddr, 10);
						p->sender_ipaddr = netif.ip;
						memcpy (p->sender_hwaddr, netif.IEEEIA, 6);
						memcpy (pkt, pkt + 6, 6);
						memcpy (pkt + 6, netif.IEEEIA, 6);
						netif.send (pkt, sizeof (ARPpkt));
					}
				}
				else if (p->op == ARP_REPLY) {
					if (p->sender_ipaddr == netif.ip) {
						netif.ip = 0;
						printf ("IP conflict : ");
						printf
							("ARP reply: %d.%d.%d.%d is-at %02X:%02X:%02X:%02X:%02X:%02X\n",
							 (p->sender_ipaddr >> 24) & 0xff,
							 (p->sender_ipaddr >> 16) & 0xff,
							 (p->sender_ipaddr >> 8) & 0xff,
							 (p->sender_ipaddr) & 0xff, p->sender_hwaddr[0],
							 p->sender_hwaddr[1], p->sender_hwaddr[2],
							 p->sender_hwaddr[3], p->sender_hwaddr[4],
							 p->sender_hwaddr[5]);
					}
					updateAddr (p);
				}
			}
		}
	}
}
