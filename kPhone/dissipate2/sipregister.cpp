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
#include <stdio.h>
#include "../kphone/kstatics.h"
#include "sipclient.h"
#include "sipcall.h"
#include "sipstatus.h"
#include "siptransaction.h"
#include "sipregister.h"
#include "sipuser.h"
#include "siputil.h"
#include <iostream>
using namespace std;

SipRegister::SipRegister( SipUser *user, const SipUri &serveruri, int expires, QString qvalue )
{
	if (KStatics::debugLevel >=2) cout << ":::::SipRegister start  expires" << expires << " minValidTime=" << minValidTime << endl;
	qValue = qvalue;
	autoregister = false;
	if( expires > 0 && expires < minValidTime ) {
		expires = minValidTime;
	} else if( expires <= 0) {
		expires = -1;
	}
	expiresTime = expires;
	rstate = NotConnected;
	regop = NoRegOpPending;
	authtype = DigestAuthenticationRequired;
	regUser = user;
	regUser->addServer( this );
	cleanRegister = false;
	timer = new QTimer( this );
	connect( timer, SIGNAL( timeout() ), this, SLOT( register_timeout() ) );
	mtimer = new QTimer( this );
	connect( mtimer, SIGNAL( timeout() ), this, SLOT( mob_timeout() ) );
	regcall = new SipCall( user, QString::null, SipCall::RegisterCall );
	regcall->setSubject( "Register Call" );
	outboundProxy = "";
	if( !serveruri.getHostname().isEmpty() ) {
		regserver = SipUri( serveruri.getHostname() );
		setOutboundProxyUri( serveruri ); 
	} else {
		if( regserver.getHostname().isEmpty() ) {
			regserver = SipUri( regUser->getMyUri()->getHostname() );
		}
	}

	//
	regCallMember = new SipCallMember(regcall,regserver );
	regreval=5;
	mobilitysupp=false; //set to true to support the feature; 
	regfailure = "none";
}

SipRegister::~SipRegister( void )
{
	regUser->removeServer( this );
	delete regCallMember;
	delete regcall;
	if (KStatics::debugLevel >=2) cout << ":::::SipRegister stop\n";
}

void SipRegister::setOutboundProxyUri( const SipUri &newregserver )
{
	outboundProxy = newregserver.proxyUri();

}

void SipRegister::requestRegister( const QString &username, const QString &password )
{
//	cout << "+++++++++++++++++requestRegister ";
	if ( regop == NoRegOpPending ) {
		regop = RegRequest;
		if( !username.isEmpty() && !password.isEmpty() && 
				( rstate == AuthenticationRequired || 
				rstate == AuthenticationRequiredWithNewPassword ) ) {
			if( authtype == DigestAuthenticationRequired ) {
				authresponse = Sip::getDigestResponse(
					username, password, "REGISTER", regserver.uri(), authstr ,1);
			} else if( authtype == BasicAuthenticationRequired ) {
				authresponse = Sip::getBasicResponse( username, password );
			} else if( authtype == ProxyDigestAuthenticationRequired ) {
				proxyauthresponse = Sip::getDigestResponse(
					username, password, "REGISTER", regserver.uri(), proxyauthstr );
			} else if( authtype == ProxyBasicAuthenticationRequired ) {
				proxyauthresponse = Sip::getBasicResponse( username, password );
			}
			rstate = TryingServer;//WithPassword;//Test 080809
			} else {
// begin IMS server: may be usefull for some IMS servers
/*			QString un = KStatics::myID;
			QString u = "";
			int at= un.find("@");
			if (at < 0) {
			    u  = regUser->getMyUri()->getHostname();
			    un += "@" + u; 
			} else {
			    u = un.mid(at+1);
			}
*/
// end of IMS server: delete between begin and end, if this disturbs
			rstate = TryingServer;
		}
		if( cleanRegister ) {
			regop = RegClear;
			KStatics::isReg = false;
			curtrans = regcall->newRegister( regserver, regCallMember, 0, authresponse, proxyauthresponse );
		} else {
			KStatics::isReg = true;
			curtrans = regcall->newRegister( regserver,  regCallMember,expiresTime, authresponse, proxyauthresponse, qValue );
		}
		cleanRegister = false;
		connect( curtrans, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdated() ) );
		if (curtrans->getStatus().getCode()>=600) localStatusUpdated();
	}
	statusUpdated();
}

void SipRegister::requestRegister( const SipUri & )
{
	statusUpdated();
}

void SipRegister::requestClearRegistration( void )
{
	if ( regop == NoRegOpPending ) {
		timer->stop();      /* reregistration */
		rstate = Disconnecting;
		regop = RegClear;
		cleanRegister = true;
		KStatics::isReg = false;
		curtrans = regcall->newRegister( regserver, regCallMember, 0 );
		connect( curtrans, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdated() ) );
	}
	statusUpdated();
}

void SipRegister::localStatusUpdated( void )
{
	QString authstrtemp;
	SipStatus status;
	bool rereg = false;
	int validTime, exTime=expiresTime*900;
	regfailure = "none";
	if( !curtrans ) {
		if (KStatics::debugLevel >1) cout << "!!!!!SipRegister: Received what was likely a retransmission, badly ignoring...\n" ;
		return;
	}

	if( curtrans->getStatus().getCode() >= 200 ) { // final response
		if( curtrans->getStatus().getCode() == 200 ) {
			switch ( regop ) {
				case RegRequest:
					rstate = Connected;
					status = curtrans->getStatus();
					validTime = status.getValidTime();
					if (exTime < validTime) validTime = exTime; // the min-expirestime is checked elsewhere!
					if (KStatics::debugLevel >1) cout <<  "ReRegistrationTimer (ms):" << exTime << "-" << validTime;
					timer->start( validTime, TRUE );
//not in use 
					if (mobilitysupp) { 
					    mtimer_triggered=false; 
					    mtimer->start( regreval * 1000, TRUE ); 
					}
					break;
				case RegClear:
					rstate = NotConnected;
					if (mobilitysupp && mtimer_triggered) { 
						mtimer_triggered=false;
						regcall->getSipClient()->setupContactUri();
						authresponse = QString::null;
						rstate = Reconnecting;
						cleanRegister = false;
						rereg=true;
					}						
					break;
				default:
					break;
			}
		} else if( curtrans->getStatus().getCode() == 401 ) {

			authstr = curtrans->getFinalWWWAuthString();
			
			if ((rstate == TryingServerWithPassword) && !authstr.contains("stale=true") ){
				rstate = AuthenticationRequiredWithNewPassword;
			} else {
				rstate = AuthenticationRequired;
			}
			authstrtemp = authstr.lower();
			if( authstrtemp.contains( "digest" ) ) {
				authtype = DigestAuthenticationRequired;
			} else {
				authtype = BasicAuthenticationRequired;
			}
			disconnect (curtrans, 0 , this, 0);
			curtrans = 0;
		} else if( curtrans->getStatus().getCode() == 407 ) {
			if (rstate == TryingServerWithPassword) {
				rstate = AuthenticationRequiredWithNewPassword;
			} else {
				rstate = AuthenticationRequired;
			}
			proxyauthstr = curtrans->getFinalProxyAuthString();
			authstrtemp = proxyauthstr.lower();
			if( authstrtemp.contains( "digest" ) ) {
				authtype = ProxyDigestAuthenticationRequired;
			} else {
				authtype = ProxyBasicAuthenticationRequired;
			}
			disconnect (curtrans, 0 , this, 0);
			curtrans = 0;
		} else if( curtrans->getStatus().getCode() == 423 ) {
			KStatics::noLine=0;
			regfailure ="Registration Failed,\n set expire time of registration to\n" + KStatics::minExT + "  sec.!!\n and restart the phone";
			rstate = NotConnected;
			if (KStatics::debugLevel >=2) cout << ":::::SipRegister " << regfailure.latin1() << endl;

		} else if( curtrans->getStatus().getCode() >= 400 ) {
			regfailure = "Registration Failed: " + SipStatus::codeString( curtrans->getStatus().getCode() );
			KStatics::noLine=0;
			rstate = NotConnected;
			if (KStatics::debugLevel >=2) cout << ":::::SipRegister " <<  regfailure.latin1() << endl;
			
		} else {
			switch ( regop ) {
				case RegRequest:
				case RegClear:
					rstate = Connected;
					break;
				default:
					break;
			}
		}
		regop = NoRegOpPending;
		if (rereg) requestRegister();
	}
	statusUpdated();
}


QString SipRegister::getAuthRealm( void )
{

	if( authstr.contains( "realm=\"" ) ) {
		QString realm;
		realm = authstr.mid( authstr.find( "realm=\"" ) + 7 );
		realm = realm.left( realm.find( '\"' ) );
		return realm;
	}

	return QString::null;
}

QString SipRegister::getProxyAuthRealm( void )
{

	if( proxyauthstr.contains( "realm=\"" ) ) {
		QString realm;
		realm = proxyauthstr.mid( authstr.find( "realm=\"" ) + 7 );
		if (KStatics::debugLevel >1) cout << "=====SipRegister: Realm is " << realm.latin1() << endl;
		realm = realm.left( realm.find( '\"' ) );
		return realm;
	}

	return QString::null;
}


void SipRegister::register_timeout()
{
	if (KStatics::debugLevel >2) cout << "=====SipRegister: Timeout\n" ;

	authresponse = QString::null;
	rstate = Reconnecting;
	requestRegister();
}


void SipRegister::updateRegister() { register_timeout(); }


void SipRegister::mob_timeout()
{

	if ( SipUtil::checkForAddressChange2( AF_INET, regserver.getHostname().latin1() ) ) {
		printf ("mob_timeout(): address changed\n");
		Sip::updateLocalAddresses();
		mtimer_triggered=true;
		requestClearRegistration();
	} else {
		mtimer->start( regreval * 1000, TRUE );
	}
}


void SipRegister::setMobilitySupport( const bool supp, const int period) 
{ 
	regreval=period;

	if (!mobilitysupp && supp) { // switch on
		mobilitysupp=supp; 
		mtimer_triggered=false; 
		//printf("------------------------>mtimer start\n");
		mtimer->start( regreval * 1000, TRUE );
	}
	if (mobilitysupp && !supp) { // switch off
		mobilitysupp=supp; 
		mtimer_triggered=false; 
		//printf("------------------------>mtimer stop\n");
		mtimer->stop();
	}
}
