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

#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include "tcpmessagesocket.h"
#include <time.h>
#include "../kphone/kstatics.h"
#include <iostream>
using namespace std;

TCPMessageSocket::TCPMessageSocket( void )
{
	type = SocketTCP;
	if ( ( socketfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
		cerr  <<  "TCPMessageSocket(): socket() failed" ;
	        cout  <<  "TCPMessageSocket(): socket() failed" ;
	}

}

TCPMessageSocket::TCPMessageSocket( int newfd )
{
	type = SocketTCP;

	socketfd = newfd;
}

TCPMessageSocket::~TCPMessageSocket( void )
{
	close( socketfd );
}

int TCPMessageSocket::connect( unsigned int portnum )
{
	socketaddress.soa4.sin_family = AF_INET;
	socketaddress.soa4.sin_port = htons( portnum );
	socketaddress.soa4.sin_addr =  ip_addr.in4_addr;
	bzero( &( socketaddress.soa4.sin_zero ), 8 );
	if( ::connect( socketfd, &socketaddress.soa,  sizeof( struct sockaddr ) ) == -1 ) {
	    	    cerr  <<   "TCPMessageSocket::connect():  \n";
		    cout  <<   "TCPMessageSocket::connect():  \n";
		    return -1;
	}
	return 0;
}

int TCPMessageSocket::send( const char *sendbuffer, unsigned int length )
{
	if ( ::send( socketfd, sendbuffer, length, 0 ) == -1 ) {
		cerr  <<  "TCPMessageSocket::send(): send() failed";
		cout  <<  "TCPMessageSocket::send(): send() failed";
		return -1;
	}

	return 0;
}

int TCPMessageSocket::receive( char *recvbuffer, unsigned int maxlength )
{
	int numbytes;

	if ( ( numbytes = recv( socketfd, recvbuffer, maxlength, 0 ) ) == -1 ) {
		cout  <<  "TCPMessageSocket::recieve(): recv() failed" ;
		cerr  <<  "TCPMessageSocket::recieve(): recv() failed" ;
		return -1;
	}

	return numbytes;
}

unsigned int TCPMessageSocket::listen( unsigned int portnum, unsigned int retry )
{
	socketaddress.soa4.sin_family = AF_INET;
	socketaddress.soa4.sin_port = htons( portnum );
	socketaddress.soa4.sin_addr.s_addr = INADDR_ANY;
	bzero( &( socketaddress.soa4.sin_zero ), 8 ); // is this portable?

	unsigned int count = 0;
	while ( (::bind( socketfd, (struct sockaddr *) &socketaddress, sizeof( struct sockaddr ) ) == -1) && (count < retry) ) {
		count++;
		portnum += 2;
		socketaddress.soa4.sin_port = htons( portnum );
	}
	if ( count > 10 ) {
		cerr  <<  "TCPMessageSocket::listen(): bind() failed" ;
		cout  <<  "TCPMessageSocket::listen(): bind() failed" ;
		return 0;
	}
	if ( ::listen( socketfd, 10 ) == -1 ) {
		cerr  <<  "TCPMessageSocket::listen(): listen() failed" ;
		cout  <<  "TCPMessageSocket::listen(): listen() failed" ;
		return 0;
	}

	return portnum;
}

int TCPMessageSocket::accept( void )
{
	int connectfd;
	socklen_t sockaddr_in_size;

	sockaddr_in_size = sizeof( struct sockaddr_in );

	if ( ( connectfd = ::accept( socketfd, (struct sockaddr *) &socketaddress, \
					&sockaddr_in_size ) ) == -1 ) {
		cerr  <<  "TCPMessageSocket::accept(): accept() failed" ;
		cout  <<  "TCPMessageSocket::accept(): accept() failed" ;
		return -1;
	}

	return connectfd;
}

int TCPMessageSocket::listenOnEvenPort( int min , int max  )
{
	return -1;
}


bool TCPMessageSocket::setHostnamePort( const char *hostname, unsigned int port )
{
struct addrinfo hints, *res;
int errcode;
void *ptr;
time_t begin, end;
double gaidiff;

if(KStatics::noLine >=1 ) {
	    return false;
	}

if ( setHostname( hostname ) ) {
	time(&begin);
	memset (&hints, 0, sizeof (hints));
	errcode = getaddrinfo (hostname, NULL, &hints, &res);  
	time(&end);
	gaidiff=difftime(end,begin);
	if (gaidiff > 1.50E1) {
	        KStatics::noLine=1;
	}
	if (errcode != 0)
	{
		cerr << "TCPMessageSocket::setHostnamePort getaddrinfo" << endl;
		cout << "TCPMessageSocket::setHostnamePort getaddrinfo" << endl;
		return false;
	}
	
	inet_ntop (res->ai_family, res->ai_addr->sa_data, myAddr, 100);
	
	switch (res->ai_family)
		{
		case AF_INET:
		ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
		break;
		case AF_INET6:
		return false;
		}
		
		memset (&myAddr,0, 100);
		inet_ntop (res->ai_family, ptr, myAddr, 100); 
		freeaddrinfo(res);
		portnum = port;
		return true;
		}
return false;
}

bool TCPMessageSocket::cmpSocket( const char *hostname, unsigned int port ) {
	struct addrinfo hints, *res;
	int errcode;
	void *ptr;
	char lAddr[100];
	time_t begin, end;
	double gaidiff;
	
	if(KStatics::noLine >=1 ) {
		    return false;
	}
// 	if ( host.contains("[") ) host=host.mid( host.find("[") + 1 ,  host.find("]") - 1 );
	time(&begin);
	memset (&hints, 0, sizeof (hints));
	errcode = getaddrinfo (hostname, NULL, &hints, &res);  
	time(&end);
	gaidiff=difftime(end,begin);
	if (gaidiff > 1.50E1) {
	        KStatics::noLine=1;
	}
	if (errcode != 0)
	{
		perror ("TCPMessageSocket::cmpSocket getaddrinfo");
		printf ("TCPMessageSocket::cmpSocket getaddrinfo");
		return false;
	}
	inet_ntop (res->ai_family, res->ai_addr->sa_data, lAddr, 100);
	switch (res->ai_family)
	{
		case AF_INET:
			ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
			break;
		case AF_INET6:
			return false;
		}
		
		memset (&lAddr,0, 100);
		inet_ntop (res->ai_family, ptr, lAddr, 100); 
		if( ( memcmp(myAddr, lAddr, 100) == 0) &&  portnum == port ) {
			freeaddrinfo(res);
			return true;
		} else {
			freeaddrinfo(res);
			return false;
		}
}
