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

#ifndef SIPCLIENT_H_INCLUDED
#define SIPCLIENT_H_INCLUDED

#include <qobject.h>
#include <qstring.h>
#include <qptrlist.h>

#include "sipuri.h"
#include "udpmessagesocket.h"
#include "tcpmessagesocket.h"
#include "sipcall.h"
#include "sipuser.h"
#include "../config.h"


class SipStatus;
class SipMessage;
class SipCallMember;
class MimeContentType;

typedef unsigned char  UInt8;
typedef unsigned short UInt16;
typedef unsigned int   UInt32;
typedef struct { unsigned char octet[16]; }  UInt128;

// define types for a stun message
const UInt16 BindRequestMsg          = 0x0001;
const UInt16 BindResponseMsg         = 0x0101;
const UInt16 BindErrorMsg            = 0x0111;
const UInt16 SharedSecretRequestMsg  = 0x0002;
const UInt16 SharedSecretResponseMsg = 0x0102;

/// define a structure to hold a stun address
const UInt8  IPv4Family = 0x01;
const UInt8  IPv6Family = 0x02;

// define  stun attribute
const UInt16 MappedAddress = 0x0001;

/**
* @short Used by SipClient and friends
*/ 
typedef struct
{
	UInt8 pad;
	UInt8 family;
	UInt16 port;
} StunAddrHdr;

/**
* @short Used by SipClient and friends
*/
typedef struct
{
	UInt16 type;
	UInt16 length;
} StunAtrHdr;

/**
* @short Used by SipClient and friends
*/ 
typedef struct
{
	UInt16 msgType;
	UInt16 msgLength;
	UInt128 id;
} StunMsgHdr;

/**
* @short Used by SipClient and friends
*/ 
typedef struct
{
	StunAddrHdr addrHdr;
	union {
		UInt32  v4addr;
		UInt128 v6addr;
	} addr;
} StunAddress;

/**
* @short Used by SipClient and friends
*/ 
typedef struct
{
	UInt16 type;
	UInt16 length;
	StunAddress address;
} StunAtrAddress;

/**
* @short Used by SipClient and friends
*/ 
typedef struct
{
	UInt16 type;
	UInt16 length;
	StunAddrHdr addrHdr;
	UInt32  v4addr;
} StunAtrAddress4;

/**
* @short Used by SipClient and friends
*/ 
typedef struct
{
	StunMsgHdr msgHdr;
} StunRequestSimple;

/**
* @short base of the SIP Machine 
* 
* Base of the SIP Processing. Receives and sends SIP datagrams, 
* analyses incoming datagrams and decides to reject or to process them.
* 
* The class is instantiated by KPhoneView
* 
* the meth. KPhoneView::incomingCall is use to start processing an incoming call
*
* Friends SipUser SipCall;
*/
class SipClient : public QObject
{
	Q_OBJECT

	friend class SipUser;
	friend class SipCall;
public:
/**
*	The constructor
*/
	SipClient( QObject *parent = 0, const char *name = 0, unsigned int newListenport = 0, unsigned int lpRep = 10,
		bool newLooseRoute = true, bool newStrictRoute = true, QString socketStr = QString::null,QString uristr = QString::null, QString dhost = QString::null,int ipv6=0, bool d2U=false,QString locServ = "NAPTR");
	~SipClient( void );	
/**
*	Contact information, used to display who we are (IP, IP type, ...)
*/
	QString getContactText(void);
/**
*	find the IP type of an address
*/
	int ipType(QString s);
/**
*	we use IPV6
*/
	bool ipv6(void) {return useIPv6;}
/**
*	core of the signal scanner, 
*	snoops on the line to retrieve SIP+STUN messages for us
*/
	void doSelect( bool block );
/**
*	keeps the line open (used to refresh the NAT on out path)
*/
	void doAlive (void);
/**
*	send a stun request for signalling
*/
	void sendStunRequest( const QString uristr = QString::null ); 
/**
* stop using STUN
*/
	void noMoreStun(void) { stopStun = true;}
/**
*test if STUN was switched off
*/
	bool haveNoMoreStun(void);
/**
* get the stored contact URI
*/
	const SipUri &getContactUri( void ) { return contacturi; }
/**
* prepare the contact URI for our SipUser
*/
	void setupContactUri(  SipUser *user = 0 );
	
/**
* return the SipUser incarnation
*/
	SipUser *getUser( void ) const { return user; }
	
/**
* store a pointer to our SipUser incarnation
*/
	void setUser( SipUser *user );
	
/**
* set the URI for CFU
*/
	void setCallForwardUri( const SipUri &u );
	
/**
* return the URI for CFU
*/
	const SipUri &getCallForwardUri( void ) { return forwarduri; }
	
/**
* toggle CFU activation?
*/
	void setCallForward( bool onoff );
	
/**
* should we use CFU?
*/
	bool getCallForward( void ) const { return fwmode; }

/**
* toggle CFB activation?
*/
	void setCallForwardB( bool onoff );
	
/**
* should we use CFB?
*/
	bool getCallForwardB( void ) const { return fwbmode; }
	
/**
* set the message transported with 302
*/
	void setCallForwardMessage( const QString &newmessage );
	
/**
* return  the message to be transported with 302
*/
	QString getCallForwardMessage( void ) const { return fwbody; }
	
/**
* unused ?
*/
	void setBusy( bool onoff );
	
/**
* unused ?
*/
	bool getBusy( void ) const { return busymode; }
	
/**
* unused ?
*/
	void setBusyMessage( const QString &newmessage );
/**
* unused ?
*/
	QString getBusyMessage( void ) const { return busybody; }
	
/**
* set usage of an outbound proxy?
*/
	void setExplicitProxyMode( bool eproxy );
	
/**
* should we use an outbound proxy?
*/
	bool getExplicitProxyMode( void ) { return useExplicitProxy; }
	
/**
* set the known proxy address
*/
	void setExplicitProxyAddress( const QString &newproxy );
	
/**
* return the proxy address
*/
	const QString &getExplicitProxyUri( void ) { return proxy; }
	
/**
* calculate the proxy address
*/
	QString getExplicitProxyAddress( void );
		
/**
* how many hops are allowed
*/
	void setMaxForwards( int newmax );
	
/**
* return the number of allowed hops
*/
	int getMaxForwards( void ) const { return maxforwards; }

/**
* the usage of the via header
*/
	enum HideViaMode {
		DontHideVia,
		HideHop,
		HideRoute };
/**
* set the usage of the via header
*/
	void setHideViaMode( HideViaMode newmode );
	
/**
* return the usage of the via header
*/
	HideViaMode getHideViaMode( void ) { return hidemode; }

/**
* define, if to use the symmetric mode for signalling 
* at this time always true, assymmetric mode is rarely used
*/
	void setSymmetricMode( bool newmode );

/**
* should we use symmetric mode for signalling
*/
	bool getSymmetricMode( void ) { return symmetricmode; }

/**
* return pointer to our list of existing calls
*/
	SipCallIterator getCallList( void ) const { return SipCallIterator( calls ); }
	
/**
* returns KStatics:me
*/
	static const QString getUserAgent( void );
	
/**
* returns the authenitcation user's name
*/
	QString getAuthenticationUsername( void ) const { return authentication_username; }
	
/**
* returns the password for authentication
*/
	QString getAuthenticationPassword( void ) const { return authentication_password; }
	
/**
* returns the server we send to
*/
	QString getSipProxy( void ) const { return proxy; }

/**
* returns our URI
*/
	SipUser *getUser( SipUri uri );

/**
* refresh the address of the server we send to
*/
	void updateIdentity( SipUser *user, QString newproxy);
	
/**
* UDP or TCP requested ?
*/
	enum Socket { UDP, TCP };
	
/**
* should we use TCP?
*/
	bool isTcpSocket( void ) const { return SocketMode == TCP; }
	
/**
* request UDP or TCP 
*/
	void setSocketMode( Socket socket ) { SocketMode = socket; }
	
/**
* DNS handling, what transformation should we request
*/
	enum Locserv {DNS,SRV,NAPTR};
	
/**
* DNS handling,set the transformation to be requested
*/
	void setLocServ (Locserv lserv) { locServ = lserv; }
	
/**
* DNS handling,return the transformation to be requested
*/
	Locserv getLocServ (void) { return locServ; }
	
/**
* DNS handling,set the transformation to be requested, withQString - enum translation
*/
	void initLocServ (QString locServ);
		
/**
* our SIP Server 
*/
	QString getSipProxySrv( QString dname );
	
/**
* should we use the loose route method for REGISTER
*/
	bool isLooseRoute( void ) const { return looseRoute; }
	
/**
* should we use the strict route method for REGISTER
*/
	bool isStrictRoute( void ) const { return strictRoute; }
	
/**
* used by Sip Options
*/
	void sendTestMessage( QString sendaddr, unsigned int port, QString msg );
	
/**
* unused
*/
	void setTest( bool on ) { testOn = on; }
	
/**
* DNS handling: NAPTR transformation
*/
	QString getNAPTR( QString strUri );
	
/**
* DNS handling: SRV transformation
*/
	QString getSRV( QString naptr );
	
	
/**
* get our queue of TCPMessageSocket in use
*/
	TCPMessageSocketIterator getTcpSocketList( void ) const { return TCPMessageSocketIterator( tcpSockets ); }
	
/**
* the port we listen on
*/
	QString  getListenPort(void);

private slots:	

/**
* Signal conection to the SipCalls
*/
	void callMemberUpdated( void );

signals:	
/**
*	signals that a new call reached us
*/
	void incomingCall( SipCall *, QString );
/**
*	make a KCallWidget set it's hide flag
*/
	void hideCallWidget( SipCall * );
/**
*	update the call list
*/	
	void callListUpdated( void );
/**
*	signals that a new IM reached us
*/
	void incomingInstantMessage( SipMessage * );
/**
*	signals that a buddy update
*/
	void incomingNotify( SipMessage * );
/**
*	signals that someone wants to subscribe our presence information
*/
	void incomingSubscribe( SipCallMember *, bool );
/**
*	experimental
*/
	void incomingTestMessage();
/**
*	signals that someone wants to refresh her subscription
*/
	void updateSubscribes( void );
/**
*	signals that someone wants to terminate her subscription
*/
	void terminateSubscribes( int mess, SipCall* call );

private:
	QString myProxy;
	QPtrList<SipCall> calls;
	QString authentication_username;
	QString authentication_password;
	Socket SocketMode;
	int clilen, newsockfd;
	struct sockaddr_in cli_addr;
	unsigned int portRepeater;
	// Our proxy, if applicable.
	QString proxy;
	unsigned int proxyport;
	bool useExplicitProxy;
	bool direct2URI;
	SipUri sipProxy;
	QString sipProxySrv;
	QString sipProxyName;
	SipUri contacturi;
	Locserv locServ;
	// Call forwarding
	bool fwmode;
	bool fwbmode;
	SipUri forwarduri;
	QString fwbody;

	// Busy
	bool busymode;
	QString busybody;

	// Max-forwards
	int maxforwards;

	// Via hide mode
	HideViaMode hidemode;

	// Symmetric signalling mode
	bool symmetricmode;

	SipUser *user;
	UDPMessageSocket *listener;
	TCPMessageSocket TCP_listener;

	// Log stuff to a file
	int loggerfd;

/**
*	Audit pending messages (retransmissions).
*/
	void auditPending( void );

/**
*	prepare the sockets needed for SIPping
*/
	bool setupSocketStuff( unsigned int newListenport, unsigned int lpRep, QString socketStr, bool useL4 );
/**
*	checks if a incomming data packet is a valid STUN or SIP message
*	assure completeness
*/
	void incomingMessage( int socketfd, bool closeS );
/**
*	evaluate an incomming SIP message
*/
	void parseMessage( QString fullmessage );
/**
*	used for request, that need sponanous respose only
*/
	void sendQuickResponse( SipMessage *origmessage, const SipStatus &status,

	const QString &body = QString::null,
	const MimeContentType &bodytype = MimeContentType::null );

/**
*	send message ACK
*/
	void sendAck( SipMessage *origmessage );

	// These methods are for SipCall
/**
*	add a call to the SipCall list
*/
	void addCall( SipCall *call );
/**
* 	delete a call from the SipCall list
*/
	void deleteCall( SipCall *call );
/**
*	build and send a SIP request
*/
	bool sendRequest( bool sendToURI, int calltype, SipMessage *msg, bool contact = true, const SipUri &regProxy = SipUri::null, const QString &branch = QString::null );
/**
*	build and send a SIP response
*/
	void sendResponse( SipMessage *msg, bool contact = true );
/**
* ????
*/
	void sendPubResponse(SipMessage *origmessage, const SipStatus &status);
/**
*	just send the stuff
*/
	void sendRaw( SipMessage *msg );
/**
*	calls callListUpdate
*/
	void callTypeUpdated( void );
/**
*	extended DNS lookup
*/
	QString getResSearch( QString dname, int type, bool UDP );

	QString messageCID;
	QString subscribeCID;
	bool useStunProxy;
	SipUri stunProxy;
	QString messageCSeq;
	QString subscribeCSeq;
	bool looseRoute;
	bool strictRoute;

	bool testOn;
	bool useIPv6;
	int debug;
	bool stopStun;
	int nlp;
	bool sentReg;
	bool noSRV;
	bool foundStun;
	TCPMessageSocket *tcpSocket;
	QPtrList<TCPMessageSocket> tcpSockets;
	QPtrList<MessageSocket> sndSockets;
};

#endif // SIPCLIENT_H_INCLUDED
