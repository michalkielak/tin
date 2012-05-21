/*
 * Copyright (c) 2000 Billy Biggs <bbiggs@div8.net>
 * Copyright (c) 2004 Wirlab <kphone@wirlab.net>
 * Copyright (c) 2006 Kphone Team  <kphone-devel@lists.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "../kphone/kstatics.h"
#include "messagesocket.h"
#include <iostream>
using namespace std;

MessageSocket::MessageSocket( void )
{
	type = None;
	bound = false;
	socketfd=-1;
	bzero(&ip_addr.in6_addr,16);
}


MessageSocket::~MessageSocket( void )
{
}
void MessageSocket::disableIpv6Lookups() {
	ipv6_lookups_enabled=0;
}


InetType MessageSocket::intype=InetNone;
int      MessageSocket::ipv6_lookups_enabled=1;


// returns 0: failure
//         4: remote address is IPv4
//         6: remote address is IPv6
int MessageSocket::lookupHostname( const char *hostname, unsigned char *store, const int len) {

	struct addrinfo hints, *res;
	int errcode;
	void *ptr;
	time_t begin, end;
	double gaidiff;

	if(KStatics::noLine >=1 ) {
		    return -1;
	}
	
	// rip brackets from IPv6 reference
	if (*hostname=='[') {
		hostname++;
		char *cb=strchr(hostname,']');
		if (!cb) return 0;
		*cb=0;
	}
        
	memset (&hints, 0, sizeof (hints));
	hints.ai_family = PF_UNSPEC;
	
	time(&begin);
	errcode = getaddrinfo (hostname, NULL, &hints, &res);
	time(&end);
	gaidiff=difftime(end,begin);
	if (gaidiff > 1.50E1) {
		cout << "MessageSocket::lookupHostname noLine \n";
	        KStatics::noLine=1;
	}
	if (errcode  != 0) {
		cout << "MessageSocket::lookupHostname getaddrinfo " << errcode << endl;
		return -1;
	}

	switch (res->ai_family)
	{
		case AF_INET:
			ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
			memcpy (store,ptr,4);
			return 4;
		case AF_INET6:
			ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
			bcopy (ptr,store,16);
			return 6;
		default:
			return 0;
	}



	return 0;

}


// returns 0: failure
//         4: remote address is IPv4
//         6: remote address is IPv6
int MessageSocket::getHostIpType( const char *hostname) {
	unsigned char b[16];
	return lookupHostname( hostname, b, 16);
}


// returns 0: failure
//         4: remote address is IPv4
//         6: remote address is IPv6
int MessageSocket::setHostname( const char *hostname )
{
	return lookupHostname( hostname, (unsigned char *)(ip_addr.raw_addr), 16);
}


int MessageSocket::getSocketIpVersion() {
	if (intype==Inet6) return 6;
	if (intype==Inet4) return 4;
	return 0;
}


void MessageSocket::forcePortNumber( unsigned int newport )
{
	ourport = newport;
}
