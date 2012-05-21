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

#ifndef UDPMESSAGESOCKET_H_INCLUDED
#define UDPMESSAGESOCKET_H_INCLUDED

#include "messagesocket.h"

/**
 * This is the UDP implementation of the MessageSocket class.  It is used
 * for performing operations on a new or existing UDP socket.
 * Partially rewritten for IPv6
 * 
 * @short Generic UDP Socket class
 * @author Billy Biggs <bbiggs@bbcr.uwaterloo.ca>
 */
class UDPMessageSocket : public MessageSocket
{
public:
/** 
*    constructor with initialisation
*/
	UDPMessageSocket( bool useIPv6 );
/** 
*    bare constructor
*/
	UDPMessageSocket( void );
/** 
*    constructor sets given socket
*/
	UDPMessageSocket(int newfd );
	~UDPMessageSocket( void );
/**
*	fills the struct remoteaddress with data
*/
	int connect( unsigned int portnum );
/**
*	set TOS field
*/
	int SetTOS( void );
/**
*	send the data
*/
	int send( const char *sendbuffer, unsigned int length );
/**
*	receive from remote
*/
	int receive( char *recvbuffer, unsigned int maxlength );
/**
*	try to bind this portnumber to out local socket, retry retries are allowed,
*	the portnumber is increased by 2 each time
*/
	unsigned int listen( unsigned int portnum, unsigned int retry );
/**
*	 In case of UDP accept simply returns the socketfd
*/
	int accept( void );
/**
*	try to bind a portnumber in this range to our local socket
*	the portnumber is increased by 2 each time
*/
	int listenOnEvenPort( int min = 0, int max = 0 );

/** 
*    reinit for constructor
*/
	void reInit( int newfd );
/** 
*    reinit for  constructor
*/
	void reInit( bool useIPv6 );
/**
*	send a STUN request package to the giveb server
*/
	int sendStunRequest( QString host, int port );
/**
*	STUN server's response
*/
	unsigned int receiveStunResponse(void );
/**
*	get the host IP after stunning
*/
	QString getStunnedHost(void) { return stunnedHost;}

private:
	bool didcomplain;
	QString stunnedHost;
};

#endif // UDPMESSAGESOCKET_H_INCLUDED
