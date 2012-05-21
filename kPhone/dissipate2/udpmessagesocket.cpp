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
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/nameser.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>
#include <resolv.h>
#include <qwidget.h>
#include "udpmessagesocket.h"
#include "sipclient.h"
#include <iostream>
using namespace std;

void UDPMessageSocket::reInit(bool useIPv6) {

if(useIPv6) {
	if (socketfd!=-1) close(socketfd);
	type = SocketUDP;
	if ( ( socketfd = socket( AF_INET6, SOCK_DGRAM, 0 ) ) == -1 ) {
		cout  << "=====UDPMessageSocket:::UDPMessageSocket(): socket() failed";
	        cerr  <<  "=====UDPMessageSocket::UDPMessageSocket(): socket() failed";
		intype=InetNone;
	} else {
		intype=Inet6;
	}
} else {
	if ( ( socketfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 ) {
		cerr  <<  "=====UDPMessageSocket::UDPMessageSocket(): socket() failed";
		cout  <<  "=====UDPMessageSocket::UDPMessageSocket(): socket() failed";
		intype=InetNone;
	} else {
		intype=Inet4;
	}
}
	didcomplain = false;
}

UDPMessageSocket::UDPMessageSocket( bool useIPv6 )
{
reInit(useIPv6);
}

UDPMessageSocket::UDPMessageSocket(void)
{
	didcomplain = false;
}

void UDPMessageSocket::reInit( int newfd )
{
	struct sockaddr soa,*soap=&soa;
	//size_t l1=sizeof(soa);
	socklen_t l = sizeof(soa);
	if (socketfd!=-1) close(socketfd);
	if ( getsockname(newfd, soap, &l) == -1 ) {
		cerr  <<  "=====UDPMessageSocket::UDPMessageSocket(): getsockname() failed";
		cout  <<  "=====UDPMessageSocket::UDPMessageSocket(): getsockname() failed";
		intype = Inet4;
	} else {
		switch ( ((struct sockaddr *)soap)->sa_family ) {
		case AF_INET  : intype = Inet4; break;
		case AF_INET6 : intype = Inet6; break;
		default       : intype = InetNone;
		}
	}

	type = SocketUDP;
	socketfd = newfd;
	didcomplain = false;
}


UDPMessageSocket::UDPMessageSocket(int newfd )
{
	reInit(newfd);
}


UDPMessageSocket::~UDPMessageSocket( void )
{

	close( socketfd );
}


int UDPMessageSocket::connect( unsigned int portnum )
{
if (intype==Inet6) {
		memset( &socketaddress.soa6,0, sizeof( sockaddr_in6 ) );
		socketaddress.soa6.sin6_port = htons( portnum );
		socketaddress.soa6.sin6_family = AF_INET6;
		memcpy ( &(socketaddress.soa6.sin6_addr), &ip_addr.in6_addr, 16);
		socketaddress.soa6.sin6_flowinfo = 0;
		socketaddress.soa6.sin6_scope_id = 0;
	} else {
		socketaddress.soa4.sin_port = htons( portnum );
		socketaddress.soa4.sin_family = AF_INET;
		socketaddress.soa4.sin_addr = ip_addr.in4_addr;
		memset( &(socketaddress.soa4.sin_zero),0, 8 );
	}
	remoteaddress = socketaddress;
	return 0;
}


int UDPMessageSocket::SetTOS( void )
{
	//unsigned char frag;
	unsigned char tos;
	socklen_t optlen;
	tos=IPTOS_THROUGHPUT;//LOWDELAY;
	optlen=1; 
//	if(setsockopt(socketfd,SOL_IP,IP_TOS,&tos,optlen) != 0){
	if(setsockopt(socketfd,IPPROTO_IP,IP_TOS,&tos,optlen) != 0){
		cout  << "!!!!!UDPMessageSocket::SetTOS";
	}	
	return 0;
}



int UDPMessageSocket::send( const char *sendbuffer, unsigned int length )
{
	int numbytes;

	int soal=intype==Inet6?sizeof(struct sockaddr_in6):intype==Inet4?sizeof(struct sockaddr_in):0;

	if ( (numbytes = sendto( socketfd, sendbuffer, length, 0,&remoteaddress.soa, soal ) ) == -1 ) {
		if( !didcomplain ) {
			cerr  <<  "=====UDPMessageSocket::send(): sendto() failed\n";
			cout  <<  "=====UDPMessageSocket::send(): sendto() failed\n";
			didcomplain = true;
		}
		return -1;
	}
	return 0;
}

int UDPMessageSocket::receive( char *recvbuffer, unsigned int maxlength )
{
	socklen_t addrlength;
	int numbytes;
	addrlength = sizeof( struct sockaddr );
	if( ( numbytes = recvfrom( socketfd, recvbuffer, maxlength, 0, &remoteaddress.soa, &addrlength ) ) == -1 ) {
		cerr <<  "=====UDPMessageSocket::receive(): recvfrom() failed\n";
		cout  <<  "=====UDPMessageSocket::receive(): recvfrom() failed\n";
		return -1;
	}
	return numbytes;
}


unsigned int UDPMessageSocket::listen( unsigned int portnum, unsigned int retry )
{
struct sockaddr name;
	socklen_t namesize;
	unsigned int count = 0;
	u_int16_t *pn;
	size_t soal=0;

	if (intype==Inet6) {
		soal = sizeof( struct sockaddr_in6 );
		memset( &socketaddress.soa6, 0,soal );
		socketaddress.soa6.sin6_port = htons( portnum );
		socketaddress.soa6.sin6_family = AF_INET6;
		pn = &socketaddress.soa6.sin6_port;
		socketaddress.soa6.sin6_addr = in6addr_any;
	} else {
		socketaddress.soa4.sin_port = htons( portnum );
		socketaddress.soa4.sin_family = AF_INET;
		socketaddress.soa4.sin_addr.s_addr = INADDR_ANY;
		soal = sizeof( struct sockaddr_in );
		pn = &socketaddress.soa4.sin_port;
		memset( &(socketaddress.soa4.sin_zero), 0,8 ); // is this portable?
	}

	/*
	 * if already bound then close this one and open
	 * a new socket so we can bind it to different port
	 * number
	 */
if (bound) {
		close(socketfd);
		bound = false;
		if(intype==Inet6) { 
		    if ( ( socketfd = socket( AF_INET6, SOCK_DGRAM, 0 ) ) == -1 ) {
			cerr  <<  "=====UDPMessageSocket::listen: socket() failed\n";
			cout  <<  "=====UDPMessageSocket::listen: socket() failed\n";
			return 0;
		    }
		} else {
		    if( ( socketfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 ) {
			cerr  <<  "=====UDPMessageSocket::listen: socket() failed\n";
			cout  <<  "=====UDPMessageSocket::listen: socket() failed\n";
			return 0;
		    }
		}

	}
	while ( ( bind( socketfd, &socketaddress.soa, soal ) == -1)  && (count < retry) ) {	
		count++;
		portnum += 2;
		*pn = htons( portnum );
	}
	if ( count > 10 ) {
		cout  <<  "!!!!!UDPMessageSocket::listen(): bind() failed\n";
		cout  << "#: EINVAL=" <<EINVAL << "EBADF=" << EBADF  << "ENOTSOCK= " << ENOTSOCK << endl;
		cout  << "#: errno=="<< errno << " socketfd=" << socketfd << endl;
		cout  << "#: AF_INET=" << AF_INET <<  "AF_INET6=" << AF_INET6 << endl;
		cout  << "#: sa_family: " <<  socketaddress.soa.sa_family << " intype:" << intype << endl;
		cout  << "#: sin6_port " << ntohs(socketaddress.soa6.sin6_port) << endl;
		cout  << "#: soal= " << (int)soal << endl;
		//hexdump((unsigned char *)&socketaddress.soa6, sizeof (struct sockaddr_in6),1);
		return 0;
	}

	bound = true;
	namesize = sizeof( struct sockaddr );
	memset( &name, 0,sizeof( struct sockaddr ) );
	if ( getsockname( socketfd, &name, &namesize ) == -1 ) {
		cerr  <<  "=====UDPMessageSocket::listen(): getsockname() failed\n" ;
		cout  <<  "=====UDPMessageSocket::listen(): getsockname() failed\n" ;
		return 0;
	}
	ourport = ntohs( ((struct sockaddr_in *) &name)->sin_port );
	return ourport;
}



int UDPMessageSocket::listenOnEvenPort(int min, int max)
{
	size_t soal;

	/* User didn't specify any range, let the OS
	 * assign source port number
	 */


	/* Only max specified, set min to the lowest limit */
	if (min == 0) min = 1026;

	/* Only min specified, set max to the highest port number */
	if (max == 0) max = 65535;

	/* min not odd, increase */
	if (min % 2) min++;

	/* Out of range ? Signal error */
	if (min > max) goto error;

	/*
	 * if already bound then close this one and open
	 * a new socket so we can bind it to different port
	 * number
	 */
	if (bound) {
		close(socketfd);
		bound = false;
		if(intype==Inet6) { 
		    if ( ( socketfd = socket( AF_INET6, SOCK_DGRAM, 0 ) ) == -1 ) {
			cout  <<  "=====UDPMessageSocket::listen: socket() failed\n";
			return -1;
		    }
		} else {
		    if( ( socketfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 ) {
			cout  <<  "=====UDPMessageSocket::listen: socket() failed\n";
			return -1;
		    }
		}

	}

loop:
if (intype==Inet6) {
		soal = sizeof( sockaddr_in6 );
		memset( &socketaddress.soa6, 0,soal );
		socketaddress.soa6.sin6_port = htons(min );
		socketaddress.soa6.sin6_family = AF_INET6;
	} else {
		socketaddress.soa4.sin_port = htons( min );
		socketaddress.soa4.sin_family = AF_INET;
		socketaddress.soa4.sin_addr.s_addr = INADDR_ANY;
		soal = sizeof( sockaddr_in );
		memset( &(socketaddress.soa4.sin_zero), 0,8 ); // is this portable?
	}


	if (bind(socketfd,&socketaddress.soa, soal) == -1) {
		min += 2;
		if (min <= max) goto loop;
		else goto error;
	}

	bound = true;

	/*
	 * FIXME: Do some test for even port number here
	 */

	ourport = min;
	//cout  << "=====UDPMessageSocket::listen(): found " << min << endl;
	return 0;

error:
	cout  << "=====UDPMessageSocket::listen(): Can't find a free port in specified range";
	return -1;

}


int UDPMessageSocket::accept( void )
{
cout  << "=====UDPMessageSocket::accept";
	return socketfd;
}



int UDPMessageSocket::sendStunRequest( QString host, int port )
{
		setHostname( host);
		connect( port);
		StunRequestSimple req;
		req.msgHdr.msgType = htons(BindRequestMsg);
		req.msgHdr.msgLength = htons( sizeof(StunRequestSimple)-sizeof(StunMsgHdr) );
		for ( int i=0; i<16; ++i ) {
			req.msgHdr.id.octet[i]=0;
		}
		int id = rand();
		req.msgHdr.id.octet[0] = id;
		req.msgHdr.id.octet[1] = id>>8;
		req.msgHdr.id.octet[2] = id>>16;
		req.msgHdr.id.octet[3] = id>>24;
		return (send( (char *)&req, sizeof( req ) ) );
}


unsigned int UDPMessageSocket::receiveStunResponse(void )
{
	unsigned int port = 0;
	int i1,i2,i3,i4;
	char ip[17] = { '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
	char inputbuf[ 8000 ];
	int bytesread;
	struct timeval timeout;
	fd_set read_fds;
	int highest_fd;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	FD_ZERO( &read_fds );
	FD_SET( getFileDescriptor(), &read_fds );
	highest_fd = getFileDescriptor() + 1;
retry:
	if( select( highest_fd, &read_fds, NULL, NULL, &timeout ) == -1 ) {
		if( errno == EINTR ) goto retry;
		cerr  <<  "=====UDPMessageSocket: select() punted";
		cout  <<  "=====UDPMessageSocket: select() punted";
		exit( 1 );
	}
	if( FD_ISSET( getFileDescriptor(), &read_fds ) ) {
		bytesread = read( getFileDescriptor(), inputbuf, 8000 - 1 );
		StunMsgHdr* hdr = reinterpret_cast<StunMsgHdr*>( inputbuf );
		if( hdr->msgType == BindResponseMsg ) {
			char* body = inputbuf + sizeof( StunMsgHdr );
			unsigned int size = ntohs( hdr->msgLength );
			while( size > 0 ) {
				StunAtrHdr* attr = reinterpret_cast<StunAtrHdr*>( body );
				unsigned int hdrLen = ntohs( attr->length );
				if( ntohs( attr->type ) == MappedAddress ) {
					StunAtrAddress* attribute =
						reinterpret_cast<StunAtrAddress*>( body );
					if ( attribute->address.addrHdr.family == IPv4Family ) {
						StunAtrAddress4* atrAdd4 =
							reinterpret_cast<StunAtrAddress4*>( body );
						if ( hdrLen == sizeof( StunAtrAddress4 ) - 4 ) {
							port = ntohs( atrAdd4->addrHdr.port );
							i1 = atrAdd4->v4addr & 0xFF;
							i2 = (atrAdd4->v4addr & 0xFF00) >> 8;
							i3 = (atrAdd4->v4addr & 0xFF0000) >> 16;
							i4 = (atrAdd4->v4addr & 0xFF000000) >> 24;
							sprintf( ip, "%d.%d.%d.%d", i1, i2, i3, i4 );
							stunnedHost= QString((char *)&ip[0]);
						}
					}
				}
				body += hdrLen+4;
				size -= hdrLen+4;
			}
		}
	}
	return port;
}

