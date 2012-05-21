/*
 * Copyright (c) 2000 Billy Biggs <bbiggs@div8.net>
 * Copyright (c) 2004 Wirlab <kphone@wirlab.net>
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

#ifndef EXTMESSAGESOCKET_H_INCLUDED
#define EXTMESSAGESOCKET_H_INCLUDED


/**
 * This Class opens a Socket and reserves it for the external
 * application. The socket should be closed shortly before
 * the application beginns to use it.
 * @short port place holder for external payloads
 */
class EXTMessageSocket 
{
public:
	EXTMessageSocket( bool usetcp, int min,int max );
	~EXTMessageSocket( void );
	static int port_count;	bool setHostname( const char *hostname );

	enum SocketType {
		None,
		SocketTCP,
		SocketUDP };

	/**
	 * Return the socket file descriptor for use in select.
	 */
	int getFileDescriptor( void ) const { return socketfd; }

	/**
	 * Returns the socket type, either MessageSocket::SocketTCP or
	 * MessageSocket::SocketUDP.
	 */
	SocketType getSocketType( void ) const { return type; }

	/**
	 * Returns the port number we are currently connected to or listening
	 * on.
	 */
	unsigned int getPortNumber( void ) const { return ourport; }

	/**
	 * Forces our local port number to be something specific.
	 */
	void forcePortNumber( unsigned int newport );
	/**
	 * For RTP badness.
	 */
	int listenOnEvenPort(void);

protected:
	struct hostent *he;
	int socketfd;
	SocketType type;
	unsigned int ourport;
	struct sockaddr_in socketaddress;
	struct sockaddr_in remoteaddress;
	bool bound;

private:
	int minP;
	int maxP;
	int buffsize;
	char *ipaddress;
	char *hostname;
	char *callid;
	int port;
	bool didcomplain;
};

#endif // EXTMESSAGESOCKET_H_INCLUDED
