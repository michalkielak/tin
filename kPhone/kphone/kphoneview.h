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
#ifndef KPHONEVIEW_H_INCLUDED
#define KPHONEVIEW_H_INCLUDED
#include <qwidget.h>
#include <qptrdict.h>
#include <qvbox.h>
#include <qradiobutton.h>
#include <qxml.h>
#include <qcombobox.h>

#include "../dissipate2/sipuri.h"
#include "../dissipate2/sippublish.h"
#include "ksipauthentication.h"
#include "kcallwidget.h"
#include "kcallview.h"
#include "callaudio.h"
#include "watcherlist.h"

#define  UPDATEWLIST 1
#define  CLEARSOCKETS 2
#define  HANGUP 3
typedef QPtrListIterator<KCallWidget> widgetIterator;
class SipClient;
class SipUser;
class SipCall;
class SipCallMember;
class SipPublish;
class KCallWidget;
class CallAudio;
class KInstantMessageWidget;
class QListViewItem;
class SessionControl;
class KPhone;
class KSipRegistrations;
class KSipAuthentication;
class ContactsListView;
class KCallView;
class IncomingCall;
class IdleTimeDetector;

/**
* @short holds the results of class ContactParser (entries of the phonebook)
*/
class PhoneEntry
{
public:
	PhoneEntry (const QString u, const QString d, bool ds = false, bool rs = false, bool sc = false);
	~PhoneEntry ( void );
	QString uri;
	QString description;
	bool dosubscribe;
	bool rejectsubscription;
	bool screencall;
};

typedef QListIterator<PhoneEntry> PhoneEntryIterator;

/**
* @short parses the XML coded PhoneBook 
*
Reads the phone book into a list of entry related classes.
The list is used for presence subscription and call screening
*/
class ContactParser : public QXmlDefaultHandler
{
public:
	ContactParser( void );
	~ContactParser( void );
	bool startDocument();
	bool startElement( const QString&, const QString&, const QString&, const QXmlAttributes& );
	bool endElement( const QString&, const QString&, const QString& );
	PhoneEntryIterator getPEIterator() const { return PhoneEntryIterator(pecontacts);}
private:
	QPtrList<PhoneEntry> pecontacts;
};

/**
* @short Part of the graphic representation of the Kphone
*
* Part of the graphic representation of the Kphone.
* It provides the user with an interface to call provisioning and call setup:
* outgoing Calls are started here, after the User raises an appropriate signal;
*
* Incoming Calls are started here, after SipCall calls the meth. KPhoneView::incomingCall
*
* KPhoneView is the anchorpoint for all KCallWidget incarnations
*
* Presence is controlled from here
*/
class KPhoneView : public QWidget
{
	Q_OBJECT
public:
	enum State { ONLINE, OFFLINE };
	KPhoneView( SessionControl *sessioncontrol, SipClient *c, SipUser *u, CallAudio *au, QWidget *parent = 0, const char *name = 0, int p2p = false );
	~KPhoneView( void );
	
/** 
*       feed the network info panel
*/
	void info(void);
/** 
*  ???
*/
	void switchCall( SipCall *call );
/** 
*       provide the class with its SipUser and SipRegister links
*/
	void updateIdentity( SipUser *newUser, SipRegister *newReg = 0 );
/** 
*       provide the class with its KSipRegistrations link
*/
	void identities( KSipRegistrations *i );
/** 
*       start a call from the contacts window 
*/	
	void startCWbyContact(SipCall::CallType ct);
/** 
*       create and start a KCallWidget
*/
	bool startCallWidget (QString num, SipCall::CallType ct);

/** 
* Run over all CallWidgets and kill all markes for killing 
*/
	void updateContacts( QString file );

/** 
* mute the BuddyList
*/
	void setContactsOffline( void );
/** 
*  reactivate the Buddylist
*/
	void setContactsOnline( void );
/** 
*  bye, bye
*/
	void kphoneQuit( void );
/** 
* Run over all CallWidgets and inform us
*/
	void testWidgetList( void);
	
/** 
* is Widget No wNr still there?
*/
	int isExistingWidget( int wNr );
/**
* Run over all SipCalls and inform about CallType and PayLoad
*/
	void rideCalls( void);
	
/** 
*  returns a id for the newly created CallWidget
*/
	int cntWidget(void) { return ++widgetCounter;}

/**
* Run over all SipCalls and count payload calls not onHold
*/
	int countActiveCalls(void);

/**
*	Check if we are the only (INVITE initiated) call
*/
	bool activateAllowed( void);
/** 
*  isOnline set or not?
*/
	bool getState( void );
/** 
*  returns a Iterator of the list of CallWidgets
*/
	widgetIterator  getWList(void)      const {return  widgetIterator ( cwList); }
/** 
*       The last error
**/
void writeLastError(QString er);

/**
*	restart subscriptions if necessary
*/	
	void requestRefreshSubscribe( void );
/**
*	clear all subscriptions
*/	
	void requestClearSubscribes( void );
/**
*	flag to control if subscriptions must be restarted 
*	after registration
*/	
	void setDoRefreshSubscribe( bool drs );
/**
*	send notify
*/	
	void requestNotification( int id );
signals:
	void stateChanged(void);
	void auditKCW(void);

public slots:
	void makeNewCall( void );
	void makeNewExt1( void );
	void makeNewExt2( void );
	void makeNewExt3( void );
	void makeNewExt4( void );
	void makeVideoCall( void );
	void makeAuViCall( void );
/**
*	do the action according to info 
*/		
	void informPhoneView( int info, int wNr);
/**
*	redirect a call 
*/	
	void redirectCall( const SipUri &calluri, const QString &subject, int ct, KCallWidget *trfrom , int wNr);
/**
*	
*/	
	void showIdentities( void );
/**
*	
*/	
	void pubStatusUpdate( void );
/**
*	
*/	
	void stateUpdated( int id );
/**
*	
*/	
	void updateSubscribes( void );
/**
*	
*/	
	void terminateSubscribes( int mess, SipCall *call );
/**
*	
*/	
	void localStatusUpdate();

private slots:
/**
*	A IM comes in, we have to display it
*/
	void incomingInstantMessage( SipMessage *message );
/**
*	Handle an incoming subscribe (phonebook update, response )
*/
	void incomingSubscribe( SipCallMember *member, bool sendSubscribe );
/**
*	Call attempt, create CallWidget and transfer control to it
*/
	void incomingCall( SipCall *call, QString body );
/**
*	Doom CallWidget for deletion
*/
	void hideCallWidget( SipCall *call );
/**
*	Start an IM from contactlist
*/
	void contactDoubleClicked( QListViewItem *i );	
/**
*	show the callwidget of an entry of the active call window
*/	
	void listDoubleClicked( QListViewItem *i );
/**
*	request a buddy state update (toggles between offline and other states
*/
	void stateUpdateClicked( void );
/**
*	hangup the call specified by the entry in the active call list
*/
	void hangupCW(void);
/**
*	show call widget specified by the entry in the active call list
*/
	void toggleCW(void);
/**
*	put call on hold specified by the entry in the active call list
*/
	void holdCW(void);
/**
*	toggle between audio and audio+video specified by the entry in the active call list
*/
	void renegCodecCW();
/**
*	transfer call  specified by the entry in the active call list
*	do not send notifications
*/
	void transferCWs(void);
/**
*	transfer call specified by the entry in the active call list
*	keep old call
*/
	void referCW(void);
/**
*	transfer call specified by the entry in the active call listl
*/
	void transferCW(void);
	
	//void busyToggled( bool onoff );
/**
*	Start an IM from contactlist
*/
	void contactSendMessage();
//methods for pulldowns and pops
/**
*	Start audio call from contactlist
*/
	void contactCall( void );
/**
*	Start external call type 1 from contactlist
*/
	void contactExt1( void );
/**
*	Start external call type 2 from contactlist
*/
	void contactExt2( void );
/**
*	Start  external call type 3 from contactlist
*/
	void contactExt3( void );
/**
*	Start external call type 4 from contactlist
*/
	void contactExt4( void );
/**
*	Start video call from contactlist
*/
	void contactVideoCall( void );
/**
*	Start audio + video call from contactlist
*/
	void contactAuViCall( void );
/**
*	show the phonebook
*/
	void showPhoneBook( void );

/**
*       user interface of stateUpdateClicked
*/
	void buttonOffOnlineClicked( void );
/**
*	show the watcher list
*/
	void WListClicked( void );
	
protected:
	SipClient *client;
	SipUser *user;
	CallAudio *callAudio;
	ContactsListView *clist;
	KCallView *tlist;
	int callcount;
	QLineEdit   *lineeditCall;
	QPushButton *buttonSipUri;
	QPushButton *buttonNewExt1;
	QPushButton *buttonNewExt2;
	QPushButton *buttonNewExt3;
	QPushButton *buttonNewExt4;
	QPushButton *buttonPhone;
	QPushButton *buttonNewCall;
	QPushButton *buttonVideoCall;
	QPushButton *buttonAuViCall;
	QLabel *labelSipUri;
	QLabel *labelFullname;
	KPhone *kphone;
	KSipRegistrations *identitiesDialog;
	KSipAuthentication *sipauthentication;
	
private:
/**
*	Check if subscriptions need to be refreshed, used by localStatusUpdate and subscription delay for publish
*/
        void refreshSubscriptions(void);
/**
*	prepare the XML-Body and request to send the message
*/
	void sendNotify( int id, SipCallMember *member = 0 );
/**
*	get the id-tag for out entry in .kpsirc
*/
	QString getUserPrefix( void );
/**
*
*/
	//void saveRejectContactList( void );
/**
*	update the list of callwidgets and delete those marked as hided
*/
	void updateWidgetList( void );
/**
*
*/
	//void saveContacts( void );
/**
*	add a new accepted contact to our phonebook
*/
	void addContactToPhoneBook( SipCallMember *member, bool reject = false );

	QString stateText( QString text );

	QPtrList<KCallWidget> cwList;
	QPtrList<KInstantMessageWidget> imwList;
	QStringList rejectedContactList;
	WatcherList *wlist;

	SipRegister *reg;
	SipPublish  *publish;
	SessionControl *sessionControl;
	SipClient *sic;
	SipCall *winfocall;
	SipCallMember *winfomember;
	IdleTimeDetector *itd;
	
	QTimer *subscribeTimer;

	QPushButton *buttonOffOnline;
	QPushButton *buttonWList;
	QPushButton *buttonUpdate;
	QComboBox *stateComboBox;
	QLabel *infostat1;
	QLabel *infostat2;
	QString phonefile;

	int atomId;
	int subscribeExpiresTime;
	int publishExpiresTime;
	int activityCheckTime;
	int firstPublish;
	int widgetCounter;
	int pointtopoint;
	bool isOnline;
	bool isWList;
	bool buttonSetOffline;
	bool setSubscribeOffline;
	bool doRefreshSub;
};

#endif // KPHONEVIEW_H_INCLUDED
