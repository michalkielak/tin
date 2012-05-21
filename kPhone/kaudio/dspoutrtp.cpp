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
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <g711.h>
#include <qtimer.h>
#include <math.h>
#include <iostream>
using namespace std;
#include "../config.h"
#ifdef SPEEX
#include "../Speex/speex.h"
#include "../Speex/speex_define.h"
#endif
#include "../dissipate2/sipuri.h"
#include "../dissipate2/sipclient.h"
#include "audiobuffer.h"
#include "rtpdataheader.h"
#define SRTPWRAPPER_HX
#include "dspoutrtp.h"
#include "../kphone/kstatics.h"
/**
*
*/
short ILBCencode_20( iLBC_Enc_Inst_t *iLBCenc_inst,short *encoded_data, short *data);
/**
*
*/short ILBCdecode_20( iLBC_Dec_Inst_t *iLBCdec_inst,short *decoded_data, short *encoded_data, short mode);
short ILBCencode_30( iLBC_Enc_Inst_t *iLBCenc_inst,short *encoded_data, short *data);
/**
*
*/
short ILBCdecode_30( iLBC_Dec_Inst_t *iLBCdec_inst,short *decoded_data, short *encoded_data, short mode);

DspOutRtp::DspOutRtp( const codecType newCodec, int newCodecNum,  int minP, int maxP,int srtpMode, const QString &hostName, UDPMessageSocket *sock ){ 
	
	portnum=0;
	curseq=0;
	qlen=0;
	numunsend=0;
	fixedrtplen=160;
	deb_frag=0L;
	deb_rtp=0L;
	ssrc=10;
	ref_sec=0;
	ref_usec=0;
	dsize=4096;
	useStun=false;
	destroySocket=false;
	useIPv6=KStatics::haveIPv6;
	sdebug =  KStatics::debugLevel;
        useSRTP=false;
	min=minP;
	max=maxP;

	setDeviceName("rtp");
	codec = newCodec;
	codecNum = newCodecNum;
	if(sdebug >= 2) cout  << "=====DspOutRtp start codecNum= " << codecNum  << endl;
	if (sock) {
		if(sdebug >= 2) cout  << "=====DspOutRtp start use UDP-Socket\n";	
		destroySocket = false;
		socket = sock;
	} else {
		if(sdebug >= 2) cout  << "=====DspOutRtp start create new  UDP-Socket\n";	
		socket = new UDPMessageSocket(useIPv6);
		destroySocket = true;
		if (socket == 0) {
			if(sdebug >= 2) cout  << QObject::tr("=====DspOutRtp:: Can't create socket") + "\n" << endl;
			abort();
		}
	}
	if( hostName != QString::null && (socket->setHostname( hostName.latin1() ) == 0) ) {
		if(sdebug >= 2) cout  << QObject::tr("=====DspOutRtp::Hostname lookup failed")  << endl;
	}
	packetbuf = new unsigned char[ sizeof( rtp_hdr_t ) + dsize ];
	dtmfbuf   = new unsigned char[ 20];
	bufunsend = new unsigned char[ dsize ];
	outbuf    = new unsigned char[ dsize ];
	tmpbuf    = new unsigned char[ dsize ];
	quebuf    = new unsigned char[ dsize ];
	
	if( !( gsmInstEnc = gsm_create() ) ) {
		if(sdebug >= 2) cout  << "=====DspOutRtp::GSM_CREATE -error " << endl;
	}
	if( !( gsmInstDec = gsm_create() ) ) {
		if(sdebug >= 2) cout  << "=====DspOutRtp::GSM_CREATE -error " << endl;
	}
	
	initEncode(&ilbcEncInst_20, 20 );
	initDecode(&ilbcDecInst_20, 20, 1);
	initEncode(&ilbcEncInst_30, 30 );
	initDecode(&ilbcDecInst_30, 30, 1);
	
#ifdef SPEEX
	initspeexEncode (&speexEncInst);
	initspeexDecode (&speexDecInst);
#endif

	audio_buf.resize( dsize * sizeof( short )  );

#ifdef  SRTP
	if(srtpMode > 0 ){
		useSRTP = true;
		wrapper = new SRTPWrapper();
		if(sdebug >= 2) cout  << "=====DspOutRtp try to use SRTP" << endl;		
		if(wrapper==0)	{
		    useSRTP=false;
		    if(sdebug >= 2) cout  << "=====DspOutRtp Do not use SRTP"<< endl;
		} else {
		    if(sdebug >= 2) cout  << "=====DspOutRtp Use SRTP" << endl;
		}
	} else{
		if(sdebug >= 2) cout  << "=====DspOutRtp Do not use SRTP" << endl;
	}
#endif		


//	cout  << ":::::DspOutRtp start\n";
}

DspOutRtp::~DspOutRtp( void )
{;

	if( gsmInstEnc ) gsm_destroy( gsmInstEnc );
	
	if( gsmInstDec ) gsm_destroy( gsmInstDec );
	
	delete[] static_cast<unsigned char *>( packetbuf );	
	delete[] static_cast<unsigned char *>( bufunsend );
	delete[] static_cast<unsigned char *>( outbuf );
	delete[] static_cast<unsigned char *>( tmpbuf );
	delete[] static_cast<unsigned char *>( quebuf ); 
	delete[] static_cast<unsigned char *>( dtmfbuf );
#ifdef SPEEX
	termspeexEncode (&speexEncInst);
	termspeexDecode (&speexDecInst);
#endif

	if (destroySocket) {
			delete socket;
			if(sdebug >= 2) cout  << " murdered";
			}
	if(sdebug >= 2) cout  << endl;;
#ifdef  SRTP
	if(useSRTP) delete wrapper;
#endif
//	cout  << ":::::DspOutRtp stop\n";

}
bool DspOutRtp::openDevice( DeviceMode mode )
{
	devmode = mode;
	if( devmode == ReadOnly ) {
		socket->listenOnEvenPort(min, max);
		portnum = socket->getPortNumber();
		if(sdebug >= 2) cout << "=====DspOutRtp::openDevice readonly minport=" << min << " maxport=" << max << "usedport=" << portnum << endl;
		SipUri stun;
		if( useStun ) {
		    stun = SipUri( stunSrv );
		    if( socket->setHostname( stun.getHostname() ) == 0 ) {
			return false;
		    }    
		    if ( socket->sendStunRequest(stun.getHostname(),stun.getPortNumber())==0 ) {
				portnum = socket->receiveStunResponse();
				if( portnum > 0 ) {
					socket->forcePortNumber( portnum );
					if(sdebug >= 2) cout << "=====DspOutRtp::openDevice readonly " << portnum << endl;
					
				}
		    }
		}
	} else {
	if(sdebug >= 2) cout  << "=====DspOutRtp::openDevice notreadonly " << portnum << endl;
	socket->connect( portnum );
	socket->SetTOS();
	if(sdebug >= 2) cout  << "=====DspOutRtp::openDevice notreadonly " << portnum << endl;
	}
	devstate = DeviceOpened;
	return true;
}

#define PCMUCODEC 0
#define GSMCODEC 3
#define PCMACODEC 8
#define SPEEXCODEC 98
#define ILBCCODEC 97

typedef struct cb {
unsigned char (*func)( 	short int frombuf );
} cb_t;

bool DspOutRtp::writeBuffer( void )
{
	short *s;
	short *frombuf;
	unsigned char *databuf;
	rtp_hdr_t *h = (rtp_hdr_t *) packetbuf;
	size_t fromsize;
	unsigned char *wlbuf;
	size_t bytesnew;
	size_t bytesgot;
	int tmpsize;
	unsigned int i;
	bytesnew = audio_buf.getSize() / sizeof( short );
	fromsize = audio_buf.getSize() / sizeof( short );
	inbuf = (unsigned char*)audio_buf.getData();
	frombuf = (short *)inbuf;
	if( codec == codecGSM ) {
		short *frombuf_test;
		frombuf_test = frombuf;
		s = (short *)inbuf;
		for( i=0; i<fromsize; ++i) {
			s[i] = (short)(*frombuf_test);
			++frombuf_test;
		}
		tmpsize = writeGSMBuffer( gsmInstEnc, inbuf, outbuf, tmpbuf, quebuf, &qlen, fromsize );
		h->version = 2;
		h->p = 0;
		h->x = 0;
		h->cc = 0;
		h->m = 0;
		h->pt = GSMCODEC;
		while( tmpsize > 0 ) {
			h->seq = htons( ++curseq );
			h->ts = htonl( ts );
			ts += GSM_DEC_SAMPLES;
			h->ssrc = ssrc;
			databuf = packetbuf + sizeof( rtp_hdr_t );
			for( int i = 0; i < tmpsize; ++i ) {
				databuf[ i ] = outbuf[ i ];
			}
			int length = sizeof( rtp_hdr_t ) + tmpsize;
#ifdef  SRTP
			if(useSRTP){
				if(sdebug >= 3) cout  << "=====DspOutRtp try to protect\n");
				wrapper->protect(packetbuf, &length);
			}
#endif
			if( socket->send( (char *) packetbuf, length) < 0 ) {
			
				if(sdebug >= 3) cout  << "=====DspOutRtp::writeBuffer: " << strerror(errno) << endl;
				return false;
			}
			tmpsize = writeGSMBuffer( gsmInstEnc, inbuf, outbuf, tmpbuf, quebuf, &qlen, 0 );
		}
	}
	else if( codec == codecILBC_20 || codec == codecILBC_30 ) {
		short *frombuf_test;
		frombuf_test = frombuf;
		s = (short *)inbuf;
		for( i=0; i<fromsize; ++i) {
			s[i] = (short)(*frombuf_test);
			++frombuf_test;
		}
		if( codec == codecILBC_20 ) {
			tmpsize = writeILBCBuffer_20( &ilbcEncInst_20, inbuf, outbuf, tmpbuf, quebuf, &qlen, fromsize );
		} else {
			tmpsize = writeILBCBuffer_30( &ilbcEncInst_30, inbuf, outbuf, tmpbuf, quebuf, &qlen, fromsize );
		}
		h->version = 2;
		h->p = 0;
		h->x = 0;
		h->cc = 0;
		h->m = 0;
		h->pt = ILBCCODEC;
		while( tmpsize > 0 ) {
			h->seq = htons( ++curseq );
			h->ts = htonl( ts );
			if( codec == codecILBC_20 ) {
				ts += BLOCKL_20MS;
			} else {
				ts += BLOCKL_30MS;
			}
			h->ssrc = ssrc;
			databuf = packetbuf + sizeof( rtp_hdr_t );
			for( int i = 0; i < tmpsize; ++i ) {
				databuf[ i ] = outbuf[ i ];
			}
			int length = sizeof( rtp_hdr_t ) + tmpsize;
#ifdef  SRTP

			if(useSRTP){
				wrapper->protect(packetbuf, &length);
			}
#endif

			if( socket->send( (char *) packetbuf, length) < 0 ) {
				if(sdebug >= 3) cout  << "=====DspOutRtp::writeBuffer: " << strerror(errno) << endl;
				return false;
			}
			if( codec == codecILBC_20 ) {
				tmpsize = writeILBCBuffer_20( &ilbcEncInst_20, inbuf, outbuf, tmpbuf, quebuf, &qlen, 0 );
			} else {
				tmpsize = writeILBCBuffer_30( &ilbcEncInst_30, inbuf, outbuf, tmpbuf, quebuf, &qlen, 0 );
			}
		}
#ifdef SPEEX
	} else if( codec == codecSpeex ) {
		short *frombuf_test;
		frombuf_test = frombuf;
		int available_payload_size;
		tmpsize = writespeexBuffer( &speexEncInst, inbuf, outbuf, tmpbuf, quebuf, &qlen, fromsize );
		while( tmpsize > 0 ) {
			available_payload_size = fixedrtplen;
			h->version = 2;
			h->p = 0;
			h->x = 0;
			h->cc = 0;
			h->m = 0;
			h->pt = SPEEXCODEC;
			h->seq = htons( curseq++ );
			h->ts = htonl( ts );
			h->ssrc = ssrc;
			databuf = packetbuf + sizeof( rtp_hdr_t );

			int j = 0;
			while (tmpsize > 0) {

				for( int i = 0; i < tmpsize; i++ ) {
					databuf[ j ] = outbuf[ i ];
					j++;
				}
				ts += FRAME_SIZE;
				available_payload_size -= FRAME_SIZE;
				if (available_payload_size >= FRAME_SIZE)
					tmpsize = writespeexBuffer( &speexEncInst, inbuf, outbuf, tmpbuf, quebuf, &qlen, 0 );
				else
					tmpsize = 0;
			}

			int length = sizeof( rtp_hdr_t ) + j;// tmpsize;
#ifdef  SRTP

			if(useSRTP){
				wrapper->protect(packetbuf, &length);
			}
#endif

			if( socket->send( (char *) packetbuf, length) < 0 ) {
				
				if(sdebug >= 3) cout  << "=====DspOutRtp::writeBuffer: " << strerror(errno) << endl;
				return false;
			}
			tmpsize = writespeexBuffer( &speexEncInst, inbuf, outbuf, tmpbuf, quebuf, &qlen, 0 );
		}
#endif	
	} else if( codec == codecALAW || codec == codecULAW ) {
		cb_t callb;
		wlbuf = bufunsend;
		h->version = 2;
		h->p = 0;
		h->x = 0;
		h->cc = 0;
		h->m = 0;
		switch ( codec ) {
          case codecULAW:
			  h->pt = PCMUCODEC;
			  callb.func = &linear2ulaw;
			  break;
	  case codecALAW:
			  h->pt = PCMACODEC;
			  callb.func = &linear2alaw;
	  default:
			  break;
		}
		while(bytesnew+numunsend >= fixedrtplen){
			++deb_rtp;
			h->seq = htons( ++curseq );
			h->ts = htonl( ts );
			ts += fixedrtplen;
			h->ssrc = ssrc;
			databuf = packetbuf + sizeof( rtp_hdr_t );
			bytesgot = 0;
			while( numunsend > 0 ) {
				*databuf = *wlbuf;
				++databuf;
				++wlbuf;
				--numunsend;
				++bytesgot;
			}
			wlbuf = bufunsend;
			while( bytesgot < fixedrtplen ) {
				*databuf = callb.func( *frombuf );
				++databuf;
				++frombuf;
				--bytesnew;
				++bytesgot;
			}
			int length = sizeof( rtp_hdr_t ) + fixedrtplen;
#ifdef  SRTP

			if(useSRTP){
				wrapper->protect(packetbuf, &length);
			}
#endif
			

			if( socket->send( (char *) packetbuf, length) < 0 ) {
				
				if(sdebug >= 3) cout  << "=====DspOutRtp::writeBuffer: " <<  strerror(errno) << endl;
				return false;
			}
		}
		if( bytesnew > 0 ) {
			wlbuf = bufunsend + numunsend;
			numunsend += bytesnew;
			while( bytesnew > 0 ){
				*wlbuf = callb.func( *frombuf );
				++wlbuf;
				++frombuf;
				--bytesnew;
			}
		}
		++deb_frag;
	}
	else {
       if(sdebug >= 3) cout  << "=====DspOutRtp::writeBuffer: unknown Codec " << codec << endl;
	   return false;
	}

	return true;
}

bool DspOutRtp::setPortNum( int newport )
{
	portnum = newport;
	return true;
}

unsigned int DspOutRtp::readableBytes( void )
{
	struct timeval timeout;
	fd_set read_fds;
	int highest_fd;
	timeout.tv_sec = 0;
	timeout.tv_usec = 5;
	FD_ZERO( &read_fds );
	FD_SET( socket->getFileDescriptor(), &read_fds );
	highest_fd = socket->getFileDescriptor() + 1;
retry:
	if ( select( highest_fd, &read_fds, NULL, NULL, &timeout ) == -1 ) {
		if ( errno == EINTR ) goto retry;
		perror( "=====DspOutRtp::doSelect(): select() punted" );
		exit( 1 );
	}
	if ( FD_ISSET( socket->getFileDescriptor(), &read_fds ) ) {
		return 1;
	}
	return 0;
}

bool DspOutRtp::readBuffer( int bytes )
{
	unsigned char *inbuf;
	short *outbuf;
	int recvsize;
	int i;
	int size;
	
//	char fl = 1;

	recvsize = socket->receive( (char *) packetbuf, sizeof( rtp_hdr_t ) + dsize );
	rtp_hdr_t *h = (rtp_hdr_t *) packetbuf;
#ifdef  SRTP
	if(useSRTP){
		int length = recvsize;
		
		wrapper->unprotect(packetbuf, &length);
		recvsize = length;
	}
#endif
	if( h->pt == PCMUCODEC ) {
		audio_buf.resize( ( recvsize - (int) sizeof( rtp_hdr_t ) ) * sizeof( short ) );
		outbuf = (short *) audio_buf.getData();
		inbuf = &packetbuf[ sizeof( rtp_hdr_t ) ];
		for( i = 0; i < recvsize - (int) sizeof( rtp_hdr_t ); ++i ) {
			*outbuf = (short) ulaw2linear( inbuf[ i ] );
			++outbuf;
		}
	} else if( h->pt == GSMCODEC ) {
		audio_buf.resize( GSM_DEC_SAMPLES * sizeof( short ) );
		outbuf = (short *) audio_buf.getData();
		inbuf = &packetbuf[ sizeof( rtp_hdr_t ) ];
		readGSMBuffer( gsmInstDec, inbuf, tmpbuf, false );
		short *s = (short *)tmpbuf;
		for( i = 0; i < GSM_DEC_SAMPLES; ++i) {
			*outbuf = (short)(s[i]);
			++outbuf;
		}
	} else if( h->pt == PCMACODEC ) {
		audio_buf.resize( ( recvsize - (int) sizeof( rtp_hdr_t ) ) * sizeof( short ) );
		outbuf = (short *) audio_buf.getData();
		inbuf = &packetbuf[ sizeof( rtp_hdr_t ) ];
		for( i = 0; i < recvsize - (int) sizeof( rtp_hdr_t ); ++i ) {
			*outbuf = (short) alaw2linear( inbuf[ i ] );
			++outbuf;
		}
	} else if( codec == codecILBC_20 && (int)h->pt == ILBCCODEC ) {
		audio_buf.resize( BLOCKL_20MS * sizeof( short ) );
		outbuf = (short *) audio_buf.getData();
		inbuf = &packetbuf[ sizeof( rtp_hdr_t ) ];
		size = readILBCBuffer_20( &ilbcDecInst_20, inbuf, tmpbuf, 1, false );
		short *s = (short *)tmpbuf;
		for( i = 0; i < size; ++i) {
			*outbuf = (short)(s[i]);
			++outbuf;
		}
	} else if( codec == codecILBC_30 && (int)h->pt == ILBCCODEC ) {
		audio_buf.resize( BLOCKL_30MS * sizeof( short ) );
		outbuf = (short *) audio_buf.getData();
		inbuf = &packetbuf[ sizeof( rtp_hdr_t ) ];
		size = readILBCBuffer_30( &ilbcDecInst_30, inbuf, tmpbuf, 1, false );
		short *s = (short *)tmpbuf;
		for( i = 0; i < size; i++) {
			*outbuf++ = (short)(s[i]);
		}
#ifdef SPEEX
	} else if((int)h->pt == SPEEXCODEC ) {
			int lg = recvsize - (int) sizeof( rtp_hdr_t ) ;
			//audio_buf.resize( FRAME_SIZE * sizeof (short));
			outbuf = (short *) audio_buf.getData();
			inbuf = &packetbuf[ sizeof( rtp_hdr_t ) ];
			size = readspeexBuffer( &speexDecInst, inbuf, tmpbuf, 1, lg );
			audio_buf.resize( size * sizeof (short));
				short *s = (short *)tmpbuf;
				for( i = 0; i < size; i++) {
					*outbuf++ = (short)(s[i]);

				}
				lg -= 10;
				inbuf += 10;
#endif
	} else {
			/* A  good place to process incoming DTMF */
			audio_buf.resize (0);
	}

	return true;
}


void DspOutRtp::setCodec( const codecType newCodec, int newCodecNum )
{
	codec = newCodec;
	codecNum = newCodecNum;
}

/************************************************************************/
/* GSM                                                                   */
/************************************************************************/

int DspOutRtp::writeGSMBuffer( gsm gsmInst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size )
{
	int outbufsize = 0;
	int inq, inx;
	short *ibuf;
	short *obuf;
	short *tbuf;
	short *qbuf;
	ibuf = (short *)input_buf;
	obuf = (short *)output_buf;
	qbuf = (short *)queue;
	inq = *qlen;
	inx = 0;
	if( inq >= GSM_DEC_SAMPLES ) {
		tbuf = (short *)tmp_buf;
		while( inx < GSM_DEC_SAMPLES ) {
			*tbuf = *qbuf;
			++tbuf;
			++qbuf;
			--inq;
			++inx;
		}
		tbuf = (short *)tmp_buf;
		gsm_encode( gsmInst, (gsm_signal *)tbuf, (gsm_byte *)obuf);
		*qlen = inq;
		outbufsize = GSM_ENC_BYTES;
	} else if( inq + size >= GSM_DEC_SAMPLES ) {
		tbuf = (short *)tmp_buf;
		while( inq > 0 ) {
			*tbuf = *qbuf;
			++tbuf;
			++qbuf;
			--inq;
			++inx;
		}
		*qlen = 0;
		while( inx < GSM_DEC_SAMPLES ) {
			*tbuf = *ibuf;
			++tbuf;
			++ibuf;
			--size;
			++inx;
		}
		tbuf = (short *)tmp_buf;
		gsm_encode( gsmInst, (gsm_signal *)tbuf, (gsm_byte *)obuf);
		outbufsize = GSM_ENC_BYTES;

	}
	if( size > 0 ) {
		inq = *qlen;
		qbuf = (short *)(queue + ( inq * sizeof(short) ) );
		while( size > 0 ) {
			*qbuf = *ibuf;
			++inq;
			--size;
			++qbuf;
			++ibuf;
		}
		*qlen = inq;
	}
	return outbufsize;
}

int DspOutRtp::readGSMBuffer( gsm Gsm_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		int ignore )
{
	short *outbuf;
	short *ibuf;
	outbuf = (short *)output_buf;
	ibuf = (short *)input_buf;
	if( !ignore ) {
		gsm_decode( Gsm_Inst, (gsm_byte *)ibuf, (gsm_signal *)outbuf );
	}
	return true;
}



/*************************************************************************/
/* iLBC 20ms                                                                  */
/*************************************************************************/

int DspOutRtp::writeILBCBuffer_20( iLBC_Enc_Inst_t *Enc_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size )
{
	int outbufsize = 0;
	int inq, inx;
	short *ibuf;
	short *obuf;
	short *tbuf;
	short *qbuf;
	ibuf = (short *)input_buf;
	obuf = (short *)output_buf;
	qbuf = (short *)queue;
	inq = *qlen;
	inx = 0;
	if( inq >= BLOCKL_20MS ) {
		tbuf = (short *)tmp_buf;
		while( inx < BLOCKL_20MS ) {
			*tbuf = *qbuf;
			++tbuf;
			++qbuf;
			--inq;
			++inx;
		}
		tbuf = (short *)tmp_buf;
		outbufsize = ILBCencode_20( Enc_Inst, obuf, tbuf);
		*qlen = inq;
	} else if( inq + size >= BLOCKL_20MS ) {
		tbuf = (short *)tmp_buf;
		while( inq > 0 ) {
			*tbuf = *qbuf;
			++tbuf;
			++qbuf;
			--inq;
			++inx;
		}
		*qlen = 0;
		while( inx < BLOCKL_20MS ) {
			*tbuf = *ibuf;
			++tbuf;
			++ibuf;
			--size;
			++inx;
		}
		tbuf = (short *)tmp_buf;
		outbufsize = ILBCencode_20( Enc_Inst, obuf, tbuf);
	}
	if( size > 0 ) {
		inq = *qlen;
		qbuf = (short *)(queue + ( inq * sizeof(short) ) );
		while( size > 0 ) {
			*qbuf = *ibuf;
			++inq;
			--size;
			++qbuf;
			++ibuf;
		}
		*qlen = inq;
	}
	return outbufsize;
}


int DspOutRtp::readILBCBuffer_20( iLBC_Dec_Inst_t *Dec_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		short mode, int ignore)
{
	short *outbuf;
	short *ibuf;
	outbuf = (short *)output_buf;
	ibuf = (short *)input_buf;
	if( !ignore ) {
		return ILBCdecode_20( Dec_Inst, outbuf, ibuf, mode );
	}
	return 0;
}


/******************************************************************
 *  Encoder interface function
 *************************************************************---*/

short ILBCencode_20( iLBC_Enc_Inst_t *iLBCenc_inst,
		short *encoded_data,short *data )
{
	float block[BLOCKL_20MS];
	int k;

/* convert signal to float */

	for(k=0;k<BLOCKL_20MS;++k) block[k] = (float)data[k];

/* do the actual encoding */

	iLBC_encode((unsigned char *)encoded_data,block,iLBCenc_inst);

	return (short)(NO_OF_BYTES_20MS);
}


/******************************************************************
 *  Decoder interface function
 *************************************************************---*/

short ILBCdecode_20(iLBC_Dec_Inst_t *iLBCdec_inst,
		short *decoded_data, short *encoded_data, short mode)
{
	int k;
	float decblock[BLOCKL_20MS], dtmp;

/* check if mode is valid */

	if(mode<0 || mode>1){
		cout  << "\nERROR - Wrong mode - 0, 1 allowed\n";
		exit(3);
	}

/* do actual decoding of block */

	iLBC_decode( decblock,
		(unsigned char *)encoded_data, iLBCdec_inst, mode);

/* convert to short */

	for(k=0;k<BLOCKL_20MS;++k){
		dtmp=decblock[k];
		if (dtmp<MIN_SAMPLE)dtmp=MIN_SAMPLE;
		else if (dtmp>MAX_SAMPLE)dtmp=MAX_SAMPLE;
		decoded_data[k] = (short) dtmp;
	}

	return (short)BLOCKL_20MS;
}

/*************************************************************************/
/* iLBC 30ms                                                                  */
/*************************************************************************/

int DspOutRtp::writeILBCBuffer_30( iLBC_Enc_Inst_t *Enc_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size )
{
	int outbufsize = 0;
	int inq, inx;
	short *ibuf;
	short *obuf;
	short *tbuf;
	short *qbuf;
	ibuf = (short *)input_buf;
	obuf = (short *)output_buf;
	qbuf = (short *)queue;
	inq = *qlen;
	inx = 0;
	if( inq >= BLOCKL_30MS ) {
		tbuf = (short *)tmp_buf;
		while( inx < BLOCKL_30MS ) {
			*tbuf = *qbuf;
			++tbuf;
			++qbuf;
			--inq;
			++inx;
		}
		tbuf = (short *)tmp_buf;
		outbufsize = ILBCencode_30( Enc_Inst, obuf, tbuf);
		*qlen = inq;
	} else if( inq + size >= BLOCKL_30MS ) {
		tbuf = (short *)tmp_buf;
		while( inq > 0 ) {
			*tbuf = *qbuf;
			++tbuf;
			++qbuf;
			--inq;
			++inx;
		}
		*qlen = 0;
		while( inx < BLOCKL_30MS ) {
			*tbuf = *ibuf;
			++tbuf;
			++ibuf;
			--size;
			++inx;
		}
		tbuf = (short *)tmp_buf;
		outbufsize = ILBCencode_30( Enc_Inst, obuf, tbuf);
	}
	if( size > 0 ) {
		inq = *qlen;
		qbuf = (short *)(queue + ( inq * sizeof(short) ) );
		while( size > 0 ) {
			*qbuf = *ibuf;
			++inq;
			--size;
			++qbuf;
			++ibuf;
		}
		*qlen = inq;
	}
	return outbufsize;
}


int DspOutRtp::readILBCBuffer_30( iLBC_Dec_Inst_t *Dec_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		short mode, int ignore)
{
	short *outbuf;
	short *ibuf;
	outbuf = (short *)output_buf;
	ibuf = (short *)input_buf;
	if( !ignore ) {
		return ILBCdecode_30( Dec_Inst, outbuf, ibuf, mode );
	}
	return 0;
}


/******************************************************************
 *  Encoder interface function
 *************************************************************---*/

short ILBCencode_30( iLBC_Enc_Inst_t *iLBCenc_inst,
		short *encoded_data,short *data )
{
	float block[BLOCKL_30MS];
	int k;

/* convert signal to float */

	for(k=0;k<BLOCKL_30MS;++k) block[k] = (float)data[k];

/* do the actual encoding */

	iLBC_encode((unsigned char *)encoded_data,block,iLBCenc_inst);

	return (short)(NO_OF_BYTES_30MS);
}


/******************************************************************
 *  Decoder interface function
 *****************************************************************/

short ILBCdecode_30(iLBC_Dec_Inst_t *iLBCdec_inst,
		short *decoded_data, short *encoded_data, short mode)
{
	int k;
	float decblock[BLOCKL_30MS], dtmp;

/* check if mode is valid */

	if(mode<0 || mode>1){
		cout  << "\nERROR - Wrong mode - 0, 1 allowed\n";
		exit(3);
	}

/* do actual decoding of block */

	iLBC_decode( decblock,
		(unsigned char *)encoded_data, iLBCdec_inst, mode);

/* convert to short */

	for(k=0;k<BLOCKL_30MS;++k){
		dtmp=decblock[k];
		if (dtmp<MIN_SAMPLE)dtmp=MIN_SAMPLE;
		else if (dtmp>MAX_SAMPLE)dtmp=MAX_SAMPLE;
		decoded_data[k] = (short) dtmp;
	}

	return (short)BLOCKL_30MS;
}



void  DspOutRtp::setStunSrv( QString newStunSrv )
{
	if(sdebug >= 2) cout  << "=====DspOutRtp::setStunSrv =" << newStunSrv.latin1() << endl;
	useStun = true;
	stunSrv = newStunSrv;
}

#ifdef SPEEX
/************************************************************************/
/* Speex                                                                 */
/************************************************************************/
int DspOutRtp::writespeexBuffer( speex_Enc_Inst_t *Enc_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		unsigned char *tmp_buf, unsigned char *queue,
		int *qlen, int size )
{
	int outbufsize = 0;
	int inq, inx;
	short *ibuf;
	short *obuf;
	short *tbuf;
	short *qbuf;
	ibuf = (short *)input_buf;
	obuf = (short *)output_buf;
	qbuf = (short *)queue;
	inq = *qlen;
	inx = 0;
	if( inq >= FRAME_SIZE ) {
		tbuf = (short *)tmp_buf;
		while( inx < FRAME_SIZE ) {
			*tbuf = *qbuf;
			tbuf++;
			qbuf++;
			inq--;
			inx++;
		}
		tbuf = (short *)tmp_buf;
		  outbufsize = speex_encode( Enc_Inst, tbuf, obuf);
		*qlen = inq;
		tbuf = (short *) queue;
		while (inq > 0) {
			*tbuf = *qbuf;
			tbuf++;
			qbuf++;
			inq--;
		}
	} else if( inq + size >= FRAME_SIZE ) {
		tbuf = (short *)tmp_buf;
		while( inq > 0 ) {
			*tbuf = *qbuf;
			tbuf++;
			qbuf++;
			inq--;
			inx++;
		}
		*qlen = 0;
		while( inx < FRAME_SIZE ) {
			*tbuf = *ibuf;
			tbuf++;
			ibuf++;
			size--;
			inx++;
		}
		tbuf = (short *)tmp_buf;
		  outbufsize = speex_encode ( Enc_Inst, tbuf, obuf);

	}
	if( size > 0 ) {
		inq = *qlen;
		qbuf = (short *)(queue + ( inq * sizeof(short) ) );
		while( size > 0 ) {
			*qbuf = *ibuf;
			inq++;
			size--;
			qbuf++;
			ibuf++;
		}
		*qlen = inq;
	}
	return outbufsize;
}
int DspOutRtp::readspeexBuffer( speex_Dec_Inst_t *Dec_Inst,
		unsigned char *input_buf, unsigned char *output_buf,
		short mode, int lg)
{
	short *outbuf;
	short *ibuf;
	outbuf = (short *)output_buf;
	ibuf = (short *)input_buf;
		 return speex_decode( Dec_Inst, outbuf, ibuf,lg );
	return 0;
}

#endif

bool DspOutRtp::sendDTMF (int DTMFtone)
{

	if(sdebug >= 2) cout  << "=====DspOutRtp::sendDTMF tone="  << DTMFtone <<endl;
	rtp_hdr_t *h = (rtp_hdr_t *) packetbuf;
	rtp_dtmf_t *d = (rtp_dtmf_t *) dtmfbuf;

	unsigned char *databuf;
	int dur = 0;
	for (int i = 0; i < 18; i++)
	{
			h->version = 2;
			h->p = 0;
			h->x = 0;
			h->cc = 0;
			if (i == 0)
				h->m = 1;
			else
				h->m = 0;
			h->pt = 101;
			h->seq = htons( curseq++ );
			h->ts = htonl( ts );
			h->ssrc = htonl(ssrc);
			d->res = 0;
			d->vol = 10;
			d->id = DTMFtone;
			if (i < 17)
				d->eoe = false;
			else
				d->eoe = true;
			dur += fixedrtplen;
			d->dur = htons (dur);
			databuf = packetbuf + sizeof( rtp_hdr_t );
			for( int i = 0; i < 4; i++ ) {
				databuf[ i ] = dtmfbuf[ i ];
			}
			if( socket->send( (char *) packetbuf, sizeof( rtp_hdr_t ) + sizeof (rtp_dtmf_t ) ) < 0 ) {
				if(sdebug >= 3) cout  << "=====DspOutRtp::writeBuffer: " <<  strerror(errno) << endl;
				return false;
			}
	}
	ts += fixedrtplen * 18;
	return true;
}

void  DspOutRtp::setPayload( int payload ) {
	switch (codec) {

	case PCMUCODEC:
	case PCMACODEC:
	case SPEEXCODEC:
		if (payload > 160) {
			fixedrtplen = 160;	
		} else {
			fixedrtplen = (size_t)payload; 
		}
	break;
	case ILBCCODEC:
		if (payload < 160) {
			fixedrtplen = 160;	
		} else {
			fixedrtplen = (size_t)payload; 
		}
	break;
	default:
	fixedrtplen = 160;
	}
}
