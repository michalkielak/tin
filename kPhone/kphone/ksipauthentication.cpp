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
#include <iostream>
using namespace std;
#include "../dissipate2/sipcall.h"
#include "../dissipate2/sipuser.h"
#include "../dissipate2/sipregister.h"
#include "../dissipate2/sipclient.h"
#include "kphone.h"
#include "kphoneview.h"
#include "ksipauthentication.h"
#include "kstatics.h"

KSipAuthenticationRequest::KSipAuthenticationRequest(
	const QString &server, const QString &sipuri, const QString &prefix, 
	const QString &authtype, QWidget *parent, const char *name )
	: QDialog( parent, prefix + authtype, true )
{

	userPrefix = prefix;
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );
	(void) new QLabel( tr("Authentication for Entry ") + userPrefix, vbox );
	(void) new QLabel( "SipUri = " + sipuri, vbox );
	(void) new QLabel( "Server = " + server, vbox );
	(void) new QLabel( tr("Username:"), vbox );
	username = new QLineEdit( vbox );
	QSettings settings;
	QString p = KStatics::dBase + userPrefix + "Registration/";
	QString user = settings.readEntry( p + "UserName", "");
	username->setText( user);
	(void) new QLabel( tr("Password:"), vbox );
	password = new QLineEdit( vbox );
	password->setEchoMode( QLineEdit::Password );
	password->setFocus();
	savePassword = new QCheckBox( tr("Save password"), vbox );
	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 );
	okPushButton = new QPushButton( this, tr("ok button"));
	okPushButton->setText("OK" );
	okPushButton->setDefault( TRUE );
	buttonBox->addWidget( okPushButton );
	cancelPushButton = new QPushButton( this,tr( "cancel button") );
	cancelPushButton->setText( tr("Cancel") );
	cancelPushButton->setAccel( Key_Escape );
	buttonBox->addWidget( cancelPushButton );
	connect( okPushButton, SIGNAL( clicked() ), this, SLOT( okClicked() ) );
	connect( cancelPushButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

KSipAuthenticationRequest::~KSipAuthenticationRequest( void )
{
}

void KSipAuthenticationRequest::setUsername( const QString &newUsername )
{
	username->setText( newUsername );
}

void KSipAuthenticationRequest::setPassword( const QString &newPassword )
{
	password->setText( newPassword );
}

QString KSipAuthenticationRequest::getUsername( void )
{
	return username->text();
}

QString KSipAuthenticationRequest::getPassword( void )
{
	return password->text();
}

void KSipAuthenticationRequest::okClicked( void )
{

	QSettings settings;
	QString p = KStatics::dBase + userPrefix + "Registration/";
	QString ppw, dspw;
	QString user;
	user = settings.readEntry( p + "UserName", "");
	if (user.isEmpty()) {
	    user = getUsername();
	    KStatics::myID = user;
	}
	if (savePassword->isChecked()) {
		ppw = getPassword();
		dspw=scramblePW(ppw.latin1(), ppw.length() );
		settings.writeEntry( p + "Password", dspw );
		settings.writeEntry( p + "UserName", user );
		KStatics::myPW=ppw;
	} else {
		KStatics::myPW=getPassword();
	}
	accept();
}

QString KSipAuthenticationRequest::scramblePW(const char *a, int len) { 
int i;
unsigned char c[16]={'a','V','t','L','n','q','z','Y','b','j','x','D','A','U','s','k'};
unsigned char d[16]={'i','w','d','y','e','r','f','m','G','N','I','R','h','H','Q','l'};
unsigned  char b0,b1;
char b[100];
i=0;
while (i <len) {
    b0 =(a[i]&0xf0)>>4;
    b1 = a[i]&0x0f;
    b[2*i]   = c[b0] ;
    b[2*i+1] = d[b1];
    i++;
}
 b[2*i]=0; 
return QString(b);
} 

KSipAuthentication::KSipAuthentication()
{
	authreq = 0;
	execAuthreq = false;
}

KSipAuthentication::~KSipAuthentication( void )
{
}

void KSipAuthentication::authRequest( SipCallMember *member, QString up )
{
	if( member->getAuthState() != SipCallMember::authState_AuthenticationRequired &&
	    member->getAuthState() != SipCallMember::authState_AuthenticationRequiredWithNewPassword ) {
		return;
	}
	if(KStatics::debugLevel >=  2) cout << "===KSipAuthentication::authRequest\n";	

	if( execAuthreq ) return;
	
	QString u = KStatics::myID; 
	QString p = KStatics::myPW;
	if( p.isEmpty() || member->getAuthState() == SipCallMember::authState_AuthenticationRequiredWithNewPassword ) {
			QSettings settings;
			QString pst = KStatics::dBase + up + "Registration/";
			QString proxy = member->getCall()->getSipProxy();
			SipUri localuri = member->getCall()->localAddress();
			if( !authreq ) {
				authreq = new KSipAuthenticationRequest( proxy, localuri.uri(), QString::null, QString::null );
			}
			authreq->setUsername( u );
			authreq->setPassword( p );
			execAuthreq = true;
			if( authreq->exec() ) {
				u = authreq->getUsername();
				p = authreq->getPassword();
				if( u.isEmpty() || p.isEmpty() ) {
					return;
				} else {
				KStatics::myID=u;
				KStatics::myPW=p;
				}
				member->getCall()->setPassword( p );
				execAuthreq = false;
			} else {
				execAuthreq = false;
				return;
			}
	}

	switch( member->getCallMemberType() ){
		case SipCallMember::Subscribe:
			member->setAuthState(SipCallMember::authState_AuthenticationOK);
			member->sendRequestSubscribe( u, p );
			break;
		case SipCallMember::Notify:
			member->setAuthState(SipCallMember::authState_AuthenticationOK);
			member->sendRequestNotify( u, p );
			break;
		case SipCallMember::Message:
			member->setAuthState(SipCallMember::authState_AuthenticationOK);
			member->sendRequestMessage( u, p );
			break;
		case SipCallMember::Invite:
			member->setAuthState(SipCallMember::authState_AuthenticationOK);
			member->sendRequestInvite( u, p );	
			break;
		case SipCallMember::Disc:
			member->setAuthState(SipCallMember::authState_AuthenticationOK);
			member->sendRequestBye( u, p );
			break;
		case SipCallMember::Options:
			member->setAuthState(SipCallMember::authState_AuthenticationOK);
			member->sendRequestOptions( u, p );
			break;	
		default:
			break;
	}
}

 
