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
#include <sys/time.h>
#include <stdlib.h>
#include <qstring.h>
#include "sipuri.h"
#include "sipuser.h"
#include "sipclient.h"
#include "sipmessage.h"
#include "siptransaction.h"
#include "sipcall.h"
#include "../kphone/kstatics.h"
 #include <iostream>
using namespace std;

SipCallMember::SipCallMember( SipCall *parent, const SipUri &uri )
{
	authstate = authState_Authentication;
	callMemberType = unknown;
	call = parent;
	memberuri = uri;
	contacturi = uri;
	local = 0;
	remote = 0;
	localExpiresTime = -1;
	state = state_Disconnected;
	parent->addMember( this );
	timer = new QTimer( this );
	connect( timer, SIGNAL( timeout() ), this, SLOT( call_timeout() ) );

        holdRefused=false;
	statuscode=0;
	if(KStatics::debugLevel>=2) cout << "SipCallMember start " << contacturi.nameAddr() << endl;
	ourUsername="";
	ourPassword="";
	nonceCounter=0;
	authresponse=QString::null;
	proxyauthresponse=QString::null;
	challengeCounter = 0;

}



SipCallMember::~SipCallMember( void )
{
	delete timer;
	if(KStatics::debugLevel>=2) cout <<":::::SipCallMember stop\n";

}

void SipCallMember::setContactUri( const SipUri &newcontact )
{
	contacturi = newcontact;
}

void SipCallMember::setUri( const SipUri &newuri )
{
	memberuri = newuri;
}

//+++++++++++++++++++++++++
// SUCBSCRIBE
//+++++++++++++++++++++++++

void SipCallMember::requestSubscribe( int expiresTime, const QString &body, const MimeContentType &bodytype )
{
	callMemberType = Subscribe;
	state = state_Idle;
	if( expiresTime > 0 ) {
		localExpiresTime = expiresTime;
		localsessiondesc = body;
		localsessiontype = bodytype;
	}
	sendRequestSubscribe(ourUsername, ourPassword);
}

void SipCallMember::sendRequestSubscribe( QString username, QString password )
{	
	ourUsername=username;
	ourPassword=password;
	proxyauthresponse=QString::null;
	authresponse=QString::null;
	call->transactions.clear();
	
	if( !username.isEmpty() && !password.isEmpty() &&
			( authstate == authState_AuthenticationRequired ||
			authstate == authState_AuthenticationRequiredWithNewPassword || authstate == SipCallMember::authState_AuthenticationOK ) ) {
		if( authtype == DigestAuthenticationRequired ) {
			authresponse = Sip::getDigestResponse(
				username, password, "SUBSCRIBE", getContactUri().theUri(), proxyauthstr,++nonceCounter );
		} else 	if( authtype == ProxyDigestAuthenticationRequired ) {
			proxyauthresponse = Sip::getDigestResponse(
				username, password, "SUBSCRIBE", getContactUri().theUri(), proxyauthstr,++nonceCounter );
		} else if( authtype == ProxyBasicAuthenticationRequired ) {
			proxyauthresponse = Sip::getBasicResponse( username, password );
		}
		local = call->newRequest( this, Sip::SUBSCRIBE, localsessiondesc, localsessiontype,
			SipUri::null, proxyauthresponse,authresponse, localExpiresTime );
	} else {
		local = call->newRequest( this, Sip::SUBSCRIBE, localsessiondesc, localsessiontype,
			SipUri::null, QString::null,QString::null, localExpiresTime );
	}
	if( localExpiresTime > 0 ) {
		timer->start( localExpiresTime * 900, TRUE );
	}
	if( local ) {
		op = opRequest;
		if( authstate == authState_AuthenticationRequired ||
		    authstate == authState_AuthenticationRequiredWithNewPassword  ) {
			authstate = authState_AuthenticationTryingWithPassword;
		} else {
			authstate = authState_AuthenticationTrying;
		}
		
		connect( local, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdated() ) );
	}
}

void SipCallMember::requestAuthSubscribe( void )
{
	challengeCounter++;
	statusUpdated( this );
    
}

void SipCallMember::requestClearSubscribe( bool setTimer )
{
	localExpiresTime = 0;
	call->setCallStatus( SipCall::callDisconnecting );
	sendRequestSubscribe(ourUsername, ourPassword);
	if (setTimer) {
	    call->setPresState(SipCall::PsTerminated);
	    call->notiTimerStart(500);
	}
}

void SipCallMember::handlingSubscribeResponse( void )
{
	QString authstrtemp;
	if( local->wasCancelled() ) {
		state = state_Disconnected; statuscode = local->getStatus().getCode();

		op = opIdle;
		local = 0;
	} else if( local->getStatus().getCode() >= 200 ) {  // final response
		if( local->getStatus().getCode() < 300 ) {
			if( call->getCallStatus() == SipCall::callDisconnecting ) {
				call->setCallStatus( SipCall::callUnconnected );
			} else {
				call->setCallStatus( SipCall::callInProgress );
			}
			authstate = authState_AuthenticationOK;
			challengeCounter = 0;
		} else if( local->getStatus().getCode() == 401) {
			proxyauthstr = local->getFinalWWWAuthString();
			authstrtemp = proxyauthstr.lower();
			if( authstrtemp.contains( "digest" ) ) {
				authtype = DigestAuthenticationRequired;
			} else {
				authtype = ProxyBasicAuthenticationRequired;
			}
			if( (authstate == authState_AuthenticationTryingWithPassword) && !authstrtemp.contains("stale=true")) {
				authstate = authState_AuthenticationRequiredWithNewPassword;
			} else {
				authstate = authState_AuthenticationRequired;
			}
			requestAuthSubscribe();

			return;

		} else if( local->getStatus().getCode() == 407) {
			proxyauthstr = local->getFinalProxyAuthString();
			authstrtemp = proxyauthstr.lower();
			if( authstrtemp.contains( "digest" ) ) {
				authtype = ProxyDigestAuthenticationRequired;
			} else {
				authtype = ProxyBasicAuthenticationRequired;
			}
			if( (authstate == authState_AuthenticationTryingWithPassword) && !authstrtemp.contains("stale=true")) {
				authstate = authState_AuthenticationRequiredWithNewPassword;
			} else {
				authstate = authState_AuthenticationRequired;
			}
			requestAuthSubscribe();

			return;

		}  else if( (local->getStatus().getCode() == 423 ) || (local->getStatus().getCode() == 481 )|| (local->getStatus().getCode() == 489) ) {
			challengeCounter = 0;	
			call->terminateSubscribes(local->getStatus().getCode());		
		}  else if (local->getStatus().getCode() == 404 ) {
			challengeCounter = 0;	
			call->terminateSubscribes(local->getStatus().getCode());
		} else if( local->getStatus().getCode() < 500 ) {
			challengeCounter = 0;
			if( call->getCallStatus() == SipCall::callInProgress ) {
				contactUpdate( false );
				call->updateSubscribes();
			}
		}
		challengeCounter = 0;	
		op = opIdle;
		local = 0;
		

	} else {
		challengeCounter = 0;
		state = state_EarlyDialog;
	}
}
//+++++++++++++++++++++++++
// NOTIFY
//+++++++++++++++++++++++++
void SipCallMember::requestRNotify( const QString &body, const MimeContentType &bodytype,QString username, QString password )
{
	callMemberType = RNotify;
	localExpiresTime = -1;
	localsessiondesc = body;
	localsessiontype = bodytype;
	proxyauthresponse=QString::null;
	authresponse=QString::null;
	call->transactions.clear(); 
	if( !username.isEmpty() && !password.isEmpty() &&
	    ( authstate == authState_AuthenticationRequired ||
	    authstate == authState_AuthenticationRequiredWithNewPassword ) ) {
		if( authtype == DigestAuthenticationRequired ) {
			authresponse = Sip::getDigestResponse(
				username, password, "NOTIFY", getContactUri().theUri(), proxyauthstr );
		} else if( authtype == ProxyDigestAuthenticationRequired ) {
			proxyauthresponse = Sip::getDigestResponse(
				username, password, "NOTIFY", getContactUri().theUri(), proxyauthstr );
		} else if( authtype == ProxyBasicAuthenticationRequired ) {
			proxyauthresponse = Sip::getBasicResponse( username, password );
		}
		local = call->newRequest( this, Sip::RNOTIFY, localsessiondesc, localsessiontype,
			SipUri::null, proxyauthresponse,authresponse, localExpiresTime );
	} else {
		local = call->newRequest( this, Sip::RNOTIFY, localsessiondesc, localsessiontype,
			SipUri::null, QString::null,QString::null, localExpiresTime );
	}
	if( local ) {
		op = opRequest;
		if( authstate == authState_AuthenticationRequired ||
		    authstate == authState_AuthenticationRequiredWithNewPassword ) {
			authstate = authState_AuthenticationTryingWithPassword;
		} else {
			authstate = authState_AuthenticationTrying;
		}
		connect( local, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdated() ) );
	}
}

void SipCallMember::requestNotify( const QString &body, const MimeContentType &bodytype )
{
	callMemberType = Notify;
	localExpiresTime = -1;
	localsessiondesc = body;
	localsessiontype = bodytype;
	sendRequestNotify(ourUsername, ourPassword);
}

void SipCallMember::sendRequestNotify( QString username, QString password )
{	
	ourUsername=username;
	ourPassword=password;
	proxyauthresponse=QString::null;
	authresponse=QString::null;
	call->transactions.clear(); 
	if( !username.isEmpty() && !password.isEmpty() &&
	    ( authstate == authState_AuthenticationRequired ||
	    authstate == authState_AuthenticationRequiredWithNewPassword || authstate == SipCallMember::authState_AuthenticationOK  ) ) {
		if( authtype == DigestAuthenticationRequired ) {
			authresponse = Sip::getDigestResponse(
				username, password, "NOTIFY", getContactUri().theUri(), proxyauthstr,++nonceCounter );
		} else if( authtype == ProxyDigestAuthenticationRequired ) {
			proxyauthresponse = Sip::getDigestResponse(
				username, password, "NOTIFY", getContactUri().theUri(), proxyauthstr,++nonceCounter );
		} else if( authtype == ProxyBasicAuthenticationRequired ) {
			proxyauthresponse = Sip::getBasicResponse( username, password );
		}
		local = call->newRequest( this, Sip::NOTIFY, localsessiondesc, localsessiontype,
			SipUri::null, proxyauthresponse, authresponse,localExpiresTime );
	} else {
		local = call->newRequest( this, Sip::NOTIFY, localsessiondesc, localsessiontype,
			SipUri::null, QString::null, QString::null, localExpiresTime );
	}
	if( local ) {
		op = opRequest;
		if( authstate == authState_AuthenticationRequired ||
		    authstate == authState_AuthenticationRequiredWithNewPassword ) {
			authstate = authState_AuthenticationTryingWithPassword;
		} else {
			authstate = authState_AuthenticationTrying;
		}
		connect( local, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdated() ) );
	}
}

void SipCallMember::requestAuthNotify( void )
{
	    challengeCounter++;
	    statusUpdated( this );
}

void SipCallMember::requestClearNotify( const  QString &body, const  MimeContentType &bodytype )
{
	callMemberType = Notify;
	localExpiresTime = 0;
	localsessiondesc = body;
	localsessiontype = bodytype;
	sendRequestNotify(ourUsername, ourPassword);
}

void SipCallMember::handlingNotifyResponse( void )
{
	QString authstrtemp;
	if( local->wasCancelled() ) {
		state = state_Disconnected; statuscode = local->getStatus().getCode();
		op = opIdle;
		local = 0;
	} else if( local->getStatus().getCode() >= 200 ) {  // final response
		if( local->getStatus().getCode() < 300 ) {
			call->setCallStatus( SipCall::callInProgress );
			local = 0;
			challengeCounter = 0;
		} else {
			if( local->getStatus().getCode() == 407) {
				proxyauthstr = local->getFinalProxyAuthString();
				authstrtemp = proxyauthstr.lower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = ProxyDigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
				if( (authstate == authState_AuthenticationTryingWithPassword) && !authstrtemp.contains("stale=true")) {
					authstate = authState_AuthenticationRequiredWithNewPassword;
				} else {
					authstate = authState_AuthenticationRequired;
				}
				requestAuthNotify();

				return;

			} else if( local->getStatus().getCode() == 401) {
				challengeCounter = 0;
				proxyauthstr = local->getFinalWWWAuthString();
				authstrtemp = proxyauthstr.lower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = DigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
				if( (authstate == authState_AuthenticationTryingWithPassword) && !authstrtemp.contains("stale=true")) {
					authstate = authState_AuthenticationRequiredWithNewPassword;
				} else {
					authstate = authState_AuthenticationRequired;
				}
				requestAuthNotify();

				return;

			} else if( local->getStatus().getCode() == 488) {
				challengeCounter = 0;
			} else if( local->getStatus().getCode() < 500 ) {
				challengeCounter = 0;
				state = state_Disconnected; statuscode = local->getStatus().getCode();
				call->setCallStatus( SipCall::callDead );
			}
		}
		challengeCounter = 0;
		op = opIdle;
		local = 0;
	} else {
		state = state_EarlyDialog;
		challengeCounter = 0;
	}
}

void SipCallMember::handlingRNotifyResponse( void )
{
	QString authstrtemp;
	if( local->wasCancelled() ) {
		state = state_Disconnected; statuscode = local->getStatus().getCode();
		op = opIdle;
		local = 0;
	} else if( local->getStatus().getCode() >= 200 ) {  // final response
		if( local->getStatus().getCode() < 300 ) {
		if(KStatics::debugLevel>=2)cout <<"SipCall::Member::handlingRNotifyResponse 2xx\n";
			state = state_r202;
			//back to the original type
			callMemberType=Invite;
			local = 0;
			challengeCounter = 0;
		} else {
			if( local->getStatus().getCode() == 407) {
				proxyauthstr = local->getFinalProxyAuthString();
				authstrtemp = proxyauthstr.lower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = ProxyDigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
				if( (authstate == authState_AuthenticationTryingWithPassword) && !authstrtemp.contains("stale=true")) {
					authstate = authState_AuthenticationRequiredWithNewPassword;
				} else {
					authstate = authState_AuthenticationRequired;
				}
				requestAuthNotify();

				return;

			} else if( local->getStatus().getCode() == 401) {
				proxyauthstr = local->getFinalWWWAuthString();
				authstrtemp = proxyauthstr.lower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = DigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
				if( (authstate == authState_AuthenticationTryingWithPassword) && !authstrtemp.contains("stale=true")) {
					authstate = authState_AuthenticationRequiredWithNewPassword;
				} else {
					authstate = authState_AuthenticationRequired;
				}
				requestAuthNotify();

				return;

			} else if( local->getStatus().getCode() == 488) {
			} else if( local->getStatus().getCode() < 500 ) {
				state = state_Disconnected; statuscode = local->getStatus().getCode();
				call->setCallStatus( SipCall::callDead );
			}
		}
		challengeCounter = 0;
		op = opIdle;
		local = 0;
	} else {
		challengeCounter = 0;
		state = state_r202;
		//back to the original type
		callMemberType=Invite;
		local = 0;
	}	
	
}

//+++++++++++++++++++++++++
// Message
//+++++++++++++++++++++++++
void SipCallMember::requestMessage( const QString &body, const MimeContentType &bodytype )
{
	callMemberType = Message;
	localsessiondesc = body;
	localsessiontype = bodytype;
	sendRequestMessage(ourUsername, ourPassword);
}

void SipCallMember::sendRequestMessage( QString username, QString password )
{	
	ourUsername=username;
	ourPassword=password;
	proxyauthresponse=QString::null;
	authresponse=QString::null;
	if( !username.isEmpty() && !password.isEmpty() &&
	    ( authstate == authState_AuthenticationRequired ||
	    authstate == authState_AuthenticationRequiredWithNewPassword|| authstate == SipCallMember::authState_AuthenticationOK  ) ) {
		if( authtype == DigestAuthenticationRequired ) {
			authresponse = Sip::getDigestResponse(
				username, password, "MESSAGE", getContactUri().theUri(), proxyauthstr,++nonceCounter );
		} else if( authtype == ProxyDigestAuthenticationRequired ) {
			proxyauthresponse = Sip::getDigestResponse(
				username, password, "MESSAGE", getContactUri().theUri(), proxyauthstr,++nonceCounter );
		} else if( authtype == ProxyBasicAuthenticationRequired ) {
			proxyauthresponse = Sip::getBasicResponse( username, password );
		}
		local = call->newRequest( this, Sip::MESSAGE, localsessiondesc, localsessiontype,
			SipUri::null, proxyauthresponse,authresponse, localExpiresTime );
	} else {
		local = call->newRequest( this, Sip::MESSAGE, localsessiondesc, localsessiontype,
			SipUri::null, QString::null, QString::null, localExpiresTime );
	}
	if( localExpiresTime > 0 ) {
		timer->start( localExpiresTime * 900, TRUE );
	}
	if( local ) {
		op = opRequest;
		if( authstate == authState_AuthenticationRequired ||
		    authstate == authState_AuthenticationRequiredWithNewPassword ) {
			authstate = authState_AuthenticationTryingWithPassword;
		} else {
			authstate = authState_AuthenticationTrying;
		}
		connect( local, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdated() ) );
	}
}

void SipCallMember::requestAuthMessage( void )
{
	    challengeCounter++;
    statusUpdated( this );
}

void SipCallMember::handlingMessageResponse( void )
{
	QString authstrtemp;
	if( local->wasCancelled() ) {
		state = state_Disconnected; statuscode = local->getStatus().getCode();
		op = opIdle;
		local = 0;
	} else if( local->getStatus().getCode() >= 200 ) {  // final response
		if( local->getStatus().getCode() < 300 ) {
		challengeCounter = 0;
		} else {
			if(local->getStatus().getCode() == 407){
				proxyauthstr = local->getFinalProxyAuthString();
				authstrtemp = proxyauthstr.lower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = ProxyDigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
				if( (authstate == authState_AuthenticationTryingWithPassword) && !authstrtemp.contains("stale=true")) {
					authstate = authState_AuthenticationRequiredWithNewPassword;
				} else {
					authstate = authState_AuthenticationRequired;
				}
				requestAuthMessage();

				return;
			} else if(local->getStatus().getCode() == 401){
				proxyauthstr = local->getFinalWWWAuthString();
				authstrtemp = proxyauthstr.lower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = DigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
				if( (authstate == authState_AuthenticationTryingWithPassword) && !authstrtemp.contains("stale=true")) {
					authstate = authState_AuthenticationRequiredWithNewPassword;
				} else {
					authstate = authState_AuthenticationRequired;
				}
				requestAuthMessage();

				return;

			} else {
				state = state_Disconnected; statuscode = local->getStatus().getCode();
				challengeCounter = 0;
			}
		}
		op = opIdle;
		local = 0;
	} else {
		challengeCounter = 0;
		state = state_EarlyDialog;
	}
}

//+++++++++++++++++++++++++
// Invite
//+++++++++++++++++++++++++

bool  SipCallMember::requestInvite( const QString &body, const MimeContentType &bodytype )
{
 	callMemberType = Invite;
	localsessiondesc = body;
	localsessiontype = bodytype;
	state = state_InviteRequested;
	return sendRequestInvite(ourUsername, ourPassword);
}

bool  SipCallMember::requestReInvite( const QString &body, const MimeContentType &bodytype )
{
 	callMemberType = Invite;
	localsessiondesc = body;
	localsessiontype = bodytype;
	state =  state_ReInviteRequested;
	return sendRequestInvite(ourUsername, ourPassword);
}

bool SipCallMember::sendRequestInvite( QString username, QString password )
{	ourUsername=username;
	ourPassword=password;
	if( !username.isEmpty() && !password.isEmpty() &&
	    ( authstate == authState_AuthenticationRequired || authstate == authState_AuthenticationRequiredWithNewPassword || SipCallMember::authState_AuthenticationOK ) ) {
		ourUsername=username;
		ourPassword=password;
		proxyauthresponse=QString::null;
		authresponse=QString::null;
		if( authtype == DigestAuthenticationRequired ) {
			authresponse = Sip::getDigestResponse(
				username, password, "INVITE", getContactUri().theUri(), proxyauthstr,++nonceCounter );
		} else if( authtype == ProxyDigestAuthenticationRequired ) {
			proxyauthresponse = Sip::getDigestResponse(
				username, password, "INVITE", getContactUri().theUri(), proxyauthstr,++nonceCounter );
		} else if( authtype == ProxyBasicAuthenticationRequired ) {
			proxyauthresponse = Sip::getBasicResponse( username, password );
		}
		local = call->newRequest( this, Sip::INVITE, localsessiondesc, localsessiontype,
			SipUri::null, proxyauthresponse,authresponse, localExpiresTime );
	} else {
		local = call->newRequest( this, Sip::INVITE, localsessiondesc, localsessiontype,
			SipUri::null, QString::null, QString::null,localExpiresTime );
	}
	if( localExpiresTime > 0 ) {
		timer->start( localExpiresTime * 900, TRUE );
	}
	if( local ) {
	
		op = opRequest;
		if( authstate == authState_AuthenticationRequired ||
		    authstate == authState_AuthenticationRequiredWithNewPassword ) {
			authstate = authState_AuthenticationTryingWithPassword;
		} else if( authstate == SipCallMember::authState_AuthenticationOK) {
		;
		} else {
			authstate = authState_AuthenticationTrying;
		}
		connect( local, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdated() ) );
	} else {
	    return false;
	}
return true;
}

void SipCallMember::requestAuthInvite( void )
{
	    challengeCounter++;
	    nonceCounter=0;
	statusUpdated( this );
}

void SipCallMember::handlingInviteResponse( void )
{
	QString authstrtemp;
	if( (state == state_Connected)  || (state == state_RequestingReInvite)){
		if( local->getStatus().getCode() >= 200 ) {
			if( local->getStatus().getCode() == 200 ) {
				statusdesc = "Response: " + local->getStatus().getReasonPhrase();
				challengeCounter = 0;
				statusUpdated( this );
								
			} else if( local->getStatus().getCode() == 401 ) {
				proxyauthstr = local->getFinalWWWAuthString();
				authstrtemp = proxyauthstr.lower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = DigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
				requestAuthInvite();

				return;
			
			} else if( local->getStatus().getCode() == 407 ) {
				proxyauthstr = local->getFinalProxyAuthString();
				authstrtemp = proxyauthstr.lower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = ProxyDigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
				requestAuthInvite();

				return;
			
			} else {
				challengeCounter = 0;
				state = state_Disconnected; statuscode = local->getStatus().getCode();
				statusdesc = "Response: " + local->getStatus().getReasonPhrase();
			}
			op = opIdle;
			local = 0;
		}
	} else if( state == state_Disconnecting ) {
		if( local->getStatus().getCode() >= 200 ) {
			
			if( local->getStatus().getCode() == 401 ) {
				proxyauthstr = local->getFinalWWWAuthString();
				authstrtemp = proxyauthstr.lower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = DigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
 				callMemberType = Disc;
				authstate=authState_AuthenticationRequired;
				requestAuthInvite();
				return;

			
			} else if( local->getStatus().getCode() == 407 ) {
				proxyauthstr = local->getFinalProxyAuthString();
				authstrtemp = proxyauthstr.lower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = ProxyDigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}	
				authstate=authState_AuthenticationRequired;
 				callMemberType = Disc;
				requestAuthInvite();
				return;

			} else {
				challengeCounter = 0;
				state = state_Disconnected; statuscode = local->getStatus().getCode();
				statusdesc = "Response: " + local->getStatus().getReasonPhrase();
			}
			op = opIdle;
			local = 0;
		}
	} else if( state == state_CancelPending ) {
		if( local->getStatus().getCode() == 408 ) {
			challengeCounter = 0;
			local->stopInviteRep();
		} else if( local->getStatus().getCode() == 200 ) {
			local = 0;
			challengeCounter = 0;
			requestDisconnect();
		} else if( local->getStatus().getCode() > 100 ) {
			challengeCounter = 0;
			local->cancelRequest();
			local = 0;
		}
		state = state_Disconnected; statuscode = local->getStatus().getCode();
	} else {
		if( local->wasCancelled() ) {
			challengeCounter = 0;
			state = state_Disconnected; statuscode = local->getStatus().getCode();
			op = opIdle;
			local = 0;
		} else if( local->getStatus().getCode() >= 200 ) {  // final response
			if( local->getStatus().getCode() < 300 ) {
				challengeCounter = 0;
				call->setCallStatus( SipCall::callInProgress );
				state = state_Connected;
				statusdesc = "Conn " + local->getStatus().getReasonPhrase();//KFl
			} else if( local->getStatus().getCode() < 400 ) {
				challengeCounter = 0;
				state = state_Redirected ;
				statusdesc = "Redir " + local->getStatus().getReasonPhrase();
				redirectlist = local->getFinalContactList();
			} else {
				if(local->getStatus().getCode() == 401){
					proxyauthstr = local->getFinalWWWAuthString();
					authstrtemp = proxyauthstr.lower();
					if( authstrtemp.contains( "digest" ) ) {
						authtype = DigestAuthenticationRequired;
					} else {
						authtype = ProxyBasicAuthenticationRequired;
					}
					if( authstate == authState_AuthenticationTryingWithPassword ||
					    authstate == authState_AuthenticationRequiredWithNewPassword ) {
						authstate = authState_AuthenticationRequiredWithNewPassword;
					} else {
						authstate = authState_AuthenticationRequired;
					}
					requestAuthInvite();

					return;
				} else if(local->getStatus().getCode() == 407){
					proxyauthstr = local->getFinalProxyAuthString();
					authstrtemp = proxyauthstr.lower();
					if( authstrtemp.contains( "digest" ) ) {
						authtype = ProxyDigestAuthenticationRequired;
					} else {
						authtype = ProxyBasicAuthenticationRequired;
					}
					if( authstate == authState_AuthenticationTryingWithPassword ||
					    authstate == authState_AuthenticationRequiredWithNewPassword ) {
						authstate = authState_AuthenticationRequiredWithNewPassword;
					} else {
						authstate = authState_AuthenticationRequired;
					}
					requestAuthInvite();

					return;

				} else {
					challengeCounter = 0;
					state = state_Disconnected;
					statuscode = local->getStatus().getCode();
					statusdesc = "!!Call Failed: " + local->getStatus().getReasonPhrase();
				}
			}
			op = opIdle;
			local = 0;
		} else {
			statusdesc = local->getStatus().getReasonPhrase();
			if( local->getStatus().getCode() != 100 ) {
			    challengeCounter = 0;	
			    state = state_EarlyDialog;
			} else if (challengeCounter >5) {
			    localStatusUpdated();
			}
		}
	}
}


void SipCallMember::noUpdateHere( void ) {
	if( !local ) return;
	local->sendResponse( SipStatus( 488 ) );
	state = state_Updated; 
	statuscode = 488;
	statusdesc = "Not Acceptable Here";
}


void SipCallMember::acceptUpdate( const QString &body, const MimeContentType &bodytype )
{
	if( !local ) return;
	if( body != QString::null ) {
		localsessiondesc = body;
		localsessiontype = bodytype;
	}
	local->sendResponse( SipStatus( 200 ), false, "true", body, bodytype );
	state = state_Updated;
	statusdesc = "Update 200OK";
	statusUpdated( this );
}	    


void SipCallMember::acceptInvite( const QString &body, const MimeContentType &bodytype )
{
	if( !remote ) return;
	if( body != QString::null ) {
		localsessiondesc = body;
		localsessiontype = bodytype;
	}
	if(saveinvite) {//we had an "inside" Tr, go back to INVITES's one
		remote = saveinvite;
		saveinvite = 0;
	}
	remote->sendResponse( SipStatus( 200 ), false, "true", body, bodytype );
	state = state_Connected;
	statusdesc = "Conn 200OK";
	statusUpdated( this );
	call->setCallStatus( SipCall::callInProgress );
}	    

void SipCallMember::forwardCall( const QString &body)
{
	QString bodytype = "text/plain";
	if( !remote ) return;

	if( body != QString::null ) {
		localsessiondesc = body;
		localsessiontype = bodytype;
	}
	if(saveinvite) {//we had an "inside" Tr, go back to INVITES's one
		remote = saveinvite;
		saveinvite = 0;
	}
	remote->sendResponseNC( SipStatus( 302 ), body, bodytype );
	    statusUpdated( this );


}
void SipCallMember::refuseInvite( void ) {
	if( !remote ) return;
	if(saveinvite) {//we had an "inside" Tr, go back to INVITES's one
		remote = saveinvite;
		saveinvite = 0;
	}
	remote->sendResponse( SipStatus( 200 ));
	    statusUpdated( this );
}

void SipCallMember::notAcceptableHere( void ) {
	if( !remote ) return;
	if(saveinvite) {//we had an "inside" Tr, go back to INVITES's one
		remote = saveinvite;
		saveinvite = 0;
	}
	remote->sendResponse( SipStatus( 488 ) );
	state = state_Disconnected; statuscode = 488;
	statusdesc = "Not Acceptable Here";
	statusUpdated( this );
}

void SipCallMember::declineInvite( const QString &body, const MimeContentType &bodytype ) {
	if( !remote ) return;
	if(saveinvite) {//we had an "inside" Tr, go back to INVITES's one
		remote = saveinvite;
		saveinvite = 0;
	}
	remote->sendResponse( SipStatus( 603 ), false, "true", body, bodytype );
	state = state_Disconnected;
	statusdesc = "Rejecting call invitation"; statuscode = 603;
	    statusUpdated( this );
}

//+++++++++++++++++++++++++
//PRACK
//+++++++++++++++++++++++++
void SipCallMember::requestPrack(void)
{
	callMemberType = Prack;
	sendRequestPrack(ourUsername, ourPassword);
}

void SipCallMember::sendRequestPrack( QString username, QString password )
{	
	ourUsername=username;
	ourPassword=password;
	proxyauthresponse=QString::null;
	authresponse=QString::null; 
	if( !username.isEmpty() && !password.isEmpty() &&
	    ( authstate == authState_AuthenticationRequired ||
	    authstate == authState_AuthenticationRequiredWithNewPassword || authstate == SipCallMember::authState_AuthenticationOK  ) ) {
		if( authtype == DigestAuthenticationRequired ) {
			authresponse = Sip::getDigestResponse(
				username, password, "PRACK", getContactUri().theUri(), proxyauthstr,1 );
		} else if( authtype == ProxyDigestAuthenticationRequired ) {
			proxyauthresponse = Sip::getDigestResponse(
				username, password, "PRACK", getContactUri().theUri(), proxyauthstr,1 );
		} else if( authtype == ProxyBasicAuthenticationRequired ) {
			proxyauthresponse = Sip::getBasicResponse( username, password );
		}
		saveinvite = local; //save INVITE's Tr
		local = call->newRequest( this, Sip::PRACK, QString::null,QString::null, SipUri::null, proxyauthresponse, authresponse,localExpiresTime );
	} else {
		saveinvite = local;//save INVITE's Tr
		local = call->newRequest( this, Sip::PRACK, QString::null,QString::null,
			SipUri::null, QString::null, QString::null, localExpiresTime );
	}
	if( local ) {
		op = opRequest;
		if( authstate == authState_AuthenticationRequired ||
		    authstate == authState_AuthenticationRequiredWithNewPassword ) {
			authstate = authState_AuthenticationTryingWithPassword;
		} else {
			authstate = authState_AuthenticationTrying;
		}
		connect( local, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdated() ) );
	}
}

void SipCallMember::requestAuthPrack( void ) {
	    challengeCounter++;
	    statusUpdated( this );
}

void SipCallMember::handlingPrackResponse( void ) {

	QString authstrtemp;
	if( local->wasCancelled() ) {
		state = state_Disconnected; statuscode = local->getStatus().getCode();
		op = opIdle;
		local = 0;
		saveinvite = 0;
	} else if( local->getStatus().getCode() >= 180 ) {  // final response
		if( local->getStatus().getCode() <= 200 ) {
			call->setCallStatus( SipCall::callInProgress );
			callMemberType = Invite;
			challengeCounter = 0;
		} else {
			if( local->getStatus().getCode() == 407) {
				proxyauthstr = local->getFinalProxyAuthString();
				authstrtemp = proxyauthstr.lower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = ProxyDigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
				if( authstate == authState_AuthenticationTryingWithPassword ) {
					authstate = authState_AuthenticationRequiredWithNewPassword;
				} else {
					authstate = authState_AuthenticationRequired;
				}
				requestAuthPrack();

				return;

			} else if( local->getStatus().getCode() == 401) {
				challengeCounter = 0;
				proxyauthstr = local->getFinalWWWAuthString();
				authstrtemp = proxyauthstr.lower();
				if( authstrtemp.contains( "digest" ) ) {
					authtype = DigestAuthenticationRequired;
				} else {
					authtype = ProxyBasicAuthenticationRequired;
				}
				if( authstate == authState_AuthenticationTryingWithPassword ) {
					authstate = authState_AuthenticationRequiredWithNewPassword;
				} else {
					authstate = authState_AuthenticationRequired;
				}
				requestAuthPrack();

				return;

			} else if( local->getStatus().getCode() == 488) {
				challengeCounter = 0;
			} else if( local->getStatus().getCode() < 500 ) {
				challengeCounter = 0;
				state = state_Disconnected; statuscode = local->getStatus().getCode();
				call->setCallStatus( SipCall::callDead );
			}
		}
		local = 0;
		call->transactions.clear();
		local = saveinvite;//back to INVITE's Tr
		call->transactions.append( local );
		challengeCounter = 0;
		op = opIdle;	
	} else {
		state = state_EarlyDialog;
		challengeCounter = 0;
	}
}


//+++++++++++++++++++++++++
//BYE
//+++++++++++++++++++++++++

void SipCallMember::sendRequestBye( QString username, QString password ) {

	if( !username.isEmpty() && !password.isEmpty() && ( authstate == authState_AuthenticationRequired || authstate == authState_AuthenticationRequiredWithNewPassword  || authstate == SipCallMember::authState_AuthenticationOK ) ) {
		ourUsername=username;
		ourPassword=password;
		proxyauthresponse=QString::null;
		authresponse=QString::null;
		if( authtype == DigestAuthenticationRequired ) {
			authresponse = Sip::getDigestResponse(
				username, password, "BYE", getContactUri().theUri(), proxyauthstr, ++nonceCounter);
		} else if( authtype == ProxyDigestAuthenticationRequired ) {
			proxyauthresponse = Sip::getDigestResponse(
				username, password, "BYE", getContactUri().theUri(), proxyauthstr, ++nonceCounter);
		} else if( authtype == ProxyBasicAuthenticationRequired ) {
			proxyauthresponse = Sip::getBasicResponse( username, password );
		}
		local = call->newRequest( this, Sip::BYE, QString::null,(MimeContentType) 0,
			SipUri::null, proxyauthresponse, authresponse,localExpiresTime );
	} else {
		local = call->newRequest( this, Sip::BYE,QString::null,(MimeContentType) 0,
			SipUri::null, QString::null,  QString::null,localExpiresTime );
	}
	if( localExpiresTime > 0 ) {
		timer->start( localExpiresTime * 900, TRUE );
	}
	if( local ) {
	
		op = opRequest;
		if( authstate == authState_AuthenticationRequired ||
		    authstate == authState_AuthenticationRequiredWithNewPassword ) {
			authstate = authState_AuthenticationTryingWithPassword;
		} else {
			authstate = authState_AuthenticationTrying;
		}
		connect( local, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdated() ) );
	} else {
	}
}

void SipCallMember::requestDisconnect( void )
{ 
	if ( (call->updateST == 2) && (state != state_Connected) ) {//we had an "inside" Tr, go back to INVITES's one
	    local = saveinvite;
	    call->updateST = 0;
	} else {//we had an "inside" Tr, clean the stuff
	    local = 0;
	    call->updateST = 0;
	    saveinvite = 0;
	}

	if( local ) {//hanging Request???
		if( state != state_r202 ) local->cancelRequest();
		disconnect( local, 0, this, 0 );
	}
	state = state_Disconnecting;
	statusdesc = "Disconnecting";
	sendRequestBye( ourUsername, ourPassword );
}


//+++++++++++++++++++++++++
//REFER
//+++++++++++++++++++++++++

void SipCallMember::requestRefer( bool shortTr,const SipUri &referto,
	const QString &body, const MimeContentType &bodytype )
{
	if( state == state_Disconnected ) return;
        state = state_r202;
	statusdesc = "Transfering";
	local = call->newRequest( this, Sip::REFER, body, bodytype, referto, QString::null, QString::null, localExpiresTime, shortTr );
	if( local ) {
		connect( local, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdated() ) );
	}
	statusUpdated( this );
}


//+++++++++++++++++++++++++
//OPTIONS
//+++++++++++++++++++++++++
void SipCallMember::requestOptions( const QString &body, const MimeContentType &bodytype )
{
	if( local ) {
		local->cancelRequest();
		disconnect( local, 0, this, 0 );
	}
	callMemberType = Options;
	statusdesc = "Querying options";
	localsessiondesc = body;
	localsessiontype = bodytype;
	statusdesc = "Querying options";
	sendRequestOptions(ourUsername, ourPassword);
}


void SipCallMember::sendRequestOptions( QString username, QString password )
{	
	ourUsername=username;
	ourPassword=password;
	if( !username.isEmpty() && !password.isEmpty() &&
			( authstate == authState_AuthenticationRequired ||
			authstate == authState_AuthenticationRequiredWithNewPassword || authstate == SipCallMember::authState_AuthenticationOK ) ) {
		if( authtype == ProxyDigestAuthenticationRequired ) {
			proxyauthresponse = Sip::getDigestResponse(
				username, password, "OPTIONS", getContactUri().theUri(), proxyauthstr );
		} else if( authtype == ProxyBasicAuthenticationRequired ) {
			proxyauthresponse = Sip::getBasicResponse( username, password );
		}
		local = call->newRequest( this, Sip::OPTIONS, localsessiondesc, localsessiontype,
			SipUri::null, proxyauthresponse,authresponse, localExpiresTime );
	} else {
		local = call->newRequest( this, Sip::OPTIONS, localsessiondesc, localsessiontype,
			SipUri::null, QString::null,QString::null);
	}
	if( local ) {
		op = opRequest;
		if( authstate == authState_AuthenticationRequired ||
		    authstate == authState_AuthenticationRequiredWithNewPassword  ) {
			authstate = authState_AuthenticationTryingWithPassword;
		} else {
			authstate = authState_AuthenticationTrying;
		}
		
		connect( local, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdated() ) );
	}
}

void SipCallMember::requestAuthOptions( void )
{
	    challengeCounter++;
    statusUpdated( this );
}


void SipCallMember::handlingOptionsResponse( void ) {
	QString authstrtemp;
	if( local->wasCancelled() ) {
		challengeCounter = 0;	

		state = state_Disconnected; statuscode = local->getStatus().getCode();
		op = opIdle;
		local = 0;
	} else if( local->getStatus().getCode() >= 200 ) {  // final response
		if( local->getStatus().getCode() < 300 ) {
			challengeCounter = 0;
			if( call->getCallStatus() == SipCall::callDisconnecting ) {
				call->setCallStatus( SipCall::callUnconnected );
			} else {
				call->setCallStatus( SipCall::callInProgress );
			}
			authstate = authState_AuthenticationOK;
		} else if( local->getStatus().getCode() == 401) {
			proxyauthstr = local->getFinalWWWAuthString();
			authstrtemp = proxyauthstr.lower();
			if( authstrtemp.contains( "digest" ) ) {
				authtype = DigestAuthenticationRequired;
			} else {
				authtype = ProxyBasicAuthenticationRequired;
			}
			if( authstate == authState_AuthenticationTryingWithPassword ) {
				authstate = authState_AuthenticationRequiredWithNewPassword;
			} else {
				authstate = authState_AuthenticationRequired;
			}
			statuscode = local->getStatus().getCode();
			requestAuthOptions();

			return;

		} else if( local->getStatus().getCode() == 407) {
			proxyauthstr = local->getFinalProxyAuthString();
			authstrtemp = proxyauthstr.lower();
			if( authstrtemp.contains( "digest" ) ) {
				authtype = ProxyDigestAuthenticationRequired;
			} else {
				authtype = ProxyBasicAuthenticationRequired;
			}
			if( authstate == authState_AuthenticationTryingWithPassword ) {
				authstate = authState_AuthenticationRequiredWithNewPassword;
			} else {
				authstate = authState_AuthenticationRequired;
			}
			statuscode = local->getStatus().getCode();
			requestAuthOptions();
			return;

		} else {
			challengeCounter = 0;
			op = opIdle;
			local = 0;
		}
	}
}

//****************
//SE coupling
//****************

void SipCallMember::localStatusUpdated( void )
{
	QString authstrtemp;
	MimeContentType mtype;
	SipStatus sipstatus;
	if(challengeCounter > 5) {
		call->stopAutoRegistration();
		cout << challengeCounter << endl;
		cout << " +----------------------------------------------------------------+\n";
		cout << " |    to many unsuccessfull authentication requests!!!            |\n";
		cout << " |    retry with new authentication username and password, please |\n";
		cout << " +----------------------------------------------------------------+\n";
		exit(-1);
	}

	if (call->updateST == 2) {
	    local = saveinvite;
	    call->updateST = 0;
	}
	if( !local ) {
		return;
	}
	mtype = local->getFinalContentType();

	if( mtype == MimeContentType( "application/sdp" ) ) {
		sessiondesc = local->getFinalMessageBody();
		localsessiontype = mtype;
	} else {
		recentbody = local->getFinalMessageBody();
		recentbodytype = mtype;
	}

	if( (callMemberType == Invite ) || (callMemberType == Disc) )  {
		if(state != state_r202) {
		    handlingInviteResponse();
		}	
	} else if( callMemberType == Message ) {
		handlingMessageResponse();
	} else if( callMemberType == Subscribe ) {
		handlingSubscribeResponse();
	} else if( callMemberType == Notify ) {
		handlingNotifyResponse(); 
	} else if( callMemberType == RNotify ) {
		handlingRNotifyResponse();
	} else if( callMemberType == Prack ) {
		handlingPrackResponse();
	} else if( callMemberType == Options ) {
		handlingOptionsResponse();
	} else if( callMemberType == Update ) {	
	} else {
		return;
	}
	    
	    statusUpdated( this );
}

void SipCallMember::UpdateStatusUpdated( void )
{
	QString authstrtemp;
	MimeContentType mtype;
	SipStatus sipstatus;

	if( !local ) {
		return;
	}
	mtype = local->getUpdateContentType();
	if( mtype == MimeContentType( "application/sdp" ) ) {
		sessiondesc = local->getUpdateMessageBody();
		localsessiontype = mtype;
	} 
	disconnect(local,0,this,0);
	if(call->updateST==2) local = 0;
	savestate = state;
	state = state_Update;
	statusUpdated( this );
}

//**********
//TR handling
//***********
void SipCallMember::incomingTransaction( SipTransaction *newtrans )
{
	MimeContentType mtype;
	remote = newtrans;
	
	if( remote->getRequest()->getMethod() == Sip::INVITE ) {
	    	connect( remote, SIGNAL( statusUpdated() ), this, SLOT( remoteStatusUpdated() ) );
		if( state == state_Disconnected ) {
			state = state_RequestingInvite;
			statusdesc = "Invitation received";
			if(isPPrack()) {
				//make ready to send Req/Supp header
				remote->sendResponse( SipStatus( 180 ), true );
			} else {
				remote->sendResponse( SipStatus( 180 ) );
			}			 
		} else {
			state = state_RequestingReInvite;
			statusdesc = "Session update requested";
		}
		mtype = remote->getRequestMessageContentType();
		if( mtype == MimeContentType( "application/sdp" ) ) {
			sessiontype = mtype;
			sessiondesc = remote->getRequestMessageBody();
		} else {
			recentbodytype = mtype;
			recentbody = remote->getRequestMessageBody();
		}
	} else if( remote->getRequest()->getMethod() == Sip::BYE ) {
		state = state_Disconnected;
		statusdesc = "Remote end disconnected";statuscode = 487;
		call->hideCallWidget();
	} else if( remote->getRequest()->getMethod() == Sip::UPDATE ) {
		
		mtype = remote->getRequestMessageContentType();
		if( mtype == MimeContentType( "application/sdp" ) ) {
			sessiontype = mtype;
			sessiondesc = remote->getRequestMessageBody();
		} else {
			recentbodytype = mtype;
			recentbody = remote->getRequestMessageBody();
		}
	} else if( remote->getRequest()->getMethod() == Sip::REFER ) {
		if( state == state_Refer ) {
			return;
		}
		QString rt = remote->getRequest()->getRefSub();
		if (isShortTrans()) { // short transfer allowed
			remote->sendResponse( SipStatus( 202 ),false, rt ); //yes, we reflect what we received
		}else {
			remote->sendResponse( SipStatus( 202 ) ,false, "True" ); //no, always with impl. subscr.
		}
		state = state_Refer;
		statusdesc = "Refer";
		redirectlist = SipUriList( remote->getRequest()->getHeaderData( SipHeader::Refer_To ) );
	}
	    statusUpdated( this );
}

void SipCallMember::remoteStatusUpdated( void )
{
	if( state == state_RequestingInvite ) {
		if( remote->wasCancelled() ) {
			state = state_Disconnected;
			statusdesc = "Request cancelled"; statuscode = 487;
			statusUpdated( this );
		}
	}
}

void SipCallMember::cancelTransaction( void )
{	    
	if (call->updateST == 2) { //back to INVITE's Tr after Update
	    local = saveinvite;
	    call->updateST = 0;
	}
	if( local ) {
		if( state == state_EarlyDialog ) {
			local->cancelRequest();
			state = state_Disconnected;
		} else {
		    local->stopInviteRep();
		    state = state_CancelPending;
		}
	}
}

QString SipCallMember::getSubject( void )
{
	return call->getSubject();
}

void SipCallMember::call_timeout()
{
	if( (call->getCallType() == SipCall::outSubscribeCall) || (call->getCallType() == SipCall::winfoSubscribeCall) ) {
	    requestSubscribe();
	} else if( call->getCallType() == SipCall::inSubscribeCall ) {
		contactUpdate ( false );
	};
}


void SipCallMember::contactUpdate( bool active, QString presence )
{
	call->setPresenceStatus( presence );
	if( active ) {
		call->setCallStatus( SipCall::callInProgress );
	} else {
		if( call->getCallStatus() != SipCall::callDead ) {
			call->setCallStatus( SipCall::callUnconnected );
		}
		memberuri.setTag( QString::null );
	}
statusUpdated( this );
}

void SipCallMember::timerStart( int time )
{
	timer->start( time );
}


bool SipCallMember::isShortTrans(void)  {

	return !call->getIsb();
}

bool SipCallMember::isPPrack(void)  {

 	QString pr = "";	
	pr = remote->getRequest()->getHeaderData( SipHeader::Require);
	if(pr.contains("100rel")) return true;
	
	pr = remote->getRequest()->getHeaderData( SipHeader::Supported);
	if( pr.contains("100rel") ) {
	    if ( call->getPPrack() == 0 ) {
		return false;//we shall use implicit subscription
	    } else {
		return true;
	    }
	} else return false;
}

//**************************************************************************
//           SipCall
//**************************************************************************

SipCall::SipCall( SipUser *local, const QString &id, SipCall::CallType ctype )
{
	callstatus = callUnconnected;
	useIPv6=KStatics::haveIPv6;
	if ( id == QString::null ) {
		callid = SipMessage::createCallId();
	} else {
		callid = id;
	}
 	havePubServer = false;
	// Remember to nuke all members and transactions
	members.setAutoDelete( true );
	transactions.setAutoDelete( true );
	myWatchers.setAutoDelete(true);
	/** user by SipCall */
	struct timeval tv;
	gettimeofday( &tv, NULL );
	srand( tv.tv_usec );
	lastseq = rand() % 8000;
	parent = local->parent();
	sessionC = local->getSC(); //value is 0 for RegisterCalls!	
	calltype = ctype;
	hasroute = false;
	initRequest = true;
	localuri = local->getUri();
	localuri.generateTag();
	parent->addCall( this );
	basePresenceStatus = false;
	presenceStatus = "offline";
	bodyMask = QString::null;
	contactstr = "";
	refnoti = QString::null;
	hasrecordroute = false;
	Inumber="0";
	onHold = preCall;
	noHold=false;
	masterHold=false;
	activeOnly=false;
	useCmdLine=false;
	RemoteStart=false;
	switchthru=false;
	rack = "";
	prack = SPSno;
	pstate = PsIdle;
	sCSeq = "";
	isAck = false;
	send2uri = false;
	notitimer = new QTimer( this );
	connect( notitimer, SIGNAL( timeout() ), this, SLOT( noti_timeout() ) );

	if(KStatics::debugLevel>=2) cout << ":::::SipCall start ct= " << calltype << endl;
	
}

SipCall::~SipCall( void )
{
	
	if(KStatics::debugLevel>=2)cout <<"SipCall stop ct= " << calltype << endl;
	
	
	parent->deleteCall( this );
	
	transactions.clear();

	members.clear();

	myWatchers.clear();

	delete notitimer;

}

void SipCall::processWatchers(QString uri, QString event, QString state,int full)
{
	WatcherInfo *wa;
	bool found= false;
	if(full == 0){
		for( wa = myWatchers.first(); wa != 0; wa = myWatchers.next() ) {
			if(wa->uri == uri) {
			wa->event = event;
			wa->state = state;
			found = true;
			break;
			} 
		}
		if (!found) {
			wa = new WatcherInfo(uri,event,state);
			myWatchers.append(wa);
		}

	} else if (full==1){
		myWatchers.clear();
	} else {
		wa = new WatcherInfo(uri,event,state);
		myWatchers.append(wa);
	}
}	
void SipCall::showWatchers(void) {
	WatcherInfo *wa;
	if(KStatics::debugLevel>=2)cout <<"SipCall::showWatchers: our watchers\n";
	for( wa = myWatchers.first(); wa != 0; wa = myWatchers.next() ) {
		if(KStatics::debugLevel>=2) cout << " uri=" << (wa->uri).latin1() << " event=" << (wa->event).latin1() << " state=" << (wa->state).latin1() << endl;
	} 
}

void SipCall::addMember( SipCallMember *newmember )
{
	members.append( newmember );
}

SipTransaction *SipCall::newRequest( SipCallMember *member, Sip::Method meth,
	const QString &body, const MimeContentType &bodytype, const SipUri &referto,
	const QString &proxyauthentication, const QString &authentication,int expiresTime, bool shortTr )
{
	SipTransaction *trans = new SipTransaction( lastseq++, member, this );
	if( trans->sendRequest( meth, body, bodytype, referto, shortTr, proxyauthentication, authentication, expiresTime ) ) {
		if(meth == Sip::PRACK) {
			transactions.take(); //take out of queue, do NOT delete
		} else {
			transactions.clear();
		}
		transactions.append( trans );
		if ( meth == Sip::INVITE ) {
		    member->saveinvite = trans;
		}
		// Audit the call
		auditCall();

		// Return the transaction object for tracking
		return trans;
	} else {
		delete trans;
		return 0;
	}
}

SipTransaction *SipCall::newRegister( const SipUri &registerserver, SipCallMember *callMember, int expiresTime, const QString &authentication, const QString &proxyauthentication, const QString &qvalue, const QString &body, const MimeContentType &bodytype )
{
	transactions.clear();
	localuri.setTag( QString::null );
	
	callMember->setUri(localuri);
	SipTransaction *trans = new SipTransaction( lastseq++, callMember, this );
	
	transactions.append( trans );
	trans->sendRegister( registerserver, expiresTime, authentication, proxyauthentication, body, bodytype, qvalue );
	return trans;
}

//Publish
SipTransaction *SipCall::newPublish(SipCallMember *callMember,const QString state, const QString sipIfMatch, const SipUri &publishserver, int expiresTime, const QString &authentication, const QString &proxyauthentication, const QString &body) { 

	transactions.clear();
	localuri.setTag( QString::null );	
	callMember->setUri(localuri);
	SipTransaction *trans = new SipTransaction( lastseq++, callMember, this );
	transactions.append( trans );
	trans->sendPublish(state,
		sipIfMatch,
		publishserver,
		expiresTime,
		authentication,
		proxyauthentication,
		body
		);
	 return trans;
}	



bool SipCall::sendRequest( SipMessage *reqmsg, bool contact, const SipUri &proxy, const QString &branch )
{

	if( (reqmsg->getMethod() == Sip::REGISTER) || (reqmsg->getMethod() == Sip::PUBLISH) ) {
		if( !localuri.hasTag() ) {
			localuri.generateTag();
		}
	}

	reqmsg->insertHeader( SipHeader::From, localuri.nameAddr() );
	reqmsg->insertHeader( SipHeader::Call_ID, callid );
	if( ( reqmsg->getMethod() == Sip::INVITE ) || ( reqmsg->getMethod() == Sip::MSG ) ) {
		reqmsg->insertHeader( SipHeader::Subject, getSubject() );
	}
	if (initRequest) {
	    initRequest = false;

	    if (KStatics::serviceRoute != "") {
		reqmsg->insertHeader( SipHeader::Route, KStatics::serviceRoute);
	    }
	}
	if( hasroute ) {
		if( route.getHead().uri().contains( ";lr" ) ) {
			reqmsg->insertHeader( SipHeader::Route, route.getUriList() );
		} else {
			reqmsg->setRequestUri( route.getHead() );
			SipUriList routewithouthead = route;
			routewithouthead.removeHead();
			reqmsg->insertHeader( SipHeader::Route, routewithouthead.getUriList() );
		}
	}
	if( parent->getExplicitProxyMode() ) {

		if( reqmsg->getMethod() == Sip::REGISTER || reqmsg->getMethod() == Sip::INVITE ||reqmsg->getMethod() == Sip::PUBLISH ||
				reqmsg->getMethod() == Sip::SUBSCRIBE || reqmsg->getMethod() == Sip::ACK ) {
			if( parent->isLooseRoute() ) {
				if( reqmsg->getMethod() == Sip::REGISTER ) {
					reqmsg->setRequestUri(
						SipUri( reqmsg->getHeaderData( SipHeader::To ) ).getRegisterUri() );
				}
				reqmsg->insertHeader( SipHeader::Route, parent->getExplicitProxyAddress() );
			} else if( parent->isStrictRoute() ) {
				if( reqmsg->getMethod() == Sip::REGISTER ) {
					reqmsg->insertHeader(
						SipHeader::Route, SipUri(
						reqmsg->getHeaderData( SipHeader::To ) ).getRouteUri() );
				} else {
					reqmsg->insertHeader(
						SipHeader::Route, "<" + reqmsg->getRequestUri().reqUri() + ">" );
				}
				reqmsg->setRequestUri( parent->getExplicitProxyAddress() );
			} else {
				if( reqmsg->getMethod() == Sip::REGISTER ) {
					reqmsg->setRequestUri(
						SipUri( reqmsg->getHeaderData( SipHeader::To ) ).getRegisterUri() );
				}
			}
		}
	}
	return parent->sendRequest( send2uri, (int)calltype, reqmsg, contact, proxy, branch );
}

void SipCall::sendResponse( SipMessage *responsemsg, bool contact )
{
	responsemsg->insertHeader( SipHeader::Call_ID, callid );
	responsemsg->insertHeader( SipHeader::To, localuri.nameAddr() );
	if( hasrecordroute ) {
		responsemsg->setRecordRoute( recordroute );
	}
	parent->sendResponse( responsemsg, contact );
	
	if (updateST == 1) {
	    SipTransaction *trans = transactions.take(); //take out of queue, do NOT delete
	    delete trans;
	    updateST = 2;	
	}
}


void SipCall::sendRaw( SipMessage *msg )
{

	parent->sendRaw( msg );
}

SipCallMember *SipCall::incomingMessage( SipMessage *message )
{
	if( message->getType() == SipMessage::Request ) {
		return incomingRequest( message );
	} else if( message->getType() == SipMessage::Response ) {
		incomingResponse( message );
	} else {
		delete message;
	}
	return 0;
}


void SipCall::incomingResponse( SipMessage *message )
{
	int mcode = message->getStatus().getCode();
	if((mcode >100) && (mcode < 200))   setRAck(message);
	
	SipUri incominguri( message->getHeaderData( SipHeader::To ) );
	setRefSub(message->getRefSub()); //store the refer-sub header
	QString cseq = message->getHeaderData( SipHeader::CSeq );
	unsigned int seqnum = cseq.left( cseq.find( " " ) ).toUInt(); 
	SipTransaction *curtrans;

	for ( curtrans = transactions.first(); curtrans != 0; curtrans = transactions.next() ) {
		if (( incominguri == curtrans->getCallMember()->getUri()) &&
		    ( seqnum == curtrans->getSeqNum() ) &&
		    ( curtrans->getDirection() == SipTransaction::LocalRequest ) ) {
			SipCallMember *member = getMember( incominguri );
			if( member == NULL ) {
			if(KStatics::debugLevel>=2) cout << "===SipCall::incomingRequest: member null - do nothing\n";
			} else if( message || message->getStatus().getCode() == 202 ) {
				// Update the Contact for this member
				if( message->getContactList().getListLength() > 0 ) {
				    member->setContactUri( message->getContactList().getHead() );
				}
				// Update the route
				if( message->getRecordRoute().getListLength() > 0 ) {
					hasroute = true;
					route = message->getRecordRoute();
					route.reverseList();
					if( !route.getHead().uri().contains( ";lr" ) ) {
						route.addToEnd( member->getContactUri() );
					}
				}
			}
			if( (message->getStatus().getCode() == 200 ) && (calltype == RegisterCall) ) {
			    QString sr = message->getHeaderData( SipHeader::Service_Route);
			    if (sr != "") {
				KStatics::serviceRoute = sr;
			    } else {
				KStatics::serviceRoute = "";
			    }
			}

			curtrans->incomingResponse( message );
			return;
		}
	}
	delete message;
}


SipCallMember *SipCall::incomingRequest( SipMessage *message )
{
	SipUri incominguri( message->getHeaderData( SipHeader::From ) );
	QString cseq = message->getHeaderData( SipHeader::CSeq );
	unsigned int seqnum = cseq.left( cseq.find( " " ) ).toUInt();
	QString seqmethod = cseq.right( cseq.find( " " ) + 1 );
	SipTransaction *curtrans;
	// Update our identity if necessary
	SipUri touri( message->getHeaderData( SipHeader::To ) );
	if( touri != localuri ) {
		localuri = touri;
		if( !localuri.hasTag() ) {
			localuri.generateTag();
		}
	}
	if(  message->getMethod() == Sip::REGISTER ) {
		QString sr = message->getHeaderData( SipHeader::Service_Route);
		if (sr != "") {
			KStatics::serviceRoute = sr;
		} else {
			KStatics::serviceRoute = "";
		}
	}

	if( ( message->getMethod() == Sip::ACK ) || ( message->getMethod() == Sip::CANCEL ) ) {
		for( curtrans = transactions.first(); curtrans != 0; curtrans = transactions.next() ) {
			if( ( incominguri == curtrans->getCallMember()->getUri() ) &&
			    ( seqnum == curtrans->getSeqNum() ) &&
			    ( curtrans->getDirection() == SipTransaction::RemoteRequest ) ) {
//Request found
				curtrans->incomingRequest( message, true );
				return 0;
			}
		}
		delete message;
		return 0;
	} 
	
	for( curtrans = transactions.first(); curtrans != 0; curtrans = transactions.next() ) {
		if( ( incominguri == curtrans->getCallMember()->getUri() ) && ( seqnum == curtrans->getSeqNum() ) && 
		    ( seqmethod == curtrans->getSeqMethod() ) &&
		    ( curtrans->getDirection() == SipTransaction::RemoteRequest ) ) {
			curtrans->incomingRequestRetransmission( message );
			return 0;
		}
	}
// We catch the NOTIFY refer contents
	if (  (message->getMethod() == Sip::NOTIFY) && loadCall() ) {
	    if (message->messageBody().contains("SIP/2.0") ) {
		    setRefNoti(message->messageBody());
	    }
	}
	// Find or create a member for this request

	SipCallMember *member = getMember( incominguri );
	if ( member == NULL ) {
		if(KStatics::debugLevel>=2) cout << "===SipCall::incomingRequest: member null\n";
		member = new SipCallMember( this, incominguri );
		if ( message->getMethod() == Sip::INVITE ) {
			member->setCallMemberType( SipCallMember::Invite );
		} else if ( message->getMethod() == Sip::SUBSCRIBE ) {
			member->setCallMemberType( SipCallMember::Subscribe );
		} else if ( message->getMethod() == Sip::NOTIFY ) {
			member->setCallMemberType( SipCallMember::Notify );
		} else if ( message->getMethod() == Sip::MESSAGE ) {
			member->setCallMemberType( SipCallMember::Message );
		} 

	}

	// Update the Contact for this member
	if( message->getContactList().getListLength() > 0 ) {
		member->setContactUri( message->getContactList().getHead() );
	}

	// Update the route
	if( message->getRecordRoute().getListLength() > 0 ) {
		hasrecordroute = true;
		recordroute = message->getRecordRoute();
		hasroute = true;
		route = recordroute;
		if( !route.getHead().uri().contains( ";lr" ) ) {
			route.addToEnd( member->getContactUri() );
		}
	}

	SipTransaction *transaction = new SipTransaction( seqnum, member, this );
	transactions.append( transaction );
	
	if ( message->getMethod() == Sip::INVITE ) {
		member->saveinvite = transaction;

	} else if ( message->getMethod() == Sip::UPDATE  )  {
		
			updateST = 1;
			member->local = transaction;
			member->savestate = member->state;		
			if ( member->state == SipCallMember::state_Connected){	
			    member->state = SipCallMember::state_Update;

			} else {
				connect( transaction, SIGNAL( statusUpdated() ), member, SLOT(UpdateStatusUpdated() ) );
				transaction->incomingRequest( message,this->getHoldMaster() );
				return member;
			}
	}
	updateST = 0;	
	
	/*
	//Cave Canem, will not work
	if(transaction->wasCancelled()) {
	    cout << "---------ir1 duud\n";
	} else {
	    cout << "---------ir1 nit duud\n";
	    member->setState(SipCallMember::state_Idle); 
	}
	*/
	transaction->incomingRequest( message,this->getHoldMaster() );

	// Update member status based on this transaction
	member->incomingTransaction( transaction );
	return member;
}

void SipCall::setSubject( const QString& newsubject )
{
    
	subject = newsubject;
	subjectChanged();
}

SipUri SipCall::getContactUri( void )
{
	return parent->getContactUri();
}

SipCallMember *SipCall::getMember( const SipUri &uri )
{
	SipCallMember *m;
	for( m = members.first(); m != 0; m = members.next() ) {
		if ( uri == m->getUri() ) {
			return m;
		}
	}

	return NULL;
}

void SipCall::setCallType( CallType newtype )
{
	calltype = newtype;
	parent->callTypeUpdated();
}


void SipCall::auditCall( void )
{
	bool foundmemb = false;
	// If there are no active call members, set the call status to 'Dead'
	for( SipCallMember *memb = members.first(); memb != 0; memb = members.next() ) {
		if( ( memb->getState() != SipCallMember::state_Disconnecting ) &&
		    ( memb->getState() != SipCallMember::state_Disconnected ) ) {
			foundmemb = true;
		}
	}
	if( !foundmemb ) {
		callstatus = callDead;
		callStatusUpdated();
	}
}

QString SipCall::getProxyUsername( void )
{
	SipUser *u = parent->getUser( localuri );
	if( u != NULL ) {
		SipUri *uri = u->getMyUri();
		if( uri->hasProxyUsername() ) {
			return uri->getProxyUsername();
		} else {
			return QString::null;
		}
	} else {
		return QString::null;
	}
}


QString SipCall::getHostname( void )
{
	SipUser *u = parent->getUser( localuri );
	if( u != NULL ) {
		SipUri *uri = u->getMyUri();
		return uri->getHostname();
	} else {
		return QString::null;
	}
}

QString SipCall::getSipProxy( void )
{
	SipUser *u = parent->getUser( localuri );
	if( u != NULL ) {
		SipClient *c = u->parent();
		QString proxy = c->getSipProxy();
		if( !proxy.isEmpty() ) {
			return proxy;
		} else {
			return c->getSipProxySrv( u->getMyUri()->getHostname() );
		}
	} 
	return QString::null;
}

void SipCall::setProxyUsername( QString newUsername )
{
	SipUser *u = parent->getUser( localuri );
	if( u != NULL ) {
		SipUri *uri = u->getMyUri();
		uri->setProxyUsername( newUsername );
	}
}

QString SipCall::getPassword( void )
{
	SipUser *u = parent->getUser( localuri );
	if( u != NULL ) {
		SipUri *uri = u->getMyUri();
		if( uri->hasPassword() ) {
			return uri->getPassword();
		} else {
			return QString::null;
		}
	} else {
		return QString::null;
	}
}

void SipCall::setPassword( QString newPassword )
{
	SipUser *u = parent->getUser( localuri );
	if( u != NULL ) {
		SipUri *uri = u->getMyUri();
		uri->setPassword( newPassword );
	}
}

void SipCall::hideCallWidget( void )
{
	parent->hideCallWidget( this );
}

void SipCall::updateSubscribes( void )
{
	parent->updateSubscribes();
}

void SipCall::terminateSubscribes( int mess )
{
	parent->terminateSubscribes(mess, this);
}

void SipCall::setPresenceStatus(QString status) {
    presenceStatus=status;
}


QString SipCall::getTrPresenceStatus( void ) {

 QString state;
 if( presenceStatus == "online" ) {
		state = "Online";
	} else if( presenceStatus == "busy" ) {
		state = "Busy";
	} else if( presenceStatus == "berightback" ) {
		state = "Be Right Back";
	} else if( presenceStatus == "away" ) {
		state = "Away";
 	} else if( presenceStatus == "permanent-absent") {
		state = "Permanent absent";
	} else if( presenceStatus == "appointment") {
		state ="Appointment";
	} else if( presenceStatus == "breakfast") {
		state ="At Breakfast";
	} else if( presenceStatus == "dinner") {
		state = "At Dinner" ;
	} else if( presenceStatus == "holiday") {
		state =  "On Holidays";
	} else if( presenceStatus == "lunch") {
		state ="At Lunch";
	} else if( presenceStatus == "meal") {
		state ="Having a Meal";
	} else if( presenceStatus == "meeting") {
		state ="In Meeting";
	} else if( presenceStatus == "travel") {
		state ="Travelling";
	} else if( presenceStatus =="in-transit") {
		state = "In Transit";
	} else if( presenceStatus =="steering") {
		state = "Steering";
	} else if( presenceStatus =="shopping") {
		state = "Shopping";
	} else if( presenceStatus =="sleeping") {
		state = "Sleeping";
	} else if( presenceStatus =="looking-for-work") {
		state = "Looking For Work";	
	} else if( presenceStatus =="tv") {
		state = "Looking TV";
	} else if( presenceStatus == "vacation") {
		state ="On Vacation";
	} else if( presenceStatus == "on-the-phone") {
		state = "On The Phone" ;
	} else if( presenceStatus == "offline" ) {
		state = "Offline";
	} else if( presenceStatus == "performance" ) {
		state = "At/In Performance";
	} else if( presenceStatus == "playing" ) {
		state = "Playing";
	} else if( presenceStatus == "presentation" ) {
		state = "Giving Presentation";
	} else if( presenceStatus == "spectator" ) {
		state = "Spectator";	
	} else if( presenceStatus == "unknown" ) {
		state = "Unknown";
	} else if( presenceStatus == "working" ) {
		state = "Working";
	} else if( presenceStatus == "worship" ) {
		state = "At Worship";
	} else if( presenceStatus == "" ) {
		state = "Offline";
	}  else {
		state = presenceStatus;
	}
  return state;
}


//transient data handling methods

void SipCall::setInvoker( QString toexec ) {

	Inumber = toexec;
}

QString SipCall::getInvoker( void ) {
	return Inumber;
}

void SipCall::setNoHold(bool nohold)
{
    noHold=nohold;
}


void SipCall::setHoldMaster(bool hmaster)
{
    masterHold=hmaster;
}


void SipCall::setHoldCall(HoldState hstate)
{
    onHold=hstate;
}

bool SipCall::getHoldMaster(void)
{
    return masterHold;
}


void SipCall::onlyMeActive(bool me)
{
    activeOnly = me;
}

//used for CT in the new call control concept
void SipCall::setCallTrType( int newtype )
{

  switch (newtype)
    {

    case (int)StandardCall:
      calltype = StandardCall;
      break;
    case (int)videoCall:
      calltype = videoCall;
      break;
    case (int)auviCall:
      calltype = auviCall;
      break;
    case (int)extCall1:
      calltype = extCall1;
      break;
    case (int)extCall2:
      calltype = extCall2;
      break;
    case (int)extCall3:
      calltype = extCall3;
      break;
    case (int)extCall4:
      calltype = extCall4;
      break;
    default:
      calltype = GarKeinCall;
    break;
    }
    parent->callTypeUpdated();
}



int  SipCall::saveLoadType (int load) {
	if((StandardCall <= load) && (load < putMeOnHold) ) {
	    calltype = (CallType)load;
	}
	return (int) calltype; 
	
}





/****************************************************************
* call/load type handling routines, they must be adapted,
* if new types are added
*****************************************************************/

bool SipCall::loadCall(void) {
	bool scB=((calltype>=StandardCall)&&(calltype<=auviCall));
	bool pvB=(calltype >= extCall1);
	return (scB||pvB);
}

bool SipCall::blockCall(void) {
	bool scB=((calltype>=StandardCall)&&(calltype<=auviCall));
	return (scB||activeOnly );
}


bool SipCall::withAudioCall(void) {
	bool scB=((calltype==StandardCall)||(calltype==auviCall));
	return scB;
}

bool SipCall::withExtCall(void) {
	bool scB=((calltype>=videoCall)&&(calltype<=auviCall));
	bool pvB=(calltype >= extCall1);
	return (scB||pvB );
}

bool SipCall::isExtCall(void) {
return (calltype >= extCall1);
}

WatcherInfo::WatcherInfo(const  QString u,const  QString e, const QString s){
	uri = u;
	event = e;
	state = s;
}
WatcherInfo::~WatcherInfo(void){
	uri = "";
	event = "";
	state = "";
}

void SipCall::setRAck(SipMessage *message)  {
//supported, required or none
	QString pr = "";
	pr = message->getHeaderData( SipHeader::Require);
	if (pr=="") {
	    	pr = message->getHeaderData( SipHeader::Supported);
		if (pr == "") {
		} else {
		    prack = SPSsup;
		}
	} else {
	    prack = SPSreq;
	}	
	if (prack == SPSsup || prack == SPSreq) {
	//save the cseq and prepare RAck (for >PUBLISH)
		sCSeq =	message->getHeaderData( SipHeader::CSeq);
		rack  = message->getHeaderData( SipHeader::RSeq).section(" ",0,0) + " " + sCSeq.section(" ",0,0) + " INVITE"; 
	}
}

void SipCall::noti_timeout() {
	parent->updateSubscribes();

}

void SipCall::notiTimerStart( int time ) {
	notitimer->start( time,true );
}

void SipCall::setSendToURI( bool st ) {
	send2uri = st;
}

int SipCall::getPrack(void) {

	if (sessionC) return sessionC->getPrack(); else return 0;

}

int SipCall::getPPrack(void){

	if (sessionC) return sessionC->getPPrack(); else return 0;

}
bool SipCall::getIsb(void){

	if (sessionC) return sessionC->getIsb(); else return 0;

}
	
void SipCall::stopAutoRegistration(void) {

	if (sessionC) sessionC->stopAutoRegistration();
}
