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
#ifndef KCALLWIDGET_H_INCLUDED
#define KCALLWIDGET_H_INCLUDED

//#include <qdialog.h>
#include <qptrdict.h>
#include <qevent.h>
#include <time.h>

#include "../dissipate2/sipuri.h"
#include "../dissipate2/sdpbuild.h"
#include "../dissipate2/sipmessage.h"
#include "../dissipate2/sipcall.h"
#include "../dissipate2/udpmessagesocket.h"
#include "kphonebook.h"
#include "kreferdialog.h"
#include "kinstantmessagewidget.h"
#include "../kaudio/dspearly.h"


class QLabel;
class QLineEdit;
class KComboBox;
class QPushButton;
class QTextEdit;
class SipUser;
class SipCall;
class SipClient;
class SipCallMember;
class SipTransaction;
class SessionControl;
class IncomingCall;
class SipMessage;
class KSipAuthentication;
class KPhoneView;
class CallAudio;



/**
* @short the graphic representation and call control of the call
*
* The graphic representation of the call.
* The class sets up the user interface to  a running call and
* is also responsible for the call control logic. 
* Functions as start and accept INVITE messages, Codec checks
* the start of the payload session and call release are handeled here. 
* The method callMemberStatusUpdated connects this class with SipCall and SipCallMember.
*
* KCallWidget is the startpoint for originating calls.
*/
class KCallWidget : public QObject
{
	Q_OBJECT
public:
	KCallWidget( KSipAuthentication *auth, SessionControl *sessioncontrol, SipCall *initcall, CallAudio *au,
		 KPhoneView *Vater=0, KCallWidget * trfrom = 0, int wNr=0, bool screen = false, const char *name = 0 );
	~KCallWidget( void );
	SipCall *getCall();
 
 
 
/**  
* Prepare a new Call,activate Buttons, fill informal Text
*/
  void switchCall( SipCall *newcall );
 
/** 
*  The remote Party
*/
  void setRemote( QString newremote );
 
/** 
*  Someone pressed the dial button
*/  
  void clickDial( void );
 
/** 
*  Hangup Button pressed
*/
  void clickHangup( void );
 
/** 
*  mark callwidget for deletion
*/  
  void setHide( void );
 
/** 
*  is callwidget marked for deletion
*/  
  bool isHided( void ) const { return hided; }
  
  SipCall * getSipCall(void) const  { return call;}
  
  SipCallMember * getSipCallMember(void) const {return member;}
 
/** 
*  manipulate myCS (holds the actual call state)
*/
  QString getCS  (void)       { return myCS; }
 
/** 
*  manipulate myCS (holds the actual call state)
*/  
 // void    setCS  (QString in) { myCS=in;     }
   void    setCS  (QString in);
/** 
*  manipulate myLT (holds the load type)
*/  
  QString getLT  (void)       { return myLT; }
 
/** 
*  manipulate myLT (holds the load type)
*/  
  void    setLT  (QString in);
 
    /** 
*  manipulate myCon (holds the actual partner URI)
*/  
  QString getCon (void)       { return myCon;}
 
/** 
*  manipulate myCon (holds the actual partner URI)
*/  
  void    setCon (QString in) { myCon=in;    }
  
  int myNumber;
  QString refnoti; 
public slots:
 
/**
* stops the ring timer and starts the call timer
*/
  void acceptCall( void );
/** 
* CFB
*/  
void forwardCall( void );
/** 
*  used for call transfer to start a call B->C
*/
  void pleaseDial( const SipUri &dialuri );
   
/** 
*  hangup the call
*/
 void hangupCall( void );
 
/** 
* Control for the Call Hold feature
* Depending on the Call Hold Flag the call is either put on
* hold or retrieved
*/
  void holdCall( void );

/**
* show the Refer Dialog to ask for the new terminating leg
*/  
  void showReferDialog( void );

/**
* show the Transfer Dialog to ask for the new terminating leg
* st=true means short transfer (w.o. implicit subscription)
*/  
  void showTransferDialog( bool st );

/**
* show the Dialog to ask for the new terminating leg
*/  
  void showDialog( void );
 
/**
* show the Transfer Dialog to ask for the new terminating leg (short CT)
  
  void showShTransferDialog( void );
*/

/**
*  toggle between audio and audio+video
*/
  void renegCodec (void);


signals:
 
/** 
*  trigger REFER receptot to send state Notification to REFER source
*/
  void sigTrNotify( QString cause);
  
  void callDeleted( void );
 
/** 
*  send signal to KPhoneView, to redirect a call 
*/  
  void redirectCall( const SipUri &calluri, const QString &subject, int ct, KCallWidget *, int wNr );
 
/** 
*  send signal to KPhoneView, used at the moment to create/change Call Info in the Active Call List
*/  
  void informPhoneView( int info, int wNr);

private slots:

 
/** 
* determins the call type, Starts the outgoing call
*/
  void dialClicked( void );
 
/** 
* sets the remote start flag and calls dialClicked
*/  
void rdialClicked( void );



  
  void audioOutputDead( void );


/**
* used by holdCall
*/  
  void doHold( void);
 
/** 
* used by holdCall
*/  
  void doRetrieve(void);

/**
* detrmines and starts the activity after SipCall/SipCallMember changed the CallState
*/
  void callMemberStatusUpdated( void );
 
/** 
* ask  the user to accept a CF, and start it
*/
  void handleRedirect( void );

/**
* ask  the user to accept a Refer, and start it
*/  
  void handleRefer( void );
 
/** 
* evaluate the Refer new terminating leg
* and start the new widget
*/  
  void newSessionRefer( const QString &referto );

/** 
* start/restart  ring tone
*/  
  void ringTimeout( void );

/**
* receive evaluate and answer the incoming Invite
*/
  void acceptCallTimeout( void );

/**
*  hide the Callwidget and doom it to be to be cleared, if in predial state
*/  
  void hideCall( void );
  
/**
*  avoid that Xten and friends  mess up Call Hold
*/
  bool isForceHold(void) { return forceHold; }

/**
*       avoid thar Xten and friends  mess up Call Hold
*/
  void doForceHold(bool dh) { forceHold = dh; }

/**
*  set load and codec
*/
  void setLoadText(int load);

/**
*  set load
*/
  void setLoadTextS(int load);
  
protected slots:
 
/** 
*  to send state Notification to REFER source
*/
  void sendTrNotify( QString cause);
 
/** 
* in case of outgoing audio calls sends a DTMF sequence when connected.
*/
  void sendDTMFSeq(void);

private:
/**
 forbide doPayload to start a new session. Used :
 1. in case of a new INVITE without changes in the session description
 2. if a CANCEL competes with INVITE
*/
  int debug;
  bool preventSessionStart;
  QString myCS;
  QString myLT;
  QString myAS;
  QString myCon;
  QString remote;
  QTimer *notiTimer;
  QTimer *referTimer;
  QTimer *viewTimer;
  QTimer *updateTimer;
  QString dtmfStr;
  int dtmfSize;
  int  referTime;
  int  notiTime;
  int  viewTime;
  int  updateTime;
  bool doTransfer;
  void permFlags(void);
  QString dirFlag;
  QPtrList<IncomingCall> receivedCalls;
  QPtrList<IncomingCall> missedCalls; 
  QPtrList<IncomingCall> dialledCalls;
  IncomingCall *incomingCall;
  void updateCallRegister( void );
  KPhoneView  *papa;
  KCallWidget *referedFrom;
  int referedNr;
  bool forceHold;
  // Sip Stuff
  SipCall *call;
  SipCallMember *member;
  SdpBuild *sdpS;
  KSipAuthentication *sipauthentication;
  SessionControl *sessionC;
  // Audio Stuff
  CallAudio *audio;
  int ringCount;
  QTimer *ringTimer;
  int isRingingTone;
  QTimer *acceptCallTimer;
  int audio_fd;
  QString subject;
  UDPMessageSocket *KWsocket;
  DspEarly *dspe;
  bool haveTone;
  bool acceptC;
  bool askRst;
  //session Stuff

/**
* Kill  Session
*/
  void detachFromCall( SipCall *lcall );

/**
* Kill RTP streams of the Session
*/
  void detachFromRTP( SipCall *lcall );

/**
* Prepare the commandline, fork and exec
*/
  void doPayload( int load, bool tone = false );


  // GUI Stuff
  KReferDialog *referdialog;
  bool hided;
  bool shortTransfer;
  QString referTo;
  QButtonGroup* keypad;
  static int isActiv;
  SipUri redirto;
  int redirTo;
  
  enum RedirHoldState {
      RdHoldNo,
      RdHoldPassive,
      RdHoldActive,
      RdHoldForward
  };

  // State stuff
  enum CallState {
    PreDial,
    Calling,
    Connected,
    Called,
    Releasing
 };
  CallState curstate;
  QString thatsMe;
  QString thatsMyHost;
  
  // Private functions

/**
* Say by, mark own class for deletion and delete 
* SipCall and SipCallMember
*/
  void forceDisconnect( void );

  QString getUserPrefix( void );
 
/** 
* reads the Identity string from theb DataBase and processes it
*/
  void MyIdentity(void);
  
/**
* are the two URIs logically the same
*/
  bool cmpUri ( QString uri1, QString uri2 );
  
/**
* translates symbolic IP names to dot and compares them
*/
  bool cmpHostDot ( QString host1, QString host2 );

/**
* makes a socket reservation for external payload
*/
  void getExtSocket(int callt);  

int getStunPort(void);

//ringing
void stopRinging(void);
int ringtonepid;



/**
*  set if call has no active media resources
*/
bool detach;

/**
*  informs state Updated, if doPayload is to be called
*  the flag is armed in state Update  
*/
bool upLoad;

/**
* informs state Connected that no media change check must be done
*/
bool noCheckConn;

/**
* caller is not wanted
*/
bool screenCall;
};
#endif // KCALLWIDGET_H_INCLUDED
