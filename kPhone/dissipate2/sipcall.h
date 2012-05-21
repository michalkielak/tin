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

#ifndef SIPCALL_H_INCLUDED
#define SIPCALL_H_INCLUDED

#include <qobject.h>
#include <qptrlist.h>

#include "sipuri.h"
#include "sipurilist.h"
#include "mimecontenttype.h"
#include "../kphone/sessioncontrol.h"

class SipCall;
class SipUser;
class SipClient;
class SipCallId;
class SipMessage;
class SipTransaction;
class TCPMessageSocket;

/**
 * @short Object to reference a member of a call
 *
 * Object to reference a member of a call.  Contains the remote URI, their
 * current Contact URI, and their current status in the call.  It also contains
 * their current session description, and a copy of our local session
 * description for this call leg.
 *
 * This class also provides an API for modifying call state to this member of
 * the call, such as sending an invitation to join the session, or accepting a
 * call.
 *
 * The authentication logic is hosted here, too
 *
 */
class SipCallMember : public QObject
{
	Q_OBJECT

	friend class SipCall;
public:


	/**
	 * Creates a new member for this call with the given URI. Initially,
	 * this sets the Contact URI for the call member to be the same.
	 */
	SipCallMember( SipCall *parent, const SipUri &uri );

	/**
	 * Call member destructor.
	 */
	~SipCallMember( void );

	enum CallMemberType {
		unknown,
		Subscribe,
		Notify,
		RNotify,
		Message,
		Invite,
		Disc,
		Options,
		Prack,
		Update };

	enum State {
		state_Idle,
		state_EarlyDialog,
		state_Connected,
		state_Disconnected,
		state_InviteRequested,
		state_ReInviteRequested,
		state_RequestingInvite,
		state_RequestingReInvite,
		state_Redirected,
		state_Disconnecting,
		state_CancelPending,
		state_Refer,
		state_Refer_handling,
		state_r202,
		state_r100,
		state_r200,
		state_RDisconnected,
		state_Prack,
		state_Update,
		state_Updated
		
		};

	enum AuthState {
		authState_Authentication,
		authState_AuthenticationOK,
		authState_AuthenticationRequired,
		authState_AuthenticationTrying,
		authState_AuthenticationTryingWithPassword,
		authState_AuthenticationRequiredWithNewPassword };

	enum AuthType {
		ProxyBasicAuthenticationRequired,
		ProxyDigestAuthenticationRequired,
		DigestAuthenticationRequired};

	
	/**
	 * Returns the current state of the call member.
	 */
	State getState( void ) const { return state; }
	State getSaveState( void ) const { return savestate; }

	/**
	 * Returns the most recent session description provided by the member
	 * for sending media to them.
	 */
	QString getSessionDescription( void ) const { return sessiondesc; }

	/**
	 * Returns the MIME Content Type of the session description provided by
	 * the call member. If none was provided, this function returns NULL.
	 */
	MimeContentType getSessionDescriptionType( void ) const { return sessiontype; }

	/**
	 * Returns the most recently sent local session description. Provided
	 * for reference.
	 */
	QString getLocalSessionDescription( void ) const { return localsessiondesc; }

	/**
	 * Returns the MIME Content Type of the most recently sent local
	 * session description.
	 */
	MimeContentType getLocalSessionDescriptionType( void ) const { return localsessiontype; }

	/**
	 * Returns a text description of our current status. Basically, this
	 * is the text from the response line of the last message we received,
	 * or a text description of what we're currently doing or waiting for.
	 * Useful for showing the user what is going on.
	 */
	QString getLocalStatusDescription( void ) const { return statusdesc; }
	/** 
	*numeric Value of teh local status description
	*/
	int getLocalStatusCode ( void ) const { return statuscode; }
	void clearLocalStatusCode ( void) { statuscode=0; }
		/**
	 * Returns the most recent message body we received that was not a
	 * session description.
	 */
	QString getMostRecentMessageBody( void ) const { return recentbody; }

	/**
	 * Returns the MIME type of the most recent message body we received
	 * that was not a session description.
	 */
	MimeContentType getMostRecentMessageBodyType( void ) const { return recentbodytype; }
	
	/**
	 * SUBSCRIBE
	 */
	void requestSubscribe( int expiresTime = 0, const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );
	/**
	 * request authenication
	 */	
	void requestAuthSubscribe( void );
	/**
	 * request the end of a subscription	
	 */
	void requestClearSubscribe( bool setTimer);
	/**
	 * prepare and send the Subscribe message
	 */
	void sendRequestSubscribe( QString username = QString::null, QString password = QString::null );
	/**
	 * handle an incoming subscribe message
	 */
	void handlingSubscribeResponse( void );
	/**
	 * unused ???
	 */
	int getLocalExpiresTime(void) {return localExpiresTime;}

	/**
	 * NOTIFY for Refer
	 */
	void requestRNotify( const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null, QString username = QString::null, QString password = QString::null);
	/**
	 * NOTIFY
	 */
	void requestNotify( const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );
	/**
	 *  prepare and send the NOTIFY message
	 */
	void sendRequestNotify( QString username = QString::null, QString password = QString::null );
	/**
	 *  request authenication
	 */
	void requestAuthNotify( void );
	/**
	 * Notify with duration 0
	 */
	void requestClearNotify( const QString &body, const MimeContentType &bodytype );
	/**
	 * Evaluation of the incoming NOTIFY
	 */
	void handlingNotifyResponse( void );

	/**
	 * Evaluation of the incoming NOTIFY for refer
	 */
	 void handlingRNotifyResponse (void);
	
	/**
	 * prepare and send BYE
	 */
	void sendRequestBye( QString username, QString password );
	
	/**
	 * MESSAGE
	 */
	void requestMessage( const QString &body, const MimeContentType &bodytype );
	/**
	 * prepare and send MESSAGE
	 */
	void sendRequestMessage( QString username = QString::null, QString password = QString::null );
	/**
	 *   request authenication
	 */
	void requestAuthMessage( void );
	/**
	 * Evaluation of the incoming MESSAGE
	 */
	void handlingMessageResponse( void );

	/**
	 * PRACK
	 */
	void requestPrack( void );
	/**
	 *  prepare and send PRACK
	 */
	void sendRequestPrack( QString username = QString::null, QString password = QString::null );
	/**
	 *    request authenication
	 */
	void requestAuthPrack( void );

	/**
	 * Evaluation of the incoming  PRACK
	 */
	void handlingPrackResponse( void );
	/**
	 * Sends a SIP INVITE request, asking the member to join in the session
	 * described in the given body. The body and MIME type provided will
	 * become the new local session description for this call member.
	 */
	bool requestInvite( const QString &body, const MimeContentType &bodytype );
	/**
	 * reInvitation
	 */
	bool requestReInvite( const QString &body, const MimeContentType &bodytype );
	/**
	 *   prepare and send INVITE
	 */
	bool sendRequestInvite( QString username = QString::null, QString password = QString::null );
	/**
	 *    request authenication
	 */
	void requestAuthInvite( void );
	/**
	 *  Evaluation of the incoming  INVITE
	 */
	void handlingInviteResponse( void );

	/**
	 * Sends a SIP BYE request to disconnect the session.
	 */
	void requestDisconnect( void );

	/**
	 * Requests to establish a session to another party by sending REFER
	 * we distinguish between the "short transfer", where no Notification is necessary and we go into state_Disconnect
	 * and the standard case where where we go into state_r202* 
	 */
	void requestRefer( bool shortTr, const SipUri &referto, const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );

	/**
	 * Sends a SIP OPTIONS request, asking the member what they support.
	 * The body and MIME type provided serve no known purpose at this time.
	 * The response to the OPTIONS request will become the new remote
	 * session description, so this should not be called on an active call.
	 * It is provided here for consistency.
	 */
	void requestOptions( const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );
	/**
	 *    prepare and send OPTIONS 
	 */
	void sendRequestOptions( QString username = QString::null, QString password = QString::null );
	/**
	 *     request authenication
	 */
	void requestAuthOptions( void );
	/**
	 *   Evaluation of the incoming OPTIONS
	 */
	void handlingOptionsResponse( void );
	/**
	 * Handles incoming UPDATE
	 */		
	void acceptUpdate( const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );
	/**
	 * send response: 488, "Not Acceptable Here";
	 */	
	void noUpdateHere(void);
		
	/**
	 * Accepts the invitation to join the session sent by the call member.
	 * The body and MIME type provided will become the new local session
	 * description for this call member.
	 */
	void acceptInvite( const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );
	/**
	 * refuse to accept an incoming Invite
 	 * but save some information 
	 */
	void refuseInvite( void );

	/**
	 * Declines the invitation to join the session sent by the call member.
	 * The body and MIME type provided are for possibly giving a reason as
	 * to why the call was declined.
	 */
	void declineInvite( const QString &body = QString::null,
		const MimeContentType &bodytype = MimeContentType::null );
	/**
	 * send response 302 
	 */
	void forwardCall( const QString &body);
		
	/**
	 * Returns the URI for this call member.
	 */
	const SipUri &getUri( void ) const { return memberuri; }

	/**
	 * Returns the current Contact URI for this call member.
	 */
	const SipUri &getContactUri( void ) const { return contacturi; }

	/**
	 * Sets the Contact URI for this call member.
	 */
	void setContactUri( const SipUri &newcontact );

	/**
	 * Updates the URI for this call member.
	 */
	void setUri( const SipUri &newuri );

	/**
	 * Returns the list of URIs where we were redirected.
	 */
	const SipUriList &getRedirectList( void ) { return redirectlist; }

	/**
	 * Cancel Transaction.
	 */
	void cancelTransaction( void );

	/**
	 * Returns the subject for this call members call.
	 */
	QString getSubject( void );

	/**
	 * Update presence and call state
	 */
	void contactUpdate( bool active, QString presence = "" );

	/**
	 * Subscription response  supervision timer
	 */
	void timerStart( int time );


	/**
	 * Call cannot be accepted by user
	 */
	void notAcceptableHere( void );

	/**
	 * get a pointer to a SipCall incarnation
	 */
	SipCall *getCall( void ) { return call; }

	/**
	 * set the call member type (see enum CallMemberType for contents)
	 */
	void setCallMemberType( CallMemberType type ) { callMemberType = type; }

	/**
	 * set the call member type (see enum CallMemberType for contents)
	 */
	CallMemberType getCallMemberType( void ) { return callMemberType; }

	/**
	 * what auth type is requested from our server?
	 */
	AuthType getAuthenticationType( void ) { return authtype; }

	/**
	 * in which state of the  are we?
	 */
	AuthState getAuthState( void ) { return authstate; }

	/**
	 * set the actual auth process state
	 */
	void setAuthState( AuthState au ) { authstate=au; }

	/**
	 * Set the operation to opIdle
	 */
	void setIdle( void ) { op = opIdle; }

	/**
	 * set the call progress state (see enum State)
	 */
	void setState( State newstate ) { state = newstate; }

	/**
	 * not used?
	 */
	bool refuseHold(void) {return holdRefused;}

	/**
	 * do we allow CT without implcite subscription of call states
	 */
	bool isShortTrans(void);
	

	/**
	 * include RSeq in 18x ???
	 */
	bool isPPrack(void);


signals:
	/**
	 * This signal is sent whenever the status of this call member has
	 * changed.
	 */
	void statusUpdated( SipCallMember *member );

private slots:
	/**
	 * reacts upon call member state changes
	 */
	void localStatusUpdated( void );
	/**
	 * reacts upon call member state changes
	*  while UPDATE processing
	 */
	void UpdateStatusUpdated( void );
	/**
	 * used to cancel a incoming transaction, if the call was canelled
	 */
	void remoteStatusUpdated( void );
	/**
	 * rerequest driver for subscriptions
	 */
	void call_timeout( void );

private:

	enum Operation {
		opIdle,
		opRequest,
		opClearRequest };
	Operation op;

	// For SipCall
	void incomingTransaction( SipTransaction *newtrans );
	SipUri memberuri;
	SipUri contacturi;

	CallMemberType callMemberType;
	State state;
	State savestate;
	AuthState authstate;
	AuthType authtype;

	SipCall *call;
	SipTransaction *local;
	SipTransaction *remote;
	SipTransaction *saveinvite;
	SipUriList redirectlist;
	QString sessiondesc;
	MimeContentType sessiontype;
	QString localsessiondesc;
	MimeContentType localsessiontype;
	QString statusdesc;
	int statuscode;
	QString recentbody;
	MimeContentType recentbodytype;
	QString proxyauthstr;
	QString proxyauthresponse;
	QString authresponse;
	int localExpiresTime;
	QTimer *timer;
	int curPid;
	bool holdRefused;
	//to avoid crossed  holds
	bool masterHold;
	QString ourUsername;
	QString ourPassword;
	int nonceCounter;
	int challengeCounter;
};

typedef QListIterator<SipCallMember> SipCallMemberIterator;

	/**
	 * watcher list subscription processing
	 * per watched peer one incarnation is generated and queued
	 */
class WatcherInfo
{
public:
	/**
	 * Constructor
	 */
	WatcherInfo( const QString u, const QString e, const QString s );
	~WatcherInfo( void );
	QString uri;
	QString event;
	QString state;
};

typedef QListIterator<WatcherInfo> WatcherInfoIterator;

/**
 * @short A class for referencing a speficic SIP call.
 * @author Billy Biggs <bbiggs@div8.net>
 *
 * A class for referencing a specific SIP call.
 *
 * It also serves al a database for the transient data of the call
 */
class SipCall : public QObject
{
	Q_OBJECT

	friend class SipClient;
	friend class SipTransaction;
	friend class SipCallMember;
public:
// Call Types, please adapt the handling routines, if you change the sequence or add new ones
	enum CallType {
		GarKeinCall,
		OptionsCall,
		RegisterCall,
		PublishCall,
		MsgCall,
		BrokenCall,
		UnknownCall,
		outSubscribeCall,
		inSubscribeCall,
		inSubscribeCall_2,
		winfoSubscribeCall,
		StandardCall, //first Payload-Type 11
		videoCall,
		auviCall,
		extCall1, 
		extCall2,
		extCall3,
		extCall4, //last callType
		putMeOnHold, //used for media negotiation to signal a Call Hold condition, must not be stored
		noChange     //used for media negotiation to inform, that payload type did noch change,  must not be stored 19
	};
	
typedef  enum CallType CallT;
	
	enum HoldState {
		preCall,
		callActive,
		callHold
	};
	enum SipPrackState {
		SPSsup, //support PRACK 		0
		SPSreq, //require PRACK			1
		SPSno,  //State no Prack		2
		SPSsent //Request PRACK sent		3
	};
	typedef enum SipPrackState PrackST;

	enum SipPresenceState {
		PsIdle,		//	0
		PsActive,	//	1
		PsTerminated,	//	2
		PsTerminating,  //	3
		PsDead		//	4
	};
	typedef enum SipPresenceState PresST;
	
	/**
	 * Creates a new call for a given SipUser.
	 */
	SipCall( SipUser *local, const QString &id = QString::null, CallType ctype = UnknownCall );

	/**
	 * SipCall destructor. Will not attempt to send any BYEs (yet).
	 */
	~SipCall( void );
	/**
	 * 
	 */	
	bool isSwitchThru(void) { return switchthru;}
	/**
	 * 
	 */	
	void setSwitchThru(bool sw) {switchthru = sw;}
	
	
	/**
	*  The Calltype is a Payload Call (StandardCall,videoCall,auviCall,
	*    	                          extCall0,extCall1,extCall2,extCall3,extCall4)
	*/
	bool loadCall(void);

	/**
	*  The Calltype is a Payload Call (StandardCall,videoCall,auviCall)
	*  or  extCall0,extCall1,extCall2,extCall3,extCall4 with activeOnly Flag
	*/
	bool blockCall(void);

	/**
	*  The Calltype is a Call with Audip (StandardCall,videoCall,auviCall)
	*/
	bool withAudioCall(void);
	
	/**
	*  The Calltype is a Call with external Payload  (videoCall,auviCall,extCall0,extCall1,extCall2,extCall3,extCall4)
	*/
	bool withExtCall(void);
	/**
	 * is this a ext. call?
	 */	
	bool isExtCall(void);
	
	/**
	 * Get the cause from Notify (refer) 
	 */
	QString getRefNoti(void) {return refnoti;}
	
	/**
	 * Set the cause for Notify (refer) 
	 */
	void    setRefNoti(QString body) { refnoti = body;}
	
	/**
	 * Get Refer-Sub Content
	 */
	QString getRefSub(void) {return refsub;}
	
	/**
	 * Set the Ref-Sub Content 
	 */
	void    setRefSub(QString body) { refsub = body;}

	/**
	 * save CallType if allowed, return CallType 
	*/
	 int  saveLoadType (int load);
	
	/**
	 * Set the Flag  CMD line used to transfer session parameters
	*/	
	void setUseLine(bool ucl){ useCmdLine =ucl;} 
	
	/**
	 * Get the Flag  CMD line used to transfer session parameters
	*/	
	bool getUseLine(void) { return useCmdLine;}
	
	/**
	 * store the CMD line used to transfer session parameters
	*/	
	void setCmdLine(const QString	cml) {theCmdLine = cml;}
	
	/**
	 * retrieve the CMD line used to transfer session parameters
	*/	
	const QString getCmdLine(void) { return theCmdLine;}	
		

	/**
	 * get our payload session's  PID to kill it
	*/
	int getCurPid(void)  {return curPid; }
	
	/**
	 * store the PID of our payload session
	*/
	void setCurPid (int pid) { curPid=pid; }
	
	/**
	 * Sets the Hold Initiator Flag.
	 */
	void	setHoldMaster(bool hmaster);
	
	/**
	 * Returns the Hold Initiator Flag
	 */
	bool	getHoldMaster(void);

	/**
	 * Sets the  Hold  State.
	 */
	void setHoldCall(HoldState hstate);// {onHold=hstate;}

	/**
	 * Returns the Hold  State
	 */
	HoldState getHoldCall(void){return onHold;}
	
	/**
	 * Returns true, if on Hold
	 */
	bool isHeld (void) {return onHold==callHold;}	
	/**
	 * Returns true, if Call is not on Hold and not uninitialized
	 */
	bool	isActive(void) { return onHold == callActive;}
	
	/**
	 * Returns  the no  Hold  Flag.
	 */
	bool	getNoHold(void){ return noHold;}

	/**
	 * Sets the no Hold  Flag.
    	 */
	void	setNoHold(bool nohold);
	
	/**
	 * Sets the allow Transfer  Flag.
	 */
	void	setTransAllowed(bool tstate) {tState=tstate;}

	/**
	 * Returns the allow Transfer  Flag.

	 */
	bool	getTransAllowed(void) {return tState;}
		
	/**
	 * Returns the prack request indication.
	 */
	PrackST	getPrackReq(void) {return prack;}

	/**
	 * Clear Indicator.
	 */
	void setPrackReq(PrackST pr) {prack = pr;}

	/**
	 * Presence State.
	 */
	void setPresState(PresST ps) {pstate = ps;}

	/**
	 * Presence State.
	 */
	PresST getPresState(void) {return pstate;}
	
	/**
	 * Returns the rSeq.
	 */
	QString	getRSeq(void) {return rseq;}

	/**
	 * sets the rSeq.
	 */
	void setRSeq(QString rs) {rseq=rs;}
	/**
	 * Sets the saved CSeq.
	 */
	void setSCSeq(QString cs) { sCSeq = cs; }

	/**
	 * Returns the saved CSeq.
	 */
	QString	getSCSeq(void) { return sCSeq; }

	/** 
	* incoming call ackepted
	* used for UPDATE processing
	*/
	void setAck (bool ak) { isAck = ak; }

	/** 
	* is incoming call ackepted
	*/
	bool getAck (void) { return isAck; }
	
	enum CallStatus {
		callDisconnecting,
		callUnconnected,
		callInProgress,
		callDead };
	/**
	 * set the call's status (Type CallStatus)
	 */	
	void setCallStatus( CallStatus status ) { callstatus = status; }
	/**
	 * get the call's status (Type CallStatus)
	 */
	CallStatus getCallStatus( void ) { return callstatus; }
	/**
	 * get the base presence status (true/false)
	 */
	bool getBasePresenceStatus( void ) const { return basePresenceStatus; }
	/**
	 * translate the presence status to text strings
	 */
	QString getTrPresenceStatus( void );
	/**
	 * is the buddy active?
	 */
	QString getActivityStatus( void ) const { return activityStatus; }
	/**
	 * get the presence status
	 */
	QString getPresenceStatus( void ) const { return presenceStatus; }
	/**
	 * set the base presence status (true/false
	 */
	void setBasePresenceStatus( bool status ) { basePresenceStatus = status; }
	/**
	 * set the presence status 
	 */
	void setPresenceStatus( QString status );// { presenceStatus = status; }
	/**
	 * set the buddy's activity status
	 */
	void setActivityStatus( QString status ) { activityStatus = status; }
	/**
	 * get the type of the payload (audio, video,...)
	 */
	CallType getCallType( void ) const { return calltype; }
	/**
	 * set the type of the payload
	 */
	void setCallType( CallType newtype );
	/**
	 * set the type of the payload input is int
	 */
	void setCallTrType( int newtype );
	/**
	 * Unused?
	 */
	void setInvoker( QString toexec );
	/**
	 * Unused?
	 */
	QString getInvoker( void );

	/**
	 * This returns the local URI that we are known by for this call.
	 * If there is none, returns the URI of our client parent.
	 */
	const SipUri &localAddress( void ) { return localuri; }

	/**
	 * Returns the associated call ID.
	 */
	const QString &getCallId( void ) { return callid; }

	/**
	 * Tries to find a @ref SipCallMember for the given URI.
	 */
	SipCallMember *getMember( const SipUri &uri );

	/**
	 * Returns the subject for this call.
	 */
	QString getSubject( void ) const { return subject; }

	/**Authentication
	 * Set the subject for this call. Will be used on all outgoing INVITEs
	 * sent afterwards.
	 */
	void setSubject( const QString &newsubject );

	/**
	*return the contact URI 
	*/
	SipUri getContactUri( void );
	/**
	 * Returns the contact string for this call.
	 */
	QString getContactStr( void ) const { return contactstr; }

	/**
	 * Set the contact string for this call.
	 */
	void setContactStr( const QString &newcontactstr ) { contactstr = newcontactstr; }

	/**
	 * Sends a SIP request under this call to the given call member.
	 * Returns a @ref SipTransaction for tracking.
	 */
	SipTransaction *newRequest( SipCallMember *member, Sip::Method meth,
			const QString &body = QString::null,
			const MimeContentType &bodytype = MimeContentType::null,
			const SipUri &referto = SipUri::null,
			const QString &proxyauthentication = QString::null,
			const QString &authentication = QString::null,
			int expiresTime = -1,
			bool shortTr = false );

	/**
	 * Sends a SIP register under this call to the given call member.
	 * Returns a @ref SipTransaction for tracking.
	 */
//	SipTransaction *newRegister( const SipUri &registerserver, int expiresTime,
	/**
	 * generates a SipTransaction instance and sends REGISTER
	 */
	SipTransaction *newRegister(
			const SipUri &registerserver,
			SipCallMember *callMember,
			int expiresTime,
			const QString &authentication = QString::null,
			const QString &proxyauthentication = QString::null,
			const QString &qvalue = QString::null,
			const QString &body = QString::null,
			const MimeContentType &bodytype = MimeContentType::null );
/**
	 * Sends a SIP publish under this call to the given call member.
	 * Returns a @ref SipTransaction for tracking.
*/
	SipTransaction *newPublish(
		SipCallMember *member,
		const QString state,
		const QString sipIfMatch,
		const SipUri &publishserver,
		int expiresTime,
		const QString &authentication = QString::null,
		const QString &proxyauthentication = QString::null,
		const QString &body = QString::null
	);

	/**
	 * Returns an iterator for the list of all current members of thisAuthentication
	 * call.
	 */
	SipCallMemberIterator getMemberList( void ) const { return SipCallMemberIterator( members ); }

	/**
	 * Returns ProxyUsername
	 */
	QString getProxyUsername( void );

	/**
	 * Returns Hostname
	 */
	QString getHostname( void );

	/**
	 * Returns uri of SipProxy
	 */
	QString getSipProxy( void );

	/**
	 * Returns uri of SipProxy
	 */
	QString getSipUri( void );

	/**
	 * Sets ProxyUsername.
	 */
	void setProxyUsername( QString newUsername );

	/**
	 * Returns password or QString:null if password not given for this call.
	 */
	QString getPassword( void );

	/**
	 * Sets the password for this call.
	 */
	void setPassword( QString newPassword );

	/**
	 * Returns pointer to localuri.
	 */
	SipUri *getPointerToLocaluri( void ) { return &localuri; }

	/**
	 * Sets the body type for presence.
	 */
	void setPresBody( int pb ) {presbody =pb; }

	/**
	 * Returns the body type for presence.
	 */
	int getPresBody( void) {return  presbody; }
	/**
	 * set a KCallWidget's hide flag
	 */
	void hideCallWidget( void );
	/**
	 * move the SDP body around
	 */
	void setSdpMessageMask( QString body ) { bodyMask = body; }
	/**
	 * move the SDP body around
	 */
	QString getSdpMessageMask( void ) { return bodyMask; }
	/**
	 * raise SipClient's updateSubscribes
	 */
	void updateSubscribes( void );
	/**
	 * raise SipClient's terminateSubscribe
	 */
	void terminateSubscribes( int mess );
	/**
	 * Set me active (artivicially)
	 * used for external applications
	 */
	void onlyMeActive(bool me);
	/**
	 * I was (artivicially) set as active
	 * used for external applications
	 */
	bool isOnlyMeActive(void) { return activeOnly; } 
/**
* set / reset allowance for remote start
*/
	void setRemoteStart(bool allow){ RemoteStart  = allow ; } 
/**
* is remote start allowed for this CallType
*/
	bool isRemoteStart(void){ return  RemoteStart; }  

/**
	* Publish Server ??
	*/
	bool isPubServer(void) {return havePubServer;}
/**
	* set Publish Server 
	*/
	void setPubServer(bool ps) {havePubServer = ps;}

/**
*	havePidf
^*/
	void setPidf(bool sp) { havepidf = sp; }
	/**
	 * 
	 */
	bool getPidf(void) { return havepidf; }

	WatcherInfoIterator getWIterator() const { return WatcherInfoIterator(myWatchers);}

	/**
	 * Evaluates PrackState and RAck from 
	 * incoming Invite or 18x
	 */
	void setRAck(SipMessage *message);

	/**
	 * get (previously sav)ed  RAck 
	 */
	QString getRAck(void) { return rack;};
	/**
	 * get the UPDATE status 
	 */
	int getUpdateST(void) { return updateST;};
	/**
	 * return a pointer to our SipClient
	 */
	SipClient *getSipClient( void ) { return parent; }
	
	/**
	 * supervision of clear subscribe
	 */
	void notiTimerStart( int time );

	/**
	 * used for call routing
	 * if true send the message to the host given by the to uri
	 */
	void setSendToURI(bool st);
	/**
	 * check if the feature "implicit subscription for refer is active
	 */
	bool getIsb(void);
	/**
	 * prach to be used
	 */	
	int getPrack(void);
	/**
	 *  prach to be used, even if our peer does not enfoce it
	 */
	int getPPrack(void);
	/**
	 * auth password cleanup
	 */
	void stopAutoRegistration(void);

signals:
	/**
	 * Triggered whenever the call status changes.
	 */
	void callStatusUpdated( void );

	/**
	 * Triggered whenever the call subject changes.
	 */
	void subjectChanged( void );
	
private slots:
	/**
	 * react on clear subscribe supervision timeout
	 */
	void noti_timeout( void );


private:
	QTimer *notitimer;

	bool havepidf;
	bool havePubServer;
	bool useIPv6;
 	bool RemoteStart;  
	bool send2uri;
	int curPid;
	int extPay;	
	
	
	bool masterHold;
	HoldState onHold; 
	bool noHold;
	bool activeOnly;
	bool tState;
	bool switchthru;
	
	bool useCmdLine;
	QString theCmdLine;
	PrackST prack;
	QString rack;
	QString rseq;
	QString sCSeq;
	PresST pstate;
	// Our client
	SipClient *parent;
	// Localuri defines who we are for this call. This is generated from
	// the given SipUser, but after that we have no more association with
	// the SipUser object
	SipUri localuri;

	// Our lists 
	QPtrList<SipCallMember> members;
	QPtrList<SipTransaction> transactions;

	// CSeq numbers, local and remote
	unsigned int lastremoteseq;
	unsigned int lastseq;

	// Call state
	CallType calltype;
	CallStatus callstatus;
	QString callid;
	QString subject;
	QString presenceStatus;
	QString activityStatus;
	bool basePresenceStatus;
	bool hasrecordroute;
	SipUriList recordroute;
	bool hasroute;
	bool initRequest;
	SipUriList route;
	SipUri getLocaluri( void ) const { return localuri; }

	// Audit call state
	/**
	 * check for messages to be repeated
	 */
	void auditCall( void );

	// For readability of the code
	/**
	 * process an incoming request
	 */
	SipCallMember *incomingRequest( SipMessage *message );
	/**
	 * process an incoming Response
	 */
	void incomingResponse( SipMessage *message );

	// For SipTransaction
	/**
	 * send a Request
	 */
	bool sendRequest( SipMessage *reqmsg, bool contact = true, const SipUri &proxy = SipUri::null, const QString &branch = QString::null );
	/**
	 * send a response
	 */	
	void sendResponse( SipMessage *reqmsg, bool contact = true );
	/**
	 * send a raw message
	 */
	void sendRaw( SipMessage *msg );

	// For SipClient
	/**
	 * process an incoming Message
	 */
	SipCallMember *incomingMessage( SipMessage *message );

	QList<SipTransaction> &getTransactionList( void ) { return transactions; }

	// For SipCallMember
	/**
	 * adds a newly created call member to the call member list
	 */	
	void addMember( SipCallMember *newmember );

	QString bodyMask;
	QString contactstr;

	bool looseRoute;

	QString refnoti;
	QString refsub;
	QString Inumber;
	int presbody;
	QPtrList<WatcherInfo> myWatchers;
	/**
	 * create/update watchers  in our watcher list
	 */
	void processWatchers(QString uri, QString event, QString state,int full);
	/**
	 * show the watchers in our watcher list
	 */
	void showWatchers(void);
	int updateST;
	bool isAck;
	SessionControl *sessionC;
};




typedef QListIterator<SipCall> SipCallIterator;
#endif // SIPCALL_H_INCLUDED

