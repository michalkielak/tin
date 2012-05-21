/*
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
#ifndef SDPBUILD_H_INCLUDED
#define SDPBUILD_H_INCLUDED

#include <qdialog.h>
#include "sipmessage.h"
#include "sipcall.h"
#include "../kphone/sessioncontrol.h"

/**
* @short construct and parse sdpmessages
*
* Used to construct SDP Messages and to store the payload
* data into the SipCall incarnation
*
* performs codec checks and security tasks
*
* CAVE!!  if you change the m= line, you have to adapt SipClient::parseMessage, too
*/
class SdpBuild
{
public:
/**
* Construct a blank SDP message object.
*/
SdpBuild( SessionControl *sco,SipCall *scall );
/**
* Deletes the SDP message.
*/
~SdpBuild( void );


/**
*       Generate a SDP-Message for INVITE to start a new session
*/
	QString prepInviteString(int load ,QString audioport, QString port);
/**
*	Generate a SDP-Message for 200 OK to acknowledge  a new session
*/
	QString prepAcceptString(int load ,QString audioport, QString port, bool isHold);

/**
*	Generate a SDP-Message for OPTIONS
*/
	QString prepOptString(void);
/**
* Read the session description
* Return values are 0 on error, the number of the Call Type otherwise
*/
	int checkCodec( bool testReInvite, SipCallMember *member , bool forceHold, bool *srtp);

/** Compare the own vectorId against the incoming
*   if not equal, we assume incompatible content of the session type and return 0
*   if equal, we return the status
**/
	int checkVectorId(int stat, QString incomingId);

/**
 * get the partner host's port
*/
	const QString &getExtPort(void) const {return testPort; }
	
/**
 * store the partner host's port
 * this will be user when we construct the commandline
*/
	void setExtPort (const QString &tp);

/**
 * get our own port, stored for the partner
*/
	const QString &getExtHomePort(void) const {return testHomePort; }
	
/**
 * store our own port
 * this will be user when we construct the commandline
*/
	void setExtHomePort (const QString &thp);
/**
 * get our own port, stored for the partner, reflect STUN
*/
	const QString &getSExtHomePort(void) const {return testSHomePort; }
	
/**
 * store our own port
 * this will be user when we construct the commandlinereflect STUN
*/
	void setSExtHomePort (const QString &thp);

/**
 * get the partner host's audioport
*/
	const QString &getAudioPort(void) const {return audioPort; }
	
/**
 * store the partner host's audio port
 * this will be user when we construct the commandline
*/
	void setAudioPort (const QString &tp);

/**
 * get our own audioport, stored for the partner
*/
	const QString &getAudioHomePort(void) const {return audioHomePort; }
	
/**
 * store our own audioport
 * this will be user when we construct the commandline
*/
	void setAudioHomePort (const QString &thp);
	
/**
 * get our Video Codec
*/
	QString  getVidCodec(void)  { return videoCodec; }

/**
 * store our Video Codec
*/
	void setVidCodec (const QString &vc);
	
/**
 * get our preferred Video Codec
*/
	QString  getVidPrefCodec(void)  { return videoPrefCodec; }
	QString  getVidPrefCodecNum(void)  { return videoPrefCodecNum; }
/**
 * store our preferredVideo Codec
*/
	void setVidPrefCodec (const QString &vc);
/**
 * store our preferredVideo Codec Number
*/
	void setVidPrefCodecNum (const QString &vc);
/**
 * get our preferred Codec
*/
	QString  getPrefCodec(void)  { return prefCodec; }
/**
 * get our preferred Codec Number
*/
	QString  getPrefCodecNum(void)  { return prefCodecNum; }
	
/**
 * store our preferred Codec
*/
	void setPrefCodec (const QString &pc);
/**
 * store our preferred Codec Number
*/
	void setPrefCodecNum (const QString &pc);
	
/**
* get our Video Codec Type
*/
	codecType  getVidCodecType(void)  { return videoCodecType; }
	
/**
* store our Video Codec Type
*/
	void setVidCodecType (codecType vct) {videoCodecType = vct;}

/**
*	translates from codec number to codec
*/	
	codecType getRtpCodec(void);	
/**
*	translates from codec number to codec	
*/	
	codecType getVideoRtpCodec(void);

/**
*	translates from codec to codec number 
*/	
	int  getRtpCodecNum(void); 
	
/**
 * get the partner host's IP address
*/
	const QString &getExtHost(void) const {return testHost; }
	
/**
 * store the partner host's IP address
 * this will be user when we construct the commandline
*/
	void setExtHost (const QString &tc);	
	
/**
* see if the transport fields of both sides are equal
*/
	bool testTrans(int status,QString trans);
	
	int doReturn(int stat);
	
	void setResponsePay(int pay);
	
	int getResponsePay(void) { return responsePay; }
	
	int getRealLoad(void) { return realLoad; }
	
/**
* Flags for a=recvonly
**/
	bool getRecvonly(void) { return sendrec;}
	void setRecvonly(bool sr) { sendrec=sr;}
	
/*public slots:
signals:
private slots:
protected slots:
*/
private:
	QString msg;
	SipCall *call;
	SessionControl *sessionC;
	QString testPort;
	QString testHomePort;
	QString testSHomePort;
	QString audioPort;
	QString audioHomePort;
	QString videoCodec;
	QString videoPrefCodec;
	QString videoPrefCodecNum;
	QString prefCodec;
	QString prefCodecNum;
	codecType videoCodecType;
	QString testHost;
	int responsePay;
/**
* Read local supported Payload Type for received AUDIO Encoding Name
*/
	QString readSupportedAudioPayloadType( QString receivedEncodingName );
/**
* Negotiation of AUDIO Codecs
*/
	bool audioMediaNegotiationINVITE(int recAttributeNumber);
/**
* Negotiation of Video Codecs
*/
	bool videoMediaNegotiationINVITE(int recAttributeNumber);
/**
* Read local supported Payload Type for received Video Encoding Name
*/
	QString readSupportedVideoPayloadType( QString receivedEncodingName );
/**
* VIDEO MEDIA FORMAT LISTE max 1byte (0.255)
*/
	QString vcc[256];
        // QString vcstr[3];
/**
* VIDEO MEDIA ATTRIBUTE VALUE LISTE max 1byte (0.255)
*/
        QString vcstr[256];
/**
* AUDIO MEDIA FORMAT LISTE max 1byte (0.255)
*/
 	QString acc[256];
        // QString acstr[3];
/**
* AUDIO MEDIA ATTRIBUTE VALUE LISTE max 1byte (0.255)
*/
        QString acstr [256];

QString trsp;
QString end;
QString auEnd;
QString viEnd;
QString telend;
QString mIs;
QString mAudio;
QString mVideo;
QString aRMap;
QString vecID;
QString transp;
QString tEvent;
bool remoteStart;
int cntParam;
bool ReInvite;
int realLoad;
int tempresponsepay;
bool sendrec;
};
#endif // SDPBUILD_H_INCLUDED
