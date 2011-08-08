
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

typedef struct {
	int sa;
	int da;
	short pt;
	short ln;
} fakehdr;

void processUDP (unsigned char *pkt, unsigned short len)
{
	register UDPpkt *p = (UDPpkt *) pkt;
	register unsigned short sum;
	fakehdr fh;

	if (len >= sizeof (UDPpkt)) {
		if ((sum = p->cks)) {
			fh.sa = p->iphdr.src_ip;
			fh.da = p->iphdr.dst_ip;
			fh.pt = p->iphdr.prot;
			fh.ln = p->length;
			sum = cks_partial (&fh, sizeof fh, 0);
			sum = ~cks_partial (&p->src_port, fh.ln, sum);
		}
		if (sum == 0) {
			if (p->src_port == BOOTP_SRV_PORT && p->dst_port == BOOTP_CLI_PORT
				&& netif.ip == 0)
				processBOOTP ((BOOTPpkt *) p);
			else if (p->dst_port == TFTP_LOCAL_PORT && netif.ip)
			if (p->dst_port == TFTP_LOCAL_PORT && netif.ip)
				processTFTP ((TFTPDpkt *) p);
		}
		else {
			printf ("UDP checksum error\n");
		}
	}
}

int sendUDP (UDPpkt * p, unsigned short len, unsigned int dst_ip,
			 unsigned short src_port, unsigned short dst_port)
{
	fakehdr fh;
	register unsigned short sum;

	len += sizeof (UDPpkt) - sizeof (IPpkt);
	p->src_port = src_port;
	p->dst_port = dst_port;
	fh.ln = p->length = len;
	p->cks = 0;
	fh.sa = netif.ip;
	fh.da = dst_ip;
	fh.pt = PROTO_UDP;
	sum = cks_partial (&fh, sizeof fh, 0);
	sum = ~cks_partial (&p->src_port, len, sum);
	p->cks = sum ? sum : -1;
	return sendIP ((IPpkt *) p, len, PROTO_UDP, dst_ip);
}
