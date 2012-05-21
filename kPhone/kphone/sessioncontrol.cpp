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
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <qsettings.h>
#include <qwidget.h> 
#include <iostream>
using namespace std;
#include "../dissipate2/sipprotocol.h"
#include "../dissipate2/sipcall.h"
#include "sessioncontrol.h"
#include "kphone.h"
#include "kstatics.h"
#include "../config.h"
#include "ksippreferences.h"
SessionControl::SessionControl(const char *name)
{
	output = 0;
	input = 0;
	useoss = true;
	setVideoSW( QString::null );
	setVideoDEV( QString::null );
	dorem = false;
	dowat = false;
	doisb = true;
 	prack=KSipPreferences::NoPRACK;
 	pprack=KSipPreferences::NoPPRACK;
	audioMode=isOSS;
	QSettings settings;
	pFix = QString::fromAscii( name);
	payload = settings.readNumEntry( KStatics::dBase+pFix+"/dsp/SizeOfPayload", 160 );
	useStun = false;
	stunSrv = "";
	occupyActive=false;
	bool ok;
	int pcodec=0,scodec=0,tcodec=0,qcodec=0,fcodec=0;
	int l=0;
	for (l=0;l<20;l++) {
	    useL4v[l] = usertpL4;
	    allowHold[l] = false;
    	    allowTransfer[l] = false;
    	    onlyActive[l] = false;
	    AllowRemoteStart[l]=false;
	}
	debug=KStatics::debugLevel;
#ifndef SRTP
settings.writeEntry( KStatics::dBase + "SRTP/Mode", "disabled" );
#endif	
	notiExpires = settings.readNumEntry(KStatics::dBase+pFix+"/General/CTNotificationExpiresTime", 10)*1000 ;
	dtmfWait = settings.readNumEntry(KStatics::dBase+pFix+"/General/DTMFSequenceTime", 1000);
	Auacnt=settings.readEntry( KStatics::dBase+pFix+"/sip/AuaCnt", "60");
	forwardNAcnt=settings.readEntry( KStatics::dBase+pFix+"/sip/callforwardNAcnt", "50");
	if(forwardNAcnt<5) forwardNAcnt=5; //minimal time
	forwardNAuri=settings.readEntry( KStatics::dBase+pFix+"/sip/callforwardNAUri", "");
	forwardBuri=settings.readEntry( KStatics::dBase+pFix+"/sip/callforwardBUri", "");

	docfna=false;
	docfnb=false;
	doaua=false;
	totalNumberOfPrefAudioCodecs=NAUDIO;
	totalNumberOfPrefVideoCodecs=NVIDEO;
	vCodecName[0] = H261D;
	vCodecNum[0]  = H261N;
	vCodecName[1] = H263D;
	vCodecNum[1]  = H263N;
	vCodecName[2] = H263PD;
	vCodecNum[2]  = H263PN;
	aCodecName[0] = PCMUD;
	aCodecNum[0]  = PCMUN;
	aCodecName[1] = PCMAD;
	aCodecNum[1]  = PCMAN;
	aCodecName[2] = GSMD;
	aCodecNum[2]  = GSMN;
	aCodecName[3] = ILBCD;
	aCodecNum[3]  = ILBCN;
	aCodecName[4] = SPEEXD;
	aCodecNum[4]  = SPEEXN;


//Initialisation of commandline and vectorID
	extSW[1]  = settings.readEntry(KStatics::dBase+pFix+"/SessionType1/exec", "kedit" );
	settings.writeEntry( KStatics::dBase+pFix+"/SessionType1/exec",extSW[1]);
	extSW[2]  = settings.readEntry(KStatics::dBase+pFix+"/SessionType2/exec", "kedit" );
	settings.writeEntry( KStatics::dBase+pFix+"/SessionType2/exec",extSW[2]);
	extSW[3]  = settings.readEntry(KStatics::dBase+pFix+"/SessionType3/exec", "kedit" );
	settings.writeEntry( KStatics::dBase+pFix+"/SessionType3/exec",extSW[3]);
	extSW[4]  = settings.readEntry(KStatics::dBase+pFix+"/SessionType4/exec", "kedit" );
	settings.writeEntry( KStatics::dBase+pFix+"/SessionType4/exec",extSW[4]);

	extPR[1]  = settings.readEntry(KStatics::dBase+pFix+"/SessionType1/param", "" );
	settings.writeEntry( KStatics::dBase+pFix+"/SessionType1/param", extPR[1]);
	extPR[2]  = settings.readEntry(KStatics::dBase+pFix+"/SessionType2/param", "" );
	settings.writeEntry( KStatics::dBase+pFix+"/SessionType2/param", extPR[2]);
	extPR[3]  = settings.readEntry(KStatics::dBase+pFix+"/SessionType3/param", "" );
	settings.writeEntry( KStatics::dBase+pFix+"/SessionType3/param", extPR[3]);
	extPR[4]  = settings.readEntry(KStatics::dBase+pFix+"/SessionType4/param", "" );
	settings.writeEntry( KStatics::dBase+pFix+"/SessionType4/param", extPR[4]);

	vectorId[1]  = settings.readEntry(KStatics::dBase+pFix+"/SessionType1/VectorId","no");
	settings.writeEntry( KStatics::dBase+pFix+"/SessionType1/VectorId",vectorId[1]  );
	vectorId[2]  = settings.readEntry(KStatics::dBase+pFix+"/SessionType2/VectorId","no");
	settings.writeEntry( KStatics::dBase+pFix+"/SessionType2/VectorId",vectorId[2]  );
	vectorId[3]  = settings.readEntry(KStatics::dBase+pFix+"/SessionType3/VectorId","no");
	settings.writeEntry( KStatics::dBase+pFix+"/SessionType3/VectorId",vectorId[3]  );
	vectorId[4]  = settings.readEntry(KStatics::dBase+pFix+"/SessionType4/VectorId","no");
	settings.writeEntry( KStatics::dBase+pFix+"/SessionType4/VectorId",vectorId[4] );

	videoSW    = settings.readEntry( KStatics::dBase+pFix+"/Video/videoSW","vic");
	videoDEV   = settings.readEntry( KStatics::dBase+pFix+"/Video/device","/dev/video0");
	setOSSFilename( settings.readEntry( KStatics::dBase+pFix+"/Audio/oss-filename", "/dev/dsp") );
	setOSSFilename2( settings.readEntry( KStatics::dBase+pFix+"/Audio/oss-filename2" , "/dev/dsp") );
	setALSAFilename( settings.readEntry( KStatics::dBase+pFix+"/Audio/alsa-filename" , "hw:0,0") );
	setRingtonedevice(settings.readEntry( KStatics::dBase+pFix+"/Audio/ringtone-device", "hw:0,0" ) );
	setRingtonefile( settings.readEntry( KStatics::dBase+pFix+"/Audio/ringtone-file" , QString(SHARE_DIR) + "/ring/ring.wav") );
	setRingtoneP( settings.readEntry( KStatics::dBase+pFix+"/Audio/ringtone-parameters", "%D %F" ) );
	setRingtoneapp( settings.readEntry( KStatics::dBase+pFix+"/Audio/ringtone-application","kphoneringsh" ) );
	setRinging (settings.readNumEntry( KStatics::dBase+pFix+"/General/Ringtone", 0 ) );
	

	setAudioRW( settings.readEntry( KStatics::dBase+pFix+"/Audio/mode","readonly_writeonly" ) );
	if( settings.readEntry( KStatics::dBase+pFix+"/Audio/audio","alsa"  ) == "oss" ) {
		setAudioSys( isOSS );
	} else if( settings.readEntry( KStatics::dBase+pFix+"/Audio/audio", "alsa" ) == "alsa" ) {
		setAudioSys( isALSA );
	} else {
		setAudioSys( isALSA );
	}	

	if( settings.readEntry( KStatics::dBase + pFix + "/sip/remdialer", "No" ) == "Yes" ) {
		setRem( true );
	} else {
		setRem( false );
	}


	if( settings.readEntry( KStatics::dBase + pFix + "/sip/implsub", "No" ) == "Yes" ) {
		setIsb( true );
	} else {
		setIsb( false );
	}

	if( settings.readEntry( KStatics::dBase + pFix + "/sip/Prack", "PrackNo" ) == "PrackNo" ) {
		setPrack( KSipPreferences::NoPRACK );
	} else if( settings.readEntry( KStatics::dBase + pFix + "/sip/Prack", "PrackNo" ) == "PrackSup" ) {
		setPrack( KSipPreferences::SupportPRACK );
	} else {
		setPrack( KSipPreferences::RequirePRACK );
	}

	if( settings.readEntry( KStatics::dBase + pFix + "/sip/PPrack", "PPrackNo" ) == "PPrackNo" ) {
		setPPrack( KSipPreferences::NoPPRACK );
	} else {
		setPPrack( KSipPreferences::YesPPRACK );
	}


	pcodec=0; scodec=0; tcodec=0; qcodec=0;
	pcodec=settings.readNumEntry( KStatics::dBase+pFix+"/Video/poscodec0",0);
	scodec=settings.readNumEntry( KStatics::dBase+pFix+"/Video/poscodec1",1);
	tcodec=settings.readNumEntry( KStatics::dBase+pFix+"/Video/poscodec2",2);
	initCodecVideo(pcodec, scodec,tcodec);

	pcodec=0; scodec=0; tcodec=0; qcodec=0;fcodec=0;
	pcodec=settings.readEntry( KStatics::dBase+pFix+"/Audio/poscodec0","0").toInt(&ok,10);
	scodec=settings.readEntry( KStatics::dBase+pFix+"/Audio/poscodec1","1").toInt(&ok,10);
	tcodec=settings.readEntry( KStatics::dBase+pFix+"/Audio/poscodec2","2").toInt(&ok,10);
	qcodec=settings.readEntry( KStatics::dBase+pFix+"/Audio/poscodec3","3").toInt(&ok,10);
#ifdef  SPEEX
	fcodec=settings.readEntry( KStatics::dBase+pFix+"/Audio/poscodec4","4").toInt(&ok,10);
#else
	fcodec = -1;
#endif
	minMediaPort = settings.readEntry( KStatics::dBase+ pFix  +"/Media/MinPort","8000" ).toInt(&ok,10);
	maxMediaPort = settings.readEntry( KStatics::dBase+ pFix  +"/Media/MaxPort","9000" ).toInt(&ok,10);
	minApplPort  = settings.readEntry( KStatics::dBase+ pFix  +"/Application/MinPort","10000" ).toInt(&ok,10);
	maxApplPort  = settings.readEntry( KStatics::dBase+ pFix  +"/Application/MaxPort","11000" ).toInt(&ok,10);
	initCodecAudio(pcodec, scodec,tcodec,qcodec,fcodec);
	rtbsel = settings.readNumEntry( KStatics::dBase+pFix+"/General/Ringtone",1);	
	if( settings.readEntry( KStatics::dBase+ pFix  +"/STUN/UseStun", "No" ) == "Yes" ) {
		setStun(true);
	} else {
		setStun(false);
	}

	if(settings.readEntry(KStatics::dBase +"SRTP/Mode", "disabled") == "PSK") {
		srtpmode = 1;
	} else if(settings.readEntry(KStatics::dBase +"SRTP/Mode", "disabled") == "PKE") {
		srtpmode = 2;
	} else {
		srtpmode = 0;	
	}

	int i=0;

//Presence related times and flags
	acti = settings.readEntry( KStatics::dBase+pFix+"/local/ActivityCheckTime","0").toInt(&ok,10);
	peti = settings.readEntry( KStatics::dBase+pFix+"/local/PublishExpiresTime","0").toInt(&ok,10);
	seti = settings.readEntry( KStatics::dBase+pFix+"/local/SubscribeExpiresTime","0").toInt(&ok,10);
	if( settings.readEntry( KStatics::dBase + pFix + "/sip/winfosub", "No" ) == "Yes" ) {
		setWat( true );
	} else {
		setWat( false );
	}


//audio
	i= (int)SipCall::StandardCall;
	allowHold[i] = true;
    	allowTransfer[i] = true;
    	onlyActive[i] = true;
	AllowRemoteStart[i]=false;
	useL4v[i] = usertpL4;

//auvi
	i= (int)SipCall::auviCall;
	allowHold[i] = true;
	allowTransfer[i] = true;
	onlyActive[i] = true;
	useL4v[i] = usertpL4;
	AllowRemoteStart[i]=false;
    	if (settings.readEntry(KStatics::dBase+pFix+"/Video/Hold", "No" ) == "Yes") {
		allowHold[i] = true;
    	} else {
		allowHold[i] = false;
    	}
    	if (settings.readEntry(KStatics::dBase+pFix+"/Video/Transfer", "No" ) == "Yes") {
		allowTransfer[i] = true;
    	} else {
		allowTransfer[i] = false;
   	}

//Video
	i = (int)SipCall::videoCall;
	if (settings.readEntry(KStatics::dBase+pFix+"/Video/Hold", "No" ) == "Yes") {
		allowHold[i] = true;
    	} else {
		allowHold[i] = false;
    	}
    	if (settings.readEntry(KStatics::dBase+pFix+"/Video/Transfer", "No" ) == "Yes") {
		allowTransfer[i] = true;
    	} else {
		allowTransfer[i] = false;
   	}
	useL4v[i] = usertpL4;
	onlyActive[i] = false;
	AllowRemoteStart[i]=false;

// Privat: general

	for (i=(int)SipCall::extCall1; i<=(int)SipCall::extCall4;i++) {
		int i1=i-(int)SipCall::extCall1+1;

		if (settings.readEntry(KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/Hold", "No" ) == "Yes") {
			allowHold[i] = true;
			settings.writeEntry( KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/Hold", "Yes"  );
    		} else {
			allowHold[i] = false;
			settings.writeEntry( KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/Hold", "No"  );
    		}
    		if (settings.readEntry(KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/Transfer", "No" ) == "Yes") {
        		allowTransfer[i] = true;
			settings.writeEntry( KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/Transfer", "Yes"  );
    		} else {
			allowTransfer[i] = false;
			settings.writeEntry( KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/Transfer", "No"  );
    		}
    		if (settings.readEntry(KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/UseSound", "No" ) == "Yes") {
			onlyActive[i] = true;
			settings.writeEntry( KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/UseSound", "Yes"  );
    		} else {
			onlyActive[i] = false;
			settings.writeEntry( KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/UseSound", "No"  );
    		}
	

		if (settings.readEntry(KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/UseL4", "UDP" ) == "UDP") {
				useL4v[i] = useudpL4;
				settings.writeEntry( KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/UseL4", "UDP"  );
		} else {
				useL4v[i] = usertpL4;
				settings.writeEntry( KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/UseL4", "RTP"  );
		}
				
		
		if (settings.readEntry(KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/Rst", "No" ) == "Yes") {
				AllowRemoteStart[i] = true;
				settings.writeEntry( KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/Rst", "Yes"  );
		} else {
				AllowRemoteStart[i] = false;
				settings.writeEntry( KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/Rst", "No"  );

		}

	}
		if(debug >=  2) cout << ":::::SessionControl start\n";

}



SessionControl::~SessionControl( void ) {
	
	//the destructor will be called when kphone terminates, so cleanup is not nec.

  	if(debug >=  2) cout <<  ":::::SessionControl stop\n";
}

void SessionControl::setALSAFilename( const QString &devname )
{
	if(debug >=  2) cout <<  "=====Audio: set AlsaFilename " << devname.latin1() << endl;
	if( devname == QString::null ) {
		alsafilename = "hw:0,0";
	} else {
		alsafilename = devname;
	}
}

void SessionControl::setOSSFilename( const QString &devname )
{
	if(debug >=  2) cout <<  "=====Audio: audio set OSSFilename " << devname.latin1() << endl;
	if( devname == QString::null ) {
		ossfilename = "/dev/dsp";
	} else {
		ossfilename = devname;
	}
}


void SessionControl::setRingtonefile( const QString &devname )
{
	if(debug >=  2) cout <<  "=====Audio: audio set RingToneFile " << devname.latin1() << endl;
	if( devname == QString::null ) {
		ringtonefile = "hw:0,0";
	} else {
		ringtonefile = devname;
	}
}

void SessionControl::setRingtonedevice( const QString &devname )
{
	if(debug >=  2) cout <<  "=====Audio: set RingTonedevice " << devname.latin1() << endl;
	if( devname == QString::null ) {
		ringtonedevice = "/dev/dsp";
	} else {
		ringtonedevice = devname;
	}
}
void SessionControl::setRingtoneP( const QString &devname )
{
	if(debug >=  2) cout <<  "=====Audio: audio set RingToneP " << devname.latin1() << endl;
	if( devname == QString::null ) {
		ringtonep = "%D %F";
	} else {
		ringtonep = devname;
	}
}
void SessionControl::setRingtoneapp( const QString &devname )
{
	if(debug >=  2) cout <<  "=====Audio: set RingToneApp " << devname.latin1() << endl;

	if( devname == QString::null ) {
		ringtoneapp = "kphoneringsh";
	} else {
		ringtoneapp = devname;
	}
}

void SessionControl::setOSSFilename2( const QString &devname )
{
	if(debug >=  2) cout <<  "=====Audio: audio set OSSFilename2 " << devname.latin1() << endl;
	if( devname == QString::null ) {
		ossfilename2 = "/dev/dsp";
	} else {
		ossfilename2 = devname;
	}
}

void SessionControl::setVideoSW( const QString &sw )
{
	if( sw == QString::null ) {
		videoSW = "vic";
	} else {
		videoSW = sw;
	}
}

void SessionControl::setVideoDEV( const QString &sw )
{
	if( sw == QString::null ) {
		videoDEV = "/dev/video0";
	} else {
		videoDEV = sw;
	}
}


void SessionControl::setExtSW( int i, const QString &sw )
{
	i= i - SipCall::extCall1 +1;
	if( sw == QString::null ) {
		extSW[i] = "vic";
	} else {
		extSW[i] = sw;
	}
}

void SessionControl::setExtPR( int i,  const QString &pr )
{
	i= i - SipCall::extCall1 +1;
	if( pr == QString::null ) {
		extPR[i] = "";
	} else {
		extPR[i] = pr;
	}
}



void SessionControl::setVectorId(int i, const QString &sw )
{
	i= i - SipCall::extCall1 +1;
	if(sw == QString::null ) {
		vectorId[i] = "no";
	} else {
		vectorId[i] = sw;
	}
}

void SessionControl::setExtIco( int i,  const QString &sw )
{
	i= i - SipCall::extCall1 +1;
	if( sw == QString::null ) {
		extIco[i] = "question";
	} else {
		extIco[i] = sw;
	}
}


const QString SessionControl::theVectorId(int i){
	i= i - SipCall::extCall1 +1;
	switch(i) {
			case 1:	return vectorId[1];
			case 2:	return vectorId[2];
			case 3:	return vectorId[3];
			case 4:	return vectorId[4];
			default: return  QString::null;
}
}

const QString SessionControl::forkSW(int i){
	i= i - SipCall::extCall1 +1;
	switch(i) {
			case 1:	return extSW[1];
			case 2:	return extSW[2];
			case 3:	return extSW[3];
			case 4:	return extSW[4];
			default: return  QString::null;
}
}

const QString SessionControl::forkSP(int i){
	i= i - SipCall::extCall1 +1;
	switch(i) {
			case 1:	return extPR[1];
			case 2:	return extPR[2];
			case 3:	return extPR[3];
			case 4:	return extPR[4];
			default: return  QString::null;
}
}




void SessionControl::setStunSrv( QString newStunSrv )
{
	stunSrv = newStunSrv;
}

void SessionControl::initCodecVideo(int pcodec,int scodec,int tcodec){
QString noCodec = "-1";	
if ( (pcodec >2) || (scodec >2) || (tcodec >2) ) return;
	if ( (pcodec <(-1)) || (scodec <(-1)) || (tcodec <(-1)) )return;
	if ( (pcodec + scodec + tcodec  >3 ) ) return; 

for(int i = 0; i< NVIDEO;i++) {
	vidPrefCodec[i] = QString::null;
	vidPrefCodecNum[i] = noCodec; 
}

if(pcodec>=0)	{
		vidPrefCodec[pcodec] = vCodecName[0];
		vidPrefCodecNum[pcodec] = vCodecNum[0];
		}

if(scodec>=0)	{
		vidPrefCodec[scodec] = vCodecName[1];
		vidPrefCodecNum[scodec] = vCodecNum[1];
		}

if(tcodec>=0)	{
		vidPrefCodec[tcodec] = vCodecName[2];
		vidPrefCodecNum[tcodec] = vCodecNum[2];
		}

	QSettings settings;
	settings.writeEntry( KStatics::dBase+pFix+"/Video/poscodec0", pcodec );
	settings.writeEntry( KStatics::dBase+pFix+"/Video/poscodec1", scodec );
	settings.writeEntry( KStatics::dBase+pFix+"/Video/poscodec2", tcodec );



}

void SessionControl::initCodecAudio(int pcodec,int scodec,int tcodec,int qcodec, int fcodec){
QString noCodec = "-1";

	if ( (pcodec >4) || (scodec >4) || (tcodec >4) || (qcodec >4) || (fcodec >4) ) return;
	if ( (pcodec <(-1)) || (scodec <(-1)) || (tcodec <(-1)) || (qcodec <(-1)) || (fcodec <(-1)) ) return;
	if ( (pcodec + scodec + tcodec + qcodec + fcodec ) > 10 ) return;


haveILBC = false;
for(int i = 0; i< NAUDIO;i++) {
	prefCodec[i] = QString::null;
	prefCodecNum[i] = noCodec;
}

if(pcodec>=0)	{
		prefCodec[pcodec] = aCodecName[0];
		prefCodecNum[pcodec] = aCodecNum[0];
		}

if(scodec>=0)	{
		prefCodec[scodec] = aCodecName[1];
		prefCodecNum[scodec] = aCodecNum[1];
		}

if(tcodec>=0)	{
		prefCodec[tcodec] = aCodecName[2];
		prefCodecNum[tcodec] = aCodecNum[2];
		}

if(qcodec>=0)	{
		prefCodec[qcodec] = aCodecName[3];
		prefCodecNum[qcodec] = aCodecNum[3];
		haveILBC = true;
		}

if(fcodec>=0)	{
		prefCodec[fcodec] = aCodecName[4];
		prefCodecNum[fcodec] = aCodecNum[4];
		}


	QSettings settings;
	settings.writeEntry( KStatics::dBase+pFix+"/Audio/poscodec0", pcodec );
	settings.writeEntry( KStatics::dBase+pFix+"/Audio/poscodec1", scodec );
	settings.writeEntry( KStatics::dBase+pFix+"/Audio/poscodec2", tcodec );
	settings.writeEntry( KStatics::dBase+pFix+"/Audio/poscodec3", qcodec );
	settings.writeEntry( KStatics::dBase+pFix+"/Audio/poscodec4", fcodec );

}

void SessionControl::setPorts(QString mmin,QString mmax,QString amin,QString amax) {
bool ok;
minMediaPort = mmin.toInt(&ok,10);
maxMediaPort = mmax.toInt(&ok,10);
minApplPort  = amin.toInt(&ok,10);
maxApplPort  = amax.toInt(&ok,10);
}

void SessionControl::setPermFlags(int i){


QSettings settings;

//Video
    if (i == (int)SipCall::videoCall) {
    		if (settings.readEntry(KStatics::dBase+pFix+"/Video/Hold", "No" ) == "Yes") {
			allowHold[i] = true;
    		} else {
			allowHold[i] = false;
    		}
    		if (settings.readEntry(KStatics::dBase+pFix+"/Video/Transfer", "No" ) == "Yes") {
			allowTransfer[i] = true;
    		} else {
			allowTransfer[i] = false;
   		}
    			onlyActive[i] = false;
			useL4v[i] = usertpL4;


// Privat: general
		
	}else if (i >=(int)SipCall::extCall1){
		int i1=i-(int)SipCall::extCall1+1;
		
		if (settings.readEntry(KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/Hold", "No" ) == "Yes") {
			allowHold[i] = true;
    		} else {
			allowHold[i] = false;
		}
    		if (settings.readEntry(KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/Transfer", "No" ) == "Yes") {
        		allowTransfer[i] = true;
    		} else {
			allowTransfer[i] = false;
    		}
    		if (settings.readEntry(KStatics::dBase+pFix+"/SessionType" + QString::number(i1) + "/UseSound", "No" ) == "Yes") {
			onlyActive[i] = true;
    		} else {
			onlyActive[i] = false;
    		}
		if(settings.readEntry(KStatics::dBase+pFix+"/SessionType"+QString::number(i1)+"/UseL4","RTP" )== "UDP") {
			useL4v[i] = useudpL4;
		} else {
			useL4v[i] = usertpL4;
    		}



//auvi
	} else if (i == (int)SipCall::auviCall) {
		allowHold[i] = true;
    		allowTransfer[i] = true;
    		onlyActive[i] = true;
		useL4v[i] = usertpL4;
    		if (settings.readEntry(KStatics::dBase+pFix+"/Video/Hold", "No" ) == "Yes") {
			allowHold[i] = true;
    		} else {
			allowHold[i] = false;
    		}
    		if (settings.readEntry(KStatics::dBase+pFix+"/Video/Transfer", "No" ) == "Yes") {
			allowTransfer[i] = true;
    		} else {
			allowTransfer[i] = false;
   		 }
    			onlyActive[i] = false;
		
//audio
    	} else {    
		allowHold[i] = true;
    		allowTransfer[i] = true;
    		onlyActive[i] = false;
		useL4v[i] = usertpL4;
		setOSSFilename( settings.readEntry( KStatics::dBase+pFix+"/Audio/oss-filename" ) );
		setOSSFilename2( settings.readEntry( KStatics::dBase+pFix+"/Audio/oss-filename2" ) );
		setALSAFilename( settings.readEntry( KStatics::dBase+pFix+"/Audio/alsa-filename" ) );
	}
}

bool  SessionControl::isHoldAllowed(int i) {

return allowHold[i];
}

bool  SessionControl::isTransferAllowed(int i) {
return allowTransfer[i];
}

bool  SessionControl::activeOnly(int i) {
return onlyActive[i];
}

void SessionControl::setOccupyActive (bool oa) {
occupyActive=oa;
}

void SessionControl::setVidPrefCodec( int i, const QString &vcodec)
{
		vidPrefCodec[i] = vcodec;
}

void SessionControl::setPrefCodec( int i, const QString &codec)
{
		prefCodec[i] = codec;
}

void SessionControl::setPrefCodecNum( int i, const QString &codecnum)
{
		prefCodecNum[i] = codecnum;
}

void SessionControl::setVidPrefCodecNum( int i, const QString &codecnum)
{
		vidPrefCodecNum[i] = codecnum;
}



QString  SessionControl::getNZPrefCodecNum (int i) {
if  (prefCodecNum[i]!= "-1") return prefCodecNum[i] + " "; else return QString::null;
}

QString  SessionControl::getNZVidPrefCodecNum (int i) {
if  (vidPrefCodecNum[i]!= "-1") return vidPrefCodecNum[i] + " "; else return QString::null;
}

void SessionControl::setOnlyActive (void) {
int i;
    for (i=(int)SipCall::extCall1; i<=(int)SipCall::extCall4;i++) {
	onlyActive[i] = true;
    }
}

void SessionControl::stopAutoRegistration (void) {
    cout << "*********************** stopAutoRegistration" << endl;
    QSettings settings;
    QString p =  KStatics::dBase+pFix+ "Registration/";
    //settings.writeEntry( p + "Password", "" );
    //settings.writeEntry( p + "UserName", "" );
    settings.writeEntry( p + "/AutoRegister", "No");
}