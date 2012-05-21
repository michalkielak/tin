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
#ifndef SESSIONCONTROL_H_INCLUDED
#define SESSIONCONTROL_H_INCLUDED

#include <qobject.h>
#include <qstring.h>
#include <sys/types.h>

#include "../dissipate2/udpmessagesocket.h"

// all those video codecs we support
#define H261D   "H261"
#define H261N   "31"
#define H263D   "H263"
#define H263N   "34"
#define H263PD  "H263-1998"
#define H263PN  "103"
// all those audio codecs we support
#define PCMUD   "PCMU"
#define PCMUN   "0"
#define PCMAD   "PCMA"
#define PCMAN   "8"  
#define GSMD    "GSM"
#define GSMN    "3" 
#define ILBCD   "ILBC"
#define ILBCN   "97"
#define SPEEXD  "SPEEX"
#define SPEEXN  "98"

//Number of supportedaudio codecs
#define NAUDIO 5

//Number of supportedaudio codecs
#define NVIDEO 3

#define useudpL4 1
#define usertpL4 2

class DspAudio;
class SipCall;
class SipCallMember;
//The list of audio codecs, used for the DSP Classes
enum codecType {
	codecUnknown,
	codecULAW,
	codecALAW,
	codecGSM,
	codecILBC_20,
	codecILBC_30,
	codecSpeex
 };
/**
* @short Permanent Data storage
*
* This Class holds all the Phone's permanent data
*
* It offers Methods for reading and manipulating those data
*
* For per Call use, the data are copied to SipCall at Call Begin
*
* The permanent data are stored at two places
*
* 	the database-file /home/$Home/.qt/kphonesirc file
*
*	this Class
*
* They are read into this Class whenever the database-file is changed,
*
* whenever the KPhoneSI is started and whenever a call is startet.
*/
class SessionControl : public QObject
{
	Q_OBJECT
public:
/**
*	the constructor
*/
	SessionControl( const char *name );
	~SessionControl( void );


	
enum audiomodeType {
	isOSS,
	isALSA
 };
typedef enum audiomodeType audioModeT;

///The Resources
/**
*	set the audio system (ALSA or OSS)
*/
void setAudioSys(audioModeT am) { audioMode=am; }
/**
*	get the audio system (ALSA or OSS)
*/
audioModeT getAudioSys (void) { return audioMode; }
/**
*	is the mode read/write or readonly_writeonly
*/
bool isAudioRW(void) { return audioRW;}
/**
*	set the mode to read/write or readonly_writeonly
*/
void setAudioRW(bool rw) { audioRW=rw; }
/**
*	use the OSS sound system
*/
	void useOSS( void );
/**
*	set the OSS device (dev/dsp is often used here)
*/
	void setOSSFilename( const QString &devname );
/**
*	the ALSA device (hw:card,device, or plughw:card,device)
*/
	void setALSAFilename( const QString &devname );
/**
*	if readonly_writeonly a second device is used
*/
	void setOSSFilename2( const QString &devname );
/**
*	we plan to be ringed
*/
	void setRinging( int rtbSel ){rtbsel= rtbSel;}
/**
*	did we plan to be ringed
*/
	int getRinging(void){return rtbsel;}
/**
*	set the audio file with the tone
*/
	void setRingtonefile( const QString &name );
/**
*	set thethe ringing application
*/
	void setRingtoneapp( const QString &name );
/**
*	may be an ALSA or OSS device, depending on the ringing application
*/
	void setRingtonedevice( const QString &name );
/**
*	set the parameter string for the  ringing application
*/
	void setRingtoneP( const QString &name );
	
/**
*	get the audio file with the tone
*/
	const QString &getRingtonefile( void ) const { return ringtonefile; }
/**
*	get the ringing application
*/
	const QString &getRingtoneapp( void ) const { return ringtoneapp; }
/**
*	get the device
*/
	const QString &getRingtonedevice( void ) const { return ringtonedevice; }
/**
*	get parameter string for the  ringing application
*/
	const QString &getRingtoneP( void ) const { return ringtonep; }
/**
*	get the OSS device 
*/
	const QString &getOSSFilename( void ) const { return ossfilename; }
/**
*	get the 2nd OSS device (readonly_writeonly mode)
*/
	const QString &getOSSFilename2( void ) const { return ossfilename2; }
/**
*	get the ALSA device 
*/
	const QString &getALSAFilename( void ) const { return alsafilename; }
/**
*	set the video application
*/
	void setVideoSW( const QString &sw );
/**
*	get the video application
*/
	const QString &getVideoSW( void ) const { return videoSW; }
/**
*	set the video device
*/
	void setVideoDEV( const QString &sw );
/**
*	get the video device
*/
	const QString &getVideoDEV( void ) const { return videoDEV; }
/**
*retrieve the vectorId 
*/
	const QString &getVectorId( int i ) const { return vectorId[i]; }	
/**
* store the vectorId 
*/
	void setVectorId( int i,const QString &sw );
/**
*store the Path to the application 
*/
	void setExtSW( int i, const QString &sw );
/**
*retrieve the Path to the application for SessionType 1
*/
	const QString &getExtSW( int i) const { return extSW[i]; }
/**
* store the cmdline of the application 
*/
	void setExtPR( int i, const QString &pr );
/**
*retrieve the cmdline of the application for SessionType 1 
*/
	const QString &getExtPR( int i ) const { return extPR[i]; }
/**
*store the icon-name  of the application 
*/
	void setExtIco( int i, const QString &sw );

/**
* Read vectorId to solicit session types
*/
	const QString theVectorId(int i);
	
/**
* Read the path to the application
*/
	const QString forkSW(int i);	

/**
* Read the parameters of the application
*/
	const QString forkSP(int i);	

/**
* read the total number of prefered Audio Codecs
*/
	int getTotalNumberOfPrefAudioCodecs(void) {return totalNumberOfPrefAudioCodecs;}
/**
* read the total number of prefered Video Codecs
*/
	int getTotalNumberOfPrefVideoCodecs(void) {return totalNumberOfPrefVideoCodecs;}

/**
* set the RTP-Payload sample in ms
*/
	void setPayload ( int newPayload ) { payload = newPayload; }
/**
*	get the RTP-Payload sample in ms
*/
	int  getPayload (void)             {return payload;}
/**
*	get the video codec both ends prefere
*/
	QString  getVidPrefCodec (int i) { return vidPrefCodec[i] ; }

/**
*	Set the video codec both ends prefere
*/
	void  setVidPrefCodec (int i, const QString &vcodec);
/**
*	get the video codec both ends prefere
*/
	void     setVidPrefCodecNum (int i, const QString &codecnum);
/**
*	Set the video codec both ends prefere
*/
	QString  getVidPrefCodecNum (int i) { return vidPrefCodecNum[i] ; }
/**
*	translate codecnumber to codecname if number not zero
*/
	QString  getNZVidPrefCodecNum (int i);

/**
*	Set the audio codec both ends prefere
*/
	void     setPrefCodec (int i, const QString &codec);
/**
*	get the audio codec both ends prefere
*/
	QString   getPrefCodec (int i) { return prefCodec[i] ; }

/**
*	set the audio codec both ends prefere
*/
	void     setPrefCodecNum (int i, const QString &codecnum);
/**
*	get the audio codec both ends prefere
*/
	QString  getPrefCodecNum (int i) { return prefCodecNum[i] ; }
/**
*	translate codecnumber to codecname if number not zero
*/
	QString  getNZPrefCodecNum (int i);

/**
*	get the video codec name for position i
*/
	QString  getCodecName (int i) { return vCodecName[i] ; }
/**
*	set the video codec number for position i
*/
	QString  getCodecNum (int i) { return vCodecNum[i] ; }

/**
*	get the audio codec name for position i
*/
	QString  getACodecName (int i) { return aCodecName[i] ; }
/**
*	get the audio codec number for position i
*/
	QString  getACodecNum (int i) { return aCodecNum[i] ; }

/**
*	initialize the video codecs to be used
*/
	void initCodecVideo(int pcodec,int scodec,int tcodec);
/**
*	initialize the audeo codecs to be used
*/
	void initCodecAudio(int pcodec,int scodec,int tcodec,int qcodec, int fcodec);

/**
*
*/
//	void setVidPermFlags(void);

/**
*
*/
//	void setBodyMask( QString body ) { bodyMask = body; }
/**
*
*/
//	QString getBodyMask( void ) { return bodyMask; }
/**
*
*/
//	int checkCodec( SipCallMember *member );
/*
Stun Server
*/
	void setStunSrv( QString newStunSrv );
	QString getStunSrv( void ) { return stunSrv;}
	bool isStun(void) { return useStun; }
	void setStun(bool st) { useStun=st; }

/**
*	the forwarded to URI for CFNA
*/
	const QString &getCallForwardNAUri( void ) { return forwardNAuri; }
/**
*	number of rings after which CFNA is activated
*/
	const QString &getCallForwardNAcnt( void ) { return forwardNAcnt; }

/**
*	the forwarded to URI for CFB
*/
	const QString &getCallForwardBUri( void ) { return forwardBuri; }
/**
*	number of rings after which AA is activated
*/
	const QString &getAuacnt( void ) { return Auacnt; }
/**
*	the forwarded to URI for CFNA
*/
	void setCallForwardNAUri( const QString &u ) { forwardNAuri=u;}

/**
*	the forwarded to URI for CFB
*/
	void setCallForwardBUri( const QString &u ) { forwardBuri=u;}
/**
*	number of rings after which CFNA is activated
*/
	void setCallForwardNAcnt( const QString &u ) { forwardNAcnt=u;}

/**
*	number of rings after which AA is activated
*/
	void setAuacnt( const QString &au ) { Auacnt=au;}

/**
* The Sessiontype i may be set on Hold
*/
	bool isHoldAllowed(int i);
/**
* The SessionType i may perform Call Transfer
*/
	bool isTransferAllowed(int i);
/**
* This session Type i reqests to be active only.
*/
        bool activeOnly(int i);

/**
* The Sessiontype i may be set on Hold
*/
	void setHoldAllowed(int i,bool val) {allowHold[i] = val;}
/**
* The SessionType i may perform Call Transfer
*/
	void setTransferAllowed(int i,bool val) {allowTransfer[i] = val;}
/**
* This session Type reqests to be active only.
*/
        void setActiveOnly(int i,bool val) { onlyActive[i] = val;}
/**
* Read the flags bool allowHold[i],bool allowTransfer[i]
* and bool onlyActive[i] from Resourcefile into memory.
*/
	void setPermFlags(int i);
/**
* interrogates if someone monopolizes the active state
*/
	bool getOccupyActive (void) {return occupyActive;}
/**
* announces the someone is active and allows no one else with onlyActive
* Flag set to become active.
*/
	void setOccupyActive (bool oa);
	
/**
* set / reset allowance for remote start
*/
void allowRemoteStart(int lt, bool allow) {AllowRemoteStart[lt]= allow; }

/**
* is remote start allowed for this CallType
*/
bool isRemoteStartAllowed (int lt) {return AllowRemoteStart[lt]; }

/**
*	set the Layer 4 protocol used for application transport
*/	
	void setUseL4 ( int i,int uL4)  { useL4v[i] = uL4; }
/**
*	get the Layer 4 protocol used for application transport
*/
	int useL4 (int i){return useL4v[i];}
/**
*	is CFnA activated 
*/
	bool getCFNA(void) {return docfna; }
/**
*	activate CFnA
*/
	void setCFNA( bool cfna) { docfna = cfna; }
/**
*	activate CFB
*/
	void setCFB( bool cfna) { docfnb = cfna; }

/**
*	is CFB activated 
*/
	bool getCFB(void) {return docfnb; }

/**
*	is "remote start" requested
*/
	bool getRem(void) {return dorem; }

/**
*	subscription to watcher info requested
*/
	bool getWat(void) {return dowat; }
/**
*	request "remote start"
*/
	void setRem( bool rem) { dorem = rem; }

/**
*	request subscription to watcher info
*/
	void setWat( bool wat) { dowat = wat; }
	
/**
*	get the time interval for activity checks
*/
	int getActivityCheckTime(void) { return acti; }

/**
*	set the time interval for activity checks
*/
	void setActivityCheckTime(int act) { acti = act; }
	

/**
*	get the time interval after a PUBLISH expires
*/
	int getPubExpiresTime(void) { return peti; }

/**
*	set the time interval after a PUBLISH expires(0 meens no publishing)
*/
	void setPubExpiresTime(int pet) { peti = pet; }
	
/**
*	get the time interval after a SUBSCRIBE expires
*/
	int getSubExpiresTime(void) { return seti; }

/**
*	set the time interval after a SUBSCRIBE expires (0 meens no subscription)
*/
	void setSubExpiresTime(int set) { seti = set; }
	


/**
*	is implicit subscription requested
*/
	bool getIsb(void) {return doisb; }

/**
*	request implicit subscription
*/
	void setIsb( bool isb) { doisb = isb; }
/**
*	get the PRACK type (requested, supported, no)
*/
	int getPrack(void) {return prack; }

/**
*	get the PRACK type (requested, supported, no)
*/
	void setPrack( int pr) { prack = pr; }

/**
*	should we send PRACK if peer supports but does not explicitely require it
*/
	int getPPrack(void) {return pprack; }

/**
*	send PRACK if peer supports but does not explicitely require it
*/
	void setPPrack( int ppr) { pprack = ppr; }

/**
*	get the AutoAnswer detection time
*/
	bool getAua(void) {return doaua; }
/**
*	get the AutoAnswer detection time
*/
	void setAua( bool aua) { doaua = aua; }

/**
*	REFER Notification Supervision timer
*/
	void setNotiExpires( int noti) { notiExpires = noti; }
/**
*	time gap between outpulsing of two DTMF sequences
*/
	void setDTMFWait( int dt) { dtmfWait = dt; }

/**
*	REFER Notification Supervision timer
*/
	int getNotiExpires( void) { return notiExpires; }
/**
*	time gap between outpulsing of two DTMF sequences
*/
	int getDTMFWait( void) { return dtmfWait; }
/**
*	set the port ranges for all media
*/	
	void setPorts(QString mmin,QString mmax,QString amin,QString amax);
/**
*	get the lower boundry for the media port
*/
	int getMinMediaPort(void) { return minMediaPort; }
/**
*	get the upper boundry for the media port
*/
	int getMaxMediaPort(void) { return maxMediaPort; }
/**
*	get the lowerr boundry for the application port
*/
	int getMinApplPort(void)  { return minApplPort;  }
/**
*	get the upper boundry for the application port
*/
	int getMaxApplPort(void)  { return maxApplPort;  }
/**
*	set we will use ILBC (used to insert specific parameters in the SDP description)
*/	
	void setILBC(bool setI) { haveILBC=setI;}
/**
*	get if  we will use ILBC
*/
	bool getILBC(void) { return haveILBC;}
/**
*	make an application call blocking resources
*/	
	void setOnlyActive (void);
/**
*	set the proxy to be used
*/	
	void setDefaultProxy(QString userdefaultproxy) { defProxy=userdefaultproxy; }
/**
*	get the proxy to be used
*/
	QString getDefaultProxy(void) { return defProxy; }
/**
	get srtp mode
*/
	int getSrtpMode(void) { return srtpmode;}
/**
	set srtp mode
*/
	void setSrtpMode (int m) { srtpmode = m; }
/**
	stop AutoRegistration on startup
*/
	void stopAutoRegistration(void);
private:
	bool haveILBC;
	bool audioRW;
	bool docfna;
	bool docfnb;
	bool dorem;
	bool dowat;
	bool doisb;
 	int prack;
 	int pprack;
	bool doaua;
	bool allowHold[20];
	bool allowTransfer[20];
	bool onlyActive[20];
	bool occupyActive;
	int useL4v[20];
	bool AllowRemoteStart[20];
	bool useoss;
	bool useStun;
	int notiExpires;
	int dtmfWait;
	int extSem;
	int payload;
	int audio_fd;
	int rtpCodecNum;
	int totalNumberOfPrefAudioCodecs;
	int totalNumberOfPrefVideoCodecs;
	DspAudio *input;
	DspAudio *output;
	pid_t pidVideo;
	pid_t pidExt;
	audioModeT audioMode;
	QString pFix;
	QString stunSrv;
	QString Auacnt;
	QString forwardNAcnt;
	QString forwardNAuri;
	QString forwardBuri;
	QString ossfilename;
	QString ossfilename2;
	QString alsafilename;
	QString videoSW;
	QString videoDEV;
	QString extSW[4];
	QString extSWI;
	QString extIco[4];
	QString extIcoI;
	QString extPR[4];
	QString extPRI;
	QString vectorIdI;
	QString vectorId[4];
	QString vectorId1;
	QString vectorId2;
	QString vectorId3;
	QString vectorId4;
	QString extContact;
	QString extHomePort;
	QString bodyMask;
	QString ringtonefile;
	QString ringtonedevice;
	QString ringtonep;
	QString ringtoneapp;
	int minMediaPort;
	int maxMediaPort;
	int minApplPort;
	int maxApplPort;
	int rtbsel;
	QString vidPrefCodec[NVIDEO+1];
	QString vidPrefCodecNum[NVIDEO+1];
	QString vCodecName[NVIDEO+1];
	QString vCodecNum[NVIDEO+1];

	
	QString prefCodec[NAUDIO+1];
	QString prefCodecNum[NAUDIO+1];
	QString aCodecName[NAUDIO+1];
	QString aCodecNum[NAUDIO+1];
	QString defProxy;
	int srtpmode;
	int debug;
	int acti;
	int seti;
	int peti;
};


#endif // SESSIONCONTROL_H_INCLUDED
