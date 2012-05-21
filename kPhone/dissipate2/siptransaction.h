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

#ifndef SIPTRANSACTION_H_INCLUDED
#define SIPTRANSACTION_H_INCLUDED

#include <qobject.h>
#include <qptrlist.h>

#include "sipuri.h"
#include "sipstatus.h"
#include "sipprotocol.h"
#include "sipurilist.h"
#include "mimecontenttype.h"

class SipClient;
class SipCall;
class SipCallMember;
class SipMessage;
class QTimer;


/**
 * @short This class represents a single SIP transaction. 
 * 
 * This class represents a single SIP transaction.
 */
class SipTransaction : public QObject
{
	Q_OBJECT

	friend class SipCall;
	friend class SipClient;
public:
	/**
	 * Creates a new SIP transaction with a given initial CSeq number, the
	 * farend call member, and the parent call.
	 */
	SipTransaction( unsigned int seqn, SipCallMember *farend, SipCall *call );

	/**
	 * SipTransaction destructor.
	 */
	~SipTransaction( void );

	/**
	 * Returns a pointer to the far end call member for this transaction.
	 */
	SipCallMember *getCallMember( void ) const { return remote; }

	/**
	 * Returns the CSeq value for this transaction (seqnum + method).
	 */
	QString getCSeq( void ) const;
	
	/**
	 * Returns the RSeq value for this transaction (seqnum). At this moment only one sending is possible
	 */
	QString getRSeq( void );

	/**
	 * Returns the RAck value for this transaction (seqnum). At this moment only one sending is possible
	 */
	QString getRAck( void );

	enum Direction {
		RemoteRequest,
		LocalRequest,
		None };

	/**
	 * Returns the Direction of this request, either
	 * SipTransaction::RemoteRequest or SipTransaction::LocalRequest.
	 */
	Direction getDirection( void ) const { return direction; }

	/**
	 * Returns the numeric part of the CSeq for this transaction.
	 */
	unsigned int getSeqNum( void ) const { return seqnum; }

	/**
	 * Returns the Method part of the CSeq for this transaction.
	 */
	QString getSeqMethod( void ) const;

	enum EvalPresence {
	    Dummy,
	    PidfXml,
	    XPidf,
	    Watcher,
	    Pidf,
	    PidfNo,
	    Cpim
	    };
	    
	/**
	 * Returns the current status of this transaction.
	 */
	const SipStatus &getStatus( void ) const { return laststatus; }

	/**
	 * Returns true if this request was cancelled.
	 */
	bool wasCancelled( void ) const { return cancelled; }

	/**
	 * Returns the initial request which initiated this transaction.
	 * Useful for extracting meaning to the whole transaction.
	 */
	SipMessage *getRequest( void ) const { return requestmessage; }

	/**
	 * Sends a SIP response under this transaction.
	 */
	void sendResponse( const SipStatus &status,
		const bool prack = false,
		const QString RefSub = "true",
		const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );
	/**
	* prepare the response to an option request
	*/
	void sendResponseOP( const SipStatus &status, const QString &body = QString::null);
	/**
	* prepare the 302 response on CF
	*/
	void sendResponseNC( const SipStatus &status,
		const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );

	/**
	 * Cancels this request transaction by sending a CANCEL request.
	 */
	void cancelRequest( const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );

	/**
	 * Returns the last applicable message body received.  Useful for
	 * getting the session description, or the reason for the redirect, or
	 * whatever.  Be sure to check the content-type as well.
	 */
	QString getFinalMessageBody( void );
	/**
	* gets the body of the request message
	*/
	QString getUpdateMessageBody( void );

	/**
	 * Returns the MIME content-type of the final message body.
	 */
	MimeContentType getFinalContentType( void );
	/**
	*Returns the MIME content-type of the request message body.
	*/	
	MimeContentType getUpdateContentType( void );

	/**
	 * Returns the contact list of the final response.
	 */
	SipUriList getFinalContactList( void );
	/**
	* Returns the WWWAuthString of the final response.
	*/
	QString getFinalWWWAuthString( void );
	/**
	* Returns the FinalProxyAuthString of the final response.
	*/
	QString getFinalProxyAuthString( void );

	/**
	 * Returns the body of the request message.
	 */
	QString getRequestMessageBody( void ) const;

	/**
	 * Returns the MIME content-type of the request message body.
	 */
	MimeContentType getRequestMessageContentType( void ) const;
	
	/** 
	* shorten repetition of invites
	*/
	void stopInviteRep(void) { inviteRep = 1;}
	/** 
	* get the minExpires value
	*/
	int getMinExpires(void);
	/** 
	* get the SipEtag previously stored
	*/
	QString getSipIfMatch(void);

signals:
	/**
	* signal a status change
	*/
	void statusUpdated( void );

private slots:
	/**
	* send a 202 response
	*/
	void send_202( void );

private:
	/**
	* find the xmls part of the body
	*/
	QString getxmls(const QString which, const QString mess);
	
	SipCall *parent;
	SipMessage *requestmessage;
	QPtrList<SipMessage> responses;

	// Last status for this transaction
	SipStatus laststatus;

	unsigned int seqnum;
	unsigned int rseq;
	QString branch;
	SipCallMember *remote;
	Direction direction;
	bool cancelled;
	QTimer *timer_202;
	/**
	* set the buddy status
	*/
	void setStatus( const SipStatus &stat );

	// For SipCall
	/**
	* evaluate an incoming requst
	*/
	void incomingRequest( SipMessage *message, bool holdmaster );
	/**
	* evaluate an incoming request
	*/
	void incomingRequestRetransmission( SipMessage *message );
	/**
	* evaluate an incoming response
	*/
	void incomingResponse( SipMessage *message );
	/**
	* prepare a REGISTER request
	*/
	void sendRegister( const SipUri &registerserver, int expiresTime,

		const QString &authentication = QString::null,
		const QString &proxyauthentication = QString::null,
		const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null,
		const QString &qValue = QString::null );

	// Publish
	int atomId;
	/**
	* prepare a PUBLISH request 
	*/
	void sendPublish(const QString state, const QString sipIfMatch, const SipUri &publishserver, int expiresTime, const QString &authentication = QString::null, const QString &proxyauthentication = QString::null, const QString &body = QString::null );
	QString	SipETag;
	int minExpires;
	/**
	* prepare a SIP request
	*/
	bool sendRequest( Sip::Method meth,
		const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null,
		const SipUri &referto = SipUri::null,
		const bool shortTr = false,
		const QString &proxyauthentication = QString::null,
		const QString &authentication = QString::null,
		const int expiresTime = -1
	 );

	// For SipClient
	/**
	* support for audit
	*/
	bool auditPending( void );

//	QString stateText( QString state );
	
	unsigned int inviteRep;
	/**
	* Evaluate the presence body
	*/
	void handlePresence(enum EvalPresence cont, bool p, QString m);
	/**
	* Evaluate the presence body format pidf
	*/
	void getpidf(const QString m);
	/**
	* Evaluate the presence body format winfo
	*/
	void getWInfo(const QString m);
	/**
	* grep the watcher out of a winfo body
	*/
	void getWatcher(const QString m,int state);
	/**
	* Evaluate the presence body format cpim
	*/
	void getcpim(QString);
	/**
	* history
	*/
	void getpidfxml(QString);

};

#endif // SIPTRANSACTION_H_INCLUDED
