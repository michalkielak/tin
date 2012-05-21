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
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>


#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>
#include <stdio.h>
#include <g711.h>
#include <qtimer.h>

#include "../dissipate2/sipuri.h"
#include "../dissipate2/sipclient.h"
#include "../dissipate2/udpmessagesocket.h"
#include "rtpdataheader.h"
#include "dspearly.h"
#include "../kphone/kstatics.h"
#include <iostream>
using namespace std;

DspEarly::DspEarly(void){
	useExt=false;
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
	packetbuf = new unsigned char[ dsize ];
	setDeviceName("rtp");
	min = 0;
	max = 0;
	codec = codecULAW;
	codecNum = 0;
	sendTimer = new QTimer();
	connect( sendTimer, SIGNAL( timeout() ),this, SLOT( writeBuffer1() ) );
//	cout  << ":::::DspEarly: start\n";
	}

DspEarly::~DspEarly( void )
{	
//	cout  << ":::::DspEarly: end\n");
	sendTimer->stop();
	delete sendTimer;
//	if (useExt) stopExt();
	delete[] static_cast<unsigned char *>( packetbuf );
}




void DspEarly::start( const codecType newCodec, int newCodecNum,  UDPMessageSocket *s, QString pipAddr,QString qpport  )
{
socket=s;
bool okd;
int portnum = qpport.toInt(&okd,10);
codec = newCodec;
codecNum = newCodecNum;
destroySocket = true;
dsize = 4096;  // initial max packet size
socket->setHostname( pipAddr.latin1() );
socket->connect( portnum );
socket->SetTOS();
devstate = DeviceOpened;
if(sdebug >= 3)cout  << " DspEarly::start tick\n";
sendTimer->start( 100);

}


#define PCMUCODEC 0
#define GSMCODEC 3
#define PCMACODEC 8

typedef struct cb {
unsigned char (*func)( int frombuf );
} cb_t;

void  DspEarly::writeBuffer1( void )
{
	unsigned char *databuf;
	rtp_hdr_t *h = (rtp_hdr_t *) packetbuf;
	int tmpsize=160;
	int length;
	unsigned char *buf=(unsigned char *)
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa444bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb4444444444444444444444444444444444444444444123456789012345678901234567890123456789";
		h->version = 2;
		h->p = 0;
		h->x = 0;
		h->cc = 0;
		h->m = 0;
		h->pt = PCMUCODEC;
		h->seq = htons( ++curseq );
		h->ts = htonl(160);
		h->ssrc = ssrc;
		databuf = packetbuf + sizeof( rtp_hdr_t );
		for( int i = 0; i < tmpsize ; ++i ) {
		databuf[ i ] = (unsigned char) buf[ i ];
		}
		length = sizeof( rtp_hdr_t ) + tmpsize;
		if(useExt) {
			if(sendto( rsock,(char *) packetbuf, length, 0,  &remoteaddress.soa, soal ) < 0 ) {
				if(sdebug >= 3) cout  << "=====DspEarly::writeBuffer: " << strerror(errno) << endl;
				return;
			}
		} else 
		{
			if( socket->send( (char *) packetbuf, length) < 0 ) {
				if(sdebug >= 3) cout  << "=====DspEarly::writeBuffer: " <<  strerror(errno) << endl;
				return;
			}
		}
		return ;
}
