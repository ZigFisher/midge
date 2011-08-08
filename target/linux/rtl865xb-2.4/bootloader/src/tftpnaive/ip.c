
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

void processIP (unsigned char *pkt, unsigned short len)
{
	register IPpkt *p = (IPpkt *) pkt;

	if (len >= sizeof (IPpkt)) {
		if ((p->vlt & 0xff00) == IPV4) {
			if ((p->frag & ~DONT_FRAGMENT_MASK) == 0) {
				if (netif.ip == 0 || p->dst_ip == netif.ip) {
					if (cks (&p->vlt, sizeof (IPpkt) - sizeof (EthHeader)) == 0) {
						if (p->prot == PROTO_ICMP) {
							processICMP (pkt, len);
						}
						else if (p->prot == PROTO_UDP) {
							processUDP (pkt, len);
						}
					}
				}
			}
		}
	}
}

int sendIP (IPpkt * p, unsigned short len, unsigned char prot,
			unsigned int dst_ip)
{
	register unsigned char *dst;
	register int trycount;

	p->vlt = IPV4;
	p->length = len + sizeof (IPpkt) - sizeof (EthHeader);
	p->id = 0;
	p->frag = 0;
	p->ttl = 3;
	p->prot = prot;
	p->src_ip = netif.ip;
	p->dst_ip = dst_ip;
	p->cks = 0;
	p->cks = cks (&p->vlt, sizeof (IPpkt) - sizeof (EthHeader));

	if (dst_ip == -1) {
		memset (p->ehdr.dst_hwadr, 0xff, 6);
	}
	else {
		for (trycount = 3; trycount; --trycount) {
			dst = lookupARP (p->dst_ip);
			if (dst) {
				memcpy (p->ehdr.dst_hwadr, dst, 6);
				break;
			}
			sendARPRequest (netif.ip, p->dst_ip);
			busyWait (100, processPacket, 0);
		}
		dst = lookupARP (p->dst_ip);
		if (dst) {
			memcpy (p->ehdr.dst_hwadr, dst, 6);
		}
		else
			return -1;
	}

	memcpy (p->ehdr.src_hwadr, netif.IEEEIA, 6);
	p->ehdr.frametype = FRAME_IP;

	return netif.send (p, len + sizeof (IPpkt));
}
