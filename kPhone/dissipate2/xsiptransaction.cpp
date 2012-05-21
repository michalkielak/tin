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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <qtimer.h>
#include <qdom.h>
#include "sipclient.h"
#include "sipuri.h"
#include "sipcall.h"
#include "sipmessage.h"
#include "siptransaction.h"
#include "sdpbuild.h"
#include "../kphone/kstatics.h"

#include <iostream>
using namespace std;

SipTransaction::SipTransaction( unsigned int seqn, SipCallMember *farend, SipCall *call )
{
	atomId = 1000;
	responses.setAutoDelete( true );
	parent = call;
	requestmessage = NULL;
	direction = None;
	remote = farend;
	seqnum = seqn;
	int uniqid = rand();
	branch.sprintf( "z9hG4bK%X", uniqid );
	cancelled = false;
	inviteRep = 7;
	SipETag = "";
	minExpires = 0;
	rseq = 0;
	timer_202 = new QTimer( this );
	connect( timer_202, SIGNAL( timeout() ), this, SLOT( send_202() ) );
        if(KStatics::debugLevel>=2) cout << ":::::SipTransaction start\n"; 
}

SipTransaction::~SipTransaction( void )
{
	if(KStatics::debugLevel>=2)  cout << ":::::SipTransaction end\n"; 
	if( requestmessage ) {
		delete requestmessage;
	}
	responses.clear();
}

QString SipTransaction::getCSeq( void ) const
{
	QString cseq;
	cseq = QString::number( seqnum ) + " " + Sip::getMethodString( requestmessage->getMethod() );
	return cseq;
}

QString SipTransaction::getRSeq( void )
{
	QString Rseq;
	if (rseq == 0) {
		rseq = seqnum;
	} else {
		rseq += 2;
	}
	Rseq = QString::number( rseq );
	parent->setRSeq(Rseq);
	return Rseq;
}

QString SipTransaction::getRAck( void )
{
	QString RAck;
	RAck = QString::number( seqnum ) + " " + QString::number(  seqnum ) +  "INVITE";
	return RAck;
}

QString SipTransaction::getSeqMethod( void ) const
{ 
	return Sip::getMethodString( requestmessage->getMethod() );
}

void SipTransaction::setStatus( const SipStatus &stat )
{
	laststatus = stat;
	statusUpdated();
}

bool SipTransaction::sendRequest( Sip::Method meth, const QString &body,
		const MimeContentType &bodytype, const SipUri &referto, const bool shortTr,
		const QString &proxyauthentication,const QString &authentication, const int expiresTime )
{
	if ( requestmessage == NULL ) {
		direction = LocalRequest;
	}

	// Construct message
	requestmessage = new SipMessage;
	requestmessage->setType( SipMessage::Request );
	requestmessage->setMethod( meth );
	// Set Request Uri
	if( meth != Sip::SUBSCRIBE ||
	    remote->getCall()->getCallStatus() == SipCall::callDisconnecting ||
	    remote->getCall()->getCallStatus() == SipCall::callInProgress ) {
		requestmessage->setRequestUri( remote->getContactUri() );
	} else {
		if (KStatics::debugLevel > 2){  cout << "======SipTransaction::sendRequest changing requestUri ";
			QString cru =  requestmessage->getRequestUri().uri() + " to " + remote->getUri().uri() + "\n";
			cout << cru;
		}
		requestmessage->setRequestUri( remote->getUri() );
	}
	requestmessage->insertHeader( SipHeader::CSeq, getCSeq() );
	if( ( meth != Sip::INVITE && meth != Sip::MESSAGE && meth != Sip::SUBSCRIBE && meth != Sip::CANCEL ) ||
	    remote->getCall()->getCallStatus() == SipCall::callDisconnecting ||
	    remote->getCall()->getCallStatus() == SipCall::callInProgress ) {
		requestmessage->insertHeader( SipHeader::To, remote->getUri().nameAddr() );
	} else {
		requestmessage->insertHeader( SipHeader::To, remote->getUri().nameAddr_noTag() );
	}

	if( meth == Sip::REFER && referto != SipUri::null ) {
		requestmessage->insertHeader( SipHeader::Refer_To, referto.uri() );
		requestmessage->insertHeader( SipHeader::Referred_By, parent->getContactUri().nameAddr() );
		if (shortTr) {
		    requestmessage->insertHeader( SipHeader::Refer_Sub, "false" );
		}
	}
	if( proxyauthentication != QString::null ) {
		requestmessage->insertHeader( SipHeader::Proxy_Authorization, proxyauthentication );
	}
	
	if( authentication != QString::null ) {
		requestmessage->insertHeader( SipHeader::Authorization, authentication );

	}

	if( expiresTime >= 0 ) {
		QString strExpiresTime;
		requestmessage->insertHeader( SipHeader::Expires, strExpiresTime.setNum( expiresTime ) );
	}

	if( meth == Sip::INVITE ) {

		if(parent->getPrack() == 1) {
			requestmessage->insertHeader( SipHeader::Allow, "INVITE, ACK, BYE, CANCEL, OPTIONS, PRACK");		
			requestmessage->insertHeader( SipHeader::Supported, "100rel" );

		} else if(parent->getPrack() == 2){
			requestmessage->insertHeader( SipHeader::Allow, "INVITE, ACK, BYE, CANCEL, OPTIONS, PRACK");		
			requestmessage->insertHeader( SipHeader::Require, "100rel" );

		} else {
			requestmessage->insertHeader( SipHeader::Allow, "INVITE, ACK, BYE, CANCEL, OPTIONS");
		}

	}

	if( meth == Sip::PRACK ) { //$
	    parent->setPrackReq (SipCall::SPSsent);
	    requestmessage->insertHeader( SipHeader::Allow, " INVITE, ACK, BYE, CANCEL, OPTIONS, PRACK");
	    requestmessage->insertHeader( SipHeader::RAck, parent->getRAck() );
	}

	if( meth == Sip::NOTIFY ) {
		if ( expiresTime == 0) {
			requestmessage->insertHeader( SipHeader::Subscription_State, "terminated;reason=deactivated" );
		} else {
            		requestmessage->insertHeader( SipHeader::Subscription_State, "active" );
		}
	}

	if( bodytype != MimeContentType::null ) {
		requestmessage->insertHeader( SipHeader::Content_Type, bodytype.type() );
	}
	requestmessage->setBody( body );

	return parent->sendRequest( requestmessage, true, SipUri::null, branch );
}

void SipTransaction::sendResponse( const SipStatus &status, const bool prack, const QString RefSub, const QString &body, const MimeContentType &bodytype )
{
	SipMessage *response = new SipMessage;
	setStatus( status ); // Update our status
	responses.append( response ); // Save this response
	response->setType( SipMessage::Response );
	response->setStatus( status );
	// Copy via list exactly
	response->setViaList( requestmessage->getViaList() );
	response->insertHeader( SipHeader::From, remote->getUri().nameAddr() );
	response->insertHeader( SipHeader::CSeq, getCSeq() );//$1
	if(prack) {
	    if(parent->getPrackReq() == SipCall::SPSreq) {
		response->insertHeader( SipHeader::Allow, " INVITE, ACK, BYE, CANCEL, OPTIONS, PRACK");		
		response->insertHeader(SipHeader::Require, "100rel");
		response->insertHeader(SipHeader::RSeq, getRSeq());
	    } else if(parent->getPrackReq() == SipCall::SPSsup) {
		response->insertHeader( SipHeader::Allow, " INVITE, ACK, BYE, CANCEL, OPTIONS, PRACK");		
		response->insertHeader(SipHeader::RSeq, getRSeq());
		response->insertHeader(SipHeader::Supported, "100rel");
	    } 
	} else {
		if (requestmessage->getMethod() == Sip::INVITE) {
			 response->insertHeader( SipHeader::Allow, " INVITE, ACK, BYE, CANCEL, OPTIONS");		
		}
	}
	
	if (requestmessage->getMethod() ==Sip::PRACK) {
		 response->insertHeader( SipHeader::Allow, " INVITE, ACK, BYE, CANCEL, OPTIONS, PRACK");		
	}
	if( bodytype != MimeContentType::null ) {
		response->insertHeader( SipHeader::Content_Type, bodytype.type() );
	}
        //cout << " ???? STR sendResponse[" << RefSub << "]" <<endl;

	if (RefSub.contains("false"))  	response->insertHeader( SipHeader::Refer_Sub, RefSub );

	response->setBody( body );
	parent->sendResponse( response );
}


void SipTransaction::sendResponseOP( const SipStatus &status, const QString &body )
{
	SipMessage *response = new SipMessage;
	setStatus( status ); // Update our status
	responses.append( response ); // Save this response
	response->setType( SipMessage::Response );
	response->setStatus( status );
	// Copy via list exactly
	response->setViaList( requestmessage->getViaList() );
	response->insertHeader( SipHeader::From, remote->getUri().nameAddr() );
	response->insertHeader( SipHeader::CSeq, getCSeq() );
	response->insertHeader( SipHeader::Event, "registration" );
	response->insertHeader( SipHeader::Allow_Events, "presence" );
	response->insertHeader( SipHeader::Accept, "application/sdp");
	parent->sendResponse( response );
}

void SipTransaction::sendResponseNC( const SipStatus &status, const QString &body, const MimeContentType &bodytype )
{
	SipMessage *response = new SipMessage;
	setStatus( status ); // Update our status
	responses.append( response ); // Save this response
	response->setType( SipMessage::Response );
	response->setStatus( status );
	// Copy via list exactly
	response->setViaList( requestmessage->getViaList() );
	response->insertHeader( SipHeader::From, remote->getUri().nameAddr() );
	response->insertHeader( SipHeader::CSeq, getCSeq() );
	response->getContactList().addToHead(body);
	parent->sendResponse( response, false );
}

void SipTransaction::sendRegister( const SipUri &registerserver,
	int expiresTime, const QString &authentication,
	const QString &proxyauthentication, const QString &body,
	const MimeContentType &bodytype, const QString &qvalue )
{
	if ( requestmessage == NULL ) {
		direction = LocalRequest;
	}

	// Create message
	requestmessage = new SipMessage;
	requestmessage->setType( SipMessage::Request );
	requestmessage->setMethod( Sip::REGISTER );
	requestmessage->setQvalue( qvalue );

	// Set request URI
	requestmessage->setRequestUri( registerserver );

	requestmessage->insertHeader( SipHeader::CSeq, getCSeq() );
	requestmessage->insertHeader( SipHeader::To, remote->getUri().nameAddr() );

	if( authentication != QString::null ) {
		requestmessage->insertHeader( SipHeader::Authorization, authentication );
	}

	if( proxyauthentication != QString::null ) {
		requestmessage->insertHeader( SipHeader::Proxy_Authorization, proxyauthentication );
	}
	if( bodytype != MimeContentType::null ) {
		requestmessage->insertHeader( SipHeader::Content_Type, bodytype.type() );
	}

	requestmessage->setBody( body );

	if( expiresTime >= 0 ) {
		QString strExpiresTime;
		requestmessage->insertHeader( SipHeader::Expires, strExpiresTime.setNum( expiresTime ) );
	}

	parent->sendRequest( requestmessage, true, registerserver );
}

//



void SipTransaction::sendPublish(const QString state,
				const QString sipIfMatch,
				const SipUri &publishserver,
				int expiresTime,
				const QString &authentication,
                                const QString &proxyauthentication,
				const QString &body )
{
	QString lstate = state;
	if ( requestmessage == NULL ) {
		direction = LocalRequest;
	}

	// Create message:
	requestmessage = new SipMessage;

	requestmessage->setType( SipMessage::Request );
	requestmessage->setMethod( Sip::PUBLISH );
	requestmessage->setRequestUri( publishserver.uri());
	requestmessage->insertHeader( SipHeader::CSeq, getCSeq() );

	SipUri anUri = remote->getUri();
	requestmessage->insertHeader( SipHeader::To, anUri.nameAddr() );
		
	if( authentication != QString::null ) {
		requestmessage->insertHeader( SipHeader::Authorization, authentication );
	}

	if( proxyauthentication != QString::null ) {
		requestmessage->insertHeader( SipHeader::Proxy_Authorization, proxyauthentication );
	}

	if (lstate == "-") {
	    expiresTime = 0; 
	}
	if( expiresTime >= 0 ) {
		QString strExpiresTime;
		requestmessage->insertHeader( SipHeader::Expires, strExpiresTime.setNum( expiresTime ) );
	}
	if ( sipIfMatch != "" ) {
		requestmessage->insertHeader( SipHeader::SIP_If_Match, sipIfMatch );
	}

	
	if (expiresTime > 0) { 
		MimeContentType bodytype = MimeContentType("application/pidf+xml");
		requestmessage->insertHeader( SipHeader::Content_Type, bodytype.type() );
		requestmessage->setBody( body);
	}

	parent->sendRequest( requestmessage, true, publishserver );
}

//
void SipTransaction::incomingResponse( SipMessage *message )
{
	responses.append( message );
	SipETag = message->getSipIfMatch();
	minExpires =atoi(message->getMinExpires());
	// Grab tags
	SipUri newto( message->getHeaderData( SipHeader::To ) );
	remote->setUri( newto );
	if( ( (requestmessage->getMethod() == Sip::INVITE ) ) && ( message->getStatus().getCode() >= 200 ) ) {
		sendRequest( Sip::ACK );	
	} else if( ( requestmessage->getMethod() == Sip::ACK ) && ( message->getStatus().getCode() >= 200 ) ) {
		sendRequest( Sip::ACK );
	}
		setStatus( message->getStatus() );
}

void SipTransaction::incomingRequest( SipMessage *message,bool holdmaster )
{
	QString p;
	if( ( message->getMethod() == Sip::ACK ) ) {

		// Make sure listeners know in case there was a new session description
		statusUpdated();
		delete message;
	} else if( ( message->getMethod() == Sip::CANCEL ) ) {
		timer_202->stop();
		if( !cancelled && requestmessage->getMethod() == Sip::INVITE ) {
			sendResponse( SipStatus( 487 ) );
		}
		requestmessage->setMethod( Sip::CANCEL );
		if( cancelled ) {
			sendResponse( SipStatus( 481 ) );
		} else {
			sendResponse( SipStatus( 200 ) );
		}

		// Mark request as cancelled and inform listeners
		cancelled = true;
		statusUpdated();
		delete message;

	} else {
		direction = RemoteRequest;
		if( requestmessage ) {
			delete requestmessage;
		}
		requestmessage = message;
		switch( message->getMethod() ) {
			case Sip::INVITE:
			if( !((parent->getCallType()==SipCall::auviCall) ||(parent->getCallType() == SipCall::videoCall) || (parent->getCallType() >= SipCall::extCall1) ) ) {
					parent->setCallType( SipCall::StandardCall );
			}
			parent->setRAck(message);
			sendResponse( SipStatus( 100 ) );
			break;
			case Sip::BYE:
				if( parent->getCallType() == SipCall::UnknownCall ) {
					parent->setCallType( SipCall::BrokenCall );
				}
				sendResponse( SipStatus( 200 ) );
				break;

			case Sip::OPTIONS:
				{
				if( parent->getCallType() == SipCall::UnknownCall ) {
					parent->setCallType( SipCall::OptionsCall );
				}
				SdpBuild *optsdp = new SdpBuild ( 0 , 0 );
				sendResponseOP( SipStatus( 200 ), optsdp->prepOptString());
				delete optsdp;
				}
				break;

			case Sip::MSG:
				parent->setCallType( SipCall::MsgCall );
				if( message->hasHeader( SipHeader::Subject ) ) {
					parent->setSubject( message->getHeaderData( SipHeader::Subject ) );
				}
				sendResponse( SipStatus( 200 ) );
				break;
			case Sip::SUBSCRIBE:
				if( parent->getCallType() == SipCall::inSubscribeCall ) {
					if( message->hasHeader( SipHeader::Accept ) ) {
						QString Accept = message->getHeaderData( SipHeader::Accept );
						if( Accept.contains("application/pidf") ) {
							parent->setPresBody(1);
						} else if( Accept.contains("application/watcherinfo") ) {
							parent->setPresBody(2);
						} else if(Accept.contains("application/xpidf") ) {
							parent->setPresBody(0);
						} else {
							parent->setPresBody(1);
						}
					
					}
					sendResponse( SipStatus( 202 ) );
				} else if( parent->getCallType() == SipCall::inSubscribeCall_2 ) {
					sendResponse( SipStatus( 403 ) );
					cancelled = true;
				}
				break;
			case Sip::NOTIFY:
				sendResponse( SipStatus( 200 ) );
				parent->setBasePresenceStatus(true);
				if( message->getMethod() == Sip::NOTIFY ) {
					bool p = true;
					if( message->hasHeader( SipHeader::Subscription_State ) ) {
						QString HSubState = message->getHeaderData( SipHeader::Subscription_State );
						if ( HSubState.contains("active") ) {
						    if(parent->getPresState() != SipCall::PsDead ) parent->setPresState(SipCall::PsActive);
						    p = false;//if partner sends no body, presence info is kept 
						}
						
						if ( HSubState.contains("deactivated") || HSubState.contains("terminated") ) { //partner terminated subscription, no presence available
							parent->setPresenceStatus ("offline");
							parent->setBasePresenceStatus(false);
							parent->setCallStatus(SipCall::callDisconnecting);
//							cout << "-------->ST: callDisconnecting\n";
							if(parent->getPresState() != SipCall::PsDead ) parent->setPresState(SipCall::PsTerminated);//PsDead id lethal!
							parent->updateSubscribes();
							return;
						}

					}
					enum EvalPresence  cont = Dummy;
					if( message->hasHeader( SipHeader::Content_Type ) ) {
						QString Content = message->getHeaderData( SipHeader::Content_Type );
						if( Content.contains("application/pidf+xml") ) {
							cont = PidfXml;
						} else if( Content.contains("application/xpidf") ) {
							cont = XPidf;
						} else 	if( Content.contains("application/watcherinfo") ) {
							cont = Watcher;
						} else if( Content.contains("application/pidf") ){
							if (message->haveBody()) cont = Pidf; else cont = PidfNo; 
						} else if ( Content.contains("application/cpim-pidf+xml") ) {
							cont = Cpim;
						} else cont = Dummy;
					}
					QString m = message->messageBody();
					handlePresence(cont, p, m);
				}
				break;

			case Sip::PRACK:
                                      if (!parent->loadCall()) {
                                          sendResponse(SipStatus(488));    //no INVITE-Call
                                      } else  {                                          

					if (message->hasHeader(SipHeader::RAck))  {
						QString rAck = message->getHeaderData(SipHeader::RAck).section(' ',0,0);
                                              if (rAck == parent->getRSeq().latin1()) {
						 
						QString rAck = message->getHeaderData(SipHeader::RAck);
				        	QString scseq  = rAck.section(" ",1,1) +  " " + rAck.section(" ",2,2); 
						sendResponse(SipStatus(200));
						parent->setPrackReq(SipCall::SPSsent);
						parent->setSCSeq(scseq);  
                                              }
                                              else {
                                                  sendResponse(SipStatus(481));     // does not match
                                              }
                                          }
                                          else {
                                              sendResponse(SipStatus(421));
                                          }
                                      }
                                      break;
			case Sip::UPDATE:
				statusUpdated();

				break;
    
			default:
				break;
		}
	}

}

void SipTransaction::incomingRequestRetransmission( SipMessage *message )
{
	QString p;
	if( ( message->getMethod() == Sip::CANCEL ) ) {

		sendResponse( SipStatus( 481 ) );
	} else {
		switch( message->getMethod() ) {
			case Sip::INVITE:
				sendResponse( SipStatus( 100 ) );
				break;
			case Sip::BYE:
				sendResponse( SipStatus( 200 ) );
				break;
			
			case Sip::OPTIONS:
				{
				SdpBuild *optsdp = new SdpBuild ( 0 , 0 );
				sendResponseOP( SipStatus( 200 ), optsdp->prepOptString() );
				delete optsdp;
				}
				break;
			case Sip::MSG:
				sendResponse( SipStatus( 200 ) );
				break;
			case Sip::SUBSCRIBE:
				if( parent->getCallType() == SipCall::inSubscribeCall ) {
					sendResponse( SipStatus( 202 ) );
				} else if( parent->getCallType() == SipCall::inSubscribeCall_2 ) {
					sendResponse( SipStatus( 403 ) );
				}
				break;
			case Sip::NOTIFY:
				sendResponse( SipStatus( 200 ) );
				break;
			default:
				break;
		}
	}
	delete message;
}


bool SipTransaction::auditPending( void )
{
	struct timeval tv;
	struct timeval *reftv;
	unsigned int diff;

	if(KStatics::noLine == 1) inviteRep=1;
	if( ( laststatus.getCode() < 200 ) && ( direction == LocalRequest ) ) {

		// Check time
		gettimeofday( &tv, NULL );
		reftv = requestmessage->getTimestamp();

		// Check difference
		diff = ( tv.tv_sec - reftv->tv_sec ) * 1000 * 1000 + tv.tv_usec - reftv->tv_usec;
		diff = diff / 1000; // ms
		if( requestmessage->getMethod() == Sip::PUBLISH ) {
			remote->setIdle();
			return true;

		} else if( requestmessage->getMethod() == Sip::PRACK ) {
			return true;
			
		} else if( requestmessage->getMethod() != Sip::INVITE ) {
			if( diff > requestmessage->lastTimeTick() ) {
											    
				if(  (requestmessage->sendCount() == 11) || (inviteRep == 1)  ) { 
					if( requestmessage->getMethod() == Sip::SUBSCRIBE ) {
						remote->contactUpdate( false );
					}
					remote->setIdle();
					setStatus( SipStatus( 408 ) );
					return false;
				}
				parent->sendRaw( requestmessage );
				requestmessage->setTimestamp();
				requestmessage->incrSendCount();
				if( requestmessage->lastTimeTick() < 4000 ) {
					if( laststatus.getCode() < 100 ) {
						requestmessage->setTimeTick( 2 * requestmessage->lastTimeTick() );
					} else {
						requestmessage->setTimeTick( 4000 );
					}
				}
				return true;
			}
		} else {
			// INVITEs should be retransmitted differently
			if( laststatus.getCode() < 100 ) {
				if( diff > requestmessage->lastTimeTick() ) {

					// Timeout
					if ( requestmessage->sendCount() >= inviteRep ) {
						setStatus( SipStatus( 408 ) );
						return false;
					}
					
					parent->sendRaw( requestmessage );
					requestmessage->setTimestamp();
					requestmessage->incrSendCount();
					requestmessage->setTimeTick( 2 * requestmessage->lastTimeTick() );
					return true;
				}
			}
		}
	}
	return false;
}

void SipTransaction::cancelRequest( const QString &body, const MimeContentType &bodytype )
{
		if(laststatus.getCode() != 408) {
		requestmessage->setMethod( Sip::CANCEL );
		sendRequest( Sip::CANCEL, body, bodytype );
	} else {
	    inviteRep = 1;
	}
}

QString SipTransaction::getFinalMessageBody( void )
{
	if( responses.last() ) {
		if( responses.last()->haveBody() ) {
			return responses.last()->messageBody();
		}
	}

	return QString::null;
}

MimeContentType SipTransaction::getFinalContentType( void )
{
	if( responses.last() ) {
		if( responses.last()->hasHeader( SipHeader::Content_Type ) ) {
			return MimeContentType( responses.last()->getHeaderData( SipHeader::Content_Type ) );
		}
	}

	return MimeContentType::null;
}

SipUriList SipTransaction::getFinalContactList( void )
{
	if( responses.last() ) {
		return responses.last()->getContactList();
	}

	return SipUriList::null;
}

QString SipTransaction::getFinalWWWAuthString( void )
{
	if( responses.last() ) {
		return responses.last()->getHeaderData( SipHeader::WWW_Authenticate );
	}

	return QString::null;
}

MimeContentType SipTransaction::getUpdateContentType( void )
{
return MimeContentType( requestmessage->getHeaderData( SipHeader::Content_Type ) );
}

QString SipTransaction::getUpdateMessageBody( void )
{
return requestmessage->messageBody();

}

QString SipTransaction::getRequestMessageBody( void ) const
{
	return requestmessage->messageBody();
}

MimeContentType SipTransaction::getRequestMessageContentType( void ) const
{
	if( requestmessage->hasHeader( SipHeader::Content_Type ) ) {
		return MimeContentType( requestmessage->getHeaderData( SipHeader::Content_Type ) );
	}

	return MimeContentType::null;
}

QString SipTransaction::getFinalProxyAuthString( void )
{
	if( responses.last() ) {
		return responses.last()->getHeaderData( SipHeader::Proxy_Authenticate );
	}

	return QString::null;
}

void SipTransaction::send_202( void )
{
	sendResponse( SipStatus( 202 ) );
}

QString SipTransaction::getxmls(const QString which, const QString mess) {
	QString ret = "";
	int i0 = mess.find(which);
	if (i0 == 0) return "";
	int i1 = mess.findRev("xmlns:",i0);
	ret = mess.mid(i1+6, mess.find('=',i1)-i1-6);
	return ret; 
		
}
int SipTransaction::getMinExpires(void) {
	return minExpires;
}

QString SipTransaction::getSipIfMatch(void) {
	return SipETag;
}

void SipTransaction::handlePresence(enum EvalPresence  cont, bool p, QString m) {

	switch (cont) {
		case Dummy:
			if (!p) { //no Body + active
				if(parent->getPresenceStatus() == "offline" ) {
				    parent->setPresenceStatus ("pending");
				}
				parent->setBasePresenceStatus(true);
			} 
			break;
		case XPidf:
			{
				parent->setBasePresenceStatus(true);
				QString subscript_stat;
				int idx = m.find("substatus=");
				if (idx != -1) {
        				m = m.mid( idx + 11 );
        				parent->presenceStatus =  m.left( m.find('"') );
    				} else {
    					idx = m.find("status status=");
					if (idx != -1) {
						m = m.mid( idx + 15 );
						parent->presenceStatus = m.left( m.find('"') );
					} else {
						parent->setBasePresenceStatus(false);
						parent->presenceStatus = "Offline";
					}
				}
			}
			break;
		case Watcher:
			getWInfo(m);
			break;
		case Pidf:
			getpidf(m);
			break;
		case PidfNo:
			if (p) {
				parent->setPresenceStatus ("offline");
				parent->setBasePresenceStatus(false);
			}
			break;
		case PidfXml:
			getpidfxml(m);
			break;
		case Cpim:
			getcpim(m);
			break;
		default:
			break;
	}
}

void SipTransaction::getcpim( const QString m ) {
	QDomDocument doc("presence");
	if (doc.setContent(m))
	{
		// The tuple with an id of
		// "summary" has a status node with
		// a value tag with the global
		// status.
		QDomNodeList nl = doc.elementsByTagName("tuple");
		for (unsigned int i=0; i<nl.count(); ++i)
		{
			QDomNode qdn = nl.item(i);
			QDomElement e = qdn.toElement();
			if (!e.isNull())
			{
				QString s = e.attribute("id");
				if ((!s.isNull()) && ( s == "summary"))
				{
					QStringList retval;
					// I want the contents of the "value" and
					// "note" elements. I'll build a string
					// containing both separated by a "/"
					QDomNodeList vlist = e.elementsByTagName("value");
					// There should be at most 1
					// of these.
					if ( 1 == vlist.count())
					{
						QDomNode value = vlist.item(0);
						QDomElement ve = value.toElement();
						if (!ve.isNull())
						{
							// Yay!  This is what we
							// wanted.
							retval.append(ve.text());
						}
					}
					vlist = e.elementsByTagName("note");
					if ( 1 == vlist.count())
					{
						QDomNode value = vlist.item(0);
						QDomElement ve = value.toElement();
						if (!ve.isNull())
						{
							// Yay!  This is what we
							// wanted.
							retval.append(ve.text());
						}
					}
					// We should have a basic node child.
					vlist = e.elementsByTagName("basic");
					if ( 1 == vlist.length() )
					{
						if (KStatics::debugLevel > 2) cout << "basic status is " << vlist.item(0).toElement().text() << endl;
						parent->setBasePresenceStatus( vlist.item(0).toElement().text() == "OPEN");
					}	
					parent->setPresenceStatus( retval.join("/") );
					return;
				}
			}
		}
	}
}

void SipTransaction::getpidfxml( const QString m) {
	if(KStatics::debugLevel>= 2) cout <<"======Begin SipTransaction::getpidfxml\n";
	QDomDocument doc("presence");
	bool bc = false;

	// You want the person tuple's status/basic node as well as the activities sub-node.
	// If the activities sub-node actually has content, you want the content.
	if (doc.setContent(m, true)) {
		QDomNodeList nl = doc.elementsByTagNameNS("urn:ietf:params:xml:ns:pidf:data-model","person");
		for (unsigned int i=0; i<nl.count(); ++i) {
			if(KStatics::debugLevel>= 2) cout << "=====SipTransaction::getpidfxml found person node\n";
			QDomElement qde = nl.item(i).toElement();
			if (!qde.isNull()) {
				// There should be one status node followed by a basic node.
				// First status node wins.
				QDomNodeList nl2 = qde.elementsByTagName("status");
				if (nl2.length() > 0) {
					if(KStatics::debugLevel>= 2) cout << "=====SipTransaction::getpidfxml found" << nl2.length() << "status node\n";
					QDomNodeList nl3 = nl2.item(0).toElement().elementsByTagName("basic");
					if (nl3.length() > 0) {
						if(KStatics::debugLevel>= 2) cout << "=====SipTransaction::getpidfxml found" << nl3.length() << "basic node\n", nl3.length();
						QDomElement qde2 = nl3.item(0).toElement();
						if(KStatics::debugLevel>= 2) cout << "=====SipTransaction::getpidfxml status node text is: " << qde2.text() << endl;
						if (!qde2.isNull()) {
							if(KStatics::debugLevel>= 2) cout << "=====SipTransaction::getpidfxml basic status is " << qde2.text() << endl;
							int idx = qde2.text().find("open");
							bc = (idx != -1);
						}
					}
				}
				// OK, now look for user-input.  It may not exist. If it does, don't bother looking for more status.
				nl2 = qde.elementsByTagNameNS("urn:ietf:params:xml:ns:pidf:rpid", "user-input");
				if (nl2.length() > 0) {
					if(KStatics::debugLevel>= 2) cout << "=====SipTransaction::getpidfxml found " << nl2.length() << " user-input node\n", nl2.length();
					if (!nl2.item(0).toElement().isNull()) {
						if(KStatics::debugLevel>= 2) cout << "=====SipTransaction::getpidfxml found user-input: " << nl2.item(0).toElement().text() << endl;
						parent->setActivityStatus(nl2.item(0).toElement().text());
						parent->setBasePresenceStatus( bc );
					}
				}

				// Effectively no user-input status. Look for activities.
				nl2 = qde.elementsByTagNameNS("urn:ietf:params:xml:ns:pidf:rpid", "activities");
				if (nl2.length() > 0) {
					QDomNode gok = nl2.item(0).firstChild();
					if ( gok.isText() ) {
						if(KStatics::debugLevel>= 2) cout << "=====SipTransaction::getpidfxml found activity text: " << gok.toText().data() << endl;
						parent->setPresenceStatus(gok.toText().data());  
						parent->setBasePresenceStatus( bc );
						return;
					}
					// Hmm. Not text node.  Get the node local name and use that.
					if(KStatics::debugLevel>= 2) cout << "=====SipTransaction::getpidfxml using activity node ";
					if(gok.localName()  == "activity") { // we have <activity>text</activity>
					    parent->setPresenceStatus(gok.toElement().text());
					    cout << "toElement " << gok.toElement().text() << endl;
					    
					} else {
					    parent->setPresenceStatus( gok.localName());
					    cout << " localName " << gok.localName() << endl;
					}
					parent->setBasePresenceStatus( bc );
					return;
				}
				
			}
		}
	        //post processing for minimalist's  UA
		if (!bc) {	
		int idx = m.find("basic>closed");//a closed not tied to a person, openSER sends this if an UA goes offline
		int idxo =m.find("basic>open");  //some phones only know this
		    if(idx >=0) {
		    	if(KStatics::debugLevel>= 2) cout << "=====SipTransaction::getpidfxml <status><basic>closed ... \n";
			parent->setPresenceStatus ("offline");
			parent->setBasePresenceStatus(false);
			return;
		    } else if (idxo >0) {
		    	if(KStatics::debugLevel>= 2) cout << "=====SipTransaction::getpidfxml <status><basic>open ... \n";
			parent->setPresenceStatus ("online");
			parent->setBasePresenceStatus(true);
			return;
		    }

		}
	} else cout << "=====SipTransaction::getpidfxml nix\n"; 
}

void SipTransaction::getpidf(const QString m){
	int idx;
	bool iac=false;
	QString pt = getxmls("pidf:data-model",m);
	pt+=":note>";

	QString rp = getxmls("pidf:rpid",m);
	QString rpa= rp + ":activities>";
	rp+=":user>";

	QString ac = getxmls("pidf:rpid-status",m);
	QString ac1= ac+ ":activity>";
	ac+= ":activities>";

	QString rpp = getxmls("rpid-person",m);
	QString rppa= rpp + ":activity>";
	rpp="</" + rpp + ":activity>";
	//find the entry
	idx = m.find("basic>open");
	if (idx != -1) {
		parent->setBasePresenceStatus(true);
	} else 	{
		idx = m.find("basic>closed");
		if(idx >=0) {
			parent->setPresenceStatus ("offline");
			parent->setBasePresenceStatus(false);
			return;
		}
	}
		idx = m.find(pt);
		if (idx <0) {
			idx = m.find(rpa);
			iac=true;
			if (idx <0) {
			idx = m.find(rppa);
			iac=false;
				if (idx <0) {
					idx = m.find(ac);
					iac=false;
					if (idx <0) {
						idx = m.find(ac1); 
						if (idx < 0) { 
							idx = m.find(rp);
							if (idx < 0) {
								idx = m.find(":note>"); //last chance part 1
								if (idx < 0) {
									idx = m.find("<note:");//last chance part 2
								}
							}
						}
					}
				}	
			}
		}
		// fetch the content
		if( idx != -1) {
			int idx1 = 0;
			QString m1="";
			QString pst;
			if(iac) {
				idx1 = m.find("<",idx);
				idx1 = m.find(":",idx1);
				m1 = m.mid( idx1+1);
				pst = m1.left( m1.find("/>") );
				parent->setPresenceStatus (pst);
			} else {
				idx1 = m.find(">",idx);
				m1 = m.mid( idx1+1);
				pst = m1.left( m1.find('<') );
				parent->setPresenceStatus (pst);
			}
		} 
}

void SipTransaction::getWInfo(const QString m){
    int i0=0,i1=0,i2=0,sti=0;
    bool  haveW = false;
    QString st;
    i0  = m.find ("<watcherinfo");
    if (i0 < 0) {
	return;
    } else {
	i0 = m.find ("state=");
	i1 = m.find("\">",i0);
	if(i1>0) {
	    i2 = m.find("<watcher-list",i1);
	    if (i2>i1) {
	        st = m.mid(i0+7,i1-i0-7);
		haveW = true;
		if(st.contains("full")) {
		    sti=1;
		    parent->processWatchers("","","",sti);//init watcher list
		}
		i0=0; i1=i2;
		while(i1>=0 && i0>=0) {
		    i0 = m.find("<watcher ",i1);
		    if(i0 > 0) {
			i1 = m.find("</watcher",i0);
			st = m.mid(i0,i1-i0);
			if(sti>0) sti++;
			getWatcher(st,sti);
		    }
		}		
	    } 
	} 
    }
}

void SipTransaction::getWatcher(const QString m, int state){

    int i0=0,i1=0;
    QString ev;
    QString st;
    QString uri;
    i0  = m.find ("event=\"",i1);
    if(i0>0) {
        i1  = m.find ("\"",i0+7);
	if (i1>0) {
	    ev = m.mid(i0+7,i1-i0-7);
	}
    }

    i0  = m.find ("status=\"");
    if(i0>0) {
        i1  = m.find ("\"",i0+8);
	if (i1>0) {
	    st = m.mid(i0+8,i1-i0-8);
	}
    }
    i0  = m.find ("sip:",i1+2);
    if(i0>0) {
	if (i1>0) {
		uri = m.mid(i0);
		parent->processWatchers(uri,ev,st,state);
	}
    }
    
}

