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
#ifndef DSPOUTRTP_H_INCLUDED
#define DSPOUTRTP_H_INCLUDED

#include <qtimer.h>

#include "../gsm/gsm.h"
#include "../gsm/private.h"
#include "../ilbc/iLBC_define.h"
#include "../ilbc/iLBC_encode.h"
#include "../ilbc/iLBC_decode.h"
#include "../dissipate2/udpmessagesocket.h"
#include "dspout.h"
#include "../kphone/callaudio.h"
#ifdef SPEEX
#include "../Speex/speex.h"
#include "../Speex/speex_define.h"
#endif
#ifdef SRTPWRAPPER_HX
#include "../srtp/SRTPWrapper.h"
#endif
#define GSM_ENC_BYTES		33
#define GSM_DEC_SAMPLES		160
#define ILBCNOOFWORDS   (NO_OF_BYTES/2)
#define TIME_PER_FRAME  30

/**
* @short RTP implementation of DspOut.
* @author Billy Biggs <vektor@div8.net>
*
*	This class is in charge of the RTP-stuff of the KPhone
*	It does 
*
*		- audio compression/decompression
*
*		- SRTP-encryption
*
*		- sending and receiving of RTP-sreams
*
*		- audio related STUN-processing
*
* 	The class is derived from DspOut.
*/
class DspOutRtp : public DspOut
{
public:
	/**
	 * Constructs a DspOutRtp object outputting to the given
	 * hostname.
	 */
	DspOutRtp(  const codecType newCodec, int newCodecNum,  int minP, int maxP,int srtpMode,
		const QString &hostName = QString::null, UDPMessageSocket *sock = NULL );

	/**
	 * Destructor.  Will close the device if it is open.
	 */
	virtual ~DspOutRtp( void );
	/**
	 * Open the device for audio io.  All setup (samplerate, format) must be
	 * done before the call to openDevice()
	*/
	bool openDevice( DeviceMode mode );
	/**	
	* prepares the audio buffer, wrappes it into RTP/SRTP and sends it
	*/
	bool writeBuffer( void );
	/**
	* set the outgoing port number 
	*/
	bool setPortNum( int newport );
	/**
	* access to the outgoing port number 
	*/
	int getPortNum( void ) const { return portnum; }
	/**
	* not used
	*/
	int getVideoPortNum( void ) const { return videoPortnum; }
	/**
	*	returns 0, if  RTP packages arrived 
	*/
	unsigned int readableBytes( void );
	/**
	*	read the incoming RTP-package
	* 	call after readableBytes
	*/
	bool readBuffer( int bytes = 0 );
	/**
	*	the size of the audio buffer in msec
	*/
	void setPayload( int payload ) ;
	/**
	*	init the STUN server access
	*/
	void setStunSrv( QString newStunSrv );
	/**
	*	set the codec to be used
	*/
	void setCodec( const codecType newCodec, int newCodecNum );
	/**
	*	send a DTMF package
	*/
	bool sendDTMF (int DTMFtone);
	/**
	*
	*/
private:
	/**
	*	Convert: audio buffer -> GSM encoded audiombuffer
	*/
	int writeGSMBuffer( gsm Gsm_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size );
	/**
	*	Convert:GSM encoded audiombuffer -> audio buffer
	*/
	int readGSMBuffer( gsm Gsm_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		int ignore );
	/**
	*	Convert: audio buffer -> ILBC encoded audiombuffer(paylooad 20 ms)
	*/
	int writeILBCBuffer_20( iLBC_Enc_Inst_t *Enc_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size );
	/**
	*	Convert: ILBC encoded audiombuffer -> audio buffer(paylooad 20 ms)
	*/
	int readILBCBuffer_20( iLBC_Dec_Inst_t *Dec_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		short mode, int ignore);
	/**
	*	Convert: audio buffer -> ILBC encoded audio buffer (paylooad 30 ms)
	*/
	int writeILBCBuffer_30( iLBC_Enc_Inst_t *Enc_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size );
	/**
	*	Convert: ILBC encoded audiombuffer -> audio buffer(paylooad 30 ms)
	*/
	int readILBCBuffer_30( iLBC_Dec_Inst_t *Dec_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		short mode, int ignore);
#ifdef SPEEX
	/**
	*	Convert: audio buffer -> SPEEX encoded audio buffer
	*/
	int writespeexBuffer( speex_Enc_Inst_t *Enc_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size );
	/**
	*	Convert: SPEEX encoded audiombuffer -> audio buffer
	*/
	int readspeexBuffer( speex_Dec_Inst_t *Dec_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		short mode, int ignore);
#endif
	UDPMessageSocket *socket;      // UDP Socket
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
#ifdef SPEEX
	speex_Enc_Inst_t speexEncInst;
	speex_Dec_Inst_t speexDecInst;
#endif
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
	QString stunSrv;
	bool destroySocket;
	int min;
	int max;
	bool useIPv6;
	bool useSRTP;
	int sdebug;
#ifdef SRTPWRAPPER_HX
	SRTPWrapper *wrapper;
#endif
};

#endif  // DSPOUTRTP_H_INCLUDED
