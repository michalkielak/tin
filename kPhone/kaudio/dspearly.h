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
#ifndef DSPEARLY_H_INCLUDED
#define DSPEARLY_H_INCLUDED

#include <qtimer.h>


#include "../dissipate2/udpmessagesocket.h"
#include "../kphone/callaudio.h"
#define GSM_ENC_BYTES		33
#define GSM_DEC_SAMPLES		160
#define ILBCNOOFWORDS   (NO_OF_BYTES/2)
#define TIME_PER_FRAME  30

/**
*	@short tone generator for applications
*
*	The class generates output buffers with noice, and sends them. 
*	Used for early audio, when the media gateway needs a rtp stream
*	It is used for applications w.o. real audio
*	The class differs from all other kaudio classes that it does "tone" 
*	as well as rtp-processing.
*/
class DspEarly :  public QObject
{

	Q_OBJECT

public:
	/**
	 * Constructs a DspEarly object outputting to the given
	 * hostname.
	 */
	DspEarly(void);

	/**
	 * Destructor.  Will close the device if it is open.
	 */
	~DspEarly( void );
	
	enum DeviceState {
		DeviceOpened,
		DeviceClosed };

	enum DeviceMode {
		ReadOnly,
		WriteOnly,
		ReadWrite };
/**
*	Initialiser
*/	
	void start( const codecType newCodec, int newCodecNum, UDPMessageSocket *s, QString pipAddr,QString qpport  );
/**
*	Not used
*/	
//	void startExt(const codecType newCodec, int newCodecNum, int socket, QString ipAddr, QString qrport, QString lipAddr,QString qsport );
/**
*	Not used
*/
	void stopExt(void);
/**
*	Not used
*/
	bool writeBuffer( void );
/**
*	Not used
*/
//	bool setPortNum( int newport );
/**
*	Not used
*/
//	int getPortNum( void ) const { return portnum; }
/**
*	Not used
*/
//	bool readBuffer( int bytes = 0 );
/**
*	Not used
*/
//	void setPayload( int payload ) { fixedrtplen = (size_t)payload; }
/**
*	Not used
*/
//	void setCodec( const codecType newCodec, int newCodecNum );
/**
*	Not used
*/
//	bool sendDTMF (int DTMFtone);
/**
*	sets the devicename to "rtp"
*/
	void setDeviceName( const QString &newDevName ) {devname=newDevName;}
private slots:
/**
*	sends a buffer when timer elapses
*/
	void writeBuffer1( void );
private:
	DeviceState devstate;  // Current state
	DeviceMode devmode;    // Current mode
	QString lasterror;     // Informative text output of last error
	QString devname;       // Device Name, (not necessarily filename!)
	int rate;              // Sampling rate (in fps)
	int portnum;
	int videoPortnum;
	int output_fd;                // The fd of the audio output
	unsigned char *packetbuf;     // Buffer for the packet data
	double lasttime;
	short curseq;
	unsigned char *bufunsend;

	unsigned char *inbuf;
	unsigned char *outbuf;
	unsigned char *tmpbuf;
	unsigned char *quebuf;
	unsigned char *dtmfbuf;

//codec
	codecType codec;
	int codecNum;
	gsm gsmInstEnc;
	gsm gsmInstDec;
	int qlen;
	iLBC_Enc_Inst_t ilbcEncInst_20;
	iLBC_Dec_Inst_t ilbcDecInst_20;
	iLBC_Enc_Inst_t ilbcEncInst_30;
	iLBC_Dec_Inst_t ilbcDecInst_30;
	//speex_Enc_Inst_t speexEncInst;
	//speex_Dec_Inst_t speexDecInst;
	size_t numunsend;
	size_t fixedrtplen;
	unsigned long deb_frag;
	unsigned long deb_rtp;
	int ts;
	int ssrc;
	int ref_sec;
	int ref_usec;
	int dsize;
	bool useStun;
	bool destroySocket;
	int min;
	int max;
	bool useIPv6;
	bool useSRTP;
	int sdebug;
	QTimer *sendTimer;
	UDPMessageSocket *socket;
//---------------------
	bool useExt;
	static InetType intype;
	bool didcomplain;
	struct hostent *he;
	int socketfd;
	int rsock;
	int tsock;
	unsigned int ourport;
	bool bound;
	int buffsize;
	char *ipaddress;
	char hostname[80];
	char **a;
//	static unsigned int curseq;
	int soal;	
		// Socket Info
	union ipaddr {
		unsigned char    raw_addr[16];
		struct in6_addr  in6_addr; // in networkbyteorder
		struct in_addr   in4_addr; // dto.
	};
	union ipaddr ip_addr;
	union sockaddr_in46 {
		struct sockaddr     soa;
		struct sockaddr_in  soa4;
		struct sockaddr_in6 soa6;
	};
	union sockaddr_in46 socketaddress;
	union sockaddr_in46 remoteaddress;


};

#endif  // DSPEARLY_H_INCLUDED
