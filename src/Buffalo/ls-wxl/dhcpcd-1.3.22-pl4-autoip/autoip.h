/*
 * dhcpcd - DHCP client daemon -
 * Copyright (C) 1996 - 1997 Yoichi Hariguchi <yoichi@fore.com>
 * Copyright (C) January, 1998 Sergei Viznyuk <sv@phystech.com>
 * 
 * dhcpcd is an RFC2131 and RFC1541 compliant DHCP client daemon.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef AUTOIP_H
#define AUTOIP_H

#define AUTOIP_CONFIG
#define AUTOIP_DEBUG

#define ARP_WAIT	50000

//RFC 3927
#define PROBE_WAIT	1000000 - ARP_WAIT
#define PROBE_NUM	3
#define PROBE_MIN	1000000 - ARP_WAIT
#define PROBE_MAX	2000000 - ARP_WAIT
#define ANNOUNCE_WAIT	2000000 - ARP_WAIT
#define ANNOUNCE_NUM	2
#define ANNOUNCE_INTERVAL	2000000 - ARP_WAIT
#define MAX_CONFLICTS	10
#define RATE_LIMIT_INTERVAL	60000000
#define DEDEND_INTERVAL	10000000

#ifdef AUTOIP_CONFIG
int setAutoIP(int i);
//void conflictIP();
int conflictIP(int interval);
#endif //AUTOIP_CONFIG

#endif //AUTOIP_H
