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
#include <iostream>
using namespace std;
#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qlayout.h>

#include "../dissipate2/sipcall.h"
#include "../dissipate2/sipuser.h"
#include "../dissipate2/sipregister.h"
#include "../dissipate2/sipclient.h"
#include "kphone.h"
#include "kphoneview.h"
#include "ksipauthentication.h"
#include "ksipregistrations.h"
#include "kstatics.h"
#include "sessioncontrol.h"

KSipIdentityEdit::KSipIdentityEdit( QWidget *parent, const char *name, QObject *reg )
	: QDialog( parent, QString( name ) + tr("Identity Editor"), true )
{
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );

	(void) new QLabel( tr("Full Name:"), vbox );
	fullname = new QLineEdit( vbox );
	fullname->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	(void) new QLabel( tr("User Part of SIP URL:"), vbox );
	username = new QLineEdit( vbox );
	(void) new QLabel( tr("Host Part of SIP URL:"), vbox );
	hostname = new QLineEdit( vbox );
	(void) new QLabel( tr("Outbound Proxy (optional):"), vbox );
	sipProxy = new QLineEdit( vbox );
	(void) new QLabel( tr("Authentication Username (optional):"), vbox );
	sipProxyUsername = new QLineEdit( vbox );
	(void) new QLabel( tr("q-value between 0.0-1.0 (optional):"), vbox );
	qValue = new QLineEdit( vbox );
	
	clearPWButton = new QCheckBox( tr("Clear Password?"), vbox );
	autoRegister = new QCheckBox( tr("Auto Register"), vbox );
	labelRegister = new QLabel( "", vbox );
	buttonRegister = new QPushButton( tr("Register"), vbox );
	connect( buttonRegister, SIGNAL( clicked() ), reg, SLOT( changeRegistration() ) );

	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 );
	helpPushButton = new QPushButton( this, tr("help button") );
	helpPushButton->setText( "&help..." );
	buttonBox->addWidget( helpPushButton );
	QSpacerItem *spacer = new QSpacerItem(
		0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	buttonBox->addItem( spacer );
	okPushButton = new QPushButton( this, tr("ok button") );
	okPushButton->setText( "OK" );
	okPushButton->setDefault( TRUE );
	buttonBox->addWidget( okPushButton );
	cancelPushButton = new QPushButton( this, tr("cancel button") );
	cancelPushButton->setText( tr("Cancel") );
	cancelPushButton->setAccel( Key_Escape );
	buttonBox->addWidget( cancelPushButton );
	connect( okPushButton, SIGNAL( clicked() ),
		this, SLOT( slotOk() ) );
	connect( cancelPushButton, SIGNAL( clicked() ),
		this, SLOT( slotCancel() ) );
	connect( helpPushButton, SIGNAL( clicked() ),
		parent, SLOT( showHelp() ) );
}

KSipIdentityEdit::~KSipIdentityEdit( void )
{
}

void KSipIdentityEdit::updateState( KsipState state )
{
	if( state == OFFLINE ) {
		buttonRegister->setText( tr("Register : not registered") );
		buttonRegister->setEnabled( false );
	} else if( state == REG ) {
		labelRegister->setText( tr("Registration : registered"));
		buttonRegister->setText( tr("Unregister") );
		buttonRegister->setEnabled( true );
	} else if ( state == UNREG ) {
		labelRegister->setText( tr("Registration : not registered"));
		buttonRegister->setText( tr("Register") );
		buttonRegister->setEnabled( true );
	} else {
          labelRegister->setText( tr("Registration : ") );
        }
}

QString KSipIdentityEdit::getFullname( void ) const
{
	return fullname->text();
}

QString KSipIdentityEdit::getUsername( void ) const
{
	return username->text();
}

QString KSipIdentityEdit::getHostname( void ) const
{
	return hostname->text();
}

QString KSipIdentityEdit::getSipProxy( void ) const
{
	return sipProxy->text();
}

QString KSipIdentityEdit::getSipProxyUsername( void ) const
{
	return sipProxyUsername->text();
}

QString KSipIdentityEdit::getQvalue( void ) const
{
	bool ok;
	float f = qValue->text().toFloat( &ok );
	if( ok && f >= 0 && f <= 1 ) {
		return qValue->text();
	}
	return "";
}

QString KSipIdentityEdit::getUri( void ) const
{
	return "\"" + fullname->text() + "\" <sip:" + username->text() + "@" + hostname->text() + ">";
}

void KSipIdentityEdit::setFullname( const QString &newFullname )
{
	fullname->setText( newFullname );
}

void KSipIdentityEdit::setUsername( const QString &newUsername )
{
	username->setText( newUsername );
}

void KSipIdentityEdit::setHostname( const QString &newHostname )
{
	hostname->setText( newHostname );
}

void KSipIdentityEdit::setSipProxy( const QString &newSipProxy )
{
	sipProxy->setText( newSipProxy );
}

void KSipIdentityEdit::setQvalue( const QString &qvalue )
{
	qValue->setText( qvalue );
}

void KSipIdentityEdit::setSipProxyUsername( const QString &newSipProxyUsername )
{
	sipProxyUsername->setText( newSipProxyUsername );
}

void KSipIdentityEdit::slotOk( void )
{
	if( username->text() == "" || hostname->text() == "" ) {
		QDialog::reject();
	} else {
		update();
		QDialog::accept();
	}
	
}

void KSipIdentityEdit::slotCancel( void )
{
	QDialog::reject();
}

bool KSipIdentityEdit::ClearPW(void) {
	return clearPWButton->isChecked();
}
KSipRegistrations::KSipRegistrations(
		SipClient *client, KPhoneView *phoneView, QWidget *p, const char *prefix )
{
	parent = p;
	c = client;
	v = phoneView;
	userPrefix = prefix;
	useStun = false;

	edit = new KSipIdentityEdit( parent, userPrefix.latin1(), this );
	connect( edit, SIGNAL( update() ), this, SLOT( update() ) );
	sipreg = 0;
	QString uristr;
	QString str;
	QString dspw;
	SipUri uri;
	
	QSettings settings;
	QString pp = KStatics::dBase + userPrefix + "/local/";
	expires = settings.readNumEntry( pp + "/RegistrationExpiresTime", constRegistrationExpiresTime );

	if( expires == 0 ) {
		expires = -1;
	}
	pp = KStatics::dBase + userPrefix + "Registration/";
	if( settings.readEntry( pp + "/SipUri", "" ) != "" ) {
		uristr = settings.readEntry( pp + "/SipUri" );
		uri = SipUri( uristr );
		u = c->getUser( uri );
		if( u == NULL ) {
			u = new SipUser( c, 0, uri );
		}
		stunSrv = "";
		if( settings.readEntry( KStatics::dBase + userPrefix + "/STUN/UseStun", "" ) == "Yes" ) {
			useStun = true;
			stunSrv = settings.readEntry( KStatics::dBase + userPrefix + "/STUN/StunServer",constStunServer );
			if( stunSrv.isEmpty() ) {
				QString dname = u->getMyUri()->getHostname();
				stunSrv = dname;
				QString srv = client->getSRV( QString( "_stun._udp." ) + dname );
				if( !srv.isEmpty() ) {
					stunSrv = srv;
				}
				stunSrv += ":3478";
			} else {
				if( !stunSrv.contains( ':' ) ) {
					stunSrv += ":3478";
				}
			}
		}
		uristr = "";
		if( settings.readEntry( pp + "/SipServer", "" ) != "" ) {
			uristr = settings.readEntry( pp + "/SipServer" );
		}
		QString qvalue = settings.readEntry( pp + "/qValue", "" );
		sipreg = new SipRegister( u, SipUri( uristr ) , expires, qvalue );
		connect( sipreg, SIGNAL( statusUpdated() ),
			this, SLOT( registerStatusUpdated() ) );
		v->updateIdentity( u, sipreg );
		c->updateIdentity( u, sipreg->getOutboundProxy() );
		str = settings.readEntry( pp + "/UserName" );
		KStatics::myID=str;
		u->getMyUri()->setProxyUsername( str );
		str = settings.readEntry( pp + "/Password" );
		dspw = retrievePW(str.latin1(),str.length());
		KStatics::myPW=dspw;
		u->getMyUri()->setPassword( dspw );
		str = settings.readEntry( pp + "/AutoRegister" );
		if( str == "Yes" ) {
			autoRegister = true;
			if( useStun ) {
				sipreg->setAutoRegister( true );
			} else {
				sipreg->requestRegister();
			}
		} else {
			autoRegister = false;
			sipreg->setAutoRegister( false );
		}
	} else {
		editRegistration();
	}
}

KSipRegistrations::~KSipRegistrations( void )
{
}

void KSipRegistrations::showIdentity( void )
{
	editRegistration();
}

void KSipRegistrations::save( void )
{
}

void KSipRegistrations::editRegistration( void )
{
	if( sipreg ) {
		setRegisterState();
		edit->setFullname( u->getUri().getFullname() );
		edit->setUsername( u->getUri().getUsername() );
//		edit->setHostname( u->getUri().getHostname() );
		edit->setHostname( u->getUri().proxyUri() );
		edit->setSipProxy( sipreg->getOutboundProxy() );
		edit->setSipProxyUsername( u->getUri().getProxyUsername() );
		edit->setAutoRegister( autoRegister );
		edit->setQvalue( sipreg->getQvalue() );
	} else {
		edit->updateState( OFFLINE );
		edit->setFullname( "" );
		edit->setUsername( "" );
		edit->setHostname( "" );
		edit->setSipProxy( "" );
		edit->setSipProxyUsername( "" );
		edit->setQvalue( "" );
		edit->setAutoRegister( true );
	}
	edit->show();
}

void KSipRegistrations::update( void )
{
	bool isDiff = false;
{
	QSettings settings;
	QString p = KStatics::dBase + userPrefix + "Registration/";
	
	// the 1st time always ask for  restart
        if ( settings.readEntry( p + "/SipUri", "" ) == "") isDiff = true;
	
	QString s = edit->getSipProxy();
	if( settings.readEntry( p + "/SipUri", "" ) != edit->getUri() ||
	    settings.readEntry( p + "/SipServer", "" ) != edit->getSipProxy() ||
	    settings.readEntry( p + "/UserName", "" ) != edit->getSipProxyUsername() ||
	    settings.readEntry( p + "/qValue", "" ) != edit->getQvalue() ) {
	    isDiff = true;
	}
	settings.writeEntry( p + "/SipUri", edit->getUri() );
	settings.writeEntry( p + "/SipServer", edit->getSipProxy() );
	settings.writeEntry( p + "/UserName", edit->getSipProxyUsername() );
	KStatics::myID=edit->getSipProxyUsername();
	if( edit->getAutoRegister() ) {
		autoRegister = true;
		settings.writeEntry( p + "/AutoRegister", "Yes");
	} else {
		autoRegister = false;
		settings.writeEntry( p + "/AutoRegister", "No");
	}
	 
	s = edit->getSipProxy();

	if (edit->ClearPW()) {
		settings.writeEntry( p + "/Password","");
		KStatics::myPW="";
	}
	
	settings.writeEntry( p + "/qValue", edit->getQvalue() );
}
	if( !sipreg && !isDiff) {
		QString uristr = edit->getUri();
		SipUri uri = SipUri( uristr );
		u = c->getUser( uri );
		if( u == NULL ) {
			u = new SipUser( c, 0, uri );
		}
		uristr = edit->getSipProxy();
		QSettings settings;
		QString p = KStatics::dBase + userPrefix + "Registration/";
		QString qvalue = settings.readEntry( p + "qValue", "" );
		sipreg = new SipRegister( u, SipUri( uristr ), expires, qvalue );
		connect( sipreg, SIGNAL( statusUpdated() ),
			this, SLOT( registerStatusUpdated() ) );
		v->updateIdentity( u, sipreg );
		c->updateIdentity( u, sipreg->getOutboundProxy() );
		QString str = edit->getSipProxyUsername();
		u->getMyUri()->setProxyUsername( str );
		if( edit->getAutoRegister() ) {
			autoRegister = true;
			sipreg->requestRegister();
		} else {
			autoRegister = false;
			sipreg->setAutoRegister( false );
		}
		if( edit->getAutoRegister() ) {
			changeRegistration();
		}
	} else {
		if( isDiff ) {
			QMessageBox::information( parent, "Identity",
				"Restart the KPhoneSI to apply identity changes." );
			exit(1);
		}
	}
}

void KSipRegistrations::changeRegistration( void )
{
	if( sipreg ) {
		if( sipreg->getRegisterState() == SipRegister::Connected ) {
			sipreg->requestClearRegistration();
		} else {
			sipreg->updateRegister();
			v->setContactsOnline();
		}
	}
}

void KSipRegistrations::unregAllRegistration( void )
{
	if( sipreg ) {
		if( sipreg->getRegisterState() == SipRegister::Connected ) {
			sipreg->requestClearRegistration();
		}
	}
}

void KSipRegistrations::setRegisterState( void )
{
	switch( sipreg->getRegisterState() ) {
		case SipRegister::NotConnected:
			edit->updateState(  UNREG );
			break;
		case SipRegister::TryingServer:
		case SipRegister::TryingServerWithPassword:
			edit->updateState( PROC_TRY );
			break;
		case SipRegister::AuthenticationRequired:
		case SipRegister::AuthenticationRequiredWithNewPassword:
			edit->updateState( AUTHREQ );
			break;
		case SipRegister::Connected:
			edit->updateState( REG );
			break;
		case SipRegister::Disconnecting:
			edit->updateState( PROC_UNREG );
			break;
		case SipRegister::Reconnecting:
			edit->updateState( PROC_REG );
			break;
	}
}

void KSipRegistrations::registerStatusUpdated( void )
{
	setRegisterState();
	if( sipreg->getRegisterState() != SipRegister::AuthenticationRequired &&
			sipreg->getRegisterState() != SipRegister::AuthenticationRequiredWithNewPassword ) {
		return;
	}
	QString authtype;
	switch( sipreg->getAuthenticationType() ) {
		case SipRegister::DigestAuthenticationRequired:
			authtype = "Digest Authentication Request"; break;
		case SipRegister::BasicAuthenticationRequired:
			authtype = "Basic Authentication Request"; break;
		case SipRegister::ProxyDigestAuthenticationRequired:
			authtype = "Proxy Digest Authentication Request"; break;
		case SipRegister::ProxyBasicAuthenticationRequired:
			authtype = "Proxy Basic Authentication Request"; break;
	}
	QString server = sipreg->getServerUri().proxyUri();
	QString sipuri = u->getUri().uri();
	
	KSipAuthenticationRequest authreq( server, sipuri, userPrefix, authtype );
	QString username = sipreg->getRegisterCall()->getProxyUsername();
	authreq.setUsername( username );
	QString password = sipreg->getRegisterCall()->getPassword();
	if( password.isEmpty() || username.isEmpty() || (sipreg->getRegisterState() == SipRegister::AuthenticationRequiredWithNewPassword) ) {
		if( authreq.exec() ) {
			if( authreq.getUsername().isEmpty() || authreq.getUsername().isEmpty() ) {
				return;
			}
			sipreg->getRegisterCall()->setProxyUsername( authreq.getUsername() );
			sipreg->getRegisterCall()->setPassword( authreq.getPassword() );
			sipreg->requestRegister( authreq.getUsername(), authreq.getPassword() );
		}
	} else {
		sipreg->requestRegister( username, password );
	}
}
QString KSipRegistrations::retrievePW(const char *b, int len) {

unsigned  char b0=0,b1=0;
char a[50];
int  i=0;
while ( i<len/2) {
switch(b[2*i]) {
    case 'a':           b0=0;          break;
    case 'V':           b0=1;          break;
    case 't':           b0=2;          break;
    case 'L':           b0=3;          break;
    case 'n':           b0=4;          break;
    case 'q':           b0=5;          break;
    case 'z':           b0=6;          break;
    case 'Y':           b0=7;          break;
    case 'b':           b0=8;          break;
    case 'j':           b0=9;          break;
    case 'x':           b0=0xa;        break;
    case 'D':           b0=0xb;        break;
    case 'A':           b0=0xc;        break;
    case 'U':           b0=0xd;        break;
    case 's':           b0=0xe;        break;
    case 'k':           b0=0xf;        break;
    }

    switch(b[2*i+1]) {
    case 'i':           b1=0;          break;
    case 'w':           b1=1;          break;
    case 'd':           b1=2;          break;
    case 'y':           b1=3;          break;
    case 'e':           b1=4;          break;
    case 'r':           b1=5;          break;
    case 'f':           b1=6;          break;
    case 'm':           b1=7;          break;
    case 'G':           b1=8;          break;
    case 'N':           b1=9;          break;
    case 'I':           b1=0xa;        break;
    case 'R':           b1=0xb;        break;
    case 'h':           b1=0xc;        break;
    case 'H':           b1=0xd;        break;
    case 'Q':           b1=0xe;        break;
    case 'l':           b1=0xf;        break;
    }

    a[i]=( (b0<<4)&0xf0) | (b1&0x0f);
    i++;
  }
  a[i]=0;
  return QString(a);
}
