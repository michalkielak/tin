/*[M i7
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
#include <stdlib.h>
#include <qtimer.h>
#include <qsettings.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
using namespace std;
#include "../dissipate2/sipuser.h"
#include "../dissipate2/sipclient.h"
#include "../dissipate2/sipcall.h"
#include "../dissipate2/siputil.h"
#include "../dissipate2/sipregister.h"
#include "sessioncontrol.h"
#include "audiocontrol.h"
#include "videocontrol.h"
#include "extcontrol.h"
#include "ksipregistrations.h"
#include "kphoneview.h"
#include "ksippreferences.h"
#include "ksipoptions.h"
#include "kphonebook.h"
#include "kphone.h"
#include "trayicon.h"
#include "callaudio.h"
#include "kstatics.h"
#include "khelp.h"
#include "kdtmf.h"
KPhone::KPhone( QString port, QString dhost, QString prefix, int p2p, bool looseRoute, bool strictRoute, int debug, bool direct2URI, bool dontRegister ) : QMainWindow(0)
{

	showDebug=debug;
	bool useIPv6=true;
	QString uristr;
	QString p;
	int ipv6 = 2;
	KStatics::debugLevel=showDebug;
	QString socketStr;
	QString l;
	userPrefix = "default";
	if( !prefix.isEmpty() ) {
		setCaption( prefix );
		userPrefix = prefix + "_";
	}
        p = KStatics::dBase + userPrefix + "Registration/";

        portrepeater = 10;
	int colpos = port.find(":");
	if (colpos == 0) {
	    port = "0";
	} else 	if (colpos >0) {
	    QString pr = port.mid(colpos+1);
	    portrepeater = pr.toLong();
	    port = port.mid(0,colpos);
	}	
	if (port == "0") {
	    listenport = 5060;
	    QSettings settings;
	    settings.writeEntry( p + "MyPort", "5060" );
	} else	if (port == "") {
	    QSettings settings;
	    listenport = settings.readEntry( p + "MyPort","5060").toLong();
	} else {
	    QSettings settings;
	    settings.writeEntry( p + "MyPort",port);
	    listenport = port.toLong();
	}
	
	if (p2p > 0) {	
		KStatics::isP2P=true;
		QSettings settings;
		if(KStatics::debugLevel>=2)cout << "=====KPhone: it's a P2P call\n";
		if(p2p == 4) useIPv6=false; 
		userPrefix = "default";
		if( !prefix.isEmpty() ) {
			setCaption( prefix );
			userPrefix = prefix + "_";
		}
		KStatics::myInd = KStatics::dBase + getUserPrefix();
		if (useIPv6) ipv6 = 1; else ipv6 = 0;
		KStatics::haveIPv6 = useIPv6;
		p = KStatics::dBase + getUserPrefix() + "Registration/";

		if(dhost != "") Sip::setLocalAddress(dhost);
		if( Sip::getLocalAddress().isEmpty() ) {
		if(KStatics::debugLevel>=2)cout << "=====KPhone: no local address\n";
			exit(0);
		}
		settings.writeEntry( p + "/AutoRegister", "No" );
		settings.writeEntry( p + "SipServer", "" );
		if (useIPv6) {
		    uristr= "<sip:" + QString::number(listenport)+ "@[" + Sip::getLocalAddress() + "]:" + QString::number(listenport) + ">";
			settings.writeEntry( p + "SipUri", uristr) ;
		} else {
			uristr= "<sip:" + QString::number(listenport)+ "@" + Sip::getLocalAddress() + ":" + QString::number(listenport) + ">";
			settings.writeEntry( p + "SipUri", uristr);
		}
		socketStr = settings.readEntry( KStatics::dBase+ getUserPrefix()+"/General/SocketMode", "UDP" );
		l = settings.readEntry( KStatics::dBase+ getUserPrefix()+"/General/LocServer", "NAPTR" );

	} else {
		QSettings settings;
		userPrefix = "default";
		if( !prefix.isEmpty() ) {
			setCaption( prefix );
			userPrefix = prefix + "_";
		}
		KStatics::myInd = KStatics::dBase + getUserPrefix();
		p = KStatics::dBase + getUserPrefix() + "Registration/";
		uristr = settings.readEntry( p + "SipUri" );
		socketStr = settings.readEntry( KStatics::dBase+ getUserPrefix()+"/General/SocketMode", "UDP" );
		l = settings.readEntry( KStatics::dBase+ getUserPrefix()+"/General/LocServer", "NAPTR" );
		if (dontRegister) settings.writeEntry( p + "/AutoRegister", "No" );


	}

//start SipClient and resolve uristring to IP
	client = new SipClient( 0, 0, listenport, portrepeater , looseRoute, strictRoute, socketStr, uristr,dhost,ipv6, direct2URI,l);
	//Avoid unnecesary multiple searchs
	SipUtil::found=1;
	{
	 //We need the brackets to close settings!	
		//set port for p2p (must be changed if the  selected port was in use, when we dropped in
		if (p2p > 0) {
			if( client->getListenPort() != QString::number(listenport) )  {
				QSettings settings;
				if (useIPv6) {
				uristr = "<sip:" + client->getListenPort()+ "@[" + Sip::getLocalAddress() + "]:" + client->getListenPort() + ">";
				settings.writeEntry( p + "SipUri", uristr);
				} else {
				uristr = "<sip:" + client->getListenPort()+ "@" + Sip::getLocalAddress() + ":" + client->getListenPort() + ">";
				settings.writeEntry( p + "SipUri", uristr);
				}
			}
		}

	}//close and write settings
	
//Supply SipClient with various settings
	QSettings settings; 
	client->setCallForward( settings.readBoolEntry( p + "/callforward", false ) );
	client->setCallForwardUri( SipUri( settings.readEntry( p + "/forwardaddr", QString::null ) ) );
	client->setCallForwardMessage( settings.readEntry( p + "/forwardmsg", QString::null ) );
	client->setMaxForwards( settings.readNumEntry( p + "/maxforwards", 70 ) );
	client->setBusyMessage( settings.readEntry( p + "/busymsg", QString::null ) );
	if( settings.readEntry( KStatics::dBase+ getUserPrefix()+"/Symmetric/Signalling", "Yes" ) == "Yes" ) {
		client->setSymmetricMode( true );
	}
	QString hvstr = settings.readEntry( KStatics::dBase+ getUserPrefix()+"/Symmetric/hideviamode", "NoHide" );
	if( hvstr == "NoHide" ) { 
		client->setHideViaMode( SipClient::DontHideVia ); }
	else if( hvstr == "HideHop" ) { 
		client->setHideViaMode( SipClient::HideHop ); }
	else if( hvstr == "HideRoute" ) { 
		client->setHideViaMode( SipClient::HideRoute ); 
	}

	sessionC = new SessionControl(getUserPrefix());

	user = new SipUser( client, sessionC, SipUri( uristr ) );

// Initially, none of the dialogs exist yet
	qdial = 0;
	userinfo = 0;
	audiocontrol = 0;
	videocontrol = 0;
	extcontrol1 =0;
	extcontrol2 =0;
	extcontrol3 =0;
	extcontrol4 =0;
	registrations = 0;
	sipprefs = 0;
	trayIcon =NULL;
	khelp = 0;
	kdtmf = 0;
// The link to the rtp and audio world
	callAudio = new CallAudio(sessionC,getUserPrefix()); 
	
//graphics, call supervision+routing presence
	view = new KPhoneView( sessionC, client, user, callAudio, this, getUserPrefix(), p2p );	
	connect( view, SIGNAL( stateChanged() ), this, SLOT( updateTrayIcon() ) );

	if( uristr != QString::null ) {
		p = KStatics::dBase + getUserPrefix() + "Registration/";
		QString userdefaultproxy = settings.readEntry( p + "SipServer", QString::null );
		if( userdefaultproxy.lower() == "sip:" ) {
			userdefaultproxy = "";
		}
		view->updateIdentity( user );
		client->updateIdentity( user, userdefaultproxy );
		sessionC->setDefaultProxy(userdefaultproxy);
		
	}

//Registration
	registrations = new KSipRegistrations( client, view, this, getUserPrefix() );
	view->identities( registrations );

//The Menues
	setCentralWidget( view );

	QPopupMenu *popup = new QPopupMenu;
	popup->insertItem( tr("DTMF Panel..."), this, SLOT( showDtmfPanel()),QKeySequence(tr("Ctrl+D","File|DTMF Panel")) );
	popup->insertSeparator();
	popup->insertItem( tr("&Quit"), this, SLOT( kphoneQuit() ),QKeySequence(tr("Ctrl+Q","File|Quit")) );
	menuBar()->insertItem( tr("&File"), popup );

	preferencesMenu = new QPopupMenu;
	preferencesMenu->insertItem( tr("SIP Preferences..."), this, SLOT( showGlobalPreferences() ),QKeySequence(tr("Ctrl+P,S","File|SIP Preferences")) );
	preferencesMenu->insertItem( tr("Audio Preferences..."), this, SLOT( showAudioPreferences() ) ,QKeySequence(tr("Ctrl+P,A","File|Audio Preferences")));
	preferencesMenu->insertItem( tr("Video Preferences..."), this, SLOT( showVideoPreferences() ) ,QKeySequence(tr("Ctrl+P,V","File|Video Preferences")));
	preferencesMenu->insertItem( tr("Phone Book"), view, SLOT( showPhoneBook() ) );
	preferencesMenu->insertSeparator();
	preferencesMenu->insertItem( tr("Session Type 1..."), this, SLOT( showExtPreferences1() ) ,QKeySequence(tr("Ctrl+P,1","File|Session Typ 1 Preferences")));
	preferencesMenu->insertItem( tr("Session Type 2..."), this, SLOT( showExtPreferences2() ) ,QKeySequence(tr("Ctrl+P,2","File|Session Typ 2 Preferences")));
	preferencesMenu->insertItem( tr("Session Type 3..."), this, SLOT( showExtPreferences3() ) ,QKeySequence(tr("Ctrl+P,3","File|Session Typ 3 Preferences")));
	preferencesMenu->insertItem( tr("Session Type 4..."), this, SLOT( showExtPreferences4() ) ,QKeySequence(tr("Ctrl+P,4","File|Session Typ 4 Preferences")));
	int id=preferencesMenu->insertItem( tr("Tray Icon"), this, SLOT( toggleTrayIcon(int) ) );
	QString usetrayicon = settings.readEntry( KStatics::dBase + getUserPrefix() + "/General/TrayIcon", "No" );
	if ( usetrayicon.lower() == "yes" ) {
		preferencesMenu->setItemChecked( id, true );
		createTrayIcon();
	} else {
		preferencesMenu->setItemChecked( id, false );
	}
	menuBar()->insertItem( tr("&Preferences"), preferencesMenu );
	menuBar()->insertSeparator();

	popup = new QPopupMenu;
	popup->insertItem( tr("Help"), this, SLOT( showHelp() ) ,QKeySequence(tr("Ctrl+H","File|Help")));
	popup->insertItem( tr("About the KPhoneSI"), this, SLOT( showAboutKPhone() ) );
	menuBar()->insertItem( tr("&Help"), popup );

//STUN support requested
	if( registrations->getUseStun() ) {
		sessionC->setStunSrv( registrations->getStunSrv() );
		client->sendStunRequest( registrations->getStunSrv() );
		QTimer *stun_timer = new QTimer( this );
		connect( stun_timer, SIGNAL( timeout() ), this, SLOT( stun_timerTick() ) );
		p = KStatics::dBase + getUserPrefix() + "/STUN/RequestPeriod";
		int timeout = settings.readNumEntry( p , constStunRequestPeriod );
		if( timeout > 0 ) {
			stun_timer->start( timeout * 1100 );
		}
	}

// SipClient
	timer = new QTimer( this );
	connect( timer, SIGNAL( timeout() ), this, SLOT( timerTick() ) );
	timer->start( 1,TRUE );

//polling timer for the remote dialler
	haveRDSock = newRDialSocket();
	if(haveRDSock) {
	    rdTimer = new QTimer( this );
	    connect( rdTimer, SIGNAL( timeout() ), this, SLOT( rdTimerTick() ) );
	    rdTimer->start( 5555,TRUE );
	}
	resize( 160, 250 );
}

KPhone::~KPhone( void )
{
    if(khelp !=0) {
	delete khelp;
    }
}

void KPhone::timerTick( void )
{
	if (rdLoop == 0) rdLoop=1;
	client->doSelect( false );
	view->info();
	if (rdLoop == 2) rdTimer->start( 5555,TRUE );
	rdLoop=0;
	    
	timer->start( 1,TRUE );	

}

/*
void KPhone::tickAlive( void )
{
	client->doAlive();
}
*/
void KPhone::rdTimerTick( void )
{
	if (rdLoop == 0) {
		 checkRDialSocket();
		 rdTimer->start( 5555,TRUE );
	} else rdLoop = 2;
	
}

void KPhone::stun_timerTick( void ) {
	if(sessionC->isStun()) {
		client->sendStunRequest();
	} else {
		client->noMoreStun();
	}
}


QString KPhone::getUserPrefix( void )
{
return userPrefix;
}


void KPhone::showAudioPreferences( void )
{
	if( !audiocontrol ) {
		audiocontrol = new AudioControl( sessionC, this, userPrefix);
	}
	audiocontrol->show();
}

void KPhone::showVideoPreferences( void )
{
	if( !videocontrol ) {
		videocontrol = new VideoControl( sessionC, this, getUserPrefix() );
	}
	videocontrol->show();
}


void KPhone::showExtPreferences1( void )
{
	if( !extcontrol1 ) {
		extcontrol1 = new ExtControl( 1, sessionC, this, getUserPrefix() );
	}
	extcontrol1->show();
}

void KPhone::showExtPreferences2( void )
{
	if( !extcontrol2 ) {
		extcontrol2 = new ExtControl( 2, sessionC, this, getUserPrefix() );
	}
	extcontrol2->show();
}

void KPhone::showExtPreferences3( void )
{
	if( !extcontrol3 ) {
		extcontrol3 = new ExtControl(3, sessionC, this, getUserPrefix() );
	}
	extcontrol3->show();
}

void KPhone::showExtPreferences4( void )
{
	if( !extcontrol4 ) {
		extcontrol4 = new ExtControl(4,  sessionC, this, getUserPrefix() );
	}
	extcontrol4->show();
}

void KPhone::showRegistrations( void )
{
	registrations->showIdentity();
}

void KPhone::showGlobalPreferences( void )
{
	if( !sipprefs ) {
		sipprefs = new KSipPreferences( client, sessionC, this, getUserPrefix() );
	}
	sipprefs->show();
}

void KPhone::showHelp( void )
{
	
	if(khelp == 0) {
		khelp = new KHelp();
		khelp->show();
	} else {
		delete khelp;
		khelp = 0;
	}
}

void KPhone::showDtmfPanel( void )
{
	
	if(kdtmf == 0) {
		kdtmf = new KDtmf(callAudio);
		kdtmf->show();
	} else {
		delete kdtmf;
		kdtmf = 0;
	}
}



void KPhone::showAboutKPhone( void )
{
	QMessageBox::about( this, tr("About KPhhoneSI"),
		tr("KPhone SI ") + KPhoneVersion + tr(" is an 'Applicaltion Over Internet' device\n")
		+ tr("utilizing the Session Initiation Protocol\n\n")
		+ tr("For more information about the KPhone SI see\n")
		+ tr("http://sourceforge.net/projects/kphone\n\n")
		+ tr("The KPhone SI is actually developed and maintained by\n")
		+ tr("Klaus Fleischmann <kgfleischmann@lists.sourceforge.net>\n\n")
		+ tr("The KPhoneSI is released under the GPL\n\n")
		 );
}

void KPhone::toggleTrayIcon( int id )
{
	if ( preferencesMenu->isItemChecked( id ) ) {
		preferencesMenu->setItemChecked( id, false );
		deleteTrayIcon();
	} else {
		preferencesMenu->setItemChecked( id, true );
		createTrayIcon();
	}
}

void KPhone::createTrayIcon( void )
{
	if ( ! trayIcon ) {
		if ( view->getState() ) {
			trayIcon=new TrayIcon( this, "trayIcon", QPixmap::fromMimeSource( SHARE_DIR "/icons/online.png" ), KStatics::xBase );
		} else {
			trayIcon=new TrayIcon( this, "trayIcon", QPixmap::fromMimeSource( SHARE_DIR  "/icons/offline.png" ),KStatics::xBase );
		}
		connect( trayIcon, SIGNAL( clicked() ), this, SLOT( toggleVisibility() ) );
		connect( trayIcon, SIGNAL( contextMenuRequested( const QPoint& ) ), this, SLOT( trayMenuRequested( const QPoint& ) ) );
		trayIcon->show();
	} else {
		updateTrayIcon();
	}
}

void KPhone::deleteTrayIcon( void )
{
	if ( trayIcon ) {
		trayIcon->close();
		delete trayIcon;
		trayIcon = NULL;
	}
}

void KPhone::updateTrayIcon( void )
{
	if ( trayIcon ) {
		if ( view->getState() ) {
			trayIcon->setPixmap( QPixmap::fromMimeSource( SHARE_DIR "/icons/online.png" ) );
		} else {
			trayIcon->setPixmap( QPixmap::fromMimeSource( SHARE_DIR "/icons/offline.png" ) );
		}
		trayIcon->repaint();
	}
}

void KPhone::toggleVisibility()
{
	if ( this->isVisible() ) {
		if(khelp !=0) {
			delete khelp;
			khelp = 0;
    		}
		if(kdtmf !=0) {
			delete kdtmf;
			kdtmf = 0;
		}
		this->hide();
	} else {
		this->show();
	}
}

void KPhone::trayMenuRequested( const QPoint& pos )
{
	QPopupMenu* trayMenu = new QPopupMenu(this);
	trayMenu->insertItem( isVisible() ? "&Hide" : "S&how", this, SLOT( toggleVisibility() ) );
	trayMenu->insertItem( view->getState() ? "&Offline" : "&Online" , view, SLOT( buttonOffOnlineClicked() ) );
	trayMenu->insertSeparator();
	trayMenu->insertItem( "&Quit", this, SLOT( kphoneQuit() ) );
	trayMenu->exec(pos);
	delete trayMenu;
}

void KPhone::kphoneQuit( void )
{
	QSettings settings;
	QString p = KStatics::dBase + getUserPrefix() + "/General/TrayIcon";
	if ( trayIcon ) {
		settings.writeEntry( p, "Yes" );
		trayIcon->close();
		delete trayIcon;
		trayIcon = NULL;
	} else {
		settings.writeEntry( p, "No" );
	}
	if(khelp) delete khelp;
	if(kdtmf) delete kdtmf;
	delRDialSocket();
	view->kphoneQuit();
	quitTimer = new QTimer( this );
	connect( quitTimer, SIGNAL( timeout() ), qApp, SLOT( quit() ) );
	quitTimer->start( quitTime );
	hide();
}


//Remote dialer Stuff

bool KPhone::newRDialSocket(void) {
	int l;
	if(!sessionC->getRem()) return false;
	/* Create socket on which to send. */
	rdSock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (rdSock < 0) {
	    if(KStatics::debugLevel>=2)cout << "=====KPhone::newRDialSocketerror opening datagram socke\n";
		perror("opening datagram socket");
		return false;
	}
	bzero( &mess[0], 100 );
	rdServer.sun_family = AF_UNIX;
	strcpy(rdServer.sun_path, "/tmp/kpsidg");
        l = strlen(rdServer.sun_path)+sizeof(rdServer.sun_family);
        if(bind (rdSock,(struct sockaddr *)&rdServer, l) <0) {
    	    perror("bind");
	    if(KStatics::debugLevel>=2)cout << "=====KPhone::newRDialSocket bind error\n";
    	    return false;
	} 
 // set the socket nonblocking, it could be called by another application!
	int flags = fcntl(rdSock, F_GETFL, 0);
        flags |= O_NONBLOCK|O_NDELAY;
	if (fcntl(rdSock, F_SETFL, flags) == -1) {
	    if(KStatics::debugLevel>=2)cout << "=====KPhone::newRDialSocket fcntl failed error\n";
                perror("fcntl: F_SETFL");
                return false;
        }
	return true;
}

void KPhone::checkRDialSocket(void) {
	int cllen=0,n=0;
	n=recvfrom(rdSock,&mess[0],100,0,(struct sockaddr *)&rdClient,(socklen_t *)&cllen);
	if (n>0) {
		SipCall::CallType ct;
		QString num = &mess[1];
		if (num.contains("sip://")) num=num.mid(6);
		else if (num.contains("sip:")) num=num.mid(4);
		switch (mess[0]){
			case 'a':
				ct=SipCall::StandardCall;
				break;
			case 'v':
				ct=SipCall::auviCall;	
				break;
			case 'o':
				ct=SipCall::videoCall;
				break;
			case '1':
				ct=SipCall::extCall1;
				break;
			case '2':
				ct=SipCall::extCall2;
				break;
			case '3':
				ct=SipCall::extCall2;
				break;
			case '4':
				ct=SipCall::extCall2;
				break;
			default:
				ct=SipCall::StandardCall;
				break;

		}
		if(KStatics::debugLevel>=2)cout << "====KPhone::checkRDialSocket: " << &mess[0] << num.latin1() << (int)ct << endl;
		view->startCallWidget (num, ct);
		bzero( &mess[0], n );
	}
}

void KPhone::delRDialSocket(void) {
    if (haveRDSock) {
	haveRDSock = false;
	delete rdTimer;
	rdTimer = 0;
	close(rdSock);
	unlink(rdServer.sun_path);
    }
}
