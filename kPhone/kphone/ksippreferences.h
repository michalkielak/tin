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
#ifndef KSIPPREFERENCES_H_INCLUDED
#define KSIPPREFERENCES_H_INCLUDED

#include <qtabdialog.h>

#include "../dissipate2/sipclient.h"
#include "sessioncontrol.h"
#include "../config.h"
class QLineEdit;
class QMultiLineEdit;
class QButtonGroup;
/**
* @short SIP Parameters
*
* Fill in the SIP Parameters, which are not used for Registration:

* Call Forwarding

* Stun

* subscription and registration timers

* Media Port Ranges

* Phone Book database file
*/	
class KSipPreferences : public QTabDialog
{
	Q_OBJECT
public:
	KSipPreferences( SipClient *client, SessionControl *ca, QWidget *parent = 0, const char *name = 0 );
	~KSipPreferences( void );
	enum PrackA {NoPRACK,SupportPRACK,RequirePRACK } prackAllow;
	enum PrackP {NoPPRACK,YesPPRACK} prackP;
protected slots:
	void slotOk( void );
	void slotApply( void );
	void slotCancel( void );
	void resetSettings( void );

private:
	enum ExpProxy { DontUseExpProxy, UseExpProxy };
	enum HideVia { NoHide, HideHop, HideRoute };
// KPhone
//	enum DefUserMode { DefMode, NoDefMode };
	enum AuA { AuaActive, AuaInactive };
	enum CallForward { FwActive, FwInactive };
	enum Socket { UDP, TCP };
	enum Locserv {DNS,SRV,NAPTR};
	enum Stun { stunYes, stunNo };
	enum RemDialer { NoRem, YesRem };
	enum Watchinfo { NoWat, YesWat };
	enum ISBDialer { NoISB, YesISB };

	#ifdef SRTP
	enum Srtp { disable, psk, pke };
	#endif
	
	SipClient *c;
	SessionControl *sessionC;
	QLineEdit *proxyaddr;
	QLineEdit *forwardaddr;
	QLineEdit *forwardBaddr;
	QLineEdit *forwardNAaddr;
	QLineEdit *forwardNAcnt;
	QLineEdit *auacnt;
	QLineEdit *maxforwards;
	QLineEdit *phonebook;
	QLineEdit *expires;
	QLineEdit *notiExpires;
	QLineEdit *dtmfWait;
	QLineEdit *subscribeExpires;
	QLineEdit *winfoExpires;
	QLineEdit *publishExpires;
	QLineEdit *activityCheck;
	QLineEdit *stunServerAddr;
	QLineEdit *stunRequestPeriod;
	QMultiLineEdit *forwardmsg;
	QMultiLineEdit *busymsg;
	QButtonGroup *expbg;
	QButtonGroup *forbg;
	QButtonGroup *forbbg;
	QButtonGroup *fnabg;
	QButtonGroup *auabg;
	QButtonGroup *hidebg;
	QButtonGroup *rembg;
	QButtonGroup *watbg;
	QButtonGroup *isbbg;
	QButtonGroup *prackbg;
	QButtonGroup *prackPbg;
//KPhone
//	QButtonGroup *usedef;
	QButtonGroup *stun;
	QButtonGroup *socket;
	QButtonGroup *sloc;
	QLineEdit *mediaMinPort;
	QLineEdit *mediaMaxPort;
	QLineEdit *appMinPort;
	QLineEdit *appMaxPort;
	QString pFix;
//	QButtonGroup* srtp;
//	QLineEdit* masterKey;
#ifdef SRTP
	QButtonGroup* srtp;
	QLineEdit* pskMasterKey;
#endif

};

#endif // KSIPPREFERENCES_H_INCLUDED
