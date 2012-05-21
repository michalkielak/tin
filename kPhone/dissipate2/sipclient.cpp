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
// #include <pwd.h>	
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <qdatetime.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "../kphone/kphone.h"

#include "siputil.h"
#include "sipuser.h"
#include "sipvia.h"
#include "sipcall.h"
#include "tcpmessagesocket.h"
#include "sipmessage.h"
#include "mimecontenttype.h"
#include "sipstatus.h"
#include "siptransaction.h"
#include "sipclient.h"
#include "../kphone/kstatics.h"
#include <iostream>
using namespace std;

typedef unsigned int u_int;
typedef unsigned short u_short;
typedef unsigned char u_char;

/** @short used by SipClient */
struct s_SOA
{
	char *mname;
	char *rname;
	u_int serial;
	u_int refresh;
	u_int retry;
	u_int expire;
	u_int minimum;
};

/** @short used by SipClient */
struct s_NULL
{
	char *anything;
	u_short length;         /* Length of valid data */
};

/** @short used by SipClient */
struct s_WKS
{
	struct in_addr address;
	char *bitmap;
	u_int maplength;
	u_char protocol;
};

/** @short used by SipClient */
struct s_HINFO
{
	char *cpu;
	char *os;
};

/** @short used by SipClient */
struct s_MINFO
{
	char *rmailbx;
	char *emailbx;
};

/** @short used by SipClient */
struct s_MX
{
	char *exchange;
	u_short preference;
};

/** @short used by SipClient */
struct s_TXT
{
	char *text;
	struct s_TXT *next;
	u_short len;
};

/** @short used by SipClient */
struct s_SRV
{
	u_short priority;
	u_short weight;
	u_short port;
	char *target;
};

/** @short used by SipClient */
struct s_NAPTR
{
	u_short order;
	u_short pref;
	char *flags;
	char *service;
	char *regexp;
	char *replacement;
};

/** AFS servers used by SipClient */
struct s_AFSDB
{
	u_short subtype;
	char *hostname;
};
/** @short Responsible Person */
struct s_RP
{
	char *mbox_dname;
	char *txt_dname;
};

/** @short ISDN Address */
struct s_ISDN
{
	char *address;
	char *sa;    /* optional */
};

/** @short Route Through */
struct s_RT
{
	u_short preference;
	char *int_host;
};

/** @short Generic RDATA RR structure */
union u_rdata
{
	char *string;            /* Any simple string record */
	u_int number;            /* Any simple numeric record */
	struct in_addr address; /* Simple address (A record) */

/** @short other structured RR types */
	struct s_SOA soa;
/** @short other structured RR types */
	struct s_NULL null;
/** @short other structured RR types */
	struct s_WKS wks;
/** @short other structured RR types */
	struct s_HINFO hinfo;
/** @short other structured RR types */
	struct s_MINFO minfo;
/** @short other structured RR types */
	struct s_MX mx;
/** @short other structured RR types */
	struct s_TXT txt;
/** @short other structured RR types */
	struct s_SRV srv;
/** @short other structured RR types */
	struct s_NAPTR naptr;


/** @short other structured RR types */
	struct s_AFSDB afsdb;
/** @short other structured RR types */
	struct s_RP rp;
/** @short other structured RR types */
	struct s_ISDN isdn;
/** @short other structured RR types */
	struct s_RT rt;
};

/** @short  Full RR structure */
typedef struct s_rr
{
	char *name;
	u_short type;
	u_short xclass;
	u_int ttl;
	u_int dlen;
	union u_rdata rdata;
} s_rr;

/** @short  DNS Question sctructure */
typedef struct s_question
{
	char *qname;
	u_short qtype;
	u_short qclass;
} s_question;

/** @short  Full DNS message structure */
typedef struct s_res_response
{
	HEADER header;
	s_question **question;
	s_rr **answer;
	s_rr **authority;
	s_rr **additional;
} res_response;


/** The SipClient
*/
SipClient::SipClient( QObject *parent, const char *name, unsigned int newListenport, unsigned int lpRep,
			bool newLooseRoute, bool newStrictRoute, QString socketStr, QString uristr,QString dhost, int ipv6, bool d2U, QString locServ )
	: QObject( parent, name )
{
	initLocServ(locServ);
	debug = KStatics::debugLevel;

	if( socketStr == "UDP" ) {
		SocketMode = UDP;
	} else {
		SocketMode = TCP;
	}

	switch (ipv6) {
	case 0 : useIPv6=false; 
		KStatics::haveIPv6=false;  
		break;
	case 1 : useIPv6=true; 
		KStatics::haveIPv6=true;  

		break;
	case 2 :
		if (ipType(uristr) == 6){
		    KStatics::haveIPv6=true;  
		    useIPv6=true;
		    if(debug >= 2) cout << "=====SipClient: use IPv6\n";
		} else {
		    KStatics::haveIPv6=false;  
		    useIPv6=false;
		    if(debug >= 2) cout  <<  "=====SipClient: use IPv4\n";
		}
		if(dhost != "") Sip::setLocalAddress(dhost);
		if( Sip::getLocalAddress().isEmpty() ) {
			if(KStatics::debugLevel>=2) cout  << "====SipClient::neither IPv6 nor IPv4\n";
		    exit(0);
		}
	}
	KStatics::haveIPv6 = useIPv6;
	direct2URI = d2U;
	debug = KStatics::debugLevel;
	nlp=newListenport;
	portRepeater = lpRep;
	if( !setupSocketStuff( newListenport, lpRep, socketStr, (int)useIPv6 ) ) {
		if(KStatics::debugLevel>=2) cout  << "====SipClient::setupSocketStuff() Failed.\n";
		exit( 1 );
	}
	myProxy = "";
	noSRV = false;
	sentReg=false;
	setupContactUri();
	KStatics::myIP = contacturi.getHostname();
	useExplicitProxy = false;
	proxyport = 5060;
	calls.setAutoDelete( false );
	tcpSockets.setAutoDelete( true );
	sndSockets.setAutoDelete( true );
	fwmode = false;
	fwbmode = false;
	busymode = false;
	user = 0;
	hidemode = DontHideVia;
	symmetricmode = false;
	maxforwards = 0;
	looseRoute = newLooseRoute;
	strictRoute = newStrictRoute;
	testOn = false;
	useStunProxy = false;
	tcpSocket = 0;
	stopStun = false;
	foundStun = false;

}

SipClient::~SipClient( void )
{
}
int SipClient::ipType(QString s) {

    QString s1,host,host1;
    int i,j,l, errcode;
    struct addrinfo hints, *res;
    if(s=="") return 4;
    char addrstr[100];
    i=s.find("<sip:");
    j=s.find(">");
    s1 = s.mid(i+1,j-i-1);
    i = s1.find("@");
    j = s1.find(";");
    if(j>0) l=j; else l = s1.length();
    host1 = s1.mid(i+1, l-i-1);
    j = s1.find("");
    memset (&hints, 0, sizeof (hints)); 
    hints.ai_family = PF_UNSPEC;
    if ( host1.contains("[") ) {
		host=host1.mid( host1.find("[") + 1 ,  host1.find("]") - 1 );
		if (host1.find(":") >0) return 6;
    } else {
        j = host1.find(":");
	if(j>0) {
	    host=host1.mid(0,j);
	} else {
	    host=host1;
	}
    }
    host=getSipProxySrv(host);
    int ind = host.findRev(':');
    if(ind>0) host = host.left(ind);
    errcode = getaddrinfo (host.latin1(), NULL, &hints, &res);  
    if (errcode != 0)
    {
      perror ("SipClient::ipType getaddrinfo");
      return 0;
    }

    inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);

    switch (res->ai_family)
    {
    case AF_INET: 
    	return 4; 
    case AF_INET6: 
    	return 6;
    default: 
	return 0;
    }
}
void SipClient::doAlive(){
	//return;
	if(proxy=="") return;
	if( !isTcpSocket() ) {
		UDPMessageSocket* s;
		UDPMessageSocket sendsocket;
		if (symmetricmode)
			s = listener;
		else
			s = &sendsocket;
		if (!s->setHostname (proxy.utf8().data())) return;
		s->connect (proxyport);
		s->send ("\r\n", 2);
	}
}


void SipClient::setupContactUri( SipUser *user )
{
	if( user ) {
		contacturi.setFullname( user->getUri().getFullname() );
		contacturi.setUsername( user->getUri().getUsername() );
	}
	contacturi.setHostname( Sip::getLocalAddress() );
	if( isTcpSocket() ) {
		contacturi.setPortNumber( TCP_listener.getPortNumber() );
		contacturi.setTransportParam( SipUri::TCP );
	} else {
		contacturi.setPortNumber( listener->getPortNumber() );
		contacturi.setTransportParam( SipUri::UDP );
	}
}

bool SipClient::setupSocketStuff( unsigned int newListenport, unsigned int lpRep, QString socketStr, bool useIPv6 )
{
	unsigned int listenport = 0;
        portRepeater = lpRep;
	if( socketStr == "UDP" ) {
		SocketMode = UDP;
	} else {
		// to TCP Socket for IPv6 so far
		if(useIPv6) return false;
		SocketMode = TCP;
	}

	if( newListenport ) {
		listenport = newListenport;
	}
	if( listenport == 0 ) {
		listenport = 5060;
	}
	if( isTcpSocket() ) {
		listenport = TCP_listener.listen( listenport, lpRep );
		if( !listenport ) { return false; }
		TCP_listener.forcePortNumber( listenport );
	} 
	//else {
	listener = new UDPMessageSocket(useIPv6);
 	listenport = listener->listen( listenport, lpRep );
	if( !listenport ) { return false; }
	nlp=listenport;
	listener->forcePortNumber( listenport );
	if(debug >= 2) cout  <<  "=====SipClient: Listening UDP on port:" << listener->getPortNumber() << endl;
//	}
	return true;
}

void SipClient::doSelect( bool block )
{
	struct timeval timeout;
	fd_set read_fds;
	int highest_fd;
	timeout.tv_sec = 0;
	timeout.tv_usec = 8;
	FD_ZERO( &read_fds );
	FD_SET( listener->getFileDescriptor(), &read_fds );
	highest_fd = listener->getFileDescriptor() + 1;

retry:
	if( select( highest_fd, &read_fds, NULL, NULL, block ? NULL : &timeout ) == -1 ) {
		if( errno == EINTR ) goto retry;
		cout  <<  "=====SipClient::doSelect(): select() punted" << endl;
	}
	if( FD_ISSET( listener->getFileDescriptor(), &read_fds ) ) {
		incomingMessage( listener->getFileDescriptor(),false );
	}
if( isTcpSocket() ) {
		FD_ZERO( &read_fds );
		FD_SET( TCP_listener.getFileDescriptor(), &read_fds );
		highest_fd = TCP_listener.getFileDescriptor() + 1;

retry2:
		if( select( highest_fd, &read_fds, NULL, NULL, block ? NULL : &timeout ) == -1 ) {
		 	if( errno == EINTR ) goto retry2; 
			    cout  <<  "====SipClient::doSelect(): select() punted\n";
		}

		if( FD_ISSET( TCP_listener.getFileDescriptor(), &read_fds ) ) {
			clilen = sizeof(cli_addr);
			newsockfd = ::accept( TCP_listener.getFileDescriptor(), (struct sockaddr *) &cli_addr, (socklen_t*)&clilen);
			//mark socket to be closed immediately after reading
			incomingMessage( newsockfd,true );
		
		}
		TCPMessageSocketIterator it( getTcpSocketList() );
		for (it.toFirst(); it.current(); ++it) {
			tcpSocket = it.current();

			if( tcpSocket != 0 ) {
				FD_ZERO( &read_fds );
				FD_SET( tcpSocket->getFileDescriptor(), &read_fds );
				highest_fd = tcpSocket->getFileDescriptor() + 1;

retry3:
				if( select( highest_fd, &read_fds, NULL, NULL, block ? NULL : &timeout ) == -1 ) {
					if( errno == EINTR ) goto retry3;
					cout  <<  "====SipClient::doSelect(): select() punted\n" ;
				}
				if( FD_ISSET( tcpSocket->getFileDescriptor(), &read_fds ) ) {
				incomingMessage( tcpSocket->getFileDescriptor(),false );
				}
			}
		}
	} else {
		auditPending();
	}
}

void SipClient::auditPending( void )
{
	SipCall *curcall;
	SipTransaction *curtrans;
	for( curcall = calls.first(); curcall != 0; curcall = calls.next() ) {
		for( curtrans = curcall->getTransactionList().first(); curtrans != 0;
		     curtrans = curcall->getTransactionList().next() ) {
			if( curtrans->auditPending() ) {
				return;
			}
		}
	}
}

void SipClient::incomingMessage( int socketfd,bool closeS )
{
	QString fullmessage;
	char inputbuf[ 8000 ];
	int bytesread=1;
	int i1,i2,i3,i4;
	unsigned int port;
	char ip[16];
	unsigned int contentLength; 
	QString lSearch;
	int lLen=0;
	int errFor = 0;
	for(;;) {	
		// Receive the message
		errFor=0;
		bytesread = read( socketfd, inputbuf, 8000 - 1 );
		if( errno == EWOULDBLOCK )  errFor = errno;
		StunMsgHdr* hdr = reinterpret_cast<StunMsgHdr*>( inputbuf );
		if( hdr->msgType == BindResponseMsg ) {
				// check that the size of the header isn't larger than what we've read
				if ((signed int)sizeof(StunMsgHdr) > bytesread)
				{
					cout  << "====SipCient: STUN response Malformed packet (sizeof(StunMsgHdr) > bytesread)\n";
					return;
				}
				//if (debug) 
				//cout  <<  "=====SipClient: STUN response:\n " << inputbuf " << \n ===== \n";
				char* body = inputbuf + sizeof( StunMsgHdr );
				unsigned int size = ntohs( hdr->msgLength );
				port = listener->getPortNumber();
				while( size > 0 ) {
					StunAtrHdr* attr = reinterpret_cast<StunAtrHdr*>( body );
					unsigned int hdrLen = ntohs( attr->length );
					// check that our attribute length is not larger than the remaining size
					if (hdrLen+4 > size)
					{
						cout  << "====SipCient: STUN response Malformed packet (hdrLen+4 > size)\n";
						return;
					}
					if( ntohs( attr->type ) == MappedAddress ) {
						StunAtrAddress* attribute = reinterpret_cast<StunAtrAddress*>( body );
						if ( attribute->address.addrHdr.family == IPv4Family ) {
							StunAtrAddress4* atrAdd4 = reinterpret_cast<StunAtrAddress4*>( body );
							if ( hdrLen == sizeof( StunAtrAddress4 ) - 4 ) {
								port = ntohs( atrAdd4->addrHdr.port );
								//if(debug) cout  <<  "=====SipCient: STUN response address_port:   %d\n", port );
								i1 = atrAdd4->v4addr & 0xFF;
								i2 = (atrAdd4->v4addr & 0xFF00) >> 8;
								i3 = (atrAdd4->v4addr & 0xFF0000) >> 16;
								i4 = (atrAdd4->v4addr & 0xFF000000) >> 24;
								sprintf( ip, "%d.%d.%d.%d", i1, i2, i3, i4 );
								if(debug >0) cout <<  "======SipClient " << Sip::getLocalAddress().latin1() << ":" << nlp << " : STUN response " << ip << ":" << port << endl;
							}
						}
					}
					body += hdrLen+4;
					size -= hdrLen+4;
				}
				SipRegister *current;
				if( !QString(ip).contains( Sip::getLocalAddress() ) ||
					listener->getPortNumber() != port ) {
					foundStun=true;	
					Sip::setLocalAddress( ip );
					listener->forcePortNumber( port );
					setupContactUri();
					QPtrListIterator<SipRegister> reg = user->getSipRegisterList();
					for( ; reg.current(); ++reg ) {
						current = reg.current();
						if( current->getRegisterState() == SipRegister::Connected ||
						current->getAutoRegister() ) {
							current->setAutoRegister( false );
							current->requestRegister();
						}
					}
				} else {
					QPtrListIterator<SipRegister> reg = user->getSipRegisterList();
					for( ; reg.current(); ++reg ) {
						current = reg.current();
						if( current->getAutoRegister() ) {
							current->setAutoRegister( false );
							current->requestRegister();
						}
					}
				}
				return;
		} else if( haveNoMoreStun() ) {
			foundStun=false;
			SipRegister *current;
			Sip::setLocalAddress( 0 );
			int listenport = listener->listen( nlp,portRepeater );
			nlp=listenport;
			listener->forcePortNumber( listenport );
			setupContactUri();
			QPtrListIterator<SipRegister> reg = user->getSipRegisterList();
			for( ; reg.current(); ++reg ) {
				current = reg.current();
				if( current->getRegisterState() == SipRegister::Connected ||
				current->getAutoRegister() ) {
					current->setAutoRegister( false );
					current->requestRegister();	
				}
			}
		}
		if( bytesread < 0 ) {
		cout  << "====SipClient: incoming message read failed\n";
		    	if ((tcpSocket != &TCP_listener) && !closeS) {
				tcpSockets.remove( tcpSocket );
				tcpSocket = 0;
			}
		return;
		} else {
			fullmessage.append( QString::fromUtf8( inputbuf, bytesread ) );
		}	
		// Check if long or short form
		if( isTcpSocket() ) {
		    if( fullmessage.contains( "Content-Length: " ) > 0 ) {
			lSearch = "Content-Length: "; //long
			lLen=16;
		    } else {
			lSearch = "l: "; // short
			lLen=3;
		    }
		}

		if( isTcpSocket() ) {
				QString s;
				if( fullmessage.contains( "\r\n\r\n" ) > 0 ) {
					if( fullmessage.findRev( lSearch) < fullmessage.findRev( "\r\n\r\n" ) ) {
						s = fullmessage.mid( fullmessage.findRev( lSearch) + lLen );
						s = s.left( s.find( '\r' ) );
						if( s.toInt() == 0 ) {
							if( fullmessage.right( 4 ) == "\r\n\r\n" ) {
								break;
							}
						} else {
							contentLength = s.toInt();
							s = fullmessage.mid( fullmessage.findRev( QString( "\r\n\r\n" ) ) + 4 );
							if( s.utf8().length() == contentLength ) {
								break;
							}
						}
					} else if( fullmessage.right( 4 ) == "\r\n\r\n" ) {
						break;
					}
				}
		} else {
				break;
		}

		// received nothing / nothingmore
		if(bytesread==0) break;

		if( errFor) break;
	
		//unblock the current socket for subsequent reading
		char fl=1;
		ioctl(socketfd,FIONBIO,&fl);

	} //end of for loop	

	//close socket immediately, if it was returned from accept, or  nothing was read
	if(closeS || (bytesread==0) ) ::close( newsockfd );
	
	//Parse message
	if (isTcpSocket() ) {
	QString s;
		while( !fullmessage.isEmpty() ) {				
			contentLength = 0;
			if( fullmessage.contains( lSearch ) > 0 ) { 
				if( fullmessage.find( lSearch ) < fullmessage.find( QString( "\r\n\r\n" ) ) ) {
					s = fullmessage.mid( fullmessage.find( lSearch ) + lLen );
					s = s.left( s.find( '\r' ) );
					contentLength = s.toInt();
				}
			}
			s = fullmessage.left( fullmessage.find( QString( "\r\n\r\n" ) ) + 4 + contentLength );
			parseMessage( s );
			fullmessage.remove( 0, s.length() );
		}
	} else	{
			parseMessage( fullmessage );
		}
}


void SipClient::parseMessage( QString fullmessage )
{
	time_t timeStamp;
	if(debug >0) {
		timeStamp = time(0);
		if(debug > 0) cout << "<<<<< \nSipClient Received message of len = " <<  fullmessage.length() << " at: " << ctime(&timeStamp);
		if( ( debug == 1) || (debug == 3) ) cout  <<  fullmessage.latin1() << "<<<<< \n";
	}
	if(fullmessage.length() <=4) { //T ONLINE sends such stuff!
	 return;
	}
	SipMessage *curmessage = new SipMessage( fullmessage);
	if( !curmessage->isValid() ) {
		delete curmessage;
		if( debug > 0 ) cout  <<  "\n>>>>>this is not valid\n";
		return;
	}
	QString body = curmessage->messageBody();
	    if( debug > 0 ) cout  <<  "\n>>>>>this is valid\n";
	if( testOn ) {
		incomingTestMessage();
		delete curmessage;
		return;
	}
	QString callid = curmessage->getHeaderData( SipHeader::Call_ID );

	// Grab address in To: header
	SipUri touri( curmessage->getHeaderData( SipHeader::To ) );

  // Grab address in From: header
    SipUri fromuri( curmessage->getHeaderData( SipHeader::From ) );
	

	
	if( curmessage->getStatus().getCode() == 487 ) {
		sendAck( curmessage );
		delete curmessage;
		return;
	}
	QString cseq = curmessage->getHeaderData( SipHeader::CSeq );
	if( curmessage->getMethod() == Sip::MESSAGE ) {
		if( callid == messageCID && cseq == messageCSeq ) {
			if(debug >= 2) cout  <<  "=====SipClient: Received what was likely a retransmission, badly ignoring...\n" ;
			delete curmessage;
			return;
		}
		messageCID = callid;
		messageCSeq = cseq;		
		sendQuickResponse( curmessage, SipStatus( 200 ) );
		incomingInstantMessage( curmessage );
		delete curmessage;
		return;
	}
	if( curmessage->getMethod() == Sip::PUBLISH ) {
		sendQuickResponse( curmessage, SipStatus( 480 ) );
		if(debug) cout  <<  "=====SipClient: We dont support this\n" ;
	}

	if( curmessage->getMethod() == Sip::INFO ) {
		sendQuickResponse( curmessage, SipStatus( 200 ) );
	}
	if( curmessage->getMethod() == Sip::SUBSCRIBE ) {
		if ( fullmessage.contains("Event: presence.presencelist")  ) {
			sendQuickResponse( curmessage, SipStatus( 488 ) ); 
			if(debug) cout  <<  "=====SipClient: We dont support this\n";
			delete curmessage;
			return;
		} else if ( fullmessage.contains("Event: dialog")  ) {
			sendQuickResponse( curmessage, SipStatus( 408 ) ); 
			if(debug) cout  <<  "=====SipClient: We dont support this\n";
			delete curmessage;
			return;
		}   
		//We just received this message
		if( callid == subscribeCID && cseq == subscribeCSeq ) {
			if(debug >= 2) cout  <<  "=====SipClient: Received what was likely a retransmission, badly ignoring...\n";
			delete curmessage;
			return;
		}
		subscribeCID = callid;
		subscribeCSeq = cseq;
		bool found = false;
		for( SipCall *curcall = calls.first(); curcall != 0; curcall = calls.next() ) {
			if( callid == curcall->getCallId() && curcall->getCallType() == SipCall::inSubscribeCall ) {
			if(debug >= 2) cout  <<  "=====SipClient: Found a subscription\n";
				curcall->incomingMessage( curmessage );
				QString expires = curmessage->getHeaderData( SipHeader::Expires );
				SipUri incominguri( curmessage->getHeaderData( SipHeader::From ) );
				if( curcall->getMember( incominguri ) ) {
					if( expires == "0" ) {
						curcall->setCallStatus( SipCall::callUnconnected );
						incomingSubscribe( 0, false );
						delete curcall;
						for( SipCall *c = calls.first(); c != 0; c = calls.next() ) {
							if( (c->getCallType() == SipCall::outSubscribeCall) || (c->getCallType() == SipCall::winfoSubscribeCall)  ) {
								if( c->getMember( incominguri ) ) {
									if( c->getCallStatus() != SipCall::callDead ) {
										c->setCallStatus(
											SipCall::callUnconnected );
									}
								}
							}
						}
					} else {
						curcall->getMember( incominguri )->timerStart( expires.toInt() * 1000 );
						curcall->setCallStatus( SipCall::callInProgress );
						incomingSubscribe( curcall->getMember( incominguri ), false );
					}
					return;
				}
				found = true;
			}
			if( callid == curcall->getCallId() && curcall->getCallType() == SipCall::inSubscribeCall_2 ) {
				curcall->incomingMessage( curmessage );
				found = true;
			}
			if( found ) {
				return;
			}
		}
		if( touri.hasTag() ) {
		if(debug >= 2) cout  <<  "=====SipClient: Wrong turi\n";				
			sendQuickResponse( curmessage, SipStatus( 481 ) );
			delete curmessage;
			return;
		}
		SipCall *newcall;
		SipCallMember *member;
		bool sendSubscribe = true;
		if( touri == user->getUri() ) {
		if(debug >= 2) cout  <<  "=====SipClient: New subscription\n";
                        newcall = new SipCall( user, curmessage->getHeaderData( SipHeader::Call_ID ), SipCall::inSubscribeCall );
			member = newcall->incomingMessage( curmessage );
		} else {
			newcall = new SipCall( user, curmessage->getHeaderData( SipHeader::Call_ID ),SipCall::inSubscribeCall_2 );
			newcall->incomingMessage( curmessage );
			if(debug >= 2) cout  << "===== delete  \n";
			delete newcall;
			return;
		}
		SipUri remoteuri( curmessage->getHeaderData( SipHeader::From ) );
		if( curmessage->getHeaderData( SipHeader::Expires ) == "0" ||
				remoteuri.reqUri() == user->getMyUri()->reqUri() ) {
			if(debug >= 2) cout  << "===== delete expired \n";
			delete newcall;
		} else {
			for( SipCall *curcall = calls.first(); curcall != 0; curcall = calls.next() ) {
				if( (curcall->getCallType() == SipCall::outSubscribeCall) || (curcall->getCallType() == SipCall::winfoSubscribeCall)  ) {
					if( curcall->getMember( remoteuri ) ) {
						if( curcall->getCallStatus() == SipCall::callInProgress ) {
							sendSubscribe = false;
						}
					}
				}
			}
			member->setContactUri( curmessage->getContactList().getHead() );
			member->setUri( remoteuri );
			if( curmessage->getHeaderData( SipHeader::Expires).toInt() > 0 ) {
				member->timerStart( curmessage->getHeaderData( SipHeader::Expires).toInt() * 1000 );
			}
			connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
				this, SLOT( callMemberUpdated() ) );
			member->getCall()->setCallStatus( SipCall::callInProgress );
			incomingSubscribe( member, sendSubscribe );
		}
		return;
	}
	// If the CallId exists already, pass the message to that call
	for( SipCall *curcall = calls.first(); curcall != 0; curcall = calls.next() ) {
		if( callid == curcall->getCallId() ) {

			// Check cseq because SUBSCRIBE and NOTIFY call have same CallID
			QString cseq = curmessage->getHeaderData( SipHeader::CSeq );
			if( cseq.contains( "NOTIFY" ) && curmessage->getMethod() != Sip::NOTIFY ) {
				curcall->incomingMessage( curmessage );
				return;
			} else {
				if( curmessage->getType() == SipMessage::Request &&
				    touri.hasTag() &&
				    curmessage->getMethod() != Sip::REFER ) {
					if( touri.getTag() != curcall->localAddress().getTag() ) {
						if( curmessage->getMethod() != Sip::ACK &&
						    curmessage->getMethod() != Sip::CANCEL ) {
//						        cout  << "!!!!!!!!!!!!!!!!!!!!> Tags not equal\n");
							sendQuickResponse( curmessage, SipStatus( 481 ) );
						} else {
							if(debug >= 2) cout  <<  "=====SipClient: Dropping ACK/CANCEL which deserved a 481\n";

						}
					delete curmessage;
						return;
					}
				}
				curcall->incomingMessage( curmessage );
				return;
			}
		}
	}

	// Check message type
	if( curmessage->getType() != SipMessage::Request ) {
		if(debug >= 2) cout  <<  "=====SipClient: No call found for incoming response. Dropping.\n";
		delete curmessage;
		return;
	}
	// Check method
	if( curmessage->getMethod() == Sip::ACK ) {
		if(debug >= 2) cout  <<  "=====SipClient: ACK received, but nobody was listening. Dropping.\n";
		delete curmessage;
		return;
	}
	if( curmessage->getMethod() == Sip::CANCEL ) {
		if(debug >= 2) cout  <<  "=====SipClient: No listener for this CANCEL, returning a 481.\n";
		sendQuickResponse( curmessage, SipStatus( 481 ) );
		delete curmessage;
		return;
	}
	if( touri.hasTag() ) {
		sendQuickResponse( curmessage, SipStatus( 481 ) );
//		cout  << "!!!!!!!!!!!!!!!!!!!!> turi.hasTag 2\n");
		delete curmessage;
		return;
	}
	if( fwmode ) {
		if(debug >= 2) cout  <<  "=====SipClient: Forwarding call\n";
		if( fwbody != QString::null ) {
			sendQuickResponse( curmessage, SipStatus( 302 ), fwbody, MimeContentType( "text/plain" ) );
		} else {
			sendQuickResponse( curmessage, SipStatus( 302 ) );
		}
		delete curmessage;
		return;
	}
	if( busymode ) {
		if (getCallForwardB()) return;
		if(debug >= 2) cout  <<  "=====SipClient: We're busy.\n";
		if( busybody != QString::null ) {
		    sendQuickResponse( curmessage, SipStatus( 486 ), busybody, MimeContentType( "text/plain" ) );
		} else {
		    sendQuickResponse( curmessage, SipStatus( 486 ) );
		}
		delete curmessage;
		return;
	}
	if( curmessage->getMethod() == Sip::REGISTER ) {
		if(debug) cout  <<  "=====SipClient:Not Implemented, Returning a 501.\n";
		sendQuickResponse( curmessage, SipStatus( 501 ) );
		delete curmessage;
		return;
	}
	if( curmessage->getMethod() == Sip::PUBLISH ) {
		if(debug) cout  <<  "=====SipClient:Not Implemented, Returning a 501.\n";
		sendQuickResponse( curmessage, SipStatus( 501 ) );
		delete curmessage;
		return;
	}
	if( curmessage->getMethod() == Sip::BadMethod ) {
		if(debug >= 2) cout  <<  "=====SipClient: I don't recognize that method... Returning a 501.\n";
		sendQuickResponse( curmessage, SipStatus( 501 ) );
		delete curmessage;
		return;
	}
	if( curmessage->hasHeader( SipHeader::Accept ) ) {
		if( curmessage->getMethod() == Sip::INVITE ) {
			if( !curmessage->getHeaderData( SipHeader::Accept ).lower().contains( "application/sdp" ) ) {
				sendQuickResponse( curmessage, SipStatus( 406 ) );
				delete curmessage;
				return;
			}
		} else if( curmessage->getMethod() == Sip::SUBSCRIBE ) {
			if( !curmessage->getHeaderData( SipHeader::Accept ).lower().contains( "application/xpidf+xml" ) ) {
				sendQuickResponse( curmessage, SipStatus( 406 ) );
				delete curmessage;
				return;
			}
		}
	}
	//Test for unwanted Require Header
	if( curmessage->hasHeader( SipHeader::Require ) ){
		QString req=curmessage->getHeaderData( SipHeader::Require);
		if (req != "100rel" ) {
			if(debug >= 2) cout  <<  "=====SipClient: This messages says it requires '" << req.latin1()  << ", returning 420.\n";
			sendQuickResponse( curmessage, SipStatus( 420 ) );
			delete curmessage;
			return;
		} else {
			if(debug >= 2) cout  <<  "=====SipClient: We got " << req.latin1() << endl;
		}
	}
	/*
	//Test for unwanted Supported Header
	if( curmessage->hasHeader( SipHeader::Supported ) ){
		QString req=curmessage->getHeaderData( SipHeader::Supported);
		if ( req.contains("100rel") || req.contains("timer") || req.contains("replaces") || req.contains("norefersub") ) {
		    if(debug >= 2) cout  <<  "=====SipClient: We got " <<req.latin1() << endl;
		} else {
		    if(debug >= 2) cout  <<  "=====SipClient: This messages says it supports '" << req.latin1() << "'returning 420.\n";
		    sendQuickResponse( curmessage, SipStatus( 420 ) );
		    delete curmessage;
		    return;	
		}
	}
	*/

	// Create a new call and pass it the message
	//if(( ( debug == 1) || (debug == 3) ) || (debug == 2)) cout  <<  "=====SipClient: Searching for a user\n" );

	if( curmessage->getMethod() == Sip::OPTIONS ) {
		SipCall *newcall = new SipCall( user, curmessage->getHeaderData( SipHeader::Call_ID ) );
		newcall->incomingMessage( curmessage );
		return;
	}

	if(debug >= 2) cout  <<  "=====SipClient: Creating new call for user " << user->getUri().nameAddr().ascii() << endl;
	SipCall *newcall = new SipCall( user, curmessage->getHeaderData( SipHeader::Call_ID ) );
	SipCallMember *member = newcall->incomingMessage( curmessage );

	if( member == 0 ) {
		return;
	}
	if( curmessage->getMethod() == Sip::PRACK ) {
			sendQuickResponse( curmessage, SipStatus( 200 ) );
			delete curmessage;
	}		
	// Signal that we have an incoming call
	if( curmessage->getMethod() == Sip::INVITE ) {
		//QString body = curmessage->messageBody();
		newcall->setRemoteStart(false); //Remotestart will be detected later
		if( body.contains( "m=audio" ) ) {
			newcall->setCallType( SipCall::StandardCall );
			if( body.contains( "m=video" ) ) {
			    newcall->setCallType( SipCall::auviCall );
			}
		} else if( body.contains( "m=video" ) ) {
				newcall->setCallType( SipCall::videoCall );
		} else if( body.contains( "m=application" ) ) {
				newcall->setCallType( SipCall::GarKeinCall );//preliminary, will be correctet in KCallWidget:switchCalls
		}
		incomingCall(newcall,body);
	} else {
		delete curmessage;
	}
}

bool SipClient::sendRequest( bool sendToURI, int calltype, SipMessage *msg, bool contact, const SipUri &regProxy, const QString &branch ) {
	time_t timeStamp;
	if( regProxy != SipUri::null ) {
		sipProxy = regProxy;
		myProxy = regProxy.getHostname();
	}
	bool sturi = direct2URI;
	
	if(sendToURI) sturi = sendToURI;
	// Create a Via tag and add it to the message at the top of the list
	SipVia regvia;
	if( isTcpSocket() ) {
		regvia.setTransport( SipVia::TCP );
	} else {
		regvia.setTransport( SipVia::UDP );
	}

	if (symmetricmode) {
		regvia.setRportParam( QString::null );
	}

	regvia.setHostname( Sip::getLocalAddress() );
	
	if( isTcpSocket() ) {
		regvia.setPortNumber( TCP_listener.getPortNumber() );
	} else {
		regvia.setPortNumber( listener->getPortNumber() );
	}

	if( branch != QString::null ) {
		regvia.setBranchParam( branch );
	} else {
		regvia.generateBranchParam();
	}


	msg->getViaList().insertTopmostVia( regvia );

	// Calculate content length
	msg->insertHeader( SipHeader::Content_Length, QString::number( msg->messageBody().utf8().length() ) );

	// Advertise shamelessly
	msg->insertHeader( SipHeader::User_Agent, getUserAgent() );
	if( msg->getMethod() == Sip::REGISTER ) {
		msg->insertHeader( SipHeader::Event, "registration" );
		msg->insertHeader( SipHeader::Allow_Events, "presence" );
		sentReg = true;
	}

	if( msg->getMethod() == Sip::SUBSCRIBE ) {
		if(calltype == SipCall::winfoSubscribeCall) {
		    msg->insertHeader( SipHeader::Event, "presence.winfo" );
		    msg->insertHeader( SipHeader::Accept, "application/watcherinfo+xml" );
		} else {
		    msg->insertHeader( SipHeader::Event, "presence" );
		    msg->insertHeader( SipHeader::Accept, "application/pidf+xml" );
		}
	}

	if( msg->getMethod() == Sip::NOTIFY ) {
		msg->insertHeader( SipHeader::Event, "presence" );
	}

	if( msg->getMethod() == Sip::PUBLISH ) {
		msg->insertHeader( SipHeader::Event, "presence" );
	}
	// NOTIFY for REFER triggered actions
	if( msg->getMethod() == Sip::RNOTIFY ) {
		msg->insertHeader( SipHeader::Event, "refer" );
	}

	// Set max-forwards
	if( maxforwards != 0 ) {
		msg->insertHeader( SipHeader::Max_Forwards, QString::number( maxforwards ) );
	}

	// Via hiding mode
	if( hidemode != DontHideVia ) {
		if( hidemode == HideHop )
			msg->insertHeader( SipHeader::Hide, "hop" );
		else
			msg->insertHeader( SipHeader::Hide, "route" );
	}

	// If this request requires the contact header, add it
	if( contact ) {
		msg->getContactList().addToHead( contacturi );
	}

	// Retransmission timestamp
	msg->setTimestamp();

	if( msg->getMethod() == Sip::REGISTER || msg->getMethod() == Sip::MESSAGE ) {
		msg->setTimeTick( 4000 );
	} else {
		msg->setTimeTick( 500 ); // T1
	}

	msg->incrSendCount();

	// Error in reguest uri
	if( msg->getRequestUri().reqUri().contains( ' ' ) ) {
		QString s = msg->getRequestUri().reqUri();
		while( s.contains( ' ' ) ) {
			s.remove( s.find( ' ' ), 1 );
		}
		msg->setRequestUri( SipUri( s ) );
	}

	// Announce that we're sending a message

if(debug > 0) {
		timeStamp = time(0);
		 if(debug > 0) cout << "<<<<< \nSipClient send Request at: " << ctime(&timeStamp);
		if( ( debug == 1) || (debug == 3) ) cout  <<  msg->message().data();
		if(debug > 0) cout  <<  "<<<<<";
}	
	// Send the message
	TCPMessageSocket *tcpSocket = 0;
	if( isTcpSocket() ) {
		bool createTcpSocket = true;
		TCPMessageSocketIterator it( getTcpSocketList() );
		if( useExplicitProxy ) {
			for (it.toFirst(); it.current(); ++it) {
				tcpSocket = it.current();
				if( tcpSocket->cmpSocket(  proxy, proxyport ) ) {
					createTcpSocket = false;
					break;
				}
			}
			if( createTcpSocket ) {
				tcpSocket = new TCPMessageSocket;
				if( !tcpSocket->setHostnamePort( proxy, proxyport ) ) {
					return false;
				}
				if( tcpSocket->connect( proxyport ) == -1 ) {
					delete tcpSocket;
					tcpSocket = 0;
					return false;
				} else {
					if(debug >= 2) cout  <<  "=====SipClient:sending TCP to " << proxy.latin1() << ":" << proxyport << endl;
					tcpSockets.append( tcpSocket );
				}
			}
		} else {
			// find destination
			QString sendtoaddr;
			unsigned int sendtoport = msg->getRequestUri().getPortNumber();
			//maddr ??
			if( msg->getRequestUri().hasMaddrParam() ) {
				sendtoaddr = msg->getRequestUri().getMaddrParam();
			} else {
				SipUri route( msg->getHeaderData( SipHeader::Route ) );
				//loose route??
				if( route.uri().contains( ";lr" ) ) {
					sendtoaddr = route.getHostname();
					sendtoport = route.getPortNumber();
				} else {
					//send to own server
					if( KStatics::isReg && myProxy != "" && !sturi) {
						sendtoaddr = myProxy; 
					//send to RURI
					} else {
					    sendtoaddr = msg->getRequestUri().getHostname();
					}
					sendtoaddr = getSipProxySrv( sendtoaddr );
			                unsigned int port;
					if( sendtoaddr.contains( "]:" )  ) {
						port = sendtoaddr.mid( sendtoaddr.find( ']' ) + 1 ).toUInt();
						msg->getRequestUri().setPortNumber( port );
						sendtoaddr = sendtoaddr.left( sendtoaddr.find( ']' )+1 );

					} else if( sendtoaddr.contains( ':') && !sendtoaddr.contains(']')  ) {
						port = 	sendtoaddr.mid( sendtoaddr.find( ':' ) + 1 ).toUInt();
						msg->getRequestUri().setPortNumber( port );
						sendtoaddr = sendtoaddr.left( sendtoaddr.find( ':' ) );

					}
						sendtoport = msg->getRequestUri().getPortNumber();

				}
			}


			if(debug >= 2) cout  <<  "=====SipClient: Sending TCP to  " <<  sendtoaddr.latin1() << ":" << sendtoport << endl;
			
			for (it.toFirst(); it.current(); ++it) {
				tcpSocket = it.current();
				if( tcpSocket->cmpSocket( sendtoaddr, sendtoport ) ) {
					createTcpSocket = false;
					break;
				}
			}
			if( createTcpSocket ) {
				tcpSocket = new TCPMessageSocket;
				if( !tcpSocket->setHostnamePort( sendtoaddr, sendtoport ) ) {
					return false;
				}
				if( tcpSocket->connect( sendtoport ) == -1 ) {
					delete tcpSocket;
					tcpSocket = 0;
					return false;
				} else {
					tcpSockets.append( tcpSocket );
				}
			}
		}
		if( tcpSocket != 0 ) {
			QString m = msg->message();
			if( tcpSocket->send( m.utf8().data(), m.utf8().length() ) == -1 ) {
				tcpSockets.remove( tcpSocket );
				tcpSocket = 0;
			}
		}
	}
	if( (tcpSocket == 0) && (!isTcpSocket()) ) {
		UDPMessageSocket* s;
		UDPMessageSocket sendsocket(useIPv6);
		if (symmetricmode) {
			s = listener;
		} else {
			s = &sendsocket;
	}
		// Choose destination	
		if( useExplicitProxy ) {
		    if( !s->setHostname( proxy.utf8().data() ) ) { 
			return false; 	
		    }
		    if( debug >= 2 ) cout  <<  "to host " << proxy.latin1() << " port=" << proxyport << " (UDP)\n";
		    s->connect( proxyport );
		} else {
			// find destination
			QString sendtoaddr;
			unsigned int sendtoport = msg->getRequestUri().getPortNumber();
			//maddr ?
			if( msg->getRequestUri().hasMaddrParam() ) {
				sendtoaddr = msg->getRequestUri().getMaddrParam();
			} else {
				//loose route
				SipUri route( msg->getHeaderData( SipHeader::Route ) );
				if( route.uri().contains( ";lr" ) ) {
					sendtoport = route.getPortNumber();
					sendtoaddr = route.getHostname();
				} else {
					//send to own server
					if( KStatics::isReg && myProxy != "" && !sturi) {
					    sendtoaddr = myProxy; 
					// send to RURI
					} else {
					    sendtoaddr = msg->getRequestUri().getHostname();
					}
					sendtoaddr = getSipProxySrv( sendtoaddr );
			                unsigned int port;
					if( sendtoaddr.contains( "]:" )  ) {
						port = sendtoaddr.mid( sendtoaddr.find( ']' ) + 1 ).toUInt();
						msg->getRequestUri().setPortNumber( port );
						sendtoaddr = sendtoaddr.left( sendtoaddr.find( ']' )+1 );

					} else if( sendtoaddr.contains( ':') && !sendtoaddr.contains(']')  ) {//ÃÂÃÂ§ && !useIPv6
						port = 	sendtoaddr.mid( sendtoaddr.find( ':' ) + 1 ).toUInt();
						msg->getRequestUri().setPortNumber( port );
						sendtoaddr = sendtoaddr.left( sendtoaddr.find( ':' ) );

					}
						sendtoport = msg->getRequestUri().getPortNumber();
				}
			}
				if(debug >= 2) cout  <<  "=====SipCient Sending UDP to " << sendtoaddr.latin1() << ":" << sendtoport << endl;
			if( !s->setHostname( sendtoaddr ) ) { return false; }
			s->connect( sendtoport );
		}
		QString m = msg->message();
		s->send( m.utf8().data(), m.utf8().length() );
	}
	return true;
}


void SipClient::setSymmetricMode( bool newmode)
{
	symmetricmode = newmode;
}


void SipClient::sendResponse( SipMessage *msg, bool contact )
{
	MessageSocket *outsocket = 0;
	SipVia topvia;
	QString sendaddr;
	bool isTCP=false;
	time_t timeStamp;

	// Calculate content length
	msg->insertHeader( SipHeader::Content_Length, QString::number( msg->messageBody().utf8().length() ) );

	// Advertise shamelesslysipvialist
	msg->insertHeader( SipHeader::User_Agent, getUserAgent() );

	// If this rquest requires the contact header, add it
	if( contact ) {
		msg->getContactList().addToHead( contacturi );
	}

	// Indicate what methods we allow if this is an answer to an OPTIONS
	if( msg->getHeaderData( SipHeader::CSeq ).contains( "OPTIONS" ) ) {
		msg->insertHeader( SipHeader::Allow,
			"INVITE, OPTIONS, ACK, BYE, MSG, CANCEL, MESSAGE, SUBSCRIBE, NOTIFY, INFO, REFER, PRACK, PUBLISH" );
	}

	// Use via to tell us where to send it and how
	topvia = msg->getViaList().getTopmostVia();
	switch( topvia.getTransport() ) {
		case SipVia::UDP:
			if(debug ) cout  <<  "<<<<<\nSipClient Sending UDP Response ";
			if (symmetricmode) {
				outsocket = listener;
			} else {
				outsocket = new UDPMessageSocket(useIPv6);
			}
			break;
		case SipVia::TCP:
			isTCP=true;
			if( debug ) cout  <<  "<<<<<\nSipClient Sending TCP Response";
			outsocket = new TCPMessageSocket;
			break;
		case SipVia::TLS:
			if(debug >= 2) cout  <<  "=====SipClient: TLS in top via, not supported (full TLS support not implemented)\n";
			break;
		case SipVia::BadTransport:
			if(debug >= 2) cout  <<  "=====SipClient: Bad transport on incoming Via\n";
			break;
	}

	// If transport was bad, no use sending
	if( !outsocket ) return;

	// maddr, received, sentby
	if( topvia.hasMaddrParam() ) {
		sendaddr = topvia.getMaddrParam();
	} else if( topvia.hasReceivedParam() ) {
		sendaddr = topvia.getReceivedParam();
	} else {
		sendaddr = topvia.getHostname();
	}

	// Announce where we're sending
		if(debug >= 2) cout  <<  "to " <<  sendaddr.utf8().data() << " port " << topvia.getPortNumber() << endl;
	
	if( !outsocket->setHostname( sendaddr.utf8().data() ) ) {
		if (outsocket != listener) {
		if(debug >= 2) cout  << "sending outsocket deleted\n";
			delete outsocket;
		}
		return;
	}
	//in case of success we queue our send-socket to delete it later
	if(isTCP) {
	    if( outsocket->connect( topvia.getPortNumber() ) == -1 ) {
		delete outsocket;
		outsocket = 0;
		return;
	    } else {
		sndSockets.append( (TCPMessageSocket *)outsocket );
	    }
	} else {
	outsocket->connect( topvia.getPortNumber() );
	}
	// Announce what we're sending
	if( debug > 0) {
		 if(debug > 0) cout  << "<<<<<\nSipClient sendMessage ";
		timeStamp = time(0);
		cout  << " at: " << ctime(&timeStamp);
		if( ( debug == 1) || (debug == 3) ) cout  <<  msg->message().data();
		if(debug > 0) cout << "<<<<<" << endl;
	}
	// Send it
	QString m = msg->message();
	outsocket->send( m.utf8().data(), m.utf8().length() );

	//clean UDP sockets immediately but TCPSockets deferred, as some proxies dislike too early FIN
	if ((outsocket != listener) && !isTCP) {
		delete outsocket;
	}
	if(isTCP && (sndSockets.count() >3)) {
	sndSockets.removeFirst();
	}
}


void SipClient::addCall( SipCall *call )
{
	if( !calls.contains( call ) ) {
		calls.append( call );
	}
	callListUpdated();
}

void SipClient::callTypeUpdated( void )
{
	callListUpdated();
}

void SipClient::deleteCall( SipCall *call )
{
	calls.remove( call );
	callListUpdated();
}

void SipClient::setExplicitProxyMode( bool eproxy )
{
	useExplicitProxy = eproxy;

}

void SipClient::setExplicitProxyAddress( const QString &newproxy )
{
    int noDot=0;
    noDot=newproxy.contains( ':');
    if(noDot >= 2 )  {
		if(newproxy.contains( '[') < 1) {
		    proxy = "["+newproxy+"]";
		    proxyport = 5060; 
		} else {
		    proxy = newproxy.left( newproxy.find( ']' )+1 );
		    proxyport = newproxy.mid( newproxy.find( ']' ) + 2 ).toUInt();
		    if (proxyport == 0) proxyport = 5060;
		}
    } else {
	if( newproxy.contains( ':' ) ) {
		proxy = newproxy.left( newproxy.find( ':' ) );
		proxyport = newproxy.mid( newproxy.find( ':' ) + 1 ).toUInt();
	} else {
		proxy = newproxy;
		proxyport = 5060;
	}
    }

}

QString SipClient::getExplicitProxyAddress( void )
{
	QString uri = "<sip:" + proxy;
	if( proxyport != 5060 ) {
		uri += ":" + QString::number( proxyport, 10 );
	}
	uri += ";lr>";
	return uri;
}

void SipClient::setUser( SipUser *u )
{
	user = u;
}


void SipClient::sendRaw( SipMessage *msg )
{
	time_t timeStamp;
	if( isTcpSocket() ) {
		return;
	}
	// Announce that we're sending
if( debug > 0) {
		 cout  << "<<<<<\nSipClient sendRaw ";
		timeStamp = time(0);
		cout  << " at: " << ctime(&timeStamp);
		if( ( debug == 1) || (debug == 3) ) cout  <<  msg->message().data();
		cout << "<<<<<" << endl;
	}	
	
	UDPMessageSocket *sendsocket;

	if (symmetricmode) {
		sendsocket = listener;
	} else {
		sendsocket = new UDPMessageSocket(useIPv6);
	}

	// Choose destination
	if( ( msg->getType() != SipMessage::Response ) && useExplicitProxy ) {
		
	if( !sendsocket->setHostname( proxy.utf8().data() ) ) {
			if (sendsocket != listener) {
				delete sendsocket;
			}
			return; 
		}
		sendsocket->connect( proxyport );
	} else {

		// Send to whatever is in the request uri
		QString sendtoaddr;
		if( msg->getRequestUri().hasMaddrParam() ) {
			sendtoaddr = msg->getRequestUri().getMaddrParam();
		} else {
			sendtoaddr = msg->getRequestUri().getHostname();
		}
		sendtoaddr = getSipProxySrv( sendtoaddr );//test
		if( !sendsocket->setHostname( sendtoaddr ) ) { 
			if (sendsocket != listener) {
				delete sendsocket;
			}
			return; 
		}
		sendsocket->connect( msg->getRequestUri().getPortNumber() );
	}
	QString m = msg->message();
	sendsocket->send( m.utf8().data(), m.utf8().length() );
	if (sendsocket != listener) {
		delete sendsocket;
	}
}




void SipClient::sendQuickResponse( SipMessage *origmessage, const SipStatus &status,
	const QString &body, const MimeContentType &bodytype )
{
time_t timeStamp;
if( ( debug == 1) || (debug == 3) ) cout  << "<<<<<\n sendQuickResponse:\n";
	MessageSocket *outsocket = 0;
	SipMessage *msg = new SipMessage;
	SipVia topvia;
	QString sendaddr;
	msg->setType( SipMessage::Response );
	msg->setStatus( status );
	bool isTCP=false;

	// Copy via list exactly
	msg->setViaList( origmessage->getViaList() );
	msg->insertHeader( SipHeader::From, origmessage->getHeaderData( SipHeader::From ) );//From-To twiddled
	msg->insertHeader( SipHeader::To, origmessage->getHeaderData( SipHeader::To ) );    //From-To twiddled
	msg->insertHeader( SipHeader::CSeq, origmessage->getHeaderData( SipHeader::CSeq ) );
	msg->insertHeader( SipHeader::Call_ID, origmessage->getHeaderData( SipHeader::Call_ID ) );
	if( origmessage->hasHeader( SipHeader::Require ) ) {
		msg->insertHeader( SipHeader::Unsupported, origmessage->getHeaderData( SipHeader::Require ) );
	}
	if( ( status.getCode() >= 300 ) && ( status.getCode() < 400 ) ) {
		msg->getContactList().addToHead( forwarduri );
	}
	if( status.getCode() == 501 ) {
		msg->insertHeader( SipHeader::Allow,
			"INVITE, OPTIONS, ACK, BYE, MSG, CANCEL, MESSAGE, SUBSCRIBE, NOTIFY, INFO, REFER, PRACK, PUBLISH" );
	}
	if( bodytype != MimeContentType::null ) {
		msg->insertHeader( SipHeader::Content_Type, bodytype.type() );
	}
	msg->setBody( body );

	// Calculate content length
	msg->insertHeader( SipHeader::Content_Length, QString::number( msg->messageBody().utf8().length() ) );

	// Advertise shamelessly
	msg->insertHeader( SipHeader::User_Agent, getUserAgent() );

	// Use via to tell us where to send it and how
	topvia = msg->getViaList().getTopmostVia();
	switch( topvia.getTransport() ) {
		case SipVia::UDP:
			if( ( debug == 1) || (debug == 3) ) cout  <<  "UDP ";
			if (symmetricmode) {
				outsocket = listener;
			} else {
				outsocket = new UDPMessageSocket(useIPv6);
			}
			break;
		case SipVia::TCP:
			if( ( debug == 1) || (debug == 3) ) cout  <<  "TCP ";
			isTCP=true;
			outsocket = new TCPMessageSocket;
			break;
		case SipVia::TLS:
			if(debug >= 2) cout  <<  "=====SipClient: TLS in top via, not supported (full TLS support not implemented)\n";
			break;
		case SipVia::BadTransport:
			if(debug >= 2) cout  <<  "=====SipClient: Bad transport on incoming Via\n";
			break;
	}
	// If transport is bad, no use sending
	if( !outsocket ) {
		delete msg;
		return;
	}
	// maddr, received, sentby
	if( topvia.hasMaddrParam() ) {
		sendaddr = topvia.getMaddrParam();
	} else if( topvia.hasReceivedParam() ) {
		sendaddr = topvia.getReceivedParam();
	} else {
		sendaddr = topvia.getHostname();
	}
	// Announce where we're sending
	if(debug >= 2) cout  << "to " << sendaddr.latin1() << " port " << topvia.getPortNumber() << endl;

	if( !outsocket->setHostname( sendaddr.utf8().data() ) ) {
		delete msg;
		if (outsocket != listener) {
			delete outsocket;
		}
		return;
	}
	//in case of success we queue our send-socket to delete it later
	if(isTCP) {
	    if( outsocket->connect( topvia.getPortNumber() ) == -1 ) {
		delete outsocket;
		outsocket = 0;
		return;
	    } else {
		sndSockets.append( (TCPMessageSocket *)outsocket ); 
	    }
	} else {
	outsocket->connect( topvia.getPortNumber() );
	}
	// Announce what we're sending
	if( debug > 0) {
		timeStamp = time(0);
		cout  << " at: " << ctime(&timeStamp);
		if( ( debug == 1) || (debug == 3) ) cout  <<  msg->message().data();
		cout << "<<<<<" << endl;
	}
	// Send it
	QString m = msg->message();
	outsocket->send( m.utf8().data(), m.utf8().length() );

	//clean UDP sockets immediately but TCPSockets deferred, as some proxies dislike too early FIN
	if ((outsocket != listener) && !isTCP) {
		delete outsocket;
	}
	if(isTCP && (sndSockets.count() >3)) {
	sndSockets.removeFirst();
	}
	delete msg;
}

void SipClient::sendTestMessage( QString sendaddr, unsigned int port, QString msg )
{
	time_t timeStamp;
	if(debug >= 2) cout  << "sendTestMessage\n";
	MessageSocket *outsocket = 0;
	if (symmetricmode) {
		outsocket = listener;
	} else {
		outsocket = new UDPMessageSocket(useIPv6);
	}

	outsocket->setHostname( sendaddr.utf8().data() );
	outsocket->connect( port );

	// Announce what we're sending

if( debug > 0) {
		 cout  << "<<<<<\nSipClient Sending ";
		timeStamp = time(0);
		cout  << " at: " << ctime(&timeStamp);
		if( ( debug == 1) || (debug == 3) ) cout  <<  msg.data();
		cout << "<<<<<" << endl;
	}
	// Send it
	outsocket->send( msg.utf8().data(), msg.utf8().length() );

	if (outsocket != listener) {
		delete outsocket;
	}
}

void SipClient::sendAck( SipMessage *origmessage )
{
	SipMessage *msg = new SipMessage;
	msg->setType( SipMessage::Request );
	msg->setMethod( Sip::ACK );
	QString s = origmessage->getHeaderData( SipHeader::CSeq );
	s = s.left( s.find( ' ' ) );
	s = s	+ " " + Sip::getMethodString( msg->getMethod() );
	msg->insertHeader( SipHeader::CSeq, s );
	msg->setRequestUri( SipUri( origmessage->getHeaderData( SipHeader::To ) ) );
	msg->insertHeader( SipHeader::From, origmessage->getHeaderData( SipHeader::From ) );
	msg->insertHeader( SipHeader::To, origmessage->getHeaderData( SipHeader::To ) );
	msg->insertHeader( SipHeader::Call_ID, origmessage->getHeaderData( SipHeader::Call_ID ) );
	// We need the correct branch parameter
	QString branch = origmessage->getViaList().getBottommostVia().getBranchParam();
	sendRequest( false,0, msg, true, SipUri::null, branch );
	delete msg;
}

void SipClient::setHideViaMode( HideViaMode newmode )
{
	hidemode = newmode;
}

void SipClient::setCallForward( bool onoff )
{
	fwmode = onoff;
}

void SipClient::setCallForwardB( bool onoff )
{
	fwbmode = onoff;
}

void SipClient::setCallForwardUri( const SipUri &u )
{
	forwarduri = u;
}

void SipClient::setCallForwardMessage( const QString &newmessage )
{
	fwbody = newmessage;
}

const QString SipClient::getUserAgent( void )
{
	return KStatics::me;
}

void SipClient::setMaxForwards( int newmax )
{
	maxforwards = newmax;
}

void SipClient::setBusy( bool onoff )
{
	busymode = onoff;
}

void SipClient::setBusyMessage( const QString &newmessage )
{
	busybody = newmessage;
}

SipUser *SipClient::getUser( SipUri uri )
{
	if( uri == user->getUri() ) {
		return user;
	} else {
		return NULL;
	}
}

void SipClient::updateIdentity( SipUser *u, QString newproxy )
{
	user = u;
	setupContactUri( user );
	if( newproxy.isEmpty() || newproxy.lower() == "sip:") {
		setExplicitProxyMode( false );
	} else {
		if( newproxy.left( 4 ).lower() == "sip:" ) {
			newproxy.remove( 0, 4 );
		}
		setExplicitProxyMode( true );
		setExplicitProxyAddress( newproxy );
	}

}

void SipClient::sendStunRequest( QString uristr ) {
	if( !uristr.isEmpty() ) {
		useStunProxy = true;
		stunProxy = SipUri( uristr );
	}
	if( useStunProxy ) {
	
		if( !listener->setHostname( stunProxy.getHostname() ) ) { return; }
		if(debug) cout  <<  "=====SipClient: STUN request\n";
		listener->connect( stunProxy.getPortNumber() );
		StunRequestSimple req;
		req.msgHdr.msgType = htons(BindRequestMsg);
		req.msgHdr.msgLength = htons( sizeof(StunRequestSimple)-sizeof(StunMsgHdr) );
		for ( int i=0; i<16; i++ ) {
			req.msgHdr.id.octet[i]=0;
		}
		int id = rand();
		req.msgHdr.id.octet[0] = id;
		req.msgHdr.id.octet[1] = id>>8;
		req.msgHdr.id.octet[2] = id>>16;
		req.msgHdr.id.octet[3] = id>>24;
		listener->send( (char *)&req, sizeof( req ) );
	}
}

QString SipClient::getSipProxySrv( QString dname )
{
	// No lookups take place in point-to-point mode
	if(KStatics::isP2P) {
		sipProxyName = dname;
		if(debug >= 2) cout  << "\n=====SipClient::getSipProxySrv(P2P) " << dname.latin1() << endl;
		return dname;
	}
	if(dname.contains ("[") && dname.contains ("]")) {
		sipProxyName = dname+" ";
		sipProxyName = sipProxyName.remove(" ");
		if(debug >= 2) cout  << "\n=====SipClient::getSipProxySrv(brackets) " <<sipProxyName.latin1() << endl;
		return sipProxyName;
	}
/*	if( sipProxyName == dname ) {
		if(debug >= 2) cout  << "=====SipClient::getSipProxySrv " << sipProxyName.latin1() << "==" << dname.latin1();
		return sipProxySrv;
	}
*/
	QString srv = QString::null;
	//NAPTR
	if (locServ == NAPTR) {
	    QString naptr = getNAPTR( dname );
	    if( !naptr.isEmpty() ) {
		srv = getSRV( naptr );
	    }
	}
	//SRV or both
	if  (locServ >= SRV) {
	    //no SRV string, use defaults
	    if( srv.isEmpty()) {	
		if(SocketMode ==  TCP) srv = getSRV( QString( "_sip._tcp." ) + dname );else srv = getSRV( QString( "_sip._udp." ) + dname );
	    } 
	} 
	
	if( !srv.isEmpty()) {
		sipProxyName = dname;
		sipProxySrv = srv;
		if(debug >= 2) cout  << "\n=====SipClient::getSipProxySrv (result)" << sipProxySrv.latin1() << endl;
		return sipProxySrv;
	} else {
		if(debug >= 2) cout  << "\n=====SipClient::getSipProxySrv(dname) " << dname.latin1() << endl;
		return dname;
	}
}

QString SipClient::getResSearch( QString dname, int type, bool UDP )
{

	dname=dname.remove('[');
	dname=dname.remove(']');
	
	unsigned char msg[PACKETSZ],*mptr,*xptr;
	int i,j,l,co;
	unsigned short *usp,ty;
	unsigned int *uip;
	res_response *res;
	char name[PACKETSZ];
	QString tmpName;
	QString domainName = "";
	u_short priority = 0;
	u_short weight = 0;
	u_short port = 5060;


	if(res_init()==-1){
		//if(debug >= 2) cout  << "res_init -error !\n");
	} else if((l=res_search( dname, C_IN, type, msg, sizeof( msg ) ) ) == -1 ) {
		//if(debug >= 2) cout  <<  "res_search: NO result !\n" );
	} else if( l <= 0 ){
		//if(debug >= 2) cout  <<  "res_search: result is empty !\n" );
	} else {
		//if(debug >= 2) cout  <<  "res_search OK (len=%d)\n", l );
		res = (res_response *)msg;
		mptr = msg + sizeof( HEADER );
		co = ntohs( res->header.qdcount );
		for( i=0; i < co; i++ ) {
			j = dn_expand( msg, msg + PACKETSZ, mptr, name, MAXDNAME );
			if( j < 0 ) {
				break;
			} else {
				mptr += j;
				usp = (unsigned short *)mptr;
				mptr += sizeof( short );
				usp = (unsigned short *)mptr;
				mptr += sizeof( short );
			}
		}
		co = ntohs( res->header.ancount );
		for( i = 0 ; i < co; i++ ) {
			j = dn_expand( msg, msg + PACKETSZ, mptr, name, MAXDNAME );
			if( j < 0 ) {
				if(debug >= 2) cout  <<  "\t\tname-error\n";
				break;
			} else {
				mptr += j;
				usp = (unsigned short *)mptr;
				ty = ntohs( *usp );
				mptr += sizeof( short );
				usp = (unsigned short *)mptr;
				mptr += sizeof(short);
				uip = (unsigned int *)mptr;
				mptr += sizeof(int);
				uip = (unsigned int *)mptr;
				j = ntohs( *uip );
				mptr += sizeof(short);
				xptr = mptr;
				mptr += j;
				if( ty == T_NAPTR ) {
					usp = (unsigned short *)xptr;
					xptr += sizeof(short);
					usp = (unsigned short *)xptr;
					xptr += sizeof(short);
					j = (int)(*xptr);
					xptr += 1;
					while( j > 0 ) {
						xptr+=1;
						j--;
					}
					j = (int)(*xptr);
					xptr += 1;
					while( j > 0 ) {
						xptr += 1;
						j--;
					}
					j=(int)(*xptr);
					xptr+=1;
					while( j > 0 ) {
						xptr += 1;
						j--;
					}
					j = dn_expand( msg, msg + PACKETSZ, xptr, name, MAXDNAME );
					if( j < 0 ) {
						break;
					} else {
						tmpName = QString( name );
						if( UDP ) {
							if( tmpName.contains( "_udp" ) ) {
								domainName = QString( name );
							}
						} else {
							if( tmpName.contains( "_tcp" ) ) {
								domainName = QString( name );
							}
						}
						xptr+=j;
					}
				} else if( ty == T_SRV ) {
					u_short pr;
					u_short we;
					u_short po;
					usp = (unsigned short *)xptr;
					pr = ntohs( *usp );
					xptr += sizeof( short );
					usp = (unsigned short *)xptr;
					we = ntohs( *usp );
					xptr += sizeof( short );
					usp = (unsigned short *)xptr;
					po = ntohs( *usp );
					xptr += sizeof( short );
					j = dn_expand( msg, msg + PACKETSZ, xptr, name, MAXDNAME );
					if( j < 0 ) {
						break;
					} else {
						if( !priority || pr < priority ||
						    (pr == priority && we < weight) ) {
							priority = pr;
							weight = we;
							port = po;
							domainName = QString( name ) +
								":" + QString::number( port );
							xptr+=j;
						}
					}
				} else {
				}
			}
		}
		co = ntohs( res->header.nscount );
		co = ntohs( res->header.arcount );
	}
	return domainName;
}

QString SipClient::getNAPTR( QString strUri )
{
	return getResSearch( strUri, T_NAPTR, true );
}

QString SipClient::getSRV( QString naptr )
{
	return getResSearch( naptr, T_SRV, true );
}

void SipClient::callMemberUpdated( void )
{
	callListUpdated();
}

QString SipClient::getContactText(void) {
QString v6;
if(useIPv6) v6  = "\nIPv6"; else v6="\nIPv4";

QString mio     = " Me : " + KStatics::myIP;
QString mpo      = ":" + QString::number(nlp,10);
QString mi      = "\nbehind NAT: " + contacturi.getHostname();
QString mp      = ":" + QString::number(contacturi.getPortNumber(),10);

QString trp;
if( isTcpSocket() ) trp  = " TCP"; else trp = " UDP";     
if(foundStun) return  mio + mpo + mi + mp+ v6 + trp; else  return  mio + mpo + v6 + trp;

}
QString SipClient::getListenPort(void) {
return QString::number(contacturi.getPortNumber());
}

bool SipClient::haveNoMoreStun(void) {
	if(stopStun) {
		// return true (Stun was switched of) and delete the info for this time
		stopStun=false;
		return true;
	} else {
		return false;
	}
}

void SipClient::initLocServ(QString locServ) {
		if (locServ == "DNS") {
		    setLocServ(DNS);
		} else if (locServ == "SRV") {
		    setLocServ(SRV);
		} else {
		    setLocServ(NAPTR);
		}
}
