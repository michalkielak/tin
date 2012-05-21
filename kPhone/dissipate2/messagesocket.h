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

#ifndef MESSAGESOCKET_H_INCLUDED
#define MESSAGESOCKET_H_INCLUDED

#include <netdb.h>
#include <netinet/in.h>

enum InetType {
	InetNone=0,
	Inet4=4,
	Inet6=6 };

/**
* @short Base Class for UDPMessageSocket  and TCPMessageSocket
*
This Class contains the routines to access socket related date
and the voids for UDPMessageSocket and TCPMessageSocket
*
The Class was parially rewritten for IPv6
*/
class MessageSocket
{
public:

	MessageSocket( void );
	virtual ~MessageSocket( void );

	/**
	 * Connect to the remote host on the given port.
	 */
	virtual int connect( unsigned int portnum ) = 0;

	/**
	 * Send the given buffer of the given length.
	 */
	virtual int send( const char *sendbuffer, unsigned int length ) = 0;

	/**
	 * Receive into the given buffer up to the given length.
	 */
	virtual int receive( char *recvbuffer, unsigned int maxlength ) = 0;

	/**
	 * Open up the socket for listening on the given port.
	 */
	virtual unsigned int listen( unsigned int portnum, unsigned int retry ) = 0;

	/**
	 * Accept the incoming connection.
	 */
	virtual int accept( void ) = 0;

	/**
	 * Open up the socket for listening on the given port.(mostly used for RTP)
	 */
	virtual int listenOnEvenPort( int min = 0, int max = 0 ) = 0;

	/**
	 * Sets the remote hostname for this socket. Be careful, this function
	 * currently calls gethostbyname.
	 */
//	bool setHostname( const char *hostname );
	/**
	* resolves the hostname, sets the socket structures and returns the IP-Type:
	  returns 0: failure
	          4: remote address is IPv4
	          6: remote address is IPv6
	*/	
	int setHostname( const char *hostname );
	/**
	* used by setHostname to resolve the hostname
	*/
	static int lookupHostname( const char *hostname, unsigned char *store, const int len);
	/**
	* return the IP-Type actually used
	*/
	static int getHostIpType( const char *hostname );

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
	* used?
	*/
	static void disableIpv6Lookups( void );
	/**
	*returns   4: numeric IPv4
	*          6: numeric IPv6
	*          0: none of both, probalby a hostname
	*/
	static int getSocketIpVersion( void );

protected:
	// Socket Info
	struct hostent *he;
	union ipaddr {
		unsigned char    raw_addr[16];
		struct in6_addr  in6_addr; // in networkbyteorder
		struct in_addr   in4_addr; // dto.
	};
	union ipaddr ip_addr;

	int socketfd;
	SocketType type;
	static InetType intype;
	static int ipv6_lookups_enabled;
	unsigned int ourport;

	union sockaddr_in46 {
		struct sockaddr     soa;
		struct sockaddr_in  soa4;
		struct sockaddr_in6 soa6;
	};
	union sockaddr_in46 socketaddress;
	struct sockaddr_in  tsocketaddress;
	union sockaddr_in46 remoteaddress;
	bool bound;


private:
	int buffsize;
	char *ipaddress;
	char *hostname;
	char *callid;
	int port;
};

#endif // MESSAGESOCKET_H_INCLUDED
