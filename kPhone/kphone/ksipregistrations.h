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
#ifndef KSIPREGISTRATIONS_H_INCLUDED
#define KSIPREGISTRATIONS_H_INCLUDED
#include <qdialog.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qbutton.h>
#include <qlabel.h>

#include "../dissipate2/sipcall.h"


class QLineEdit;
class SipUser;
class SipClient;
class SipRegister;
class KPhoneView;

/**
* @short the registration states
*
*creates the identity panel, to allow to enter the user information
*/
struct Kstate {
  public:
        enum KsipState {
            OFFLINE    = 0,
            UNREG      = 1,
            REG        = 2, 
            AUTHREQ    = 3,
            PROC_UNREG = 10,
            PROC_REG   = 11,
            PROC_TRY   = 12,
            LAST = 99
        };
};

/**
* @short the identity panel
*
*creates the identity panel, to allow to enter the user information
*/
class KSipIdentityEdit : public QDialog , public Kstate
{
	Q_OBJECT
public:
/**
*	constructor
*/
	KSipIdentityEdit( QWidget *parent, const char *name,  QObject *reg );
	~KSipIdentityEdit( void );
/**
*
*/
//	void setReadOnly( bool mode );
/**
*	update  the reg state for KSipIdentity
*/
	void updateState( KsipState state );
/**
*	register automatically if kphone comes up
*/
	void setAutoRegister( bool newAutoRegister ) { autoRegister->setChecked( newAutoRegister ); }
/**
*	see if autoregister is set
*/
	bool getAutoRegister( void ) { return autoRegister->isChecked(); }
/**
*	get the user's name 
*/
	QString getFullname( void ) const;
/**
*	get the user part of the user's SIP-URI
*/
	QString getUsername( void ) const;
/**
*	get the hostname part of the user's SIP-URI
*/
	QString getHostname( void ) const;
/**
*	get the outbound proxy
*/
	QString getSipProxy( void ) const;
/**
*	get the authentication user name
*/
	QString getSipProxyUsername( void ) const;
/**
*
*/
	QString getUri( void ) const;
/**
*	get the Q-value 
*/
	QString getQvalue( void ) const;
/**
*	set the user's name 
*/
	void setFullname( const QString &newFullname );
/**
*	set the user part of the user's SIP-URI
*/
	void setUsername( const QString &newUsername );
/**
*	set the hostname part of the user's SIP-URI
*/
	void setHostname( const QString &newHostname );
/**
*	set the outbound proxy
*/
	void setSipProxy( const QString &newSipProxy );
/**
*	set the authentication user name
*/
	void setSipProxyUsername( const QString &newSipProxyUsername );
/**
*
*/
//	void setDefault( bool newDefault );
/**
*	set the Q-value 
*/
	void setQvalue( const QString &newSipProxy );
/**
*	clear the authentication userpassword on reboot
*/
	bool ClearPW(void);

signals:

/**
*	update the settings
*/
	void update( void );

protected slots:

/**
*	ok pressed
*/
	void slotOk( void );
/**
*	cancel pressed
*/
	void slotCancel( void );

private:
	QLineEdit *fullname;
	QLineEdit *username;
	QLineEdit *hostname;
	QLineEdit *sipProxy;
	QLineEdit *sipProxyUsername;
	QLineEdit *qValue;
	QCheckBox *autoRegister;
	QCheckBox *clearPWButton;
	QLabel *labelRegister;
	QPushButton *buttonRegister;
	bool editMode;
	QPushButton *helpPushButton;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
	bool isDiff;

};
/**
* @short registration starter
*
* reads the data from the configuration file
*
* creates a SipRegister incarnation and starts the registration process
*
* controlls the provisioning of reg. data
*/
class KSipRegistrations : public QObject , public Kstate
{
	Q_OBJECT
public:
/**
*
*/
	KSipRegistrations( SipClient *client,
		KPhoneView *phoneView = 0, QWidget *p = 0, const char *name = 0 );
	~KSipRegistrations( void );
/**
*	populate the identity pannel and show it
*/
	void showIdentity( void );
/**
*	stop registration
*/
	void unregAllRegistration( void );
/**
*	check if a STUN server is used
*/
	bool getUseStun( void ) { return useStun; }
/**
*	return the stun server address
*/
	QString getStunSrv( void ) { return stunSrv; }
/**
*	unscramble the auth. password
*/
	QString retrievePW(const char *b,int len);

private slots:
/**
*	registration has changed, setup SipRegister
*/
	void changeRegistration( void );
/**
*	helps showIdentity to do its job
*/
	void editRegistration( void );
/**
*	update the settings
*/
	void update( void );
/**
*	
*/
	void registerStatusUpdated( void );

private:
/**
*
*/
	void setRegisterState( void );
/**
*
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
};

#endif // KSIPREGISTRATIONS_H_INCLUDED
