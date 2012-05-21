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
#include <signal.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qvbox.h>
#include <qdir.h>
#include <iostream>
using namespace std;

#include "../dissipate2/sipcall.h"
#include "../dissipate2/sipclient.h"
#include "../dissipate2/sipstatus.h"
#include "../dissipate2/siptransaction.h"
#include "../dissipate2/sipmessage.h"
#include "../dissipate2/sippublish.h"

#include "calllistview.h"
#include "kcallview.h"
#include "kcallwidget.h"
#include "sessioncontrol.h"
#include "kphone.h"
#include "ksipauthentication.h"
#include "kphonebook.h"
#include "kphoneview.h"
#include "callaudio.h"
#include "kstatics.h"
#include "watcherlist.h"
#ifdef PRESACT
#include "idletimedetector.h"
#endif

PhoneEntry::PhoneEntry (const QString u, const QString d, bool ds, bool rs, bool sc)
{
uri = u;
description = d;
dosubscribe =ds;
rejectsubscription = rs;
screencall = sc;
}

PhoneEntry::~PhoneEntry (void)
{
uri = "";
description = "";
dosubscribe = false;
rejectsubscription = false;
screencall = false;
}

ContactParser::ContactParser( void )
	: QXmlDefaultHandler()
{
	pecontacts.setAutoDelete(true);
}

bool ContactParser::startDocument()
{
	return TRUE;
}

bool ContactParser::startElement( const QString&, const QString&,
		const QString& qName,
		const QXmlAttributes& attributes)
{
	if( qName == "identity" ) {
		QString description = attributes.value( "description" );
		QString uri = attributes.value( "uri" );
		bool contact = false, rejsub = false, sccall = false;
		QString selector = attributes.value( "contact" );
		if( selector.lower() == "yes" )  contact = true;
		
		selector = attributes.value( "RejectIncomingSubscription" );
		if( selector.lower() == "yes" ) rejsub  = true;
		
		selector = attributes.value( "ScreenCall" );
		if( selector.lower() == "yes" ) sccall  = true;  
		
		if( uri.left( 4 ).lower() != "tel:" && uri.left( 4 ).lower() != "sip:" ) {
			uri = "sip:" + uri;
		}
		if (description.isEmpty()) description = uri;
		
		PhoneEntry *pe = new PhoneEntry (uri , description, contact, rejsub, sccall);
		pecontacts.append(pe);
	}
	return TRUE;

}

ContactParser::~ContactParser(void) {
pecontacts.clear();
}

bool ContactParser::endElement( const QString&, const QString&, const QString& )
{
	return TRUE;
}

KPhoneView::KPhoneView( SessionControl *sessioncontrol, SipClient *c, SipUser *u,CallAudio *au, QWidget *parent, const char *name , int p2p)
	: QWidget( parent, name , WStyle_NoBorder), client( c ), user( u ), callAudio(au) 
{
	pointtopoint = p2p;
	reg = 0;
	widgetCounter=0;
	sic = c;
	wlist = 0;
	buttonSetOffline = false;
	setSubscribeOffline = false;
	sessionControl = sessioncontrol;
	QSettings settings;
	connect( client, SIGNAL( updateSubscribes() ),
		this, SLOT( updateSubscribes() ) );

	connect( client, SIGNAL( terminateSubscribes(int, SipCall *) ),
		this, SLOT( terminateSubscribes(int, SipCall *) ) );
	connect( client, SIGNAL( incomingInstantMessage( SipMessage * ) ),
		this, SLOT( incomingInstantMessage( SipMessage * ) ) );
	connect( client, SIGNAL( incomingSubscribe( SipCallMember *, bool ) ),
		this, SLOT( incomingSubscribe( SipCallMember *, bool ) ) );
	connect( client, SIGNAL( incomingCall( SipCall *, QString ) ),
		this, SLOT( incomingCall( SipCall *, QString ) ) );
	connect( client, SIGNAL( hideCallWidget( SipCall * ) ),
		this, SLOT( hideCallWidget( SipCall * ) ) );
		QVBoxLayout *vbox = new QVBoxLayout( this );

	QHBox *infoline = new QHBox( this );
	infoline->setSpacing( 2 );
	vbox->insertWidget( 0, infoline );
	
	QGridLayout *headergrid = new QGridLayout( 9,16 );//5
	vbox->insertLayout( 1, headergrid );
	
	QHBox *infoline1 = new QHBox( this );
	infoline1->setSpacing( 2 );
	infostat1 = new QLabel( infoline1 );
	infostat1->setText(c->getContactText());
	headergrid->addMultiCellWidget( infoline1,0 ,0, 0, 6 );//

	buttonSipUri = new QPushButton( "Set Identity", this );
	headergrid->addMultiCellWidget( buttonSipUri,0 ,0, 7, 16 );//
	connect( buttonSipUri, SIGNAL( clicked() ),
		this, SLOT( showIdentities() ) );

//phonebook
	buttonPhone = new QPushButton( "", this );
	buttonPhone->setFixedWidth( 30 );
	QIconSet iconPh;
	iconPh.setPixmap(SHARE_DIR "/icons/phonebook.png", QIconSet::Automatic );
	buttonPhone->setIconSet( iconPh );
	headergrid->addWidget( buttonPhone, 1, 0 );
	buttonPhone->setAccel(QKeySequence(tr("Ctrl+B","File|PhoneBook")));
	connect( buttonPhone, SIGNAL( clicked() ),
		this, SLOT( showPhoneBook() ) );

	lineeditCall = new QLineEdit( this );
	lineeditCall->setFocus();
	connect( lineeditCall, SIGNAL( returnPressed() ),
		this, SLOT( makeNewCall() ) );
	headergrid->addMultiCellWidget( lineeditCall, 1, 1, 1, 9);

// The buttons

//Audio
	buttonNewCall = new QPushButton( "", this );
	buttonNewCall->setFixedWidth( 30 );
	QIconSet icon;
	icon.setPixmap(SHARE_DIR "/icons/audiocall.png", QIconSet::Automatic );
	buttonNewCall->setIconSet( icon );
	headergrid->addWidget( buttonNewCall, 1, 10 );
	buttonNewCall->setAccel(QKeySequence(tr("Ctrl+N","File|Audio")));
	connect( buttonNewCall, SIGNAL( clicked() ),
		this, SLOT( makeNewCall() ) );

//video
	buttonVideoCall = new QPushButton( "", this );
	buttonVideoCall->setFixedWidth( 30 );
	QIconSet icon3;
	icon3.setPixmap(SHARE_DIR "/icons/videocall.png", QIconSet::Automatic );
	buttonVideoCall->setIconSet( icon3 );
	headergrid->addWidget( buttonVideoCall, 1, 11 );
	buttonVideoCall->setAccel(QKeySequence(tr("Ctrl+V","File|Video")));
	connect( buttonVideoCall, SIGNAL( clicked() ),
		this, SLOT( makeVideoCall() ) );

//audio+video
	buttonAuViCall = new QPushButton( "", this );
	buttonAuViCall->setFixedWidth( 30 );
	QIconSet icon4;
	icon4.setPixmap(SHARE_DIR "/icons/auvi.png", QIconSet::Automatic );
	buttonAuViCall->setIconSet( icon4 );
	headergrid->addWidget( buttonAuViCall, 1, 12 );
	buttonAuViCall->setAccel(QKeySequence(tr("Ctrl+A","File|AuVi")));
	connect( buttonAuViCall, SIGNAL( clicked() ),
		this, SLOT( makeAuViCall() ) );


// the gereral sessions
	QString icoPic;

	buttonNewExt1 = new QPushButton( "", this );
	buttonNewExt1->setFixedWidth( 30 );
	QIconSet iconT1;	
	icoPic = settings.readEntry( KStatics::dBase + getUserPrefix() + "/SessionType1/icon", "question" );
	iconT1.setPixmap(SHARE_DIR "/icons/"+icoPic+".png", QIconSet::Automatic );
	buttonNewExt1->setIconSet( iconT1 );
	headergrid->addWidget( buttonNewExt1, 1, 13 );
	buttonNewExt1->setAccel(QKeySequence(tr("Ctrl+1","File|Ext1")));
	connect( buttonNewExt1, SIGNAL( clicked() ),
		this, SLOT( makeNewExt1() ) );


	buttonNewExt2 = new QPushButton( "", this );
	buttonNewExt2->setFixedWidth( 30 );
	QIconSet iconT2;
	icoPic = settings.readEntry( KStatics::dBase + getUserPrefix() + "/SessionType2/icon", "question" );
	iconT2.setPixmap(SHARE_DIR "/icons/"+icoPic+".png", QIconSet::Automatic );
	buttonNewExt2->setIconSet( iconT2 );
	headergrid->addWidget( buttonNewExt2, 1, 14 );
	buttonNewExt2->setAccel(QKeySequence(tr("Ctrl+2","File|Ext2")));
	connect( buttonNewExt2, SIGNAL( clicked() ),
		this, SLOT( makeNewExt2() ) );
	
	buttonNewExt3 = new QPushButton( "", this );
	buttonNewExt3->setFixedWidth( 30 );
	QIconSet iconT3;
	icoPic = settings.readEntry( KStatics::dBase + getUserPrefix() + "/SessionType3/icon", "question" );
	iconT3.setPixmap(SHARE_DIR "/icons/"+icoPic+".png", QIconSet::Automatic );
	buttonNewExt3->setIconSet( iconT3 );
	headergrid->addWidget( buttonNewExt3, 1, 15 );
	buttonNewExt3->setAccel(QKeySequence(tr("Ctrl+3","File|Ext3")));
	connect( buttonNewExt3, SIGNAL( clicked() ),
		this, SLOT( makeNewExt3() ) );
 
	buttonNewExt4 = new QPushButton( "", this );
	buttonNewExt4->setFixedWidth( 30 );
	QIconSet iconT4;
	icoPic = settings.readEntry( KStatics::dBase + getUserPrefix() + "/SessionType4/icon", "question" );
	iconT4.setPixmap(SHARE_DIR "/icons/"+icoPic+".png", QIconSet::Automatic );
	buttonNewExt4->setIconSet( iconT4 );
	headergrid->addWidget( buttonNewExt4, 1, 16 );
	buttonNewExt4->setAccel(QKeySequence(tr("Ctrl+4","File|Ext4")));
	connect( buttonNewExt4, SIGNAL( clicked() ),
		this, SLOT( makeNewExt4() ) );


	callcount = 1;
	
	tlist = new KCallView( this, this );
	headergrid->addMultiCellWidget( tlist, 2, 2, 0, 16);// 0 7
	tlist->setMinimumWidth ( 400 );
	tlist->setMinimumHeight( 90 );
	tlist->addMenuItem( tr("toggle hold "), this, SLOT( holdCW() ) );
	tlist->addMenuItem( tr("Transfer and keep "), this, SLOT( referCW() ) );
	tlist->addMenuItem( tr("Transfer and hang up "), this, SLOT( transferCW() ) );
	tlist->addMenuItem( tr("ShortTransfer "), this, SLOT( transferCWs() ) );
	tlist->addMenuItem( tr("renegCodec"), this, SLOT( renegCodecCW() ) );
	tlist->addMenuItem( tr("hang up"), this, SLOT( hangupCW() ) );
	tlist->addMenuItem( tr("toggle CallWidget"), this, SLOT( toggleCW() ) );
	
	connect( tlist, SIGNAL( doubleClicked( QListViewItem * ) ),
		this, SLOT( listDoubleClicked( QListViewItem * ) ) );
	
	connect( this, SIGNAL( auditKCW(void) ),
		tlist, SLOT( auditList(void) ) );
	
	clist = new ContactsListView( client, this );
	headergrid->addMultiCellWidget( clist, 3, 3, 0, 16);// 0 7
	clist->setMinimumHeight (90); //( kphoneMinimunWidth );
	clist->setMinimumWidth (400); //( kphoneMinimunWidth );
	clist->addMenuItem( tr("Send Message"), this, SLOT( contactSendMessage() ) );
	clist->addMenuItem( tr("Audio Call"), this, SLOT( contactCall() ) );
	clist->addMenuItem( tr("Video Call"), this, SLOT( contactVideoCall() ) );
	clist->addMenuItem( tr("Audio+Video Call"), this, SLOT( contactAuViCall() ) );
	clist->addMenuItem( tr("Session Type 1") , this, SLOT( contactExt1() ) );
	clist->addMenuItem( tr("Session Type 2") , this, SLOT( contactExt2() ) );
	clist->addMenuItem( tr("Session Type 3") , this, SLOT( contactExt3() ) );
	clist->addMenuItem( tr("Session Type 4") , this, SLOT( contactExt4() ) );
	clist->addMenuItem( tr("Phone Book"), this, SLOT( showPhoneBook() ) );


	connect( clist, SIGNAL( doubleClicked( QListViewItem * ) ),
		this, SLOT( contactDoubleClicked( QListViewItem * ) ) );
	connect( client, SIGNAL( callListUpdated() ),
		clist, SLOT( auditList() ) );	
	
	buttonOffOnline = new QPushButton( "", this );
	buttonOffOnline->setFixedWidth( 30 );
	QIconSet icon1;
	icon1.setPixmap(SHARE_DIR "/icons/offline.png", QIconSet::Automatic );
	buttonOffOnline->setIconSet( icon1 );
	connect( buttonOffOnline, SIGNAL( clicked() ),
		this, SLOT( buttonOffOnlineClicked() ) );
	headergrid->addWidget( buttonOffOnline, 4,0);
	stateComboBox = new QComboBox( TRUE, this, "" );
	static const char* items[] = {
	"Online", "Busy", "Be Right Back", "Away", "Permanent absent", "Appointment", "At Breakfast", "At Dinner", "On Holidays", "At Lunch", "Having a Meal", "In Meeting", "Travelling", "In Transit", "Steering", "Shopping", "Sleeping", "Looking For Work", "Looking TV", "On Vacation", "On The Phone", "Offline", "At/In Performance", "Playing", "Giving Presentation", "Spectator", "Unknown", "Working", "At Worship",0 };
	stateComboBox->insertStrList( items );
	connect( stateComboBox, SIGNAL( activated( const QString & )  ),
		this, SLOT( stateUpdateClicked() ) );
	headergrid->addMultiCellWidget( stateComboBox, 4, 4, 2, 8 );//4 14->7

	buttonWList = new QPushButton( "", this );
	buttonWList->setFixedWidth( 30 );
	QIconSet iconw;
	iconw.setPixmap(SHARE_DIR "/icons/question.png", QIconSet::Automatic );
	buttonWList->setIconSet( iconw );
	connect( buttonWList, SIGNAL( clicked() ),
		this, SLOT( WListClicked() ) );
	headergrid->addWidget( buttonWList, 4,9);


	QHBox *infoline2 = new QHBox( this );
	infoline2->setSpacing( 2 );
	infostat2 = new QLabel( infoline2 );
	infostat2->setText("No error yet");
	headergrid->addMultiCellWidget( infoline2,4 ,4, 10, 14 );//
	QString label;
	QString uristr;
	QString contact;
	sipauthentication = new KSipAuthentication();

	SipCall *newcall;
	SipCallMember *member;
//  SipCall  for Watcherlist
	uristr = user->getUri().uri();	
	QString p = KStatics::dBase + getUserPrefix() + "/local/";
	subscribeExpiresTime = settings.readNumEntry(
	p + "/SubscribeExpiresTime", constSubscribeExpiresTime );
	publishExpiresTime = settings.readNumEntry(
	p + "/PublishExpiresTime", 0 );
	winfocall = 0;
	winfomember = 0;
	if (sessionControl->getWat()) {
	    winfocall = new SipCall( user, QString::null, SipCall::winfoSubscribeCall );
	    winfocall->setSubject( uristr );
	    winfocall->setContactStr( uristr );
	    SipUri remoteuri( uristr );
	    winfomember = new SipCallMember( winfocall, remoteuri );
    	    wlist = new WatcherList(winfocall,winfomember);
	    wlist->setCaption(uristr);
	    connect( winfomember, SIGNAL( statusUpdated( SipCallMember * ) ),
		wlist, SLOT( refresh() ) );
	    connect( winfomember, SIGNAL( statusUpdated( SipCallMember * ) ),
		sipauthentication, SLOT( authRequest( SipCallMember * ) ) );
	    }
//  SipCall  for Outgoing Subscribes

	p = KStatics::dBase + getUserPrefix() + "/local/";
	phonefile = settings.readEntry( p + "/PhoneBook", "" );
	
	if( phonefile.isEmpty() ) {
		if( getUserPrefix().isEmpty() ) {
			phonefile = QDir::homeDirPath() + "/." + KStatics::xBase + "-phonebook.xml";
		} else {
			phonefile = QDir::homeDirPath() + "/." + KStatics::xBase + "_" + getUserPrefix() + "phonebook.xml";
		}
	}
	QFile xmlFile( phonefile );
	QXmlInputSource source( xmlFile );
	QXmlSimpleReader reader;
	ContactParser parser;
	reader.setContentHandler( &parser );
	reader.parse( source );
	PhoneEntryIterator itp(parser.getPEIterator());
	for ( itp.toFirst();itp.current(); ++itp ) {
		uristr =itp.current()->uri;
		contact = itp.current()->description;
		if( itp.current()->dosubscribe) {	
			newcall = new SipCall( user, QString::null, SipCall::outSubscribeCall );
			newcall->setSubject( uristr );
			newcall->setContactStr( contact );
			SipUri remoteuri( uristr );
			member = new SipCallMember( newcall, remoteuri );
			connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
				clist, SLOT( auditList() ) );
			connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
				sipauthentication, SLOT( authRequest( SipCallMember * ) ) );
		}
	}
	clist->auditList();
	atomId = 1000;
	
//Subscriptions	
	subscribeExpiresTime = sessionControl->getSubExpiresTime();
	publishExpiresTime = sessionControl->getPubExpiresTime();
	if( subscribeExpiresTime == 0 ) {
		subscribeExpiresTime = -1;
	}
	
	doRefreshSub = false;

	#ifdef PRESACT
	activityCheckTime = sessionControl->getActivityCheckTime();
	#else 
	activityCheckTime = 0;
	#endif
	
	isOnline = false;
	isWList = false;


//Publish
	cwList.setAutoDelete( true );
	imwList.setAutoDelete( true );
	publish = 0;
	firstPublish=0;
	if(publishExpiresTime > 0 ) {
		firstPublish = 2;
		publish = new SipPublish(user, user->getUri().uri(),publishExpiresTime,activityCheckTime);
		#ifdef PRESACT
		if( activityCheckTime > 0) {

		    itd = new IdleTimeDetector( activityCheckTime );
		    connect(itd, SIGNAL(userIdle(const QDateTime)), publish, SLOT(inactive(const QDateTime)));
		    connect(itd, SIGNAL(userActive(const QDateTime)), publish, SLOT(active(const QDateTime)));
		    itd->startIdleDetection();
		}
		#endif
	}
}

KPhoneView::~KPhoneView( void )
{
}


//Call Control

void KPhoneView::incomingInstantMessage( SipMessage *message )
{
	QString subject;
	QString remote;
	KInstantMessageWidget *imwidget;
	SipCall *c;
	subject = user->getUri().uri();
	remote = (message->getHeaderData( SipHeader::From ));
	if ( (message->getHeaderData( SipHeader::Content_Type)).contains("application/im-iscomposing")) return;//No use for this
	for( imwidget = imwList.first(); imwidget != 0; imwidget = imwList.next() ) {
		c = imwidget->getCall();
		if( c->getSubject() == subject ) {
			imwidget->instantMessage( message );
			imwidget->show();
			return;
		}
	}
	c = new SipCall( user );
	c->setSubject( user->getUri().uri() );
	imwidget = new KInstantMessageWidget( sipauthentication, c, this );
	imwList.append( imwidget );
	imwidget->instantMessage( message );
	imwidget->show();
}


void KPhoneView::incomingCall( SipCall *call, QString body )
{
	updateWidgetList();
	if( call->getSubject() == QString::null ) {
	    call->setSdpMessageMask( body );
	    call->setSubject( "Incoming call" );
	    SipCallMember *member;
	    member = call->getMemberList().toFirst();
	    bool screen = false;
	    if (member) {
		QString uri = member->getUri().uri();
		QFile xmlFile( phonefile );
		QXmlInputSource source( xmlFile );
		QXmlSimpleReader reader;
		ContactParser parser;
		reader.setContentHandler( &parser );
		reader.parse( source );
		PhoneEntryIterator itp(parser.getPEIterator());
		for ( itp.toFirst();itp.current(); ++itp ) {
		    if ( (itp.current()->uri == uri) && itp.current()->screencall){
		     screen = true;
		     break;
		    }
		}	    
	    member = 0;
	    } 	
		KCallWidget *widget = new KCallWidget(  sipauthentication, sessionControl, call, callAudio, this ,NULL ,0,screen );
		cwList.append( widget );

		connect( widget, SIGNAL( redirectCall( const SipUri &, const QString &, int  , KCallWidget *, int) ),
			this, SLOT( redirectCall( const SipUri &, const QString &, int ,KCallWidget *, int ) ) );
		connect( widget, SIGNAL( informPhoneView( int ,int) ),
			this, SLOT( informPhoneView( int ,int) ) );
	}
}

void KPhoneView::hideCallWidget( SipCall *call )
{

	KCallWidget *widget;
	SipCall *c;
	for( widget = cwList.first(); widget != 0; widget = cwList.next() ) {
		c = widget->getCall();
		if( c == call ) {
			widget->setHide();//now bound to be deleted
		}
	}
}

void KPhoneView::listDoubleClicked( QListViewItem *i ) {
	KCallWidget *k;
	k= ( (KCallViewItem *) i )->call;
	if(sessionControl->getCFB()) {
   		QMessageBox mb(tr( "Call Forward Busy"),
			tr("You want to forward this Call  to: \n") + sessionControl->getCallForwardBUri() + "\n\n" ,
		QMessageBox::Information, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, 0 );
		if( mb.exec() == QMessageBox::Yes ) {
			k->forwardCall();
		} else {
			k->acceptCall();
		}
	} else {
	 k->acceptCall();
	}
}

void KPhoneView::contactDoubleClicked( QListViewItem *i )
{
	if( !buttonSipUri->text().compare( "Set Identity" ) ){
		QMessageBox::information( this, "Dialling", "Set First Identity." );
		return;
	}
	QString subject = ( (ContactsListViewItem *) i )->getCall()->getSubject();
	if (subject == "") return;
	SipUri uri = SipUri( buttonSipUri->text() );
	KInstantMessageWidget *imwidget;
	for( imwidget = imwList.first(); imwidget != 0; imwidget = imwList.next() ) {
	    if( imwidget->getCall()->getSubject() == uri.uri()) {
			imwidget->show();
			return;
		}
	}
	SipCall *initcall = new SipCall( user );
	initcall->setSubject( uri.uri() );
	imwidget = new KInstantMessageWidget( sipauthentication, initcall, this );
	imwList.append( imwidget );
	imwidget->setRemote( subject );
	imwidget->show();
}

void KPhoneView::contactSendMessage()
{
	QString subject = "";
	if( clist->currentItem() != 0 ) {
		subject = ((ContactsListViewItem *)clist->currentItem())->getCall()->getSubject();
	}
	if( !buttonSipUri->text().compare( "Set Identity" ) ){
		QMessageBox::information( this, KStatics::xBase , "Set First Identity." );
		return;
	}
	SipUri uri = user->getUri();
	KInstantMessageWidget *imwidget;
	if( clist->currentItem() ) {
		QString subject = ( (ContactsListViewItem *)clist->currentItem() )->getCall()->getSubject();
		if( clist->currentItem() != 0 ) {
			for( imwidget = imwList.first(); imwidget != 0; imwidget = imwList.next() ) {
				if( imwidget->getCall()->getSubject() == uri.uri() &&
				    imwidget->getRemote() == subject ) {
					imwidget->show();
					return;
				}
			}
		}
	}
	SipCall *initcall = new SipCall( user );
	initcall->setSubject( uri.uri() );
	imwidget = new KInstantMessageWidget( sipauthentication, initcall, this );
	imwList.append( imwidget );
	imwidget->setRemote( subject );
	imwidget->show();
}

void KPhoneView::hangupCW() {
    updateWidgetList();
    if( tlist->currentItem() != 0 ) {
		 ((KCallViewItem *) tlist->currentItem() )->call->hangupCall();
    }
}

void KPhoneView::holdCW() {
    KCallWidget *k;
    updateWidgetList();
    if( tlist->currentItem() != 0 ) {
        k=((KCallViewItem *) tlist->currentItem() )->call;
        if(k) {
		k->holdCall();
	}
    }
}

void KPhoneView::transferCW() {
    KCallWidget *k;
    updateWidgetList();
    if( tlist->currentItem() != 0 ) {
        k=((KCallViewItem *) tlist->currentItem() )->call;
        if(k) {
	        k->showTransferDialog(false);
	}
    }
}
void KPhoneView::transferCWs() {
    KCallWidget *k;
    updateWidgetList();
    if( tlist->currentItem() != 0 ) {
        k=((KCallViewItem *) tlist->currentItem() )->call;
        if(k) {
	        k->showTransferDialog(true);
	}
    }
}

void KPhoneView::referCW() {
    KCallWidget *k;
    updateWidgetList();
    if( tlist->currentItem() != 0 ) {
        k=((KCallViewItem *) tlist->currentItem() )->call;
        if(k) {
	        k->showReferDialog();
	}
    }
}

void KPhoneView::renegCodecCW() {
    KCallWidget *k;
    updateWidgetList();
    if( tlist->currentItem() != 0 ) {
        k=((KCallViewItem *) tlist->currentItem() )->call;
        if(k) {
	        k->renegCodec();
	}
    }
}

void KPhoneView::toggleCW() {
    KCallWidget *k;
    updateWidgetList();
    if( tlist->currentItem() != 0 ) {
        k=((KCallViewItem *) tlist->currentItem() )->call;
    }
}

void  KPhoneView::startCWbyContact(SipCall::CallType ct)
{
	updateWidgetList();
	QString subject = "";
	if( clist->currentItem() != 0 ) {
		subject = ((ContactsListViewItem *)clist->currentItem())->getCall()->getSubject();
	}
	if (subject == "") return;
	if( !buttonSipUri->text().compare( "Set Identity" ) ){
		QMessageBox::information( this, "Dialling", "Set First Identity." );
		return;
	}
	SipCall *newcall = new SipCall( user, QString::null, ct );
	newcall->setSubject( user->getUri().uri() );
	KCallWidget *widget = new KCallWidget( sipauthentication, sessionControl, newcall, callAudio, this ,NULL ,0 );
	cwList.append( widget );
	widget->setRemote( subject );
	connect( widget, SIGNAL( redirectCall( const SipUri &, const QString &, int, KCallWidget *, int  ) ),
		this, SLOT( redirectCall( const SipUri &, const QString &, int, KCallWidget *, int  ) ) );
	connect( widget, SIGNAL( informPhoneView( int ,int) ),
			this, SLOT( informPhoneView( int ,int) ) );
	widget->clickDial();

}

void KPhoneView::contactCall()
{

	startCWbyContact(SipCall::StandardCall);
}


void KPhoneView::contactExt1()
{
	startCWbyContact(SipCall::extCall1);

}
void KPhoneView::contactExt2()
{
	startCWbyContact(SipCall::extCall2);

}
void KPhoneView::contactExt3()
{
	startCWbyContact(SipCall::extCall3);
}
void KPhoneView::contactExt4()
{
	startCWbyContact(SipCall::extCall4);
}

void KPhoneView::contactVideoCall()
{
	startCWbyContact(SipCall::videoCall);
}

void KPhoneView::contactAuViCall()
{
	startCWbyContact(SipCall::auviCall);
}


//PhoneBook

void KPhoneView::showPhoneBook()
{
	infostat1->setText(sic->getContactText());
	QSettings settings;
	QString p = KStatics::dBase + getUserPrefix() + "/local/";
	QString file = settings.readEntry( p + "/PhoneBook", "" );
	QString label;
	QString s;
	if( file.isEmpty() ) {
		if( getUserPrefix().isEmpty() ) {
			file = QDir::homeDirPath() +
				"/." + KStatics::xBase + "-phonebook.xml";
		} else {
			file = QDir::homeDirPath() + "/." + KStatics::xBase + "_" + getUserPrefix() + "phonebook.xml";
		}
	}
	
	IncomingCall *incomingCall;
	QPtrList<IncomingCall> r;
	QPtrList<IncomingCall> m;
	QPtrList<IncomingCall> dlc;
	
	QDateTime dt;
	int year,month,day,hour,min,sec;
	int count = 0;
	QString cp = KStatics::cBase + getUserPrefix() + "CallRegister";
	label.setNum( count );
	label = cp + "/Received" + label;
	s = settings.readEntry( label, "" );
	while( !s.isEmpty() ) {
		year = s.left( s.find( '.' ) ).toInt();
		s = s.mid( s.find( '.' ) + 1 );
		month = s.left( s.find( '.' ) ).toInt();
		s = s.mid( s.find( '.' ) + 1 );
		day = s.left( s.find( '-' ) ).toInt();
		s = s.mid( s.find( '-' ) + 1 );
		hour = s.left( s.find( ':' ) ).toInt();
		s = s.mid( s.find( ':' ) + 1 );
		min = s.left( s.find( ':' ) ).toInt();
		s = s.mid( s.find( ':' ) + 1 );
		sec = s.left( s.find( '{' ) ).toInt();
		dt.setDate( QDate( year, month, day ) );
		dt.setTime( QTime( hour, min, sec ) );
		s = s.left( s.find( '}' ) );
		s = s.mid( s.find( '{' ) + 1 );
		incomingCall = new 
		IncomingCall( s, dt );
		r.append( incomingCall );
		label.setNum( ++count );
		label = cp + "/Received" + label;
		s = settings.readEntry( label, "" );
	}
	count = 0;
	label.setNum( count );
	label = cp + "/Missed" + label;
	s = settings.readEntry( label, "" );
	while( !s.isEmpty() ) {
		year = s.left( s.find( '.' ) ).toInt();
		s = s.mid( s.find( '.' ) + 1 );
		month = s.left( s.find( '.' ) ).toInt();
		s = s.mid( s.find( '.' ) + 1 );
		day = s.left( s.find( '-' ) ).toInt();
		s = s.mid( s.find( '-' ) + 1 );
		hour = s.left( s.find( ':' ) ).toInt();
		s = s.mid( s.find( ':' ) + 1 );
		min = s.left( s.find( ':' ) ).toInt();
		s = s.mid( s.find( ':' ) + 1 );
		sec = s.left( s.find( '{' ) ).toInt();
		dt.setDate( QDate( year, month, day ) );
		dt.setTime( QTime( hour, min, sec ) );
		s = s.left( s.find( '}' ) );
		s = s.mid( s.find( '{' ) + 1 );
		incomingCall = new IncomingCall( s, dt );
		m.append( incomingCall );
		label.setNum( ++count );
		label = cp + "/Missed" + label;
		s = settings.readEntry( label, "" );
	}
	count = 0;
	label.setNum( count );
	label = cp + "/Dialled" + label;
	s = settings.readEntry( label, "" );
	while( !s.isEmpty() ) {
		year = s.left( s.find( '.' ) ).toInt();
		s = s.mid( s.find( '.' ) + 1 );
		month = s.left( s.find( '.' ) ).toInt();
		s = s.mid( s.find( '.' ) + 1 );
		day = s.left( s.find( '-' ) ).toInt();
		s = s.mid( s.find( '-' ) + 1 );
		hour = s.left( s.find( ':' ) ).toInt();
		s = s.mid( s.find( ':' ) + 1 );
		min = s.left( s.find( ':' ) ).toInt();
		s = s.mid( s.find( ':' ) + 1 );
		sec = s.left( s.find( '{' ) ).toInt();
		dt.setDate( QDate( year, month, day ) );
		dt.setTime( QTime( hour, min, sec ) );
		s = s.left( s.find( '}' )  );
		s = s.mid( s.find( '{' ) + 1 );
		incomingCall = new IncomingCall( s, dt );
		dlc.append( incomingCall );
		label.setNum( ++count );
		label = cp + "/Dialled" + label;
		s = settings.readEntry( label, "" );
	}
	PhoneBook *phoneBook = new PhoneBook(file, this, getUserPrefix() +
		"Phone Book", r, m, dlc );
	phoneBook->exec();
	updateContacts( file );
	lineeditCall->setText("");
	QString di=phoneBook->getUri();
	lineeditCall->setText(di.mid(di.find("sip")));
}

void KPhoneView::addContactToPhoneBook( SipCallMember *member, bool reject )
{
	QPtrList<IncomingCall> r;
	QPtrList<IncomingCall> m;
	QPtrList<IncomingCall> di;
	PhoneBook *phoneBook = new PhoneBook(
		phonefile, this, getUserPrefix() + "Phone Book", r, m, di);
	phoneBook->addContact( member,reject );
	updateContacts( phonefile );
}

//Button Control

void KPhoneView::buttonOffOnlineClicked( void ) {
	if( isOnline ) {
		buttonSetOffline = true;
		isOnline = false;
		stateComboBox->setEnabled( false );
		QIconSet icon1;
		icon1.setPixmap(SHARE_DIR "/icons/offline.png", QIconSet::Automatic );
		buttonOffOnline->setIconSet( icon1 );
		stateUpdated( OFFLINE );
	} else {
		buttonSetOffline = false;
		isOnline = true;
		KStatics::noLine=0;
		stateComboBox->setEnabled( true );
		QIconSet icon2;
		icon2.setPixmap(SHARE_DIR "/icons/online.png", QIconSet::Automatic );
		buttonOffOnline->setIconSet( icon2 );
		stateUpdated( ONLINE );
	}
}
void KPhoneView::WListClicked( void ){
	if (wlist == 0) return;
	if( isWList ) {
		isWList = false;
		wlist->hide();
	} else {
		isWList = true;
		wlist->show();
	}
}

void KPhoneView::stateUpdateClicked( void ){
	if( isOnline ) {
	if(publish) publish->publishRequest(stateText(stateComboBox->currentText()));
	stateUpdated( ONLINE );
	}
}

void KPhoneView::stateUpdated( int id )  {
	QString subject;
	if( id == OFFLINE ) {
	cout << "OFFLINE \n";
		isOnline = false;
		QIconSet icon;
		icon.setPixmap(SHARE_DIR "/icons/offline.png", QIconSet::Automatic );
		buttonOffOnline->setIconSet( icon );
	}
	SipCallIterator it( client->getCallList() );
	it.toFirst();
	requestNotification(id);
	emit (stateChanged());
}

//Presence 

void KPhoneView::incomingSubscribe( SipCallMember *member, bool sendSubscribe )
{
	clist->auditList();
	if( member == 0 ) {
		return;
	}
	bool remove_subscribe = false;
	SipUri uri = member->getUri();
	QString uristr = member->getUri().reqUri();

	QFile xmlFile( phonefile );
	QXmlInputSource source( xmlFile );
	QXmlSimpleReader reader;
	ContactParser parser;
	reader.setContentHandler( &parser );
	reader.parse( source );
	PhoneEntryIterator itp(parser.getPEIterator());
	//We search for existing buddies who may not subscribe
        for ( itp.toFirst();itp.current(); ++itp ) {
		if( (uristr == itp.current()->uri) && itp.current()->rejectsubscription) {
			remove_subscribe = true;
		}
	}
	//We search for known buddies (i.e. ex. outgoing subscriptions)
	SipCallIterator it( client->getCallList() );
	bool find = false;
	if( !remove_subscribe ) {
		for( it.toFirst(); it.current(); ++it ) {
			if( it.current()->getCallType() == SipCall::outSubscribeCall ) {
				if( it.current()->getMember( uri ) ) {
					find = true;
					
				}
			}
		}
	}
	//New Buddy, if we want her, she will be added to our phonebook
	if( !find && !remove_subscribe) {
		QString uristr = member->getUri().reqUri();
		QMessageBox mb( getUserPrefix() ,
	    			"Subscribe message from uri:\n" + uristr +
				"\n\nDo you want to accept and create a contact ?",
				QMessageBox::Information,
				QMessageBox::Yes | QMessageBox::Default,
				QMessageBox::No,
				QMessageBox::Cancel | QMessageBox::Escape );
		mb.setButtonText( QMessageBox::Yes, "Accept" );
		mb.setButtonText( QMessageBox::No, "Reject permanently" );
		mb.setButtonText( QMessageBox::Cancel, "Reject this time" );
		switch( mb.exec() ) {
			case QMessageBox::Yes:
				addContactToPhoneBook( member );
				sendSubscribe = false;
				break;
			case QMessageBox::No:
				addContactToPhoneBook( member,true );
				remove_subscribe = true;
				break;
			case QMessageBox::Cancel:
				remove_subscribe = true;
				break;
		}
	}
	//clean unwanted subscribers
	if( remove_subscribe ) {
		for( it.toFirst(); it.current(); ++it ) {
			if( it.current()->getCallType() == SipCall::inSubscribeCall ) {
				if( it.current()->getMember( uri ) ) {
					delete it.current();
				}
			}
		}
		return;
	}
	//connect member and notify
        if( isOnline ) {
		connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
			sipauthentication, SLOT( authRequest( SipCallMember * ) ) );
		sendNotify( ONLINE, member );
	}
	//Subscribe to that buddy
	for( it.toFirst(); it.current(); ++it ) {
		if( it.current()->getCallType() == SipCall::outSubscribeCall ) {
			if( it.current()->getCallStatus() != SipCall::callDead ) {
				if( it.current()->getMember( uri ) ) {
					if( sendSubscribe ) {
						if( reg->getRegisterState() == SipRegister::Connected ) {
							QString uristr = it.current()->getSubject();
							QString contactStr = it.current()->getContactStr();
							
							if( it.current()->getCallStatus() == SipCall::callInProgress ) {
								it.current()->getMember( uri )->requestClearSubscribe(false);
							}
							delete it.current();
							SipCall *newcall = new SipCall( user, QString::null,
								 SipCall::outSubscribeCall );
							newcall->setSubject( uristr );
							SipUri remoteuri( uristr );
							member = new SipCallMember( newcall, remoteuri );
							connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
								clist, SLOT( auditList() ) );
							connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
								sipauthentication, SLOT( authRequest( SipCallMember * ) ) );
							member->requestSubscribe( subscribeExpiresTime );
							newcall->setContactStr( contactStr );
							break;
						}
					}
				}
			}
		}
	}

}


void KPhoneView::terminateSubscribes( int mess, SipCall *call ){
delete call;
}

void KPhoneView::updateSubscribes( void ){
	updateContacts( phonefile );
}

void KPhoneView::updateContacts( QString file )
{
	QString uristr;
	SipCallMember *member;
	SipCall *newcall;
	QFile xmlFile( file );
	QXmlInputSource source( xmlFile );
	QXmlSimpleReader reader;
	ContactParser parser;
	reader.setContentHandler( &parser );
	reader.parse( source );
	SipCallIterator it( client->getCallList() );
	
	PhoneEntryIterator itp(parser.getPEIterator());
	bool find;
	it.toFirst();
	//Loop over all OutSubscribeCalls
	while ( it.current() ) {
		find = true;
		if( it.current()->getCallType() == SipCall::outSubscribeCall ) {
			find = false;
			for ( itp.toFirst();itp.current(); ++itp ) {
				QString subject = itp.current()->uri;
				if( (subject == it.current()->getSubject() ) && itp.current()->dosubscribe ){
					find = true;
					break;
				}
			}
			if( !find ) {
				SipCallMember *member = it.current()->getMemberList().toFirst();
				if( member ) {
				    if (it.current()->getPresState() < SipCall::PsTerminated) {
					member->requestClearSubscribe(true);
				    } else  if (it.current()->getPresState() >= SipCall::PsTerminated) { 
					    delete it.current();
					    disconnect( it.current(), 0, 0, 0 );
				    } else {
				    }
				    ++it;
				}
			}
		}
		if( find ) {
			++it;
		}
	}
	for ( itp.toFirst();itp.current(); ++itp ) {
		QString subject = itp.current()->uri;
		if( itp.current()->dosubscribe ) {
			find = false;
			QString contactStr = itp.current()->description;
			for ( it.toFirst(); it.current(); ++it ) {
             		    if( subject == it.current()->getSubject() &&
				    it.current()->getCallStatus() != SipCall::callDead ) {
					if( !contactStr.isEmpty() ) {
						it.current()->setContactStr( contactStr );
					} else {
						it.current()->setContactStr( subject );
					}
					find = true;
					break;
				}
			}
			if( !find ) {
			    if(reg) { // we only start a subscription if the user ist registered or needs no registation
				if ( (reg->getRegisterState() == SipRegister::Connected ) || (pointtopoint >0) ){
				    newcall = new SipCall( user, QString::null, SipCall::outSubscribeCall );
				    newcall->setSubject( subject );
				    SipUri remoteuri( subject );
				    member = new SipCallMember( newcall, remoteuri );
				    connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
					    clist, SLOT( auditList() ) );
				    connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
					    sipauthentication, SLOT( authRequest( SipCallMember * ) ) );
				    member->requestSubscribe( subscribeExpiresTime );
				    if( !contactStr.isEmpty() ) {
					newcall->setContactStr( contactStr );
				    } else {
					newcall->setContactStr( subject );
				    }
				}
			   }
		       }
		}
	}
	clist->auditList();
}

void KPhoneView::pubStatusUpdate( void ) {
    enum SipPublish::PublishState ps = publish->getPubState();
    if ( ps == SipPublish::PublishFailed) {
        cout << "===>KPhoneView: ALERT Publish rejected by the server\n";
	if(publish) delete publish;
	publish = 0;
	firstPublish = 0;
	refreshSubscriptions();
    } else if(firstPublish == 1) {
	if( (ps > SipPublish::NotConnected ) && (ps <= SipPublish::AuthenticationRequired) ) {
	    firstPublish = 0;
	    refreshSubscriptions();

	}
    }
}

void KPhoneView::refreshSubscriptions( void ) {
	QString subject;
	SipCallIterator it( client->getCallList() );
	if( reg == 0 ) {
	cout << "===>KPhoneView::refreshSubscriptions not jet registered\n";
		return;
	}
	//Waiting for 200OK of 1st PUBLISH?
	if( (firstPublish == 2) && (reg->getRegisterState() == SipRegister::Connected ) ) {
	    if(publish) publish->publishRequest(stateText(stateComboBox->currentText()));
	    connect( publish, SIGNAL( statusUpdated() ), this, SLOT( pubStatusUpdate() ) );
	    firstPublish = 1;
	    return;
	}
	if( reg->getRegisterState() == SipRegister::Connected ) {
			if( !buttonSetOffline ) {
				it.toFirst();
				for( it.toFirst(); it.current(); ++it ) {
					if( (it.current()->getCallType() == SipCall::outSubscribeCall) || (it.current()->getCallType() == SipCall::winfoSubscribeCall) ) {
						if( it.current()->getCallStatus() != SipCall::callDead ) {
							SipCallMember *member = it.current()->getMemberList().toFirst();
							if( member ) {
								member->requestSubscribe( subscribeExpiresTime );
							}
						}
					}
				}
			}
		}
}

void KPhoneView::requestNotification( int id ) {
	SipCallIterator it( client->getCallList() );
	it.toFirst();
	for( it.toFirst(); it.current(); ++it ) {
		if( it.current()->getCallType() == SipCall::inSubscribeCall ) {
			if( it.current()->getCallStatus() == SipCall::callInProgress ) {
				SipCallMember *member = it.current()->getMemberList().toFirst();
				if( member ) {
					sendNotify( id, member );
				}
			} else {
			    delete it.current();
			}
		}
	}
}




QString KPhoneView::stateText( QString text )
{
  QString state;
	if( text == "Online" ) {
		state = "online";
	} else if( text == "Busy" ) {
		state = "busy";
	} else if( text == "Be Right Back" ) {
		state = "berightback";
	} else if( text == "Away" ) {
		state = "away";
 	} else if( text =="Permanent absent") {
		state = "permanent-absence";
	} else if( text == "Appointment") {
		state ="appointment";
	} else if( text == "At Breakfast") {
		state ="breakfast";
	} else if( text == "At Dinner") {
		state =  "dinner";
	} else if( text == "On Holidays") {
		state = "holiday";
	} else if( text == "At Lunch") {
		state ="lunch";
	} else if( text == "Having a Meal") {
		state ="meal";
	} else if( text == "In Meeting") {
		state ="meeting";
	} else if( text == "Travelling") {
		state ="travel";
	} else if( text =="In Transit") {
		state = "in-transit";
	} else if( text =="Steering") {
		state = "steering";
	} else if( text =="Shopping") {
		state = "shopping";
	} else if( text =="Sleeping") {
		state = "sleeping";	
	} else if( text =="Looking For Work") {
		state = "looking-for-work";	 
	} else if( text =="Looking TV") {
		state = "tv";
	} else if( text == "On Vacation") {
		state ="vacation";
	} else if( text == "On The Phone" ) {
		state = "on-the-phone";
	} else if( text == "Offline" ) {
		state = "offline";
	} else if( text == "At/In Performance" ) {
		state = "performance";
	} else if( text == "Playing" ) {
		state = "playing";
	} else if( text == "Giving Presentation" ) {
		state = "presentation";
	} else if( text == "Spectator" ) {
		state = "spectator";	
	} else if( text == "Unknown" ) {
		state = "unknown";
	} else if( text == "Working" ) {
		state = "working";
	} else if( text == "At Worship" ) {
		state = "worship";
	}  else {
		state = (text.lower()).simplifyWhiteSpace();
	}
  return state;
}

void KPhoneView::sendNotify( int id, SipCallMember *member )
{
	QString s;
	QString tmp;
	QString state = "offline";
	QString basic = "closed";
	QString mime;
	time_t ti;
	tm *t;
	ti = time( NULL );
	t = gmtime( &ti );
	QDateTime dt(
		QDate( t->tm_year+1900, t->tm_mon + 1, t->tm_mday ),
		QTime( t->tm_hour, t->tm_min, t->tm_sec ) );
	
	if( id == ONLINE ) {
			basic = "open";	
			if( stateComboBox->currentText().isEmpty() ) {
				state = "online";
			} else {
				state = stateText(stateComboBox->currentText());
			}
	} 	
	if(member->getCall()->getPresBody() == 0) { 
		s =  "<?xml version=\"1.0\"?>\n";
		s += "<!DOCTYPE presence\n";
		s += "PUBLIC \"-//IETF//DTD RFCxxxx XPIDF 1.0//EN\" \"xpidf.dtd\">\n";
	
		s += "<presence>\n";
		s += "<presentity uri=\"";
		s += member->getUri().reqUri();
		s += ";method=SUBSCRIBE\"/>\n";
		s += "<atom id=\"";
		s += tmp.setNum( atomId++ ).latin1();
		s += "\">\n";
		s += "<address uri=\"";
		s += client->getContactUri().reqUri();
		s += ";user=ip\" priority=\"0,800000\">\n";
		s += "<status status=\"open\"/>\n";
		s += "<msnsubstatus substatus=\"";
		s += state;
		s += "\" />\n";
		s += "</address>\n";
		s += "</atom>\n";
		s += "</presence>\n";
		mime = "application/xpidf+xml";
	} else {
		QString entity = member->getUri().reqUri();
		if (!entity.contains("sip:"))  entity = "sip:" + entity;
		s =  "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n";
		s += "<pr:presence\n";
		s += "    xmlns:pr=\"urn:ietf:params:xml:ns:pidf\"\n";
		s += "    entity=\"" + entity  + "\"\n";
		s += "    xmlns:caps=\"urn:ietf:params:xml:ns:pidf:caps\"\n";
		s += "    xmlns:cipid=\"urn:ietf:params:xml:ns:pidf:cipid\"\n";
		s += "    xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\"\n";
		s += "    xmlns:rpid=\"urn:ietf:params:xml:ns:pidf:rpid\">\n";
		s += "    <pr:tuple id=\"bs35r9\">\n";
		s += "     <pr:status>\n";
		s += "      <pr:basic>" + basic + "</pr:basic>\n";
		s += "     </pr:status>\n";
		s += "     <pr:note>" + state +"</pr:note>\n";
		s += "     <pr:timestamp>" + dt.toString(Qt::ISODate)+ "</pr:timestamp>\n";
		s += "    </pr:tuple>\n";
		s += "    <dm:person id=\"p1\">\n";
		s += "      <rpid:activities><rpid:" + state.lower() + "/></rpid:activities>\n";
		s += "      <dm:note>" + state + "</dm:note>\n";
		s += "    </dm:person>\n";
		s += "</pr:presence>\n";
		mime = "application/pidf+xml";
	}
	
	if( stateComboBox->currentText().lower() == "busy" ) {
			client->setBusy( true );
	} else {
			client->setBusy( false );
	}
	if( member != 0 ) {
		if( id == OFFLINE ) {
			member->requestClearNotify( s.latin1(), MimeContentType(mime) );
			} else {
				if( !buttonSetOffline ) {
					member->requestNotify( s.latin1(), MimeContentType( mime ) );
				}
			}
	}
	
}

void  KPhoneView::informPhoneView( int info, int wNr){
	if(info == (UPDATEWLIST) ) {
		emit auditKCW();	
	} 
}

void KPhoneView::redirectCall( const SipUri &calluri, const QString &subject, int  ct, KCallWidget *trfrom , int wNr)
{
	updateWidgetList();
	SipCall *newcall = new SipCall( user );
	newcall->setSubject( subject );
	newcall->setCallTrType(ct);
	KCallWidget *widget = new KCallWidget( sipauthentication, sessionControl, newcall, callAudio, this, trfrom ,wNr  );
	cwList.append( widget );
	widget->pleaseDial( calluri );
	connect( widget, SIGNAL( redirectCall( const SipUri &, const QString &, int, KCallWidget *, int ) ),
		this, SLOT( redirectCall( const SipUri &, const QString &, int, KCallWidget *, int  ) ) );
	connect( widget, SIGNAL( informPhoneView( int ,int) ),
			this, SLOT( informPhoneView( int ,int) ) );
    
}

bool KPhoneView::startCallWidget(QString num, SipCall::CallType ct) {

	updateWidgetList();
	if (num == "") return false;
	

	if( !buttonSipUri->text().compare( "Set Identity" ) ){
		QMessageBox::information( this, "Dialling", "Set First Identity." );
		return false;
	}
	SipCall *newcall = new SipCall( user, QString::null, ct );
	QString subject;
	subject.sprintf( user->getUri().uri() );
	newcall->setSubject( subject );
	KCallWidget *widget = new KCallWidget( sipauthentication, sessionControl, newcall, callAudio,this ,NULL ,0 );
	cwList.append( widget );
	if( !num.isEmpty() ) {
		widget->setRemote( num );
		widget->clickDial();
	}
	connect( widget, SIGNAL( redirectCall( const SipUri &, const QString &, int, KCallWidget *, int  ) ),
		this, SLOT( redirectCall( const SipUri &, const QString &, int, KCallWidget *, int  ) ) );
	connect( widget, SIGNAL( informPhoneView( int ,int) ),
			this, SLOT( informPhoneView( int ,int) ) );
	return true;
}
	
void KPhoneView::makeNewCall( void )
{
	if (startCallWidget(lineeditCall->text(),SipCall::StandardCall) )
	lineeditCall->setText("");
}


void KPhoneView::makeNewExt1( void )
{
	if (startCallWidget(lineeditCall->text(),SipCall::extCall1) )
	lineeditCall->setText("");
}

void KPhoneView::makeNewExt2( void )
{
	if (startCallWidget(lineeditCall->text(),SipCall::extCall2) )
	lineeditCall->setText("");
}

void KPhoneView::makeNewExt3( void )
{
	if (startCallWidget(lineeditCall->text(),SipCall::extCall3) )
	lineeditCall->setText("");
}

void KPhoneView::makeNewExt4( void )
{
	if (startCallWidget(lineeditCall->text(),SipCall::extCall4) )
	lineeditCall->setText("");
}

void KPhoneView::makeVideoCall( void )
{
	if (startCallWidget(lineeditCall->text(),SipCall::videoCall) )
	lineeditCall->setText("");
}

void KPhoneView::makeAuViCall( void )
{
	if (startCallWidget(lineeditCall->text(),SipCall::auviCall) )
	lineeditCall->setText("");
}

void KPhoneView::updateIdentity( SipUser *newUser, SipRegister *newReg )
{
	if( newReg != 0 ) {
		if( reg != 0 ) {
			disconnect( reg, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdate() ) );
		}
		reg = newReg;
		connect( reg, SIGNAL( statusUpdated() ), this, SLOT( localStatusUpdate() ) );
		localStatusUpdate();
	}
	buttonSipUri->setText( newUser->getUri().uri() );
	user = newUser;
}

void KPhoneView::localStatusUpdate( void ) {
	QString subject;
	QIconSet icon;
	if( reg == 0 ) {
	cout << "===>KPhoneView::localStatusUpdate not jet registered\n";
		return;
	}

	if( (reg->getRegisterState() == SipRegister::NotConnected)  && (reg->getRegFailure() != "none") ) 
		infostat2->setText(reg->getRegFailure());
	if( setSubscribeOffline && reg->getRegisterState() == SipRegister::Connected ) {
		setSubscribeOffline = false;
		isOnline = false;
		QIconSet icon;
		icon.setPixmap(SHARE_DIR "/icons/offline.png", QIconSet::Automatic );
		buttonOffOnline->setIconSet( icon );
		buttonOffOnline->setEnabled( false );
	        //stop subscription
		requestClearSubscribes();
	} else {
		if( reg->getRegisterState() == SipRegister::NotConnected ) {
			isOnline = false;
			QIconSet icon1;
			icon1.setPixmap(SHARE_DIR "/icons/offline.png", QIconSet::Automatic );
			buttonOffOnline->setIconSet( icon1 );
			buttonOffOnline->setEnabled( false );
		} else if( reg->getRegisterState() == SipRegister::Connected ) {
			if( buttonSetOffline ) {
				buttonOffOnline->setEnabled( true );
			} else {
				isOnline = true;
				QIconSet icon2;
				icon2.setPixmap(SHARE_DIR "/icons/online.png", QIconSet::Automatic );
				buttonOffOnline->setIconSet( icon2 );
				buttonOffOnline->setEnabled( true );
				//begin with the subscriptions if any
				requestRefreshSubscribe();
                        }
		}
	}
emit(stateChanged());
}


void KPhoneView::requestRefreshSubscribe( void )
{
	if(!doRefreshSub) {
	    doRefreshSub = true;
	    refreshSubscriptions();
	}
}

void KPhoneView::requestClearSubscribes( void )
{
	SipCallIterator it( client->getCallList() );
	doRefreshSub = false;
	it.toFirst();
	for( it.toFirst(); it.current(); ++it ) {
		if( (it.current()->getCallType() == SipCall::outSubscribeCall)  || (it.current()->getCallType() == SipCall::winfoSubscribeCall) ) {
			if( it.current()->getCallStatus() == SipCall::callInProgress ) {
				SipCallMember *member = it.current()->getMemberList().toFirst();
				if( member ) {
				    member->requestClearSubscribe(false);
				}
			}
		}
	}
}
void KPhoneView::setDoRefreshSubscribe( bool drs )
{
       doRefreshSub = drs;	

}
void KPhoneView::showIdentities( void )
{
	identitiesDialog->showIdentity();
}

void KPhoneView::identities( KSipRegistrations *i )
{
	identitiesDialog = i;
}

void KPhoneView::setContactsOffline( void )
{
	stateUpdated( KPhoneView::OFFLINE );
	setSubscribeOffline = true;
	localStatusUpdate();
}

void KPhoneView::setContactsOnline( void )
{
	setDoRefreshSubscribe(true);
       stateUpdated( KPhoneView::ONLINE );
	
}

void KPhoneView::kphoneQuit( void )
{
	if(publish) publish->publishRequest("-");
	KCallWidget *widget;
	for( widget = cwList.first(); widget != 0; widget = cwList.next() ) {
		if( widget->isHided() ) {
			cwList.remove( widget );
		} else {
			widget->clickHangup();
		}
	}
	setContactsOffline();
	identitiesDialog->unregAllRegistration();
	if(wlist) {
    delete wlist;
    wlist = 0;
}

}

QString KPhoneView::getUserPrefix( void )
{
	return ((KPhone *)parentWidget())->getUserPrefix();
}


void KPhoneView::updateWidgetList( void )
{
	KCallWidget *widget;
	for( widget = cwList.first(); widget != 0; widget = cwList.next() ) {
		if( widget->isHided() ) {
			cwList.remove( widget );
			emit auditKCW();	

		}
	}
}

void KPhoneView::testWidgetList( void )
{
	KCallWidget *widget;
	
	for( widget = cwList.first(); widget != 0; widget = cwList.next() ) {
	}
}

int KPhoneView::isExistingWidget( int wNr )
{
	KCallWidget *widget;
	
	for( widget = cwList.first(); widget != 0; widget = cwList.next() ) {
		if ( (widget->myNumber == wNr)  && (!widget->isHided() ) ){
			return wNr; 
		} else return 0;
	}
	return 0;
}

void KPhoneView::rideCalls( void ) {
SipCallIterator it( client->getCallList() );
	for( it.toFirst(); it.current(); ++it ) {
		cout << "next:\n";
		switch ( it.current()->getCallType()) {
			case SipCall::GarKeinCall:
				cout << "Call not yet defined";
			break;
			case SipCall::StandardCall:
				cout << "StandardCall";
			break;
			case SipCall::videoCall:
				cout << "videoCall";
			break;
			case SipCall::OptionsCall:
				cout << "OptionsCall";
			break;
			case SipCall::RegisterCall:
				cout << "RegisterCall";
			break;
			case SipCall::MsgCall:
				cout << "MsgCall";
			break;
			case SipCall::BrokenCall:
				cout << "BrokenCall";
			break;
			case SipCall::UnknownCall:
				cout << "UnknownCall";
			break;
			case SipCall::outSubscribeCall:
				cout << "outSubscribeCall";
			break;
			case SipCall::inSubscribeCall:
				cout << "inSubscribeCall";
			break;
			case SipCall::inSubscribeCall_2:
				cout << "inSubscribeCall_2";
			break;
			case SipCall::extCall1:
				cout << "ExtCall1";
			break;
			case SipCall::extCall2:
				cout << "ExtCall2";
			break;
			case SipCall::extCall3:
				cout << "ExtCall3";
			break;
			case SipCall::extCall4:
				cout << "ExtCall4";
			break;
			default:
				cout << "undefined call = " << (int)it.current()->getCallType();
			break;
		}
		
		cout << endl;

		switch ( it.current()->getCallStatus()) {
			case SipCall::callDisconnecting:
			    cout << "callDisconnecting\n";
			break;
			case SipCall::callUnconnected:
			    cout << "callUnconnected\n";
			break;
			case SipCall::callInProgress:
			    cout << "callInProgress\n";
			break;
			case SipCall::callDead :
			    cout << "callDead\n";
			break;
			default:
				cout << "undefined\n";
			break;
		}


		cout << 	"\nHoldFlag       = " << (int)it.current()->getHoldCall() << endl;
		cout << 	"HoldMaster       = " << (int)it.current()->getHoldMaster() << endl;

  }
}

int KPhoneView::countActiveCalls(void) {
int cnt=0;
SipCallIterator it( client->getCallList() );

    for( it.toFirst(); it.current(); ++it ) {
	if( it.current()->blockCall()   && (it.current()->isActive() ) ) {
    	    cnt++;
	}
    }
return cnt;
}

bool KPhoneView::activateAllowed( void ) {
SipCallIterator it( client->getCallList() );
	bool isfree = true;
		for( it.toFirst(); it.current(); ++it ) {
		    if(it.current()->isOnlyMeActive() && !(it.current()->getHoldCall())) {
			isfree = false;
			break;
		    }
		}
return isfree;
	}

void KPhoneView::info(void) {
infostat1->setText(sic->getContactText());
}

bool KPhoneView::getState( void )
{
	return isOnline;
}

void KPhoneView::writeLastError(QString er) {
er = "  Last Error: " + er;
infostat2->setText(er);
}
