
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

static BootData bdata;
BootData *tftp_req;

void processBOOTP (BOOTPpkt * bp)
{
	if (bp->op_type == BOOTP_ETH_REPLY && bp->id == BOOT_RQ_ID
		&& tftp_req == 0 && !memcmp (netif.IEEEIA, bp->hw_addr, 6)) {
		int l;

		printf ("BOOTP: ");
		printf ("IP %d.%d.%d.%d, ",
				(bp->your_ip >> 24) & 0xff,
				(bp->your_ip >> 16) & 0xff,
				(bp->your_ip >> 8) & 0xff,
				bp->your_ip & 0xff);
		printf ("server %d.%d.%d.%d, ",
				(bp->srv_ip >> 24) & 0xff,
				(bp->srv_ip >> 16) & 0xff,
				(bp->srv_ip >> 8) & 0xff,
				bp->srv_ip & 0xff);
		printf ("boot file '%s'\n", bp->boot_filename);
		netif.ip = bp->your_ip;
		l = strlen (bp->boot_filename);
		if (l > 0 && l < 128) {
			memset (&bdata, 0, sizeof bdata);
			tftp_req = &bdata;
			tftp_req->server = bp->srv_ip;
			strcpy (tftp_req->file, bp->boot_filename);
		}
	}
}

int sendBOOTPRequest (void)
{
	BOOTPpkt bootp_pkt;

	tftp_req = 0;
	memset (&bootp_pkt, 0, sizeof bootp_pkt);
	bootp_pkt.op_type = BOOTP_ETH_REQUEST;
	bootp_pkt.adr_hop = 0x600;
	bootp_pkt.id = BOOT_RQ_ID;
	bootp_pkt.sec = BOOT_RQ_SEC;
	memcpy (&bootp_pkt.hw_addr, netif.IEEEIA, 6);

	return sendUDP ((UDPpkt *) & bootp_pkt,
					sizeof bootp_pkt - sizeof (UDPpkt), -1, BOOTP_CLI_PORT,
					BOOTP_SRV_PORT);
}
