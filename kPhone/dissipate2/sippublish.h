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
#ifndef SIPPUBLISH_H_INCLUDED
#define SIPPUBLISH_H_INCLUDED

#include <qobject.h>
#include <qptrlist.h>
#include <qtimer.h>
#include <qdatetime.h>

#include "sipuri.h"

class QTimer;
class SipCall;
class SipCallMember;
class SipUser;
class SipClient;
class SipTransaction;

/**
 * Class creates PUBLISH requests and analyses the responses on them
 */
class SipPublish : public QObject
{
	Q_OBJECT
public:

	enum PublishState {
		NotConnected,  //0
		Connected,     //1
		Disconnecting, //2
		Reconnecting,  //3
		pubPending,    //4
		pubPublished,  //5 
		AuthenticationRequired,                 //6
		AuthenticationRequiredWithNewPassword,  //7
		TryingServerWithPassword,               //8  
		TryingServer,                           //9
		PublishFailed
};		
	/**
	 * Creates a new SipPublish object representing the registration with
	 * the given proxy.
	 */
	SipPublish( SipUser *user, const QString serveruri, int PubExp, int actT);

	/**
	 * SipPublish destructor.
	 */
	~SipPublish( void );

	
	/**
	 * Sets the URI of the registration server.
	 */
	void setOutboundProxyUri( const SipUri &newpubserver );

	/**
	 * Requests publication from the publish server.
	 */
	void requestPublish( const QString state, const QString &username = QString::null,
		const QString &password = QString::null);
	/**
	*	prepare for requestPublish
	*/
	void publishRequest(const QString state);
	

	/**
	 * Returns pointer to transaction.
	 */
	SipTransaction *getCurtrans( void ) { return curtrans; }

//	void updateContact( void );
	/**
	*	construct publish's body
	*/
	QString buildPublishBodyPIDF(const QString state);
	/**
	*	fetch the PublishStat
	*/	
	enum PublishState getPubState(void) {return pubstate;}
	
signals:
	/**
	*	signal the state of emmiting a PUBLISH
	*	used to react on the response codes
	*/
	void statusUpdated( void );

public slots:
	/**
	* used for the "user activ" feature
	*/
	void inactive( const QDateTime lastActive );
	/**
	* used for the "user activ" feature
	*/
	void active( const QDateTime lastActive );

private  slots:
/**
*	react on the statusUpdated signal
*/
	void localStatusUpdated( void );
/**
*	publish refresh timer timed out
*/
	void publish_timeout( void );


private:
	QTimer	*publishtimer;
	int publishExpiresTime;
	enum PublishOp {
		PubNull,
		PubInitial,
		PubRefresh,
		PubModify,
		PubRemove };
	PublishOp pubop;
	
	PublishState pubstate;


	enum AuthType {
		BasicAuthenticationRequired,
		DigestAuthenticationRequired,
		ProxyBasicAuthenticationRequired,
		ProxyDigestAuthenticationRequired 
};
	AuthType authtype;

	QString iniState;
	QString curState;
	QString newState;
	QString curNote;
	bool pubQueue;
	SipUser *pubUser;	
	SipCallMember *pubCallMember;
	SipCall *pubcall;
	SipTransaction *curtrans;
	SipTransaction *pubtrans;
	SipUri *pubserver;	
	SipUri *presuri;
	QString sipIfMatch;
 	QString authstr;
	QString proxyauthstr;

	QString authresponse;
	QString proxyauthresponse;

	QString outboundProxy;
	QString tuple;
	QString person;
	bool initPublish;
	int pubExpires;
	bool isActive;
	bool useActive;
	QDateTime _lastActive;
	int noncecounter;
};


#endif // SIPPUBLISH_H_INCLUDED
