
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
#include <qvbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qdir.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qmultilineedit.h>
#include <qsettings.h>

#include "../dissipate2/sipprotocol.h"
#include "../dissipate2/sipclient.h"
#include "kphone.h"
#include "sessioncontrol.h"
#include "ksippreferences.h"
#include "kstatics.h"
KSipPreferences::KSipPreferences( SipClient *client, SessionControl *ca, QWidget *parent, const char *name )
	: QTabDialog( parent, QString( name ) + tr("Global SIP Preferences") )
{
	c = client;
	sessionC = ca;
	
	pFix = QString::fromAscii( name);
	
	QVBox *vboxadv = new QVBox();
	addTab( vboxadv, tr("Settings 1"));

	// Via hiding radio buttons
	hidebg = new QHButtonGroup( tr("Hide Via"), vboxadv );
	QRadioButton *nohide = new QRadioButton( tr("Don't Hide"), hidebg );
	hidebg->insert( nohide, NoHide );
	QRadioButton *hophide = new QRadioButton( tr("Request Next Hop"), hidebg );
	hidebg->insert( hophide, HideHop );

	QRadioButton *routehide = new QRadioButton( tr("Request Full Route"), hidebg );
	hidebg->insert( routehide, HideRoute );

	// Set max forwards
	(void) new QLabel( tr("Max Forwards"), vboxadv );
	maxforwards = new QLineEdit( vboxadv );

	// PhoneBook
	(void) new QLabel( tr("XML-File for Phone Book:"), vboxadv );
	phonebook = new QLineEdit( vboxadv );

	// RegistrationExpiresTime
	(void) new QLabel(tr( "Expire Time of Registration (seconds):"), vboxadv );
	expires = new QLineEdit( vboxadv );

	// DTMF Sequence Timer
	(void) new QLabel( tr("Wait between DTMF seqences (milliseconds):"), vboxadv );
	dtmfWait = new QLineEdit( vboxadv );

	// SubscribeExpiresTime
	(void) new QLabel( tr("Expire Time of Presence Subscription (seconds);restart KPhone to activate new value:"), vboxadv );
	subscribeExpires = new QLineEdit( vboxadv );

	// PublishExpiresTime
	(void) new QLabel( tr("Expire Time of Publish (seconds);restart KPhone to activate new value"), vboxadv );
	publishExpires = new QLineEdit( vboxadv );

	// ActivityCheckTime
	(void) new QLabel( tr("User activity check time (minutes):"), vboxadv );
	activityCheck = new QLineEdit( vboxadv );


	// Watcher Subscription ?
	watbg = new QHButtonGroup( tr("Watcher Subscription"), vboxadv );
	QRadioButton *nowat = new QRadioButton(tr( "No"), watbg );
	watbg->insert( nowat, NoWat );
	QRadioButton *yeswat = new QRadioButton( tr("Yes"), watbg );
	watbg->insert( yeswat, YesWat );
	if( sessionC->getWat() ) {
	    watbg->setButton( YesWat );
	} else {
		watbg->setButton( NoWat );
	}
	
	
	QVBox *vboxadv1 = new QVBox();
	addTab( vboxadv1, tr("Settings 2"));

	// Remote Dialer
	rembg = new QHButtonGroup( tr("Remote Dialer"), vboxadv1 );
	QRadioButton *norem = new QRadioButton(tr( "No remote dialer"), rembg );
	rembg->insert( norem, NoRem );
	QRadioButton *yesrem = new QRadioButton( tr("Use remote dialer"), rembg );
	rembg->insert( yesrem, YesRem );
	if( sessionC->getRem() ) {
	    rembg->setButton( YesRem );
	} else {
		rembg->setButton( NoRem );
	}
// RFC3262  Reliability of Provisional Responses 
	prackbg = new QHButtonGroup( tr("RFC3262  Request Reliability of Provisional Responses"), vboxadv1 );
	QRadioButton *noprack = new QRadioButton( tr("No"), prackbg );
	prackbg->insert( noprack, NoPRACK );
	QRadioButton *supportprack = new QRadioButton( tr("Support"), prackbg );
	prackbg->insert( supportprack, SupportPRACK );
	QRadioButton *requireprack = new QRadioButton( tr("Require"), prackbg );
	prackbg->insert( requireprack, RequirePRACK );
	if( sessionC->getPrack() == NoPRACK ) {
	    prackbg->setButton( NoPRACK );
	} else if ( sessionC->getPrack() ==  SupportPRACK  ){
		prackbg->setButton(  SupportPRACK  );
	} else {
		prackbg->setButton(  RequirePRACK  );
	} 

	prackPbg = new QHButtonGroup( tr("RFC3262 Provide Reliability  if Peer only Suports it"), vboxadv1 );
	QRadioButton *nopprack = new QRadioButton( tr("No"), prackPbg );
	prackPbg->insert( nopprack, NoPPRACK );
	QRadioButton *yespprack = new QRadioButton( tr("Yes"), prackPbg );
	prackPbg->insert( yespprack, YesPPRACK );
	if( sessionC->getPPrack() == NoPPRACK ) {
	    prackPbg->setButton( NoPPRACK );
	} else {
		prackPbg->setButton(  YesPPRACK  );
	} 

	// Call Transfer: Implicit Subscription
	isbbg = new QHButtonGroup( tr("CT Implicit Subscription"), vboxadv1 );
	QRadioButton *yesisb = new QRadioButton( tr("Use Implicit Subscription"), isbbg );
	isbbg->insert( yesisb, YesISB );
	QRadioButton *noisb = new QRadioButton(tr( "No Implicit Subscription"), isbbg );
	isbbg->insert( noisb, NoISB );
	if( sessionC->getIsb() ) {
	    isbbg->setButton( YesISB );
	} else {
		isbbg->setButton( NoISB );
	}
	
	// CT Notification Timer
	(void) new QLabel( tr("Expire Time of CT Notification (seconds):"), vboxadv1 );
	notiExpires = new QLineEdit( vboxadv1 );


#ifdef SRTP
	QVBox* sboxs = new QVBox();
	//sboxs->setFixedHeight(350);
	addTab( sboxs, tr("SRTP") );
	
	srtp = new QHButtonGroup( tr(tr("Mode")), sboxs );
	QRadioButton* disabledB = new QRadioButton( tr(tr("Disabled")), srtp );
	srtp->insert( disabledB, disable );
	QRadioButton* pskB = new QRadioButton( tr(("SRTP Encryption)")), srtp );
	srtp->insert( pskB, psk );
	(void) new QLabel( tr("SRTP Master Key (length 30 characters):"), sboxs );
	pskMasterKey = new QLineEdit( sboxs );

#endif
	sessionC->setSrtpMode( 0 );	
	// Socket
	QVBox *vboxs = new QVBox( );
	addTab( vboxs, "Socket" );

	// UDP / TCP
	socket = new QHButtonGroup( tr("Socket Protocol (Kphone restart needed to apply change)"), vboxs );

	QRadioButton *udp = new QRadioButton( "UDP", socket );
	socket->insert( udp, UDP );

	QRadioButton *tcp = new QRadioButton( "TCP", socket );
	socket->insert( tcp, TCP );

	//Server Location
	sloc = new QHButtonGroup( tr("Server Location "), vboxs );

	QRadioButton *dns = new QRadioButton( tr("DNS only"), sloc );
	sloc->insert( dns, SipClient::DNS );

	QRadioButton *srv = new QRadioButton( tr("use SRV"), sloc );
	sloc->insert( srv, SipClient::SRV );

	QRadioButton *naptr = new QRadioButton( tr("use NAPTR"), sloc );
	sloc->insert( naptr, SipClient::NAPTR );
	
	

	// STUN
	stun = new QHButtonGroup( tr("Use STUN Server (First activation works after reboot of kphone only)"), vboxs );
	QRadioButton *StunYes = new QRadioButton( tr("Yes"), stun );
	stun->insert( StunYes, stunYes );
	QRadioButton *StunNo = new QRadioButton( tr("No"), stun );
	stun->insert( StunNo, stunNo );
	(void) new QLabel( tr("STUN Server (address:port):"), vboxs );
	stunServerAddr = new QLineEdit( vboxs );

	// Request period
	(void) new QLabel( tr("Request Period for STUN Server (second):"), vboxs );
	stunRequestPeriod = new QLineEdit( vboxs );

	(void) new QLabel( tr("Media Min Port:"), vboxs );
	mediaMinPort = new QLineEdit( vboxs );

	(void) new QLabel( tr("Media Max Port:"), vboxs );
	mediaMaxPort = new QLineEdit( vboxs );

	(void) new QLabel( tr("Application Min Port:"), vboxs );
	appMinPort = new QLineEdit( vboxs );

	(void) new QLabel( tr("Application Max Port:"), vboxs );
	appMaxPort = new QLineEdit( vboxs );
	// Call Preferences
	QVBox *vboxprefs = new QVBox( );

	// Call Forwarding
	QVBox *vboxfor = new QVBox( );
	addTab( vboxfor, tr("Call Preferences") );

	// CFU
	forbg = new QHButtonGroup(tr( "Call Forwarding Unconditional"), vboxfor );

	QRadioButton *nocforward = new QRadioButton( tr("Inactive"), forbg );
	forbg->insert( nocforward, FwInactive );

	QRadioButton *cforward = new QRadioButton( tr("Active"), forbg );
	forbg->insert( cforward, FwActive );

	// Forward to Uri
	(void) new QLabel( tr("Forward to:"), vboxfor );
	forwardaddr = new QLineEdit( vboxfor );

	//CFB
	forbbg = new QHButtonGroup( tr("Call Forwarding on Busy"), vboxfor );

	QRadioButton *noBcforward = new QRadioButton( tr("Inactive"), forbbg );
	forbbg->insert( noBcforward, FwInactive );

	QRadioButton *cBforward = new QRadioButton( tr("Active"), forbbg );
	forbbg->insert( cBforward, FwActive );

	// Forward to Uri
	(void) new QLabel(tr( "CFB Forward to:"), vboxfor );
	forwardBaddr = new QLineEdit( vboxfor );
	forwardBaddr->setText(sessionC->getCallForwardBUri());

	// CFNA
	fnabg = new QHButtonGroup( tr("CFNA"), vboxfor );

	QRadioButton *nocfna = new QRadioButton( tr("Inactive"), fnabg );
	fnabg->insert( nocfna, FwInactive );

	QRadioButton *cfna = new QRadioButton( tr("Active"), fnabg );
	fnabg->insert( cfna, FwActive );

	// CFNA Forward to Uri
	(void) new QLabel( tr("CFNA Forward to:"), vboxfor );
	forwardNAaddr = new QLineEdit( vboxfor );
	forwardNAaddr->setText(sessionC->getCallForwardNAUri());

	// CFNA Ring Count
	(void) new QLabel( tr("CFNA Count:"), vboxfor );
	forwardNAcnt = new QLineEdit( vboxfor );
	forwardNAcnt->setText(sessionC->getCallForwardNAcnt());

	// AA
	auabg = new QHButtonGroup( tr("AutoAnswer"), vboxfor );

	QRadioButton *noaua = new QRadioButton( tr("Inactive"), auabg );
	auabg->insert( noaua, AuaInactive );

	QRadioButton *aua = new QRadioButton( tr("Active"), auabg );
	auabg->insert( aua, AuaActive );

	// CFNA Ring Count
	(void) new QLabel( tr("AutoAnswer Count:"), vboxfor );
	auacnt = new QLineEdit( vboxfor );
	auacnt->setText(sessionC->getAuacnt());



	addTab( vboxprefs, tr("Messages") );
//Kphone
	// Default user mode
/*	usedef = new QHButtonGroup( "Policy for Unknown Incoming URIs", vboxprefs );

	QRadioButton *defmode = new QRadioButton( "Accept", usedef );
	usedef->insert( defmode, DefMode );

	QRadioButton *nodefmode = new QRadioButton( "Reject with Not Found", usedef );
	usedef->insert( nodefmode, NoDefMode );
*/
	// Forward to message
	(void) new QLabel( tr("Call Forward Message:"), vboxprefs );
	forwardmsg = new QMultiLineEdit( vboxprefs );

	// Busy message
	(void) new QLabel( tr("Busy Message:"), vboxprefs );
	busymsg = new QMultiLineEdit( vboxprefs );
	
	setCancelButton();
	connect( this, SIGNAL( cancelButtonPressed() ), this, SLOT( slotCancel() ) );
	setOKButton();
	connect( this, SIGNAL( applyButtonPressed() ), this, SLOT( slotOk() ) );
	resetSettings();
}

KSipPreferences::~KSipPreferences( void )
{
}

void KSipPreferences::slotOk( void )
{
	slotApply();
	QDialog::accept();
}

void KSipPreferences::slotApply( void )
{
	switch( hidebg->id( hidebg->selected() ) ) {
		case HideHop:
			c->setHideViaMode( SipClient::HideHop );
			break;
		case HideRoute:
			c->setHideViaMode( SipClient::HideRoute );
			break;
		default:
		case NoHide:
			c->setHideViaMode( SipClient::DontHideVia );
			break;
	}
	if( forbg->id( forbg->selected() ) == FwActive ) {
		c->setCallForward( true );
	} else {
		c->setCallForward( false );
	}

	if( forbbg->id( forbbg->selected() ) == FwActive ) {
		c->setCallForwardB( true );
		sessionC->setCFB( true );
	} else {
		c->setCallForwardB( false );
		sessionC->setCFB( false );		
	}

	if( fnabg->id( fnabg->selected() ) == FwActive ) {
		sessionC->setCFNA( true );
	} else {
		sessionC->setCFNA( false );
	}

	if( auabg->id( auabg->selected() ) == AuaActive ) {
		sessionC->setAua( true );
	} else {
		sessionC->setAua( false );
	}

	if( rembg->id( rembg->selected() ) == YesRem  ) {
		sessionC->setRem( true );
	} else {
		sessionC->setRem( false );
	}

	if( watbg->id( watbg->selected() ) == YesWat  ) {
		sessionC->setWat( true );
	} else {
		sessionC->setWat( false );
	}


	if( isbbg->id( isbbg->selected() ) == YesISB  ) {
		sessionC->setIsb( true );
	} else {
		sessionC->setIsb( false );
	}

	if( prackbg->id( prackbg->selected() ) == NoPRACK  ) {
		sessionC->setPrack( NoPRACK );
	} else if( prackbg->id( prackbg->selected() ) == SupportPRACK ) {
		sessionC->setPrack( SupportPRACK );
	} else {
		sessionC->setPrack( RequirePRACK );
	}

	if( prackPbg->id( prackPbg->selected() ) == NoPPRACK  ) {
		sessionC->setPPrack( NoPPRACK );
	} else {
		sessionC->setPPrack( YesPPRACK );
	}
	
	sessionC->setCallForwardBUri( forwardBaddr->text() );
	sessionC->setCallForwardNAUri( forwardNAaddr->text() );
	sessionC->setCallForwardNAcnt( forwardNAcnt->text()  );
	sessionC->setAuacnt( auacnt->text()  );
	c->setCallForwardUri( SipUri( forwardaddr->text()  ) );
	c->setCallForwardMessage( forwardmsg->text() );
	c->setMaxForwards( maxforwards->text().toUInt() );
	c->setBusyMessage( busymsg->text() );

	QSettings settings;
	settings.writeEntry( KStatics::dBase+pFix+"/sip/AuaCnt",sessionC->getAuacnt());
	settings.writeEntry( KStatics::dBase+pFix+"/sip/callforwardNAcnt",sessionC->getCallForwardNAcnt());
	settings.writeEntry( KStatics::dBase+pFix+"/sip/callforwardNAUri",sessionC->getCallForwardNAUri());
	settings.writeEntry( KStatics::dBase+pFix+"/sip/callforwardBUri",sessionC->getCallForwardBUri());
	settings.writeEntry( KStatics::dBase+pFix+"/sip/callforward", c->getCallForward() );
	settings.writeEntry( KStatics::dBase+pFix+"/sip/forwardaddr", c->getCallForwardUri().nameAddr() );
	settings.writeEntry( KStatics::dBase+pFix+"/sip/forwardmsg", c->getCallForwardMessage() );
	settings.writeEntry( KStatics::dBase+pFix+"/sip/maxforwards", c->getMaxForwards() );
	settings.writeEntry( KStatics::dBase+pFix+"/sip/busymsg", c->getBusyMessage() );
	
	QString hvstr;
	switch( c->getHideViaMode() ) {
		default:
		case SipClient::DontHideVia: hvstr = "NoHide"; break;
		case SipClient::HideHop: hvstr = "HideHop"; break;
		case SipClient::HideRoute: hvstr = "HideRoute"; break;
	}
	settings.writeEntry( KStatics::dBase+pFix+"/sip/hideviamode", hvstr );

	if( sessionC->getRem() ) {
		hvstr = "Yes";
	} else {
		hvstr = "No";
	}
	settings.writeEntry( KStatics::dBase+pFix+"/sip/remdialer", hvstr );

	if( sessionC->getWat() ) {
		hvstr = "Yes";
	} else {
		hvstr = "No";
	}
	settings.writeEntry( KStatics::dBase+pFix+"/sip/winfosub", hvstr );


	if( sessionC->getIsb() ) {
		hvstr = "Yes";
	} else {
		hvstr = "No";
	}
	settings.writeEntry( KStatics::dBase+pFix+"/sip/implsub", hvstr );

	if( sessionC->getPPrack() == NoPPRACK ) {
		hvstr = "PPrackNo" ;
	} else {
		hvstr = "PPrackYes" ;
	} 

	settings.writeEntry( KStatics::dBase+pFix+"/sip/PPrack", hvstr );



	if( sessionC->getPrack() == NoPRACK ) {
		hvstr = "PrackNo" ;
	} else if (sessionC->getPrack() == SupportPRACK){
		hvstr = "PrackSup" ;	
	} else {
		hvstr = "PrackReq" ;
	} 

	settings.writeEntry( KStatics::dBase+pFix+"/sip/Prack", hvstr );


	QString p = KStatics::dBase + ((KPhone *)parentWidget())->getUserPrefix() + "/local/";
	settings.writeEntry( p + "/PhoneBook", phonebook->text() );
	if( expires->text() ) {
		settings.writeEntry( p + "/RegistrationExpiresTime", expires->text() );
	}
	if( notiExpires->text() ) {
		bool ok;
		settings.writeEntry(KStatics::dBase+pFix+"/General/CTNotificationExpiresTime", notiExpires->text() );
		sessionC->setNotiExpires(notiExpires->text().toInt(&ok,10));
	}	
	if( dtmfWait->text() ) {
		bool ok;
		settings.writeEntry(KStatics::dBase+pFix+"/General/DTMFSequenceTime", dtmfWait->text() );
		sessionC->setDTMFWait(dtmfWait->text().toInt(&ok,10));
	}
	if( subscribeExpires->text() ) {
		bool ok;
		int dec = subscribeExpires->text().toInt( &ok, 10 );
		if( ok ) {
			if( dec < 60 ) {
				subscribeExpires->setText( "60" );
				dec = 60;
			}
			settings.writeEntry( p + "/SubscribeExpiresTime", subscribeExpires->text() );
		}  else {
			settings.writeEntry( p + "/SubscribeExpiresTime", "0" );		
			dec = 0;
		}
		sessionC->setSubExpiresTime(dec);
	}

	if( publishExpires->text() ) {
		bool ok;
		int dec = publishExpires->text().toInt( &ok, 10 );
		if( ok ) {
			if(( dec < 60 ) && (dec != 0) ) {
				publishExpires->setText( "60" );
				dec = 60;
			}
			settings.writeEntry( p + "/PublishExpiresTime", publishExpires->text() );
		} else {
			settings.writeEntry( p + "/PublishExpiresTime", "0" );		
			dec = 0;
		}
		sessionC->setPubExpiresTime(dec);
	}


	if ( activityCheck->text() ) {
		bool ok;
		int dec = activityCheck->text().toInt( &ok, 10 );
		if ( ok ) {
			if ( (dec < 60 ) && (dec != 0) ) {
				activityCheck->setText( "60" );
				dec = 60;
			}
			settings.writeEntry( p + "/ActivityCheckTime", activityCheck->text() );
		} else {
			settings.writeEntry( p + "/ActivityCheckTime", "0" );		
			dec = 0;
		}
		sessionC->setActivityCheckTime(dec);
	}
	
	if( watbg->id( watbg->selected() ) == YesWat  ) {
		sessionC->setWat( true );
	} else {
		sessionC->setWat( false );
	}

#ifdef SRTP
	switch( srtp->id( srtp->selected() ) ) {
		case psk:
			settings.writeEntry(KStatics::dBase +"SRTP/Mode", "PSK");
			sessionC->setSrtpMode( 1);	
			break;
		default:
			settings.writeEntry(KStatics::dBase +"SRTP/Mode", "disabled");
			sessionC->setSrtpMode( 0 );	
			break;
	}
	settings.writeEntry( KStatics::dBase + "SRTP/KeyValue", pskMasterKey->text() );
#else 
	settings.writeEntry( KStatics::dBase + "SRTP/mode", "disabled" );
#endif 	
	switch( socket->id( socket->selected() ) ) {
		case TCP:
			settings.writeEntry( KStatics::dBase+pFix+"/General/SocketMode", "TCP" );
			break;
		default:
			settings.writeEntry( KStatics::dBase+pFix+"/General/SocketMode", "UDP" );
			break;
	}
	
	switch( sloc->id( sloc->selected() ) ) {
		case SipClient::DNS:
			settings.writeEntry( KStatics::dBase+pFix+"/General/LocServer", "DNS" );
			break;
		case SipClient::SRV:
			settings.writeEntry( KStatics::dBase+pFix+"/General/LocServer", "SRV" );
			break;
		default:
			settings.writeEntry( KStatics::dBase+pFix+"/General/LocServer", "NAPTR" );
			break;
	}
	
	
	settings.writeEntry( KStatics::dBase+ pFix  +"/STUN/StunServer", stunServerAddr->text() );
	settings.writeEntry( KStatics::dBase+ pFix  +"/STUN/RequestPeriod", stunRequestPeriod->text() );
	switch( stun->id( stun->selected() ) ) {
		case stunYes:
			settings.writeEntry( KStatics::dBase+ pFix  +"/STUN/UseStun", "Yes" );
			sessionC->setStun(true);
			break;
		default:
			settings.writeEntry( KStatics::dBase+ pFix  +"/STUN/UseStun", "No" );
			sessionC->setStun(false);
			break;
	}

	settings.writeEntry(KStatics::dBase+ pFix  +"/Media/MinPort", mediaMinPort->text());
	settings.writeEntry(KStatics::dBase+ pFix  +"/Media/MaxPort", mediaMaxPort->text());
	
	settings.writeEntry(KStatics::dBase+ pFix  +"/Application/MinPort", appMinPort->text());
	settings.writeEntry(KStatics::dBase+ pFix  +"/Application/MaxPort", appMaxPort->text());
	sessionC->setPorts(mediaMinPort->text(),  mediaMaxPort->text(), appMinPort->text() ,appMaxPort->text());
	
}

void KSipPreferences::slotCancel( void )
{
	resetSettings();
	QDialog::reject();
}

void KSipPreferences::resetSettings( void )
{
	forwardBaddr->setText(sessionC->getCallForwardBUri());
	forwardNAaddr->setText(sessionC->getCallForwardNAUri());
	forwardNAcnt->setText(sessionC->getCallForwardNAcnt());
	forwardaddr->setText( c->getCallForwardUri().nameAddr() );
	forwardmsg->setText( c->getCallForwardMessage() );
	maxforwards->setText( QString::number( c->getMaxForwards() ) );
	busymsg->setText( c->getBusyMessage() );
	QSettings settings;
	
#ifdef SRTP
	if(settings.readEntry(KStatics::dBase +"SRTP/Mode", "disabled") == "PSK") {
		srtp->setButton( psk );
		sessionC->setSrtpMode( 1);	
	} else {
		srtp->setButton( disable );	
		sessionC->setSrtpMode( 0);	
	}
	
	pskMasterKey->setText( settings.readEntry(KStatics::dBase + "SRTP/KeyValue", "" ) );
#endif

	switch( c->getHideViaMode() ) {
		default:
		case SipClient::DontHideVia: hidebg->setButton( NoHide ); break;
		case SipClient::HideHop: hidebg->setButton( HideHop ); break;
		case SipClient::HideRoute: hidebg->setButton( HideRoute ); break;
	}

	if( sessionC->getRem() ) {
		rembg->setButton( YesRem );
	} else {
		rembg->setButton( NoRem );
	}

	if( sessionC->getWat() ) {
		watbg->setButton( YesWat );
	} else {
		watbg->setButton( NoWat );
	}

	if( sessionC->getIsb() ) {
		isbbg->setButton( YesISB );
	} else {
		isbbg->setButton( NoISB );
	}

	if( sessionC->getPPrack() == NoPPRACK ) {
	    prackPbg->setButton( NoPPRACK );
	} else {
		prackPbg->setButton(  YesPPRACK  );
	} 
	if( sessionC->getPrack() == NoPRACK ) {
	    prackbg->setButton( NoPRACK );
	} else if ( sessionC->getPrack() ==  SupportPRACK  ){
		prackbg->setButton(  SupportPRACK  );
	} else {
		prackbg->setButton(  RequirePRACK  );
	} 


	if( c->getCallForward() ) {
		forbg->setButton( FwActive );
	} else {
		forbg->setButton( FwInactive );
	}

	if( c->getCallForwardB() ) {
		forbbg->setButton( FwActive );
	} else {
		forbbg->setButton( FwInactive );
	}

	if( sessionC->getCFNA() ) {
		fnabg->setButton( FwActive );
	} else {
		fnabg->setButton( FwInactive );
	}	
	
	if( sessionC->getAua() ) {
		auabg->setButton( AuaActive );
	} else {
		auabg->setButton( AuaInactive );
	}
//	QSettings settings;
	QString p = KStatics::dBase + ((KPhone *)parentWidget())->getUserPrefix() + "/local/";
	QString xmlFile = settings.readEntry( p + "/PhoneBook", "" );

	if( xmlFile.isEmpty() ) {
		if( ((KPhone *)parentWidget())->getUserPrefix().isEmpty() ) {
			xmlFile = QDir::homeDirPath() + "/." + KStatics::xBase + "-phonebook.xml";
		} else {
			xmlFile = QDir::homeDirPath() + "/." + KStatics::xBase + "_" + ((KPhone *)parentWidget())->getUserPrefix() + "phonebook.xml";
		}
	}
	phonebook->setText( xmlFile );
	QString strDefault;
	
	notiExpires->setText( settings.readEntry( KStatics::dBase+pFix+"/General/CTNotificationExpiresTime","10"));
	dtmfWait->setText( settings.readEntry(KStatics::dBase+pFix+"/General/DTMFSequenceTime", "1000") );
	
	expires->setText( settings.readEntry( p + "/RegistrationExpiresTime",
		strDefault.setNum( constRegistrationExpiresTime ) ) );
	
	subscribeExpires->setText( settings.readEntry( p + "/SubscribeExpiresTime",
		strDefault.setNum( constSubscribeExpiresTime ) ) );
		sessionC->setSubExpiresTime(constSubscribeExpiresTime);
	publishExpires->setText( settings.readEntry( p + "/PublishExpiresTime",
		strDefault.setNum( 0 ) ) );
		sessionC->setPubExpiresTime(0);

	activityCheck->setText( settings.readEntry( p + "/ActivityCheckTime", strDefault.setNum( 0 ) ) );
	sessionC->setActivityCheckTime(0);
	
	if( settings.readEntry( KStatics::dBase+ pFix  +"/STUN/UseStun", "No" ) == "Yes" ) {
		stun->setButton( stunYes );
		sessionC->setStun(true);
	} else {
		stun->setButton( stunNo );
		sessionC->setStun(false);
	}
	stunServerAddr->setText( settings.readEntry( KStatics::dBase+ pFix  +"/STUN/StunServer", constStunServer ) );
	stunRequestPeriod->setText( settings.readEntry( KStatics::dBase+ pFix  +"/STUN/RequestPeriod",
		strDefault.setNum( constStunRequestPeriod ) ) );
	mediaMinPort->setText( settings.readEntry(KStatics::dBase+ pFix  +"/Media/MinPort", strDefault.setNum(constMinPort)));
	mediaMaxPort->setText( settings.readEntry(KStatics::dBase+ pFix  +"/Media/MaxPort", strDefault.setNum(constMaxPort)));
	appMinPort->setText( settings.readEntry(KStatics::dBase+ pFix  +"/Application/MinPort", strDefault.setNum(constMinMPort)));
	appMaxPort->setText( settings.readEntry(KStatics::dBase+ pFix  +"/Application/MaxPort", strDefault.setNum(constMaxMPort)));

	if( c->isTcpSocket() ) {
		socket->setButton( TCP );
	} else {
		socket->setButton( UDP );
	}
	
	if( c->getLocServ() == SipClient::NAPTR ) {
		sloc->setButton( SipClient::NAPTR );
	} else if( c->getLocServ() == SipClient::SRV ) {
		sloc->setButton( SipClient::SRV );
	} else {
		sloc->setButton( SipClient::DNS );
	}
	
}
