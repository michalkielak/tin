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

#ifndef TCPMESSAGESOCKET_H_INCLUDED
#define TCPMESSAGESOCKET_H_INCLUDED

#include <qptrlist.h>

#include "messagesocket.h"

/**
 * This is the TCP implementation of the MessageSocket class.  It is used
 * for performing operations on a new or existing TCP socket.
 *
 * @short Generic TCP Socket class
 * @author Billy Biggs <bbiggs@bbcr.uwaterloo.ca>
 */
class TCPMessageSocket : public MessageSocket
{
public:
	TCPMessageSocket( void );
	TCPMessageSocket( int newfd );
	virtual ~TCPMessageSocket( void );

/**
*	Connect to the remote host on the given port.
*/
	int connect( unsigned int portnum );
/**
*	Send the given buffer of the given length.
*/
	int send( const char *sendbuffer, unsigned int length );
/**
* 	Receive into the given buffer up to the given length.
*/
	int receive( char *recvbuffer, unsigned int maxlength );
/**
*	try to bind this portnumber to out local socket, retry retries are allowed,
*	the portnumber is increased by 2 each time
*/
	unsigned int listen( unsigned int portnum, unsigned int retry );
/**
*	Accept the incoming connection.
*/
	int accept( void );
/**
*	try to bind a portnumber in this range to our local socket
*	the portnumber is increased by 2 each time
*/
	int listenOnEvenPort( int min = 0, int max = 0 );
/**
*	Sets the remote hostname and port for this socket.
*/
	bool setHostnamePort( const char *hostname, unsigned int port );
/**
*	compare the socket's address with the input
*/
	bool cmpSocket( const char *hostname, unsigned int port );

private:
	char myAddr[100];
	unsigned int portnum;
};


typedef QPtrListIterator<TCPMessageSocket> TCPMessageSocketIterator;


#endif // TCPMESSAGESOCKET_H_INCLUDED
