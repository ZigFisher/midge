
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

void processICMP (unsigned char *pkt, unsigned short len)
{
	register ICMPpkt *p = (ICMPpkt *) pkt;

	if (len >= sizeof (ICMPpkt)) {
		if (cks (&p->op, len - sizeof (IPpkt)) == 0) {
			switch (p->op & 0xff00) {
			case ICMP_ECHO_REQUEST:
				if (netif.ip) {	/* reply only if we have our IP address */
					memcpy (p->iphdr.ehdr.dst_hwadr, p->iphdr.ehdr.src_hwadr,
							6);
					memcpy (p->iphdr.ehdr.src_hwadr, netif.IEEEIA, 6);
					p->iphdr.dst_ip = p->iphdr.src_ip;
					p->iphdr.src_ip = netif.ip;
					p->iphdr.cks = 0;
					p->iphdr.cks =
						cks (&p->iphdr.vlt,
							 sizeof (IPpkt) - sizeof (EthHeader));
					p->op = ICMP_ECHO_REPLY;
					p->cks = 0;
					p->cks = cks (&p->op, len - sizeof (IPpkt));
					netif.send (p, len);
				}
				break;
#if TFTPNAIVE_TIMEREQUEST
			case ICMP_TIMESTAMP_REPLY: {
					extern int timestamp;
					if ((((ICMPTSpkt *) p)->tra) < 86400000)
						timestamp = ((ICMPTSpkt *) p)->tra;
				}
				break;
#endif
			case 0x0300:
				printf ("ICMP error: destination unreachable\n");
				break;
			case 0x0400:
				printf ("ICMP error: source quench\n");
				break;
			case 0x0500:
				printf ("ICMP error: redirect\n");
				break;
			case 0x0b00:
				printf ("ICMP error: time-to-live exceeded\n");
				break;
			case 0x0c00:
				printf ("ICMP error: parameter problem\n");
				break;
			default:
				break;
			}
		}
	}
}

#if TFTPNAIVE_TIMEREQUEST
int sendICMPTimestampRequest (unsigned int dst_ip)
{
	ICMPTSpkt p;

	memset (&p, 0, sizeof p);
	p.icmphdr.op = ICMP_TIMESTAMP_REQUEST;
	p.icmphdr.seq = 1;
	p.icmphdr.cks = cks (&p.icmphdr.op, sizeof (ICMPTSpkt) - sizeof (IPpkt));
	return sendIP ((IPpkt *) & p, sizeof (ICMPTSpkt) - sizeof (IPpkt),
				   PROTO_ICMP, dst_ip);
}
#endif
