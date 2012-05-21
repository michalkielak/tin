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
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qtimer.h>
#include <iostream>
using namespace std;
#include "../kaudio/dspearly.h"
#include "kcallwidget.h"
#include "kphone.h"
#include "kstatics.h"
#include "kphoneview.h" 
#include "ksipauthentication.h"
#include "sessioncontrol.h"
#include "callaudio.h"
#include "../dissipate2/mimecontenttype.h"
#include "../dissipate2/sdpbuild.h"
#include "../dissipate2/sipcall.h"
#include "../dissipate2/sipmessage.h"
#include "../dissipate2/siptransaction.h"
#include "../dissipate2/sipuser.h"
#include "../dissipate2/udpmessagesocket.h"



KCallWidget::KCallWidget( KSipAuthentication *auth, SessionControl *sessioncontrol, SipCall *initcall, CallAudio *au,
	 KPhoneView *Vater, KCallWidget *trfrom, int wNr, bool screen, const char *name ): call( initcall )
{	
	sipauthentication = auth;
	missedCalls.setAutoDelete( true );
	receivedCalls.setAutoDelete( true );
	dialledCalls.setAutoDelete( true );
	hided = false;
	screenCall = screen;
	curstate = PreDial;
	member = 0;
	shortTransfer=false;
	referdialog= 0;
	audio = au;
	sessionC = sessioncontrol;
	papa=Vater;
	referedFrom=trfrom;
	referedNr= wNr;
	refnoti="none";
	forceHold=false;
	dirFlag = "";// i or o 
	KWsocket=0;
	call->setCurPid(0);
	redirTo=RdHoldNo;
	myNumber=papa->cntWidget();
	MyIdentity();
	referTo="";
	dtmfSize = 0;
	dtmfStr = "";
	doTransfer = false;
	preventSessionStart=false;
	dspe = 0;
	ringtonepid = 0;
	haveTone = false;
	debug=KStatics::debugLevel;
	acceptC = true;
	askRst = false;
	if(debug >=2) cout << ":::::KCallWidget start number" << myNumber << endl;

//+++++++++++ the timer
	ringCount = 0;
	ringTimer = new QTimer();
	connect( ringTimer, SIGNAL( timeout() ),
		this, SLOT( ringTimeout() ) );
	notiTime=sessionC->getNotiExpires();
	notiTimer = new QTimer();
	connect( notiTimer, SIGNAL( timeout() ),
		this, SLOT( hangupCall() ) );
	acceptCallTimer = new QTimer();
	connect( acceptCallTimer, SIGNAL( timeout() ),
		this, SLOT( acceptCallTimeout() ) );
	referTime  = 500;
	referTimer = new QTimer();
	connect( referTimer, SIGNAL( timeout() ),
		this, SLOT( handleRefer() ) );
	viewTime  = 1000;
	viewTimer = new QTimer();
	connect( viewTimer, SIGNAL( timeout() ),
		this, SLOT( hideCall() ) );
	
	updateTime  = 500;
	updateTimer = new QTimer();
	connect( updateTimer, SIGNAL( timeout() ),
		this, SLOT( hangupCall() ) );

	// Remember to give remote completion
	int count = 0;
	QString s;
	QString remoteuri;
	QSettings settings;
	QString cp = KStatics::cBase + getUserPrefix() + tr("CallRegister");
//+++++++++++++++++++++++++++++ read the CallRegister
	QDateTime dt;
	int year,month,day,hour,min,sec;
	count = 0;
	QString label;
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
		s = s.left( s.find( '}' )  );
		s = s.mid( s.find( '{' ) + 1 );
		incomingCall = new IncomingCall( s, dt );
		receivedCalls.append( incomingCall );
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
		missedCalls.append( incomingCall );
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
		s = s.left( s.find( '}' ) );
		s = s.mid( s.find( '{' ) + 1 );
		incomingCall = new IncomingCall( s, dt );
		dialledCalls.append( incomingCall );
		label.setNum( ++count );
		label = cp + "/Dialled" + label;
		s = settings.readEntry( label, "" );
	}
	QString sx, ssx, s1x, s2x, s3x;
	QTime t;
	QDate d;
	QDateTime t1=QDateTime::currentDateTime();	
	t = t1.time();
	d = t1.date();
	ssx = s1x.setNum( d.year() ) + "." + s2x.setNum( d.month() ) +	"." + s3x.setNum( d.day() ) + "-" + t.toString() + "[" + "die URI" + "]";
//The  Sdp Message Builder
	sdpS = new SdpBuild (sessionC, call);

//If we are the refered A-Side, so to send Notification set a sigslot to old B-Side 	
	if(referedFrom) {
    	    connect(this,SIGNAL(sigTrNotify(QString)),referedFrom,SLOT(sendTrNotify(QString)));
         }
	setCS( "Start" ); 		

// Start Call Handling
	switchCall( initcall );
}

KCallWidget::~KCallWidget( void )
{
	if(debug >=  2) cout << "\n:::::KCallWidget end   Nummer " << myNumber << endl;

	if(sdpS) delete sdpS;	
	delete notiTimer;
	delete viewTimer;
	delete referTimer;
	delete ringTimer;
	delete acceptCallTimer;
	delete updateTimer;
	missedCalls.clear();
	receivedCalls.clear();
	dialledCalls.clear();

	if( call ) {
	delete call;
	call = 0;
	}
	
	if(audio) {  
	    audio->clearMe(myNumber);	
	}
	
	if(KWsocket) {
	    delete KWsocket;
	    KWsocket = 0;
	}
	if(dspe) {
	    delete dspe;
	    dspe = 0;
	}
	
	if(referdialog) {
	    delete referdialog;
	    referdialog = 0;
	}
}


void KCallWidget::switchCall( SipCall *newcall )
{	
	detach = false;
	upLoad = false;
	noCheckConn = false;
	dspe=0;
	int callT=0, loadT=0;
	bool srtpErr = false;
	if(debug >=  2) cout <<  "=====KCallWidget:switchCall\n";
	stopRinging();
	call = newcall;
	if( member ) disconnect( member, 0, this, 0 );
	
	member = call->getMemberList().toFirst();
/* INCOMING CALL                                                                                          */	
	if( member ) {
		noCheckConn = true;
		if( screenCall)  {
		    papa->writeLastError(tr("Unwanted Call"));
		    setLT(tr("unwanted call"));
		    member->notAcceptableHere();
		    viewTimer->start(viewTime,TRUE);
		    return;
		}
		loadT = sdpS->checkCodec(false,member,isForceHold(),&srtpErr); 
		callT = call->saveLoadType(loadT);
		if(debug >=  2) cout <<  "=====KCallWidget: Switching calls: incoming LoadType=" << loadT << "CallType=" << callT << endl;

		if(loadT==0){ // no PayLoad good bye
		if(debug >=  2) cout <<  " codec not found\n";
		    papa->writeLastError(tr("Accepted codec not found"));
		    setLT(tr("invalid codec"));
		    member->notAcceptableHere();
		    viewTimer->start(viewTime,TRUE);
		    return;
		}
		if(debug >=  2) cout <<  "\n";
		dirFlag = "i";
		if( member->getState() == SipCallMember::state_Disconnected ) {
			forceDisconnect();
			return;
		}

		if( member->getState() == SipCallMember::state_Redirected ) {
			handleRedirect();
			return;
		}
		setCS( member->getLocalStatusDescription() ); 		
		connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
			this, SLOT( callMemberStatusUpdated() ) );
		if( call->getSubject() == "Incoming call" ) {
			setLoadTextS (callT);
			QString ss = member->getUri().uri();
			QDateTime t = QDateTime::currentDateTime();
			incomingCall = new IncomingCall( ss, t );
			missedCalls.append( incomingCall );
			updateCallRegister();
			setCS( member->getLocalStatusDescription() );
			// Ringing tone
			QSettings settings;
			QString p = KStatics::dBase+ getUserPrefix()+"/General/";
			isRingingTone = sessionC ->getRinging();
			ringCount = 0;
			ringTimer->start( ringTime_1, TRUE );
		}
// Modify Result if CLIR indicated ( sip:-)
		QString ss1 = member->getUri().uri();
		if(ss1.contains("sip:-")) ss1 = "hallo from a stranger";
		setCon(ss1);
		subject = call->getSubject();
		if( member->getState() == SipCallMember::state_InviteRequested ) {
			curstate = Calling;
		} else if( member->getState() == SipCallMember::state_RequestingInvite ) {
			curstate = Called;
		} else {
			curstate = Connected;
		}
		// start the remoely triggered application
		
		if(call->isRemoteStart()) {
		    if(sessionC->isRemoteStartAllowed(callT)){
			emit clickDial( );
		    } else {
			forceDisconnect();
		    }    
		}
/* OUTGOING     CALL                                                                                          */	
	} else {
		callT=(int)call->getCallType();
		loadT = callT;
		setLoadTextS(loadT);
		dirFlag = "o";
		if(debug >=  2) cout <<  "=====KCallWidget: Switching calls: outgoing CallType = " << callT << endl;

		if( call->isExtCall()){
			if( sessionC->isRemoteStartAllowed(callT) ) {
				askRst = true;
			} 
		}
		subject = call->getSubject();
		curstate = PreDial;
	}    
}

void KCallWidget::clickDial()
{
	if( askRst ) {
	QMessageBox mb(tr( "Remote Start"),
			tr("You want this a remote start") + "\n\n" ,
		QMessageBox::Information, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, 0 );
		if( mb.exec() == QMessageBox::Yes ) {
			call->setRemoteStart(true);
		} 
	} 
	dialClicked();
}

void KCallWidget::pleaseDial( const SipUri &dialuri )
{
	remote = dialuri.reqUri();
	dialClicked();
}

void KCallWidget::rdialClicked( void )
{
	call->setRemoteStart(true);
	dialClicked();
}

void KCallWidget::forwardCall( void )  {
	if(debug >=  2) cout << "=====KCallWidget::CFB";
	member->forwardCall("Contact: <" + sessionC->getCallForwardBUri() + ">\r\n");
	forceDisconnect();
	return;
}

void KCallWidget::dialClicked( void )
{
	QString auxPort = ""; QString MyPort  = ""; 	QString MyAuPort = "";	QString Mask="";

	int callT;
	callT=call->getCallType();

	if(debug >=  2) cout << "=====KCallWidget::dialClicked callType=" << callT << endl;
	
	//  Permissions for Hold and Refer
	sessionC->setPermFlags(callT);

	bool cFlag=sessionC->activeOnly(callT);
	call->onlyMeActive(cFlag);

	if(!sessionC->isHoldAllowed(callT)){
		call->setHoldMaster(true);
	}
	if(sessionC->isTransferAllowed(callT) ) {
		call->setTransAllowed(true);
	}  else {
		call->setTransAllowed(false);
	}
	
	int ac=papa->countActiveCalls();
	if( ac >= 1) { //some other blocking session active
	    if( cFlag || call->withAudioCall()) { //generalized with Flag or audio
		if(debug >=  2) cout << "=====KCallWidget::dialClicked Someone blocks the Audio\n";
		papa->writeLastError(tr("Someone blocks the Audio. Clear this and redial, please"));
		return;
	    }
	}
	
	if( curstate == Called ) {  // accept clicked, incoming Call
		call->setAck(true);
		missedCalls.setAutoDelete( false );
		missedCalls.remove( incomingCall );
		missedCalls.setAutoDelete( true );
		receivedCalls.append( incomingCall );
		updateCallRegister();
		if(debug >=  2) cout << "=====KCallWidget::dialClicked accept incoming Call\n";
		acceptCall();
		return;
	}


//check if we have not spezified a remote yet
	if( remote.length() == 0 ) {
	return;
	}
//transform the input to a SIP-URI, extract a possible DTMF-String

	QString strRemoteUri;
	QString s = remote;
	if ( s.find("##") == 0 ) {
	    	call->setSendToURI(true);
		s = s.mid(2);
	}
	int dtmfPos = s.find(",");
	int Usize=s.length();
	if (dtmfPos>0) {
	    dtmfSize = Usize-dtmfPos-1;
	    dtmfStr  = s.right(dtmfSize);
	    s = s.left(dtmfPos);
	}
	if( s.contains( '[' ) && s.contains( ']' ) ) {
	    strRemoteUri = s;
	} else {
		if( s.left( 4 ).lower() != "tel:" ) {
			if( s.left( 4 ).lower() != "sip:" ) {
				s = "sip:" + s;
			}
			if( !s.contains( '@' ) ) {
				s = s + "@" + call->getHostname();
			}
		}
	strRemoteUri = s;
	}
	if(debug >=  2) cout << "=====KCallWidget::dialClicked  Uri = " << s.latin1() << endl;
		
//check if we are calling ourself
	if(cmpUri(s,thatsMe)) {
	     hangupCall();
	     return;
	}

	remote = strRemoteUri;
	QSettings settings;

//update CallRegister
	QDateTime t = QDateTime::currentDateTime();
	incomingCall = new IncomingCall( strRemoteUri, t );
	dialledCalls.append( incomingCall );
	updateCallRegister();

// our callmember
	SipUri remoteuri( strRemoteUri );
	member = new SipCallMember( call, remoteuri );
	connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
		this, SLOT( callMemberStatusUpdated() ) );
	QString ss = member->getUri().uri();
	setCon(ss);
	
//Invite schnitzen!
	if ( call->withAudioCall() ) {
		MyAuPort = QString::number(audio->audioOut(sdpS,myNumber ));
		sdpS->setAudioHomePort(MyAuPort);
	}
	if( call->withExtCall() ){
	    getExtSocket(callT);
	    MyPort = QString::number(KWsocket->getPortNumber());
    	    sdpS->setExtHomePort(MyPort);
	    if(sessionC->isStun() ) {
		auxPort=QString::number(getStunPort());
	    } else {
		auxPort = MyPort;
	    }
	    sdpS->setSExtHomePort(auxPort);
	}
	if(!member->requestInvite(sdpS->prepInviteString( call->getCallType() ,MyAuPort,auxPort), MimeContentType( "application/sdp" ) ))   {
	detachFromCall(call);
	setHide();
	informPhoneView(UPDATEWLIST,myNumber);

	}
	setLoadText (call->getCallType());
//Common Handling
	curstate = Calling;
	setCS( member->getLocalStatusDescription() );
// we initiated a new call
}
void KCallWidget::stopRinging(void) {
// kill ringtone child process if user accepts or declines the callif
if(debug >=  2)cout << "=====KCallWidget::stopRinging\n";

	if(ringTimer)	ringTimer->stop();
	if(ringtonepid )  {
		kill( ringtonepid, SIGKILL );
		ringtonepid = 0;
	}
}

void KCallWidget::ringTimeout( void ) {
if(debug >=  2)cout << "=====KCallWidget::ringTimeout \n";
setCS( member->getLocalStatusDescription() );
 bool ok;	   
int auaring  = 2000000; //almost forever
int cfnaring = 2000000; //almost forever
if(sessionC->getAua())  auaring  = sessionC->getAuacnt().toInt(&ok,10);//get the real value, if activated
if(sessionC->getCFNA()) cfnaring = sessionC->getCallForwardNAcnt().toInt(&ok,10);//get the real value, if activated
if( (ringCount < cfnaring ) && (ringCount < auaring) ) {
	ringCount++;
	if((ringtonepid == 0) && (isRingingTone == 1) ) {
		QString xSP = sessionC->getRingtoneP();
		QString xSA = sessionC->getRingtoneapp();
		QString xSF = sessionC->getRingtonefile();
		QString xSD = sessionC->getRingtonedevice();
		xSA.replace( "\n" , "");
		xSP.replace( "\n" , "");
		xSP.replace( "_" , " ");
		xSF.replace( "\n" , "");
		xSD.replace( "\n" , "");
		
		QString xSAmod = xSA;
		if( xSAmod.contains( "/" ) ) {
			xSAmod = xSAmod.mid( xSAmod.findRev( '/' ) + 1 );
		}
		QString xSPmod = xSP;
		if(xSPmod.contains("%F")) {
			xSPmod.replace( "%F" , xSF);
		}

		if(xSPmod.contains("%D")) {
			xSPmod.replace( "%D" , xSD);
		}
		ringtonepid = fork ();
		if (!ringtonepid) { // in child process
		if(debug >=  2)cout << "=====KCallWidget::ringTimeout " << xSAmod.latin1() << xSPmod.latin1() << endl;
		execlp( xSA.latin1(), xSAmod.latin1(),xSPmod.latin1(),(char*)NULL);
		exit(1);
		}
    	}
	ringTimer->start( ringTime_3, TRUE );
} else {
	if(ringCount == sessionC->getCallForwardNAcnt().toInt(&ok,10)) {
		if(sessionC->getCFNA()) {
			if(debug >=  2) cout << "=====KCallWidget::ringTimeout CFNA\n";
			    member->forwardCall("Contact: <" + sessionC->getCallForwardNAUri() + ">\r\n");
			    forceDisconnect();
			    return;
			} 
		}
		if (ringCount == sessionC->getAuacnt().toInt(&ok,10) ) {
			if(sessionC->getAua()) {
		if(debug >=  2) cout << "=====KCallWidget::ringTimeout AA\n";
			    emit clickDial( );
			    return;
			}
		}
		hangupCall();	
		return;
	}
}

void KCallWidget::acceptCall( void )
{
	if(acceptC) {
		stopRinging();
		acceptCallTimer->start( acceptTime, true );
		acceptC = false;
	}
}

void KCallWidget::acceptCallTimeout( void )
{
	QString auxPort ="";
	QString MyPort =""; QString MyAuPort = "";
	bool srtpErr=false;
	int loadT;
	loadT = sdpS->checkCodec(false,member,isForceHold(), &srtpErr); 

	int callT=call->saveLoadType(loadT);
	if(debug >=  2) cout <<  "=====KCallWidget: acceptCallT LoadType=" << loadT << " CallType=" << callT << endl;
	call->onlyMeActive(sessionC->activeOnly(callT));
	if( papa->countActiveCalls() > 1) {  //someone else active
	    if( sessionC->activeOnly(callT) || call->withAudioCall()) {//generalized with Flag or audio
		return;
	    }
	}
	
	if( isRingingTone ) {
		::close( audio_fd );
	}

	sessionC->setPermFlags(callT);
	call->onlyMeActive(sessionC->activeOnly(callT));
	if(sessionC->isHoldAllowed(callT)){ 
	} else { 
		call->setHoldMaster(true);
		call->setNoHold(true);
	}
	if(sessionC->isTransferAllowed(callT) ) {
		call->setTransAllowed(true);
	} else {
		call->setTransAllowed(false);
	}
	// Prepare 200 ok
	if( call->loadCall()) { 	
		if (call->withAudioCall()) { 
			MyAuPort = QString::number(audio->audioOut(sdpS,myNumber ));
			sdpS->setAudioHomePort(MyAuPort);
		}
		if(loadT != (int)SipCall::GarKeinCall) {
			QSettings settings;
			getExtSocket(callT);
			MyPort = QString::number(KWsocket->getPortNumber());
			sdpS->setExtHomePort(MyPort);
			if(sessionC->isStun() ) {
				auxPort=QString::number(getStunPort());
			} else {
				auxPort = MyPort;
			}
			sdpS->setSExtHomePort(auxPort);
		}		
		member->acceptInvite( sdpS->prepAcceptString( loadT,MyAuPort,auxPort,false) , MimeContentType( "application/sdp" ) );
		setLoadText(loadT);
	} else {
	setLT(tr("no valid session"));
	member->notAcceptableHere();
	if(debug >=  2) cout << "=====KCallWidget acceptCall:  member->notAcceptableHere \n";
	papa->writeLastError(tr("Accepted codec not found"));

} 
}

void KCallWidget::clickHangup()
{
		   hangupCall();
}


void KCallWidget::holdCall( void )   {
	if(debug >=  2) cout << "=====KCallWidget toggle hold\n";
	preventSessionStart=false;
	int callT= (int)call->getCallType();
	if(debug >=  2) cout << "=====KCallWidget ";
	if( call->isHeld())    {   //Call on Hold ?
		if(debug >=  2)cout << " on Hold";
		if( call->getHoldMaster()){
			if(debug >=  2)cout << " hold Master";
			call->onlyMeActive(sessionC->activeOnly(callT));
			if( papa->countActiveCalls() == 0) {  //someone else active
					if(debug >=  2) cout << " request Retrieve";
					doRetrieve();
			} else if( !sessionC->activeOnly(callT) ) {//generalized with Flag or audio
					if(debug >=  2) cout << " request Retrieve";
					doRetrieve();
	  		}
		} else { 
			if(debug >=  2)cout << " nix hold  Master";
		}
		cout << endl;
	} else {
		if(!sessionC->isHoldAllowed(call->getCallType())) return; 
		if(debug >=  2) cout << " request Hold \n";
		doHold();
	}
	
}

void KCallWidget::doHold( void ) {
    call->setHoldMaster(true);
    call->setHoldCall(SipCall::callHold);
    doForceHold(true);
    member->requestReInvite( sdpS->prepAcceptString( call->getCallType() ,sdpS->getAudioHomePort() ,sdpS->getSExtHomePort() ,true)
    ,MimeContentType( "application/sdp" ) );
    setCS( tr("Call on Hold" ));

}

void KCallWidget::doRetrieve( void ) {
if(debug >=  2) cout << "=====KCallWidget do retrieve\n";
QString MyPort = "";
QString MyAuPort = "";;
QString auxPort = "";
noCheckConn = true;
call->setHoldMaster(false);
doForceHold(false);
if ( call->withAudioCall() ) {
	MyAuPort = QString::number(audio->audioOut(sdpS,myNumber ));
	sdpS->setAudioHomePort(MyAuPort);
	}
if( call->withExtCall() ){ //$
	getExtSocket(call->getCallType());
	MyPort = QString::number(KWsocket->getPortNumber());
	if(sessionC->isStun() ) {
		auxPort=QString::number(getStunPort());
	} else {
		auxPort = MyPort;
	}
	 sdpS->setSExtHomePort(auxPort);
}

	member->requestReInvite( sdpS->prepAcceptString( call->getCallType() ,MyAuPort,auxPort ,false) ,MimeContentType( "application/sdp" ) )  ;
}

void KCallWidget::handleRedirect( void )
{
	if(debug >=  2) cout <<  "=====KCallWidget: Handling redirect...\n";
	preventSessionStart=false;
	QValueList<SipUri>::Iterator it;
	SipUriList urilist = member->getRedirectList();
	while ( urilist.getListLength() > 0 ) {
		SipUri redirto = urilist.getPriorContact();
			if(cmpUri(redirto.reqUri(),thatsMe)) {
	     		    forceDisconnect();
	    		    return;
			}
		QMessageBox mb(tr( "Redirect"),
			tr("Call redirected to: \n") + redirto.reqUri() + "\n\n" +
			tr("Do you want to proceed ? "),
			QMessageBox::Information, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, QMessageBox::Cancel | QMessageBox::Escape );
		switch( mb.exec() ) {
		case QMessageBox::No:
			continue;
		case QMessageBox::Cancel:
			forceDisconnect();
			return;
		}
		int ctSave;
		ctSave=(int)call->getCallType();
		detachFromCall(call);
		if( member ) disconnect( member, 0, this, 0 );
		member = 0;
		if( call ) delete call;
		call = 0;
		// Hier muessen ALLE relevanten Parameter uebergeben werden!!
		redirectCall( redirto, subject, ctSave, 0, 0);
		setHide();
		return;
	}
	forceDisconnect();
}

void KCallWidget::handleRefer( void )
{
	preventSessionStart=false;
	int  ctSave;
        ctSave=(int)call->getCallType();	
	if(debug >=  2) cout <<  "=====KCallWidget: Handling Refer..." << ctSave << endl;
	if(redirTo==RdHoldNo) {
	    redirto = member->getRedirectList().getHead();
	    referTo=redirto.reqUri();
	}
		if(cmpUri(thatsMe,redirto.reqUri())) {
		    sendTrNotify("405 Method not Allowed");
		    
		    return;
		}
	if( sessionC->activeOnly(ctSave) || call->withAudioCall() ) { //put the call on hold, first
	    if(!call->isHeld()) {
		redirTo=RdHoldActive;
		doHold();
		return;
		    }
		}
	
	QMessageBox mb( "Refer", tr("Attempt to establish a new session to: \n") + redirto.reqUri() + "\n\n" + tr("Do you want to proceed ? "),QMessageBox::Information, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, 0 );
	if( mb.exec() == QMessageBox::Yes ) {
	//inform KPhoneView
		redirectCall( redirto, subject,ctSave,this,myNumber );
		member->setState(SipCallMember::state_Connected);
		} else {
		member->setState(SipCallMember::state_Connected);
		}
}

void KCallWidget::sendTrNotify(QString cause) {
	QString theNoti; 
	if(debug >=  2) cout << "=====KCallWidget::sendTrNotify cause=" << cause.latin1() <<   "me=" << myNumber << endl;
	theNoti="SIP/2.0 " + cause + "\r\n";
	if(call->isHeld()) {
	    if( call->getHoldMaster() ) setCS( tr("Connected Hold" )); else setCS(tr( "Connected Held" )); 
	} else {
	    setCS(tr( "Connected") );
	}
	member->requestRNotify(theNoti,MimeContentType( "message/sipfrag;version=2.0"));
}

void KCallWidget::showTransferDialog( bool st ) {
    if(debug >=  2) cout << "=====KCallWidget showTransferDialog\n";
    if(st) { //short transfer requested
	if(sessionC->getIsb()) { 
		 shortTransfer=false; // if not provided do nothing
		 
		 return; 
        } else {
		shortTransfer=true; 
	}
    } else { //transfer w. implicit subscription
	shortTransfer=false;
    }	
    doTransfer = true;
    showDialog();
}

void KCallWidget::showReferDialog( void ) {
 if(debug >=  2) cout << "=====KCallWidget showReferDialog\n";
doTransfer = false;
showDialog();
}

void KCallWidget::showDialog( void )
{
    if(member->getState() == SipCallMember::state_r202 ) return;
    if(call->blockCall() || call->withAudioCall()) {//XX
	if(!call->isHeld()) {
		redirTo=RdHoldForward;
		doHold();
		return;
	} 
    }

    if( !referdialog ) {
	referdialog = new KReferDialog(papa,myNumber,getUserPrefix());
	connect( referdialog, SIGNAL( newSessionRefer( const QString & ) ),
		this, SLOT( newSessionRefer( const QString & ) ) );
    }
	referdialog->show();
}

void KCallWidget::newSessionRefer( const QString &referto )
{
if(debug >=  2) cout << "=====KCallWidget newSessionRefer\n";

// Ignore this button if there is no call setup yet.
	if( curstate != Connected ) {
	    return;
	}
	if( call->getCallStatus() != SipCall::callDead ) {
		QString s = "";
		s+=referto;
		if( s.left( 4 ).lower() != "tel:" ) {
			if( s.left( 4 ).lower() != "sip:" ) {
				s = "sip:" + s;
			}
			if( !s.contains( '@' ) ) {
				s = s + "@" +  member->getUri().getHostname();
			}
		}
		    if(cmpUri(s,member->getUri().uri())  || cmpUri(s,thatsMe)) {
			
			return;
		    }
		redirTo=RdHoldPassive;
		referTo=s;
		member->requestRefer( shortTransfer, SipUri( s ) );
		notiTimer->start(notiTime,TRUE);
		return;

	}
}

void KCallWidget::renegCodec( void )
{
	int callT=0, loadT=0;
	preventSessionStart=false;
	QString MyPort = "";
	QString MyAuPort = "";
	if(debug >=  2) cout << "=====KCallWidget renegCodec \n";
	callT = loadT = call->getCallType();
	// Codec renegotiation is only used to switch video on and off at this time

	detachFromCall(call);

	if (call->withAudioCall()) { 
		// add video
		if (callT == SipCall::StandardCall) {
			call->setCallType(SipCall::auviCall);
			callT = SipCall::auviCall;
		} else 	if (callT == SipCall::auviCall) {
			call->setCallType(SipCall::StandardCall);
			callT = SipCall::StandardCall;
		}
		
		//new port
		MyAuPort = QString::number(audio->audioOut(sdpS,myNumber ));
		sdpS->setAudioHomePort(MyAuPort);
		// new video port	
		if((callT == SipCall::auviCall)) {
			getExtSocket(callT);
			MyPort = QString::number(KWsocket->getPortNumber());
			sdpS->setExtHomePort(MyPort);
			}
		if (callT == SipCall::StandardCall){
			member->requestReInvite( sdpS->prepInviteString((int)SipCall::StandardCall,MyAuPort,MyPort) ,MimeContentType( "application/sdp" )  );
		} else if (callT == SipCall::auviCall){
			member->requestReInvite( sdpS->prepInviteString((int)SipCall::auviCall,MyAuPort,MyPort) ,MimeContentType( "application/sdp" ) );
		}
	}
}

void KCallWidget::callMemberStatusUpdated( void )
{
	bool srtpErr=false;
	QString MyPort  = "";
	QString MyAuPort = "";
	QString auxPort = "";
	int loadT=SipCall::GarKeinCall;
	if(debug >=  2) cout << "\n=====KCallWidget callMemberStatusUpdated memberState= " <<  (int) member->getState() << " mynumber = " << myNumber << endl;

// ---------------------Member State Disconnecting 

if( member->getState() == SipCallMember::state_Disconnecting ) {
	if ( member->getCallMemberType() == SipCallMember::Disc) sipauthentication->authRequest(member,getUserPrefix());
	if(debug >=  2) cout << "state_Disconnecing " << member->getCallMemberType() << endl;
}

// ---------------------Member State Disconnected

else if( member->getState() == SipCallMember::state_Disconnected ) {
	if(debug >=  2) cout << "state_Disconnected\n";
	if( member->getLocalStatusDescription().left( 2 ) == "!!" ) {
		if(referedFrom &&  papa->isExistingWidget(referedNr)) {
		    papa->testWidgetList( );
		    emit sigTrNotify(QString::number(member->getLocalStatusCode()) + " " + member->getLocalStatusDescription().remove(0,2));
		    if(debug >=  2) cout << "=====KCallWidget callMemberStatusUpdated buddy we may emit \n";
		}
		setCS( tr("Call Failed")) ;
		papa->writeLastError(member->getLocalStatusDescription().remove(0,2));
		setHide();
	} else {
		setCS( member->getLocalStatusDescription() );
	}
/*	Use This if you want to inspect all release causes
	setCS( member->getLocalStatusDescription() );
	QMessageBox::critical( this, "Alert",
	member->getLocalStatusDescription());
	setHide();
*/	
	forceDisconnect();

// ---------------------Member State Redirected 

} else if( member->getState() == SipCallMember::state_Redirected ) {
	if(debug >=  2) cout << " state_Redirected: ";
	setCS( member->getLocalStatusDescription() ); 
	if(debug >=  2) cout << " handleRedirect\n";
	handleRedirect();

// ---------------------Member State Refer	

} else if( member->getState() == SipCallMember::state_Refer ) {
	if(debug >=  2) cout << " state_Refer: handleRefer\n";	
	member->setState( SipCallMember::state_Refer_handling );
	setCS( member->getLocalStatusDescription() );
	handleRefer(); 

// ---------------------Member State Connected 

} else if( member->getState() == SipCallMember::state_Connected ) {
	
	if(debug >=  2) cout << " state_Connected ";
	if(dspe) {
		delete dspe; dspe=0;
		if(debug >=  2) cout << " earlySBC stopped \n";
	} 	
	
	bool checkC=true; //we always check for SDP-changes
	if(noCheckConn) { checkC = false; noCheckConn = false; } // codec was checked earlyer
	int loadT = sdpS->checkCodec(checkC,member,isForceHold(),&srtpErr); 
	if ( (loadT == SipCall::GarKeinCall) && !detach) return; //we have a running call and no change in media 
	if(debug >=  2) cout << " loadT=" << loadT;

	if (srtpErr) {
		hangupCall(); 
		return;
	}
		
	call->setAck(true);

	if(loadT == (int)SipCall::putMeOnHold ) { //peer attempt to put us on hold
		if( call->getHoldMaster() ) setCS(tr( "Connected Hold") ); else setCS(tr( "Connected Held") ); 
		if(redirTo==RdHoldActive) {  //Hold SubCycle started by handleRefer
			if(debug >=  2) cout << " RdHoldActive\n";
			redirTo=RdHoldNo;
		        if(call->getNoHold()){
			    hangupCall();
			    return;
			}	    
			doPayload(loadT,false);
			referTimer->start(referTime,TRUE); // we wait for a notification patiently, before we released the call
			return;
		} else if(redirTo==RdHoldPassive) { //Hold SubCycle started by peer, i.e. the passive side
			if(debug >=  2) cout << " RdHoldPassive\n";
			member->setState(SipCallMember::state_r202);
			redirTo=RdHoldNo;
		}
	} else {
		if ( loadT != SipCall::GarKeinCall) {
		call->saveLoadType (loadT);
		setCS( member->getLocalStatusDescription() );
        	setLoadText(loadT);
		}
		if(referedFrom &&  papa->isExistingWidget(referedNr)) {
			emit sigTrNotify("200 OK");
			if(referedFrom) {
	        	    disconnect(this,0,referedFrom,0);
			    
        		}
			referedFrom=0; referedNr=0;
		}
	}
	
	//Notify the Original A-Side 
	curstate = Connected;
	
	if(loadT){
		doPayload(loadT,false);
		if(redirTo==RdHoldForward) { // we started refer and had to put the call on hold first
			redirTo=RdHoldNo;
			showDialog();
			return;
		} 
		if (dtmfSize >0) sendDTMFSeq();
	} else { 
		if(detach) {
		    setLT(tr("no valid session"));
		    member->notAcceptableHere();
		}
		return;
	}

// ---------------------Member State state_InviteRequested

} else if( member->getState() == SipCallMember::state_InviteRequested ) {
	
	if(debug >=  2) cout << " state_InviteRequested\n";
	
	//Notify the Original A-Side
	if(referedFrom &&  papa->isExistingWidget(referedNr)) { 
		emit sigTrNotify("100 Trying");
	}
	
	setCS( tr("Trying") );
	
	if ( member->getCallMemberType() == SipCallMember::Invite) sipauthentication->authRequest(member,getUserPrefix());

// ---------------------Member State state_ReInviteRequested 

} else if( member->getState() == SipCallMember::state_ReInviteRequested ) {

	if(debug >=  2) cout << " state_ReInviteRequested curstate = " << curstate << endl;
	if ( member->getCallMemberType() == SipCallMember::Invite) sipauthentication->authRequest(member,getUserPrefix());

// ---------------------Member State RequestingReInvite 

} else if( member->getState() == SipCallMember::state_RequestingReInvite ) {
	if(debug >=  2) cout << " state_RequestingReInvite curstate = " << curstate << endl;
	noCheckConn = true;
	// Hold retrieve and ICM for the called leg 
	loadT = sdpS->checkCodec(true, member, isForceHold(), &srtpErr);
	if (loadT == SipCall::noChange) { // no new session parameters
	    if( (sdpS->getRealLoad() == SipCall::StandardCall) && (call->getCallType() == SipCall::auviCall) ) { //ICM: audi+video ->audio
		loadT = sdpS->getRealLoad();
	    } else {
		preventSessionStart=true;	//nothing to be done, as the load did not change
						// but we answer gently
		member->acceptInvite( sdpS->prepAcceptString( sdpS->getRealLoad() ,sdpS->getAudioHomePort(),sdpS->getSExtHomePort() ,false), MimeContentType( "application/sdp" ) );
		return; 
	    }
	}
	 //we are on hold or received a renegotiate request : accept new INVITE
	 if(loadT == SipCall::GarKeinCall)  {// no codec at all
		if(debug >=  2) cout << " connected something inacceptable\n";
		setLT(tr("no valid session"));
		member->notAcceptableHere();
		papa->writeLastError(tr("Can't accept call, No accepted codec found."));
	} else if(loadT< SipCall::putMeOnHold) { // retrieve or new media
		int callT = call->saveLoadType(loadT); // save the new CallState
		
		// if we are on hold, specific handling follows
		if( call->isHeld())    {   //Call on Hold ?
			call->onlyMeActive(sessionC->activeOnly(callT));// Check if the resouurces are free
			if( papa->countActiveCalls() > 0) { //someone else active
				if( sessionC->activeOnly(callT) || call->withAudioCall()) {//generalized with Flag or audio
					papa->writeLastError(tr("Can't accept call, No resources"));
					member->notAcceptableHere();
					return;
				}
			}
			if(call->getHoldMaster() ){			//Only Hold initiator may retrieve
			    member->acceptInvite( sdpS->prepAcceptString( call->getCallType() ,sdpS->getAudioHomePort() ,sdpS->getSExtHomePort() ,true), MimeContentType( "application/sdp" ) );
			    return;
			}
		}
		if( !call->isHeld() ) { // we clean our payload if not yet done 
		    detachFromCall(call);
		}

		call->setHoldCall(SipCall::callActive); 			//allow later holds 
		setCS( member->getLocalStatusDescription() );
		if ( call->withAudioCall() ) { // get new audio resources
			if(debug >=  2) cout << "=====KCallWidget statusUpdated call active\n";
			MyAuPort = QString::number(audio->audioOut(sdpS,myNumber ));
			sdpS->setAudioHomePort(MyAuPort);
		}
		if( call->withExtCall() ) { //resources for external payload
			getExtSocket(call->getCallType());
			MyPort = QString::number(KWsocket->getPortNumber());
			sdpS->setExtHomePort(MyPort);
			if(sessionC->isStun() ) {
				auxPort=QString::number(getStunPort());
			} else {
				auxPort = MyPort;
			}
			sdpS->setSExtHomePort(auxPort);
		}
		// send invite
		member->acceptInvite( sdpS->prepAcceptString( loadT ,MyAuPort,auxPort ,false), MimeContentType( "application/sdp" ) );
		setLoadText(loadT); 
		
	}  else { // we are asked to put the call on hold 

		if(call->getNoHold()) { //Hold forbidden ?
			papa->writeLastError(tr("Feature Call Hold not allowed, Call released."));
			member->notAcceptableHere();
			return;
		}
		
		if(debug >=  2) cout << " accept Hold\n";
		call->setHoldCall(SipCall::callHold); 			//we are a hold call 
		member->acceptInvite( sdpS->prepAcceptString( call->getCallType() ,sdpS->getAudioHomePort() ,sdpS->getSExtHomePort() ,true), MimeContentType( "application/sdp" ) );
		
		if(redirTo==RdHoldActive) { //Hold SubCycle started by handleRefer
			if(debug >=  2) cout << " RdHoldActive\n";
			redirTo=RdHoldNo;
			detachFromCall(call);
			referTimer->start(referTime,TRUE);
			return;
		} else if(redirTo==RdHoldPassive) { //Hold SubCycle passive side
			if(debug >=  2) cout << " RdHoldPassive\n";
			member->setState(SipCallMember::state_r202);
			redirTo=RdHoldNo;
		}
		detachFromCall(call);
	}

// ---------------------Member State r202

} else if( member->getState() == SipCallMember::state_r202 ) {
	refnoti=call->getRefNoti();
	if( (call->loadCall()) && (refnoti != QString::null) ) { //we received a NOTIFY refer
	if(debug >=  2) cout << " state r202, refnoti= <" << refnoti.latin1() << ">" << endl; 
		if(refnoti.contains("200"))	{ // New Session established, we may say good by (CT) or keep the session
			notiTimer->stop();
			if(doTransfer) {
				if(debug >=  2) cout << " doTransfer\n";
				doTransfer=false;
				 hangupCall(); 
				return;
			} 
			QMessageBox mb( "Refer", tr("Session established You like to release? "),QMessageBox::Information, QMessageBox::Yes | QMessageBox::Default, QMessageBox::No, 0 );
			if( mb.exec() == QMessageBox::Yes ) {
				 hangupCall(); 
			} else {
				if(call->isHeld()) {
					if( call->getHoldMaster() ) setCS( tr("Connected Hold") ); else setCS(tr( "Connected Held") ); 
				} else {
					setCS( tr("Connected") );
				}
			hideCall();
			return;
			}
		} else if(refnoti.contains("100") || refnoti.contains("180")){ // good Start
			//refresh notiTimer
			notiTimer->stop();
			notiTimer->start(notiTime,TRUE);
			setCS(tr( "CT ongoing" ));
		} else {			
			if (notiTimer->isActive()) notiTimer->stop();
			call->setRefNoti(QString::null);
			refnoti="none";
			member->setState(SipCallMember::state_Connected);
			if(call->isHeld()) {
				if( call->getHoldMaster() ) setCS( tr("Connected Hold")); else setCS( tr("Connected Held")); 

			} else {
				setCS( tr("Connected") );
			}
			hideCall();
			return;
		}
	} 
	 if (call->getRefSub() == "false") { //peer reflected refer-sub false ?
		if(debug >=  2) cout << " state r202 no Notification!\n"; 
		if (notiTimer->isActive()) notiTimer->stop();
		shortTransfer = true;    
	         forceDisconnect();//hangupCall();
	} else {
		if(debug >=  2) cout << " state r202 nothing to do\n"; 
	        shortTransfer=false;    
		}	

// ---------------------Early dialog + PRACK
} else if( member->getState() == SipCallMember::state_EarlyDialog ) {

    if(debug >=  2) cout << " state EarlyDialog: prack state " << call->getPrackReq(); 
	
	if ( call->getPrackReq() == SipCall::SPSreq ) { // PRACK requested
		member->requestPrack();
		call->setPrackReq(SipCall::SPSsent);
		return;
	} else if (call->getPrackReq() == SipCall::SPSsup) { // Prack supported
		if (sessionC->getPPrack() == 1) { // we use it
		member->requestPrack();
		call->setPrackReq(SipCall::SPSsent);
		return;
		}
	}
	if (call->getPrackReq() == SipCall::SPSsent ) {
	    call->setAck(true);
	}

	int oct=call->getCallType();
	int loadT = sdpS->checkCodec(true,member,false,&srtpErr); 
	int callT = call->saveLoadType(loadT);
	if(debug >=  2) cout << "  loadT = " << loadT << " callT= " << callT ;
	
	if (srtpErr) {
		if(debug >=  2) cout << "  error\n";
		hangupCall(); 
		return;
	}

	if(loadT == SipCall::putMeOnHold) { //makes no sense -> ignore
		if(debug >=  2) cout << "  hold\n";
	} else if ( (loadT > 0) && (loadT < SipCall::noChange)) {
		if ( (oct >= SipCall::extCall1) && (oct <= SipCall::extCall4) ) {
			if(dspe) delete dspe; dspe=0;
			dspe = new DspEarly();
			dspe->start(sdpS->getRtpCodec(), sdpS->getRtpCodecNum(), audio->getSocket(), sdpS->getExtHost(), sdpS->getAudioPort() );
                        if(debug >=  2) cout << "  DspEarlyactive\n";
			
		} else {
		    if(debug >=  2) cout << "  active\n";
		    call->onlyMeActive(sessionC->activeOnly(callT));
		    doPayload((int)call->getCallType(),true);
		    setLoadTextS ((int)call->getCallType());
		}
	} else {
		if(debug >=  2) cout << "  No change/contents\n";
	}
	

// ---------------------state_Update

} else if( member->getState() == SipCallMember::state_Update ) {
	if(debug >=  2) cout << " UPDATE  callmemberstate = " << member->getState() << " curstate = " << curstate << " UpdateST= " <<  call->getUpdateST() << endl;
	if ( (!call->getAck()) || (curstate == Called) ){
	    if(debug >=  2) cout << "  noUpdate\n";
	    member->noUpdateHere();
	    updateTimer->start(updateTime,TRUE);
	    return;
	}

	int loadT = sdpS->checkCodec(true,member,false,&srtpErr); 
	int callT = call->saveLoadType(loadT);
	upLoad = true;
	if(debug >=  2) cout << "  loadT = " << loadT << " callT= " << callT << " Update\n";
	
	if (srtpErr) {
		hangupCall(); 
		return;
	}
	if(loadT == SipCall::putMeOnHold) {
		if(debug >=  2) cout << "  Update hold\n";
		detachFromCall(call);
	    	upLoad = false;	//no new Media
		member->acceptUpdate( sdpS->prepAcceptString( loadT,sdpS->getAudioHomePort() ,sdpS->getSExtHomePort(),true) , MimeContentType( "application/sdp" ) );
		call->setHoldMaster(true);
	        call->setHoldCall(SipCall::callHold);
	        doForceHold(true);
	        setCS( tr("Call on Hold" ));
	} else if ( (loadT > 0) && (loadT < SipCall::noChange)) {
			call->onlyMeActive(sessionC->activeOnly(callT));
			if( papa->countActiveCalls() == 0) {  //someone else active
				setCS( tr("Conn OK" ));
				detachFromCall(call);
			} else if( !sessionC->activeOnly(callT) ) {//generalized with Flag or audio
				setCS( tr("Conn OK" ));
				detachFromCall(call);
	  		} else {
			    member->noUpdateHere();
			    upLoad = false;
			    //updateTimer->start(updateTime,TRUE);
			    return;
			}	
	    if ( call->withAudioCall() ) {
		if(debug >=  2) cout << "=====KCallWidget statusUpdated call active\n";
		MyAuPort = QString::number(audio->audioOut(sdpS,myNumber ));
		sdpS->setAudioHomePort(MyAuPort);
	    }
	    if( call->withExtCall() ){
		getExtSocket(callT);
		MyPort = QString::number(KWsocket->getPortNumber());
    		sdpS->setExtHomePort(MyPort);
		if(sessionC->isStun() ) {
		    auxPort=QString::number(getStunPort());
		} else {
		    auxPort = MyPort;
		}
		sdpS->setSExtHomePort(auxPort);
	    }
	    member->acceptUpdate( sdpS->prepAcceptString( loadT,MyAuPort,auxPort,false) , MimeContentType( "application/sdp" ) );
	} else {
	    	upLoad = false;	
		if(debug >=  2) cout << "  Update no change\n";
		member->acceptUpdate( sdpS->prepAcceptString( callT,sdpS->getAudioHomePort() ,sdpS->getSExtHomePort(),false) , MimeContentType( "application/sdp" ) );	
		setCS( tr("Conn OK" ));
	}
	
// ---------------------state_Updated

} else if( member->getState() == SipCallMember::state_Updated ) {
	if(debug >=  2) cout << " UPDATED  callmemberstate = " << member->getState() << " curstate = " << curstate << " updateST = " << call->getUpdateST() <<  endl;
	if(call->getUpdateST() == 0) member->setState(SipCallMember::state_Connected);
	if(upLoad) {
	    doPayload((int)call->getCallType(),false);
	}    
	setLoadTextS ((int)call->getCallType());


// ---------------------The rest

} else  {
	if(debug >=  2) cout << " the rest callmemberstate = " << member->getState() << endl;
	if (call->getPrackReq() == SipCall::SPSsent ) {
	    call->setAck(true);
	}

}
}


void KCallWidget::setRemote( QString newremote )
{	
	if(debug >=  2) cout << "=====KCallWidget setRemote " << newremote << endl;
	remote =  newremote;
}

SipCall *KCallWidget::getCall()
{
	return call;
}

void KCallWidget::setLoadTextS (int load) {
	if( load  == SipCall::videoCall ) {
		setLT("video");
	} else if( load == SipCall::StandardCall ){
		setLT("audio");
	}else if( load == SipCall::auviCall ) {
		setLT("audio+video");
	} else if( call->isExtCall() ) {
		int l=load-SipCall::extCall1+1;
		setLT("ext" + QString::number(l) + ": "+ sessionC->theVectorId((load)));
	}
}

void KCallWidget::setLoadText (int load) {
QString loadtext;
int l = load-SipCall::extCall1+1;
switch (load) {
    case (int)SipCall::extCall1 :
    case (int)SipCall::extCall2 :   
    case (int)SipCall::extCall3 : 
    case (int)SipCall::extCall4 :
	setLT ( "ext" + QString::number(l) + " :" +sessionC->theVectorId(load) ); 
	break;
    case (int)SipCall::StandardCall : 
    	loadtext = "audio:" + sdpS->getPrefCodec();
	setLT(loadtext); 
	break;
    case (int)SipCall::videoCall : 
    	loadtext = "video:" + sdpS->getVidPrefCodec();
    	setLT(loadtext);
	break;
    case (int)SipCall::auviCall : 
    	loadtext = "a+v:" + sdpS->getPrefCodec() + " " + sdpS->getVidPrefCodec();
    	setLT(loadtext);
    }

}

void KCallWidget::audioOutputDead( void )
{
	if(debug >=  2) cout <<  "=====KCallWidget: Broken output pipe, disconnecting unpolitely\n";
	forceDisconnect();
}


void KCallWidget::updateCallRegister( void )
{
	QString s, ss, s1, s2, s3;
	QTime t;
	QDate d;
	QString label;
	IncomingCall *c;
	IncomingCall *first;
	int count = 0;

	QSettings settings;
	QString p = KStatics::cBase + getUserPrefix() + "CallRegister/";

	label.setNum( count );
	label = p + "/Missed" + label;
	s = settings.readEntry( label, "" );
	while( !s.isEmpty() ) {
		settings.writeEntry( label, "");
		label.setNum( ++count );
		label = p + "/Missed" + label;
		s = settings.readEntry( label, "" );
	}
	count = 0;
	label.setNum( count );
	label = p + "/Received" + label;
	s = settings.readEntry( label, "" );
	while( !s.isEmpty() ) {
		settings.writeEntry( label, "");
		label.setNum( ++count );
		label = p + "/Received" + label;
		s = settings.readEntry( label, "" );
	}

	count = 0;
	label.setNum( count );
	label = p + "/Dialled" + label;
	s = settings.readEntry( label, "" );
	while( !s.isEmpty() ) {
		settings.writeEntry( label, "");
		label.setNum( ++count );
		label = p + "/Dialled" + label;
		s = settings.readEntry( label, "" );
	}
	
	
	int i = 0;
	int last = receivedCalls.count();
	if ( last > 30 ) {
		first = receivedCalls.at( last - 30 );
	} else {
		first = receivedCalls.first();
	}
	for ( c = first; c != 0; c = receivedCalls.next() ) {
		label.setNum( i++ );
		label =  p + "/Received" + label;
		t = c->getDateTime().time();
		d = c->getDateTime().date();
		ss = s1.setNum( d.year() ) + "." + s2.setNum( d.month() ) +	"." + s3.setNum( d.day() ) + "-" + t.toString();
		settings.writeEntry( label, ss + " {" + c->getContact() + "}");
	}
	
	i = 0;
	last = missedCalls.count();
	if ( last > 30 ) {
		first = missedCalls.at( last - 30 );
	} else {
		first = missedCalls.first();
	}
	for ( c = first; c != 0; c = missedCalls.next() ) {
		label.setNum( i++ );
		label = p + "/Missed" + label;
		t = c->getDateTime().time();
		d = c->getDateTime().date();
		ss = s1.setNum( d.year() ) + "." + s2.setNum( d.month() ) +
			"." + s3.setNum( d.day() ) + "-" + t.toString();
		settings.writeEntry( label, ss + "{" + c->getContact() + "}");
	}
	
	i = 0;
	last = dialledCalls.count();
	if ( last > 30 ) {
		first = dialledCalls.at( last - 30 );
	} else {
		first = dialledCalls.first();
	}
	for ( c = first; c != 0; c = dialledCalls.next() ) {
		label.setNum( i++ );
		label = p + "/Dialled" + label;
		t = c->getDateTime().time();
		d = c->getDateTime().date();
		ss = s1.setNum( d.year() ) + "." + s2.setNum( d.month() ) +
			"." + s3.setNum( d.day() ) + "-" + t.toString();
		settings.writeEntry( label, ss + "{" + c->getContact() + "}");
	}
}

void KCallWidget::hideCall( void )
{
	if( curstate == PreDial ) {
		if(call) {
		delete call;
		call = 0;
		}
		hided = true;
	}
		
		 
}

QString KCallWidget::getUserPrefix( void ) {
	return ((KPhone *)papa->parentWidget())->getUserPrefix();
}

void KCallWidget::MyIdentity(void) {

    QSettings settings;
    int i,j;
    QString p = KStatics::dBase + getUserPrefix() + "Registration/SipUri";
    QString s = settings.readEntry( p, "" );
    QString si;
    i=s.find("<sip:");
    j=s.find(">");
    thatsMe = s.mid(i+1,j-i-1);
}
bool KCallWidget::cmpUri(QString uri1, QString uri2) {
int i,j,l;

i = uri1.find("@");
if (i<0) return true;

j = uri1.find(";");
if(j>0) l=j; else l = uri1.length();

QString user1 = uri1.mid(4,i-4); 
QString host1 = uri1.mid(i+1, l-i-1);

i = uri2.find("@");
j = uri2.find(";");
if(j>0) l=j; else l = uri2.length();

QString user2 = uri2.mid(4,i-4);
QString host2 = uri2.mid(i+1, l-i-1);

    if (user1 == user2) {
	if (cmpHostDot(host1,host2)) {
	    return true; 
	} else {
	    return false;
	} 
    } else	{
	return false;
    }
}

bool KCallWidget::cmpHostDot(QString host1, QString host2) {
  struct addrinfo hints, *res1,*res2;
  int errcode,cmp,j;
  char addrstr1[100],addrstr2[100];
  void *ptr1, *ptr2;
  time_t begin, end;
  double gaidiff;
  
  if(KStatics::noLine >=1 ) {
	   if(debug >=  2) cout << "=====KCallWidget::cmpHostDot no getaddrinfo\n";
	    return false;
  } 
  
  memset (&hints, 0, sizeof (hints));
 
	if ( host1.contains("[") ) {
		host1=host1.mid( host1.find("["),  host1.find("]") + 1 );
	} else {
        	j = host1.find(":");
		if(j>0) {
			host1=host1.mid(0,j);
		} 
	}


	if ( host2.contains("[") ) {
		host2=host2.mid( host2.find("["),  host2.find("]") + 1 );
	}else {
		j = host2.find(":");
		if(j>0) {
			host2=host2.mid(0,j);
		} 
	}


  time(&begin);
  errcode = getaddrinfo (host1.latin1(), NULL, &hints, &res1);  
  
  time(&end);
  gaidiff=difftime(end,begin);
  
  if (gaidiff > 1.50E1) {
        KStatics::noLine=1;
 }
 
 if (errcode != 0)
    {
      perror ("KCallWidget::cmpHostDot getaddrinfo");
      return true;
    }
errcode = getaddrinfo (host2.latin1(), NULL, &hints, &res2);  
  if (errcode != 0)
    {
      perror ("KCallWidget::cmpHostDot getaddrinfo");
      return true;
    }
    
      inet_ntop (res1->ai_family, res1->ai_addr->sa_data, addrstr1, 100);

      switch (res1->ai_family)
        {
        case AF_INET:
          ptr1 = &((struct sockaddr_in *) res1->ai_addr)->sin_addr;
          break;
        case AF_INET6:
          ptr1 = &((struct sockaddr_in6 *) res1->ai_addr)->sin6_addr;
          break;
        }
	
	inet_ntop (res2->ai_family, res2->ai_addr->sa_data, addrstr2, 100);

      switch (res2->ai_family)
        {
        case AF_INET:
          ptr2 = &((struct sockaddr_in *) res2->ai_addr)->sin_addr;
          break;
        case AF_INET6:
          ptr2 = &((struct sockaddr_in6 *) res2->ai_addr)->sin6_addr;
          break;
        }
	
	memset (&addrstr1,0, 100); 
	memset (&addrstr2,0, 100);
	inet_ntop (res1->ai_family, ptr1, addrstr1, 100); 
	inet_ntop (res2->ai_family, ptr2, addrstr2, 100);
	cmp=memcmp(addrstr1, addrstr2, 100);
	freeaddrinfo(res1);
	freeaddrinfo(res2);
	if(cmp==0) return true; else return false;
}


void KCallWidget::setHide( void )
{
	hided = true;
	
}


void KCallWidget::hangupCall( void )
{
	informPhoneView(UPDATEWLIST,myNumber);
	if(debug >=  2) cout <<  "=====KCallWidget hangupCall:  curstate = " << curstate << endl;
	preventSessionStart=true; //don't accept call, while hangup is in the queue
	stopRinging();
	if(member) {
	// Reject call if that's our current state
	    if( curstate == Called ) {
		member->declineInvite();
	    } else 	if( call->getCallStatus() != SipCall::callDead ) {
		if( ( member->getState() == SipCallMember::state_Connected) ||  ( member->getState() >= SipCallMember::state_Refer ) ) {
			member->requestDisconnect();
		} else {
			member->cancelTransaction();
		}
	    }
	}
	curstate=Releasing; //we begin to release the session
	detachFromCall( call ); // stop audio and external resources if any
        if(call) call->setHoldCall(SipCall::preCall); //having no resources
	setHide();
	return;
}




void KCallWidget::detachFromCall( SipCall *lcall )
{
	
	detach = true;

	if(debug >=  2) cout << "=====KCallWidget: detachFromCall " << myNumber << endl;
	int locPid = 0;
	if( lcall ) {
		locPid = lcall->getCurPid();
	}
	if( locPid ) {
		if(debug >= 2) cout << "=====KCallWidget: detachFromCall  kill Pid  " <<  locPid << endl;
		kill( locPid, SIGKILL );
		lcall->setCurPid(0);
	}
	
	if(dspe) {
		    if(debug >= 2) cout << "=====KCallWidget: detachFromCall  stop early media\n";
		    delete dspe; dspe=0;
		}
		
	if(audio) {
		if(debug >= 2) cout << "=====KCallWidget: detachFromCall  stop sending audio\n";
		audio->stopAudio(myNumber);
		}

	
	if (KWsocket != 0) { 
	    if(debug >= 2) cout << "=====KCallWidget: detachFromCall  delete KWsocket\n";
	    delete KWsocket; 
	    KWsocket=0;
	}
	if(debug >=  2) cout << endl;

}

void KCallWidget::detachFromRTP( SipCall *lcall )
{
	
	detach = true;

	if(debug >=  2) cout << "=====KCallWidget: detachFromCall " << myNumber << endl;
	int locPid = 0;
	if( lcall ) {
		locPid = lcall->getCurPid();
	}
	if( locPid ) {
		if(debug >= 2) cout << "=====KCallWidget: detachFromCall  kill Pid  " <<  locPid << endl;
		kill( locPid, SIGKILL );
		lcall->setCurPid(0);
	}
	
	if(dspe) {
		    if(debug >= 2) cout << "=====KCallWidget: detachFromCall  stop early media\n";
		    delete dspe; dspe=0;
		}
		
	if(audio) {
		if(debug >= 2) cout << "=====KCallWidget: detachFromCall  stop sending audio\n";
		audio->stopRTPAudio(myNumber);
		}

	
	if (KWsocket != 0) { 
	    if(debug >= 2) cout << "=====KCallWidget: detachFromCall  delete KWsocket\n";
	    delete KWsocket; 
	    KWsocket=0;
	}
	if(debug >=  2) cout << endl;

}


void KCallWidget::forceDisconnect( void ){

	curstate=Releasing; //we begin to release the session
	if(sdpS) {
	    delete sdpS;
	    sdpS=0;
	}
	updateTimer->stop();
	if(debug >=  2) cout <<  "=====KCallWidget: Starting force disconnect...  \n";
	preventSessionStart=true; //don't accept call, while hangup is in the queue

	detachFromCall(call);
        if(call) call->setHoldCall(SipCall::preCall); //having no resources
	if( call ) delete call;
	call=0;
	callDeleted();
	stopRinging();
	//a running notiTimer interfers with short refer!
	if (notiTimer->isActive()) notiTimer->stop();
	setHide();
	informPhoneView(UPDATEWLIST,myNumber);
}

void KCallWidget::doPayload( int load, bool tone ){

    	if(preventSessionStart) {
	if(debug >=  2) cout <<  "\n=====KCallWidget::doPayload  prevent session start" <<endl;
	    preventSessionStart=false;
	    return;  //do exactly what the flag tells you
	}

	if(debug >=  2) cout <<  "\n=====KCallWidget::doPayload  " << load ;
	
	if (tone ) {
	    haveTone = true; //reset after early ringing
	    if(debug >=  2) cout <<  " tone " << tone << " haveTone " << haveTone << endl;
	} else {
	    if(debug >=  2) cout << endl;
	
	}
	
	QSettings xsettings;
	QString xSW;
	QString str1;
	QString xSP;
	QString xSWmod = xSW;
	QString xSPmod = xSP;
	QString xpCodec;
	// That's a nasty trick, to protect us from illicit 200OK from Xten
	if(isForceHold()) {
	    load = (int)SipCall::putMeOnHold;
	}
	switch(load){
		case  (int)SipCall::extCall1:
		case  (int)SipCall::extCall2:
		case  (int)SipCall::extCall3:
		case  (int)SipCall::extCall4:
				xSP = sessionC->forkSP(load);
    				xSW = sessionC->forkSW(load);
				xSWmod = xSW;
				xSPmod = xSP;
				if( xSWmod.contains( "/" ) ) {
				  xSWmod = xSWmod.mid( xSWmod.findRev( '/' ) + 1 );
				}
				xSPmod.replace( "\n" , "");
				if(xSPmod.contains("%A")) {
				  QString xSWAdr = sdpS->getExtHost();
				  xSWAdr.replace( "\n" , "");	
				  xSPmod.replace( "%A" , xSWAdr);
				}
				if(xSPmod.contains("%P")) {
				  QString xSWPp = sdpS->getExtPort();
				  xSWPp.replace( "\n" , "");	
				  xSPmod.replace( "%P" , xSWPp);
				}
				if(xSPmod.contains("%O")) {
				  QString xSWPort = sdpS->getExtHomePort();
				  xSWPort.replace( "\n" , "");	
				  xSPmod.replace( "%O" , xSWPort);
				}
				if(xSPmod.contains("%I")) {
				  xSPmod.replace( "%I" , dirFlag);
				}
				if(xSPmod.contains("%D")) {
				  xSPmod.replace( "%D" , getUserPrefix() + " " + dirFlag + " " + call->getCallId() );
				}
				if(xSPmod.contains("%M")) {
				  xSPmod.replace( "%M" , thatsMe);
				}
				if(xSPmod.contains("%C")) {
				  xSPmod.replace( "%C" , member->getUri().uri().latin1());
				}
				if(xSPmod.contains("%L")) {
				QString my = KStatics::myIP;
				my.replace("[","");
				my.replace("]","");
				xSPmod.replace( "%L" , my);
				}
				if(xSPmod.contains("%T")) {
				  xSPmod.replace( "%T" , QString::number(sessionC->useL4((int)call->getCallType())));
				}
				if(xSPmod.contains("%R")) {
					if(referTo == "") {
						if(referedNr!=0) {
							referTo =  member->getUri().uri().latin1();
						} else {
							referTo = thatsMe;
						} 
					}
					xSPmod.replace( "%R" , referTo.latin1());
				}
				break;
                case  (int)SipCall::StandardCall:
				if(debug >=  2) cout << "\n=====KCallWidget: doPayload Audio\n";
				if( !audio->audioIn(sdpS,tone,myNumber) ) {
				    forceDisconnect();
			            return;
				}
				detach = false;
				call->setHoldCall(SipCall::callActive);
				if(!call->getNoHold()) {
				   call->setHoldMaster(false); // Holdmaster blocks subsequent INVITEs
				}
				return;
                case  (int)SipCall::auviCall:
				if(debug >=  2) cout << "\n=====KCallWidget: doPayload Audio\n";
				if( !audio->audioIn(sdpS,tone,myNumber) ) forceDisconnect();
				detach = false;
				call->setHoldCall(SipCall::callActive);
				if(!call->getNoHold()) {
				   call->setHoldMaster(false); // Holdmaster blocks subsequent INVITEs
				}
                case  (int)SipCall::videoCall:
				if(debug >=  2) cout << "\n=====KCallWidget: doPayload Video\n";
		    		xSW = sessionC->getVideoSW();
				xSP = "";
				xSWmod = xSW;
				xSPmod = xSP;
				xpCodec = sdpS->getVidPrefCodec().lower();
				if(xpCodec.contains("h263-1998")) xpCodec = "h263+";// Xten
				if( xSWmod.contains( "/" ) ) {
				  xSWmod = xSWmod.mid( xSWmod.findRev( '/' ) + 1 );
				}
				xSPmod = sdpS->getExtHost() + "/" + sdpS->getExtPort() + "/" 
				         + xpCodec + "/16/" + sdpS->getExtHomePort() ;
				break;
		case  (int)SipCall::putMeOnHold:
				if(debug >=  2) cout << "\n=====KCallWidget: doPayload On Hold\n";
			       if(call->getNoHold()){
			         forceDisconnect();
			         return;
				}	    
	    			call->setHoldCall(SipCall::callHold);
				if (KWsocket != 0) { 
				    if(debug >= 2) cout << " delete KWsocket\n";
				    delete KWsocket; KWsocket=0;
				}
			        detachFromRTP(call);//detachFromCall(call);
			      return;		
		default: 
			    if (haveTone & (member->getState() == SipCallMember::state_Connected)) {
				if(debug >=  2) cout <<  "=====KCallWidget::doPayload change tone to alsa\n";
				audio->audioTone(tone,myNumber);
			    }
			    haveTone = false;
			    return;
	}
	// common part for all external Payloads
	call->setHoldCall(SipCall::callActive);
	if(!call->getNoHold())  {
	if(debug >=  2) cout <<  "=====KCallWidget::doPayload common set hm false\n";
	    call->setHoldMaster(false); // Holdmaster blocks subsequent INVITEs
	}
	if (KWsocket != 0) { 
	    if(debug >= 2) cout << " delete KWsocket\n";
	    delete KWsocket; KWsocket=0;
	}
	int locPid;
	if(debug >= 2) cout << "\n=====KCallWidget: doPayload We are forking (" << xSW.latin1() << ")(" << xSWmod.latin1() << ") (" << xSPmod.latin1() << ")\n";
	locPid  = fork();
	call->setCurPid(locPid);
	if( !locPid ) {
	    execlp( xSW.latin1(), xSWmod.latin1(), xSPmod.latin1(),(char*)NULL );
	    detach = false;
	    exit(1);
	}  

}

void KCallWidget::sendDTMFSeq(void) {

    if( (dirFlag=="o") && call->withAudioCall()) {
	
audio->startDTMFSeq(dtmfStr,dtmfSize);
    } 	
    dtmfSize = 0;
}

void KCallWidget::getExtSocket(int callt) {
	int minP=0;
	int maxP=0;
	maxP= sessionC->getMaxApplPort();
	minP= sessionC->getMinApplPort();
	if(dirFlag == "o") minP+=2;
	KWsocket = new 	UDPMessageSocket(KStatics::haveIPv6);
	KWsocket->listenOnEvenPort(minP,maxP); //Max und MinPort!!!
	
}

void KCallWidget::setCS  (QString in) {
	myCS=in;
	informPhoneView(UPDATEWLIST,myNumber);
}

void KCallWidget::setLT  (QString in) {
	myLT=in;
	informPhoneView(UPDATEWLIST,myNumber);
}

int KCallWidget::getStunPort(void) {
	int portnum=0;
	SipUri stun;
	stun = SipUri( sessionC->getStunSrv() );
	if ( KWsocket->sendStunRequest( stun.getHostname(), stun.getPortNumber() ) == 0) {
		portnum = KWsocket->receiveStunResponse();
	}
	if(debug >= 2) cout << "\n=====KCallWidget::getStunport port=" << portnum << endl;
	return portnum;
}
