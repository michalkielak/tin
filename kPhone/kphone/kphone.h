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
#ifndef KPHONE_H_INCLUDED
#define KPHONE_H_INCLUDED
#include <qmainwindow.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

class SipClient;
class SipUser;
class PhoneList;
class SessionControl;
class AudioControl;
class VideoControl;
class WbdControl;
class ExtControl;
class KSipUserInfo;
class KPhoneView;
class CallAudio;
class KSipRegistrations;
class KSipAuthentication;
class KSipPreferences;
class KSipOptions;
class SipRegister;
class KTestOptions;
class TrayIcon;
class IMControl;
class KHelp;
class KDtmf;
QString const KPhoneVersion = "1.0";
int const quitTime = 5000;
int const ringTime_1 = 200;
int const ringTime_2 = 2000;
int const ringTime_3 = 3000;
int const acceptTime = 500;
int const acceptSubscribeSendToSubscribe_time = 5000;
int const constRegistrationExpiresTime = 241;
int const constSubscribeExpiresTime = 600;
int const constStunRequestPeriod = 1111;
int const constMinPort = 8000;
int const constMaxPort = 9000;
int const constMinMPort = 10000;
int const constMaxMPort = 11000;
int const kphoneMaximumWidht = 480;
int const kphoneMinimunWidht = 80;
QString const constStunServer = "stunservername:XXXX";

/**
*@short KPhone's base class
* 
* This class is KPhone's base class
* 
* It administrates the provisioning of the call and starts the Classes
* 
*   SessionControl	(Provisioning)
* 
*   KphoneView		(Parent of the graph. Representation and the session control)
* 
*   SipClient		(Parent of the SIP/SDP-Machine	
*/
class KPhone : public QMainWindow
{
	Q_OBJECT
public:
	KPhone( QString port, QString dhost, QString prefix, int p2p, bool looseRoute, bool strictRoute, int debug, bool direct2URI, bool dontRegister );
	~KPhone( void );
/**
*
*/	
	//void showUsers( void );
/**
*	get the id-tag for out entry in .kpsirc
*/
	QString getUserPrefix( void );
/**
*	delete remot dialler's socket
*/
	void delRDialSocket(void);

public slots:
  
/**
*	short intro to kphone usage
*/
	void showHelp( void );
/**
*	from here you send DTMF tones
*/	
	void showDtmfPanel( void );
/**
*	refresh the tray icon
*/
	void updateTrayIcon( void );
/**
*	show/hide the help panel and the DTMF panel
*/
	void toggleVisibility( void );
/**
*	used to control kphone from the tray menue
*/
	void trayMenuRequested( const QPoint& pos );

private slots:
/**
*	checks for signalling input
*/
	void timerTick( void );
/**
*	emits stun requests
*/
	void stun_timerTick( void );
/**
*	checks for dialler input
*/
	void rdTimerTick( void );

//	void showUserInfo( void );
/**
*	start the audio preferences pulldown
*/
	void showAudioPreferences( void );
/**
*	start the video preferences pulldown
*/
	void showVideoPreferences( void );
/**
*	start the  preferences pulldown
*/
	void showExtPreferences1( void );
/**
*	start the  preferences pulldown
*/
	void showExtPreferences2( void );
/**
*	start the  preferences pulldown
*/
	void showExtPreferences3( void );
/**
*	start the  preferences pulldown
*/
	void showExtPreferences4( void );
/**
*	pop up the registration panel
*/
	void showRegistrations( void );
/**
*	pop up the KSipPreferences panel
*/
	void showGlobalPreferences( void );
/**
*	not used
*/
//	void showQueryOptions( void );
/**
*	not used
*/
//	void showTestOptions( void );
/**
*	the well known about 
*/ 
 	void showAboutKPhone( void );
/**
*	switch tray icon on/off
*/
  	void toggleTrayIcon( int );
/**
*	BYE kphone
*/
	void kphoneQuit( void );
/**
*	not used	
*/
//	void tickAlive();

protected:
/**
* not used
*/
//	void closeEvent( QCloseEvent* );
/**
*	create a tray icon
*/
	void createTrayIcon( void );
/**
*	delete the  tray icon
*/
	void deleteTrayIcon( void );

	CallAudio *callAudio;
	KPhoneView *view;
	TrayIcon *trayIcon;
	// Global state
	SipClient *client;
	SipUser *user;
	SessionControl *sessionC;
	QPopupMenu *preferencesMenu;

	// Dialogs
	PhoneList *qdial;
	KSipUserInfo *userinfo;
	AudioControl *audiocontrol;
	VideoControl *videocontrol;
	ExtControl  *extcontrol1;
	ExtControl  *extcontrol2;
	ExtControl  *extcontrol3;
	ExtControl  *extcontrol4;
	KSipRegistrations *registrations;
	KSipAuthentication *authentication;
	KSipPreferences *sipprefs;
	KSipOptions *queryoptions;
	KTestOptions *testoptions;

private:
	QTimer *quitTimer;
	QTimer *timer;
	QTimer *timerAlive;
	QString userPrefix;
	int showDebug;
	unsigned int listenport;
	unsigned int portrepeater;
//RDial
/**
*
*/
	bool newRDialSocket(void);
/**
*
*/
	void checkRDialSocket(void);
	bool haveRDSock;
	struct sockaddr_un rdServer,rdClient;
	int rdSock;
	int rdLoop;
	char mess[100];
	QTimer *rdTimer;
	KHelp *khelp;
	KDtmf *kdtmf;
};

#endif // KPHONE_H_INCLUDED
