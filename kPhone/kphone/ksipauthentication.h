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
#ifndef KSIPAUTHENTICATION_H_INCLUDED
#define KSIPAUTHENTICATION_H_INCLUDED
#include <qdialog.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qbutton.h>
#include <qlabel.h>

#include "../dissipate2/sipcall.h"
#include "ksipregistrations.h"

/**
* @short User Authentification
*
* requests authentication userid and password, if not stored
*/
class KSipAuthenticationRequest : public QDialog
{
	Q_OBJECT
public:
/**
*	Constructor
*/
	KSipAuthenticationRequest( const QString &server, const QString &sipuri, const QString &prefix, const QString &authtype, 	QWidget *parent = 0, const char *name = 0 );
	~KSipAuthenticationRequest( void );
/**
*	set the authentication username
*/	
	void setUsername( const QString &newUsername );
/**
*	set the authentication Password
*/
	void setPassword( const QString &newPassword );
/**
*	get the authentication username
*/
	QString getUsername( void );
/**
*	set the authentication Password	
*/
	QString getPassword( void );
/**
*	make the password a bit more unreadable
*/
	QString scramblePW(const char *a, int len);

private slots:
/**
*	save your inpot
*/
	void okClicked( void );

private:
	QCheckBox *savePassword;
	QLineEdit *username;
	QLineEdit *password;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
	QString userPrefix;
};

/**
* @short User Authentification
* User Authentification on Proxy demand, 
* currently supports challenge
*/
class KSipAuthentication : public QObject
{
	Q_OBJECT
public:
/**
*	Constructor
*/
	KSipAuthentication( void );
	~KSipAuthentication( void );
	

public slots:
/**
*	after authentication is done, send the message which was authenticated again
*/	void authRequest( SipCallMember *member, QString up="" );

private:

/**
*	translate the reg state for KSipIdentity
*/
	void setRegisterState( void );
/**
*	does nothing
*/
	void save( void );
	SipClient *c;
	KSipIdentityEdit *edit;
	KPhoneView *v;
	QString userPrefix;
	SipUser *u;
	SipRegister *sipreg;
	bool autoRegister;
	int expires;
	QWidget *parent;
	bool useStun;
	QString stunSrv;
	KSipAuthenticationRequest *authreq;
	bool execAuthreq;
};

#endif // KSIPAUTHENTICATION_H_INCLUDED
