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
#include <qmessagebox.h>
#include <qdatetime.h>
#include <stdio.h>
#include "../kphone/kstatics.h"
#include "sipclient.h"
#include "sipcall.h"
#include "sipstatus.h"
#include "siptransaction.h"
#include "sippublish.h"
#include "sipuser.h"
#include <iostream>
using namespace std;

SipPublish::SipPublish( SipUser *user, const QString serveruri, int PubExp, int actT )
{

	
	isActive = false;
	_lastActive =  QDateTime::currentDateTime( Qt::UTC );
	pubtrans = 0;
	curtrans = 0;
	pubUser = user;
	pubExpires = PubExp;
	noncecounter = 0;
	pubop = PubNull;
	pubstate = pubPublished;
	initPublish = true;
	pubQueue = false;

	publishtimer = new QTimer(this);
	connect( publishtimer, SIGNAL( timeout() ), this, SLOT( publish_timeout() ) );

	pubcall = new SipCall( user, QString::null, SipCall::PublishCall );
	pubcall->setSubject( "Publish Call" );
	pubserver = new SipUri(serveruri);
	pubserver->generateTag();
	pubCallMember = new SipCallMember(pubcall,*pubserver );
	presuri = new SipUri(serveruri);
	presuri->generateTag();
	
	//Problems with OMA, may be correct in non-OMA environments
	//presuri->setProtocolName("pres");

	curState = "other";
	curNote = "Offline";
	newState = "0";
	sipIfMatch = "";
	int uniqid = rand();
	tuple.sprintf( "tp%X", uniqid );
	person.sprintf( "pe%X", uniqid );
	publishtimer->start( pubExpires*900, TRUE ); 

	if (actT > 0) useActive = true; else useActive = false;

        if (KStatics::debugLevel >= 2) cout << ":::::SipPublish start\n";

}

SipPublish::~SipPublish( void )
{
	delete publishtimer;
	delete pubcall;
 	if (KStatics::debugLevel >= 2) cout << ":::::SipPublish stop\n";
}

void SipPublish::requestPublish(const QString state , const QString &username, const QString &password)
{
		curState = state;
		if( !username.isEmpty() && !password.isEmpty() && ( pubstate == AuthenticationRequired || pubstate == AuthenticationRequiredWithNewPassword ) ) {
			if( authtype == DigestAuthenticationRequired ) {
				authresponse = Sip::getDigestResponse(username, password, "PUBLISH", pubserver->uri(), authstr ,++noncecounter);
			} else if( authtype == BasicAuthenticationRequired ) {
				authresponse = Sip::getBasicResponse( username, password );
			} else if( authtype == ProxyDigestAuthenticationRequired ) {
				proxyauthresponse = Sip::getDigestResponse(username, password, "PUBLISH", pubserver->uri(), proxyauthstr,++noncecounter );
			} else if( authtype == ProxyBasicAuthenticationRequired ) {
				proxyauthresponse = Sip::getBasicResponse( username, password );
			}
			pubstate = TryingServerWithPassword;
		} else {
			pubstate = TryingServer;
		}
		
		QString body = buildPublishBodyPIDF(state);
		if(pubtrans) {
			disconnect( pubtrans, 0, this, 0 );
			pubtrans = 0;
		}
		pubtrans = pubcall->newPublish(pubCallMember,state, sipIfMatch ,*pubserver,pubExpires,authresponse,proxyauthresponse,body);
		connect( pubtrans, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdated() ) );
		statusUpdated();
}
	


void SipPublish::localStatusUpdated( void )
{
	QString authstrtemp;
	SipStatus status;
	if ( (curtrans == 0) && (pubtrans == 0) ) {
		if (KStatics::debugLevel > 0)cout <<  "=====SipPublish::localStatusUpdated : Received what was likely a retransmission, badly ignoring...\n";
		return;
	}
		
	int respCode = pubtrans->getStatus().getCode();
	if (KStatics::debugLevel >= 2) cout << "=====SipPublish::localStatusUpdated lsta is here: pubop=" <<  pubop << " pubstate=" <<  pubstate << " resp=" << respCode << endl;
	if (respCode < 200) {
	} else if (respCode == 200) {
	    QString sim = pubtrans->getSipIfMatch();
	    if (sim != "") {
		sipIfMatch = sim; 
	    }
	    switch ( pubop ) {
		case PubInitial:
			pubstate = pubPublished;
			pubop = PubNull;
			if (pubQueue) {
				pubstate = pubPending;
				pubop = PubModify;
				requestPublish (  curState, pubUser->getUri().getProxyUsername(), pubUser->getUri().getPassword());
				pubQueue = false;
			} else {
				publishtimer->start( pubExpires*900, TRUE ); 
			}
		break;
		case PubRefresh:
			pubstate = pubPublished;
			pubop = PubNull;
			if (pubQueue) {
				pubstate = pubPending;
				pubop = PubModify;
				requestPublish (  curState, pubUser->getUri().getProxyUsername(), pubUser->getUri().getPassword());
				pubQueue = false;
			} else {
				publishtimer->start( pubExpires*900, TRUE ); 
				}
		break;
		case PubModify:
			pubstate = pubPublished;
			pubop = PubNull;
		case PubRemove:
			pubstate = NotConnected;
			pubop = PubNull;
			pubQueue = false; // to late !
		break;
		default:
			pubstate = pubPublished;		
		break;
		}
		statusUpdated();
	} else if ( respCode == 404) {//User is apparently not known, we stop publishing
		publishtimer->stop( ); 
		pubQueue = false;
		pubstate = pubPublished;
		pubop = PubNull;
	} else if (respCode == 412) { //START OF A NEW PUBLISH INITIAL PROCESS
		sipIfMatch = "";
		publishtimer->stop( ); 
		pubQueue = false;
		pubop = PubInitial;
		requestPublish (  curState, pubUser->getUri().getProxyUsername(), pubUser->getUri().getPassword() );
	} else if (respCode == 423) { // restart timer
		publishtimer->stop( ); 
		pubQueue = false;
		publishtimer->start( pubExpires*900, TRUE ); 
	} else if ( (respCode == 480) || ( respCode == 403) || (respCode == 503) ) {
		pubstate = PublishFailed;
		statusUpdated();
		// the reaction on 503 may be to harsh. If the message contains a RetryAfter Header, restart of 
		// the Publish process  after that time may be better than just stop publishing. Open  for discussion!
	} else if( respCode == 401 ) {
     		if (pubstate == TryingServerWithPassword) {
			pubstate = AuthenticationRequiredWithNewPassword;
		} else {
			pubstate = AuthenticationRequired;
			}
		authstr = pubtrans->getFinalWWWAuthString(); 
		authstrtemp = authstr.lower();
		if( authstrtemp.contains( "digest" ) ) {
			authtype = DigestAuthenticationRequired;
		} else {
			authtype = BasicAuthenticationRequired;
		}
		requestPublish ( curState , pubUser->getUri().getProxyUsername(), pubUser->getUri().getPassword() );
	} else if ( respCode == 407 ) {
		if (pubstate == TryingServerWithPassword) {
			pubstate = AuthenticationRequiredWithNewPassword;
		} else {
			pubstate = AuthenticationRequired;
		}
		proxyauthstr = pubtrans->getFinalProxyAuthString();
		authstrtemp = proxyauthstr.lower();
		if( authstrtemp.contains( "digest" ) ) {
			authtype = ProxyDigestAuthenticationRequired;
		} else {
			authtype = ProxyBasicAuthenticationRequired;
		}

		requestPublish ( curState , pubUser->getUri().getProxyUsername(), pubUser->getUri().getPassword() ) ;
	} else {
	if (KStatics::debugLevel >= 2) cout << "=====SipPublish::localStatusUpdated the rest restart publish timer\n";
		publishtimer->stop( ); 
		publishtimer->start( pubExpires*900, TRUE ); 
	}
}



void SipPublish::publish_timeout() 
{
		pubstate = pubPending;
		pubop = PubRefresh;
		requestPublish ( curState , pubUser->getUri().getProxyUsername(), pubUser->getUri().getPassword() ); 
	
if (pubstate == NotConnected) {
		// should not happen, timer is activated with the  1st PUBLISH
	} else if ( pubstate == pubPublished) {
		pubstate = pubPending;
		pubop = PubRefresh;

		requestPublish ( curState , pubUser->getUri().getProxyUsername(), pubUser->getUri().getPassword() ); 
	} else {
		publishtimer->start( pubExpires*900, TRUE );
	}
}

void SipPublish::publishRequest(const QString state) {
         if (state == "-") publishtimer->stop();
	 if	( pubstate >= pubPublished) {
		if( pubstate == pubPublished) {
		    pubstate = pubPending;
		    pubop = PubRefresh;
		}
		curState=state;
		requestPublish (  curState, pubUser->getUri().getProxyUsername(), pubUser->getUri().getPassword());
	} else {
		// we have to wait until operation is done!
		curState=state;
		pubQueue = true;
	}
}

QString SipPublish::buildPublishBodyPIDF(const QString state) {

	QString body;
	QString tmp;
	QString note = "Offline";
	QString basic = "open";
	if ( (state == "offline") || (state == "-") ) basic = "closed";
	if(useActive) {
	    note = (isActive ? "active" : "idle");
	} 
	time_t ti;
	tm *t;
	ti = time( NULL );
	t = gmtime( &ti );
	QDateTime dt =QDateTime::currentDateTime();
	
	body =  "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>";
	body += "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" ";
	body += "entity=\"" + presuri->uri() + "\" ";
	body += "xmlns:caps=\"urn:ietf:params:xml:ns:pidf:caps\" "; 
	body += "xmlns:cipid=\"urn:ietf:params:xml:ns:pidf:cipid\" "; 
	body += "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" ";
	body += "xmlns:rpid=\"urn:ietf:params:xml:ns:pidf:rpid\">";
	body += "<tuple id=\"" + tuple + "\">";
	body += "<status>";
	body += "<basic>" + basic + "</basic>";
	body += "</status>";
	body += "<rpid:class>" + KStatics::me + "</rpid:class>";
	// The rpid class has to be the following to get a Nortel IMS 5.0/MCP 5200 system to pay attention to your active/idle state.
	// body += "<rpid:class>Nortel PCC 5.0.236</rpid:class>";
	body += "</tuple>";	
	body += "<dm:person id=\"" + person + "\">";
	body += "<status><basic>" + basic + "</basic></status>";
	if(useActive) {
		body += "<rpid:user-input last-input=\"" + _lastActive.toString(Qt::ISODate) + "\">" + note  + "</rpid:user-input>";
	}
	body += "<rpid:activities>";
	body += "<rpid:";
	body += state;
	if ("other" == state) {
		body += ">" + note + "</rpid:other>";
	} else {
		if(!useActive) note = state;
		body += "/>"; 
	}
	body += "</rpid:activities>";
	body += "<dm:note>";
	body += note;
	body += "</dm:note>";
	body += "</dm:person>";
	body += "</presence>";
	return body;
}
void SipPublish::inactive( const QDateTime lastActive ) {
	_lastActive = lastActive;
	isActive = false;
	publishRequest(curState);
}

void SipPublish::active( const QDateTime lastActive ) {
	_lastActive = lastActive;
	isActive = true;
	publishRequest(curState);
}
