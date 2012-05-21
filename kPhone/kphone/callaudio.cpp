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
//
// C++ Implementation: callaudio2
//
// Description:
//
//
// Author: root <root@papa>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <sys/ioctl.h>
#include <iostream>
using namespace std;

#include "../config.h"
#include "../dissipate2/sipprotocol.h"
#include "../dissipate2/sipcall.h"
#include "../dissipate2/sdpbuild.h"
#include "kstatics.h"
#include "../kaudio/dspaudioout.h"
#include "../kaudio/dspaudioin.h"
#include "../kaudio/dspoutrtp.h"
#include "../kaudio/dspoutoss.h"
#include "../kaudio/dspoutalsa.h"
#include "../kaudio/dspouttone.h"

#include "callaudio.h"

CallAudio::CallAudio(SessionControl *sessionC, const char *name )
{
	sc = sessionC;
	outrtp = 0;
	inrtp = 0;
	inoss = 0;
	outoss = 0;
	inalsa = 0;
	outalsa = 0;
	output = 0;
	intone = 0;
	input = 0;
	useoss = true;
	payload = sc->getPayload();
	useStun = false;
	audio_fd = -1;
	charDTMF[0]='0';
	charDTMF[1]='1';
	charDTMF[2]='2';
	charDTMF[3]='3';
	charDTMF[4]='4';
	charDTMF[5]='5';
	charDTMF[6]='6';
	charDTMF[7]='7';
	charDTMF[8]='8';
	charDTMF[9]='9';
	charDTMF[10]='*';
	charDTMF[11]='#';
	dtmfCount = 0;
	dtmfSize  = 0;
	dtmfTimer = new QTimer();
	connect( dtmfTimer, SIGNAL( timeout() ),
		this, SLOT( handleDTMFSeq() ) );
	socket.reInit(KStatics::haveIPv6);
	debug = KStatics::debugLevel;
	audioUser=0;
	outRunning = false;
	 if (KStatics::debugLevel >= 2)cout << ":::::Callaudio start\n";

}


CallAudio::~CallAudio()
{

if( output ) {
		if( output->running() ) {
			output->disableAudio();
			output->wait();
		}
		delete output;
	}
	if( input ) {
		if( input->running() ) {
			input->disableAudio();
			input->wait();
		}
		delete input;
	};
if(dtmfTimer) {
		delete dtmfTimer;
		dtmfTimer=0;
}
	 if (KStatics::debugLevel >= 2)cout << ":::::Callaudio end\n";

}


void CallAudio::audioTone( bool tone, int aU )
{
//bool ok;
audioUser=aU;
if( sc->getAudioSys() == SessionControl::isALSA ) {
    if (!inalsa || !intone) return;
    if( tone ) {
    	if(debug >= 2) cout <<  "=====CallAudio  audioTone:  tone\n";
	if (input){	
	    input->newConn( intone,outrtp); 
	} else {
	    input = new DspAudioIn( intone, outrtp );
	}
    } else { 
	if(debug >= 2) cout <<  "=====CallAudio  audioTone:  alsa\n";
	if (input) { 
	    input->newConn( inalsa,outrtp);
	} else {
	    input = new DspAudioIn( inalsa, outrtp );
	}
    }
} else if( sc->getAudioSys() == SessionControl::isOSS ) {
    if (!inoss || !intone) return;
    if( tone ) {
    	if(debug >= 2) cout <<  "=====CallAudio  audioTone:  tone\n";
	if (input){	
	    input->newConn( intone,outrtp); 
	} else {
	    input = new DspAudioIn( intone, outrtp );
	}
    } else { 
	if(debug >= 2) cout <<  "=====CallAudio  audioTone:  alsa\n";
	if (input) { 
	    input->newConn( inoss,outrtp);
	} else {
	    input = new DspAudioIn( inoss, outrtp );
	}
    }

}

}
int CallAudio::audioIn( SdpBuild *call, bool tone, int aU )
{
	bool ok;
	audioUser=aU;
	int min, max;
	min = sc->getMinMediaPort();
	max = sc->getMaxMediaPort();
	if(debug >= 2) cout << "=====CallAudio audioIn: " << (int) sc->getAudioSys();
	if(!output) {
	    if(debug >= 2) cout <<  "\n" ;
	    return 0;
	}
	QString hostname = call->getExtHost();
	unsigned int portnum = call->getAudioPort().toInt( &ok, 10 );;
	if(debug >= 2) cout <<  " Sending to " <<  hostname.latin1() << ":" << portnum << endl;
/*	if( input ) {
		if(debug >= 2) cout << "+++++++++=====CallAudio audioIn: stopListeningAudio\n" ;
		stopListeningAudio(aU);
	}
*/	
	if( sc->getAudioSys() == SessionControl::isOSS ) {
		outrtp = new DspOutRtp(call->getRtpCodec(), call->getRtpCodecNum(), min, max, sc->getSrtpMode(), hostname, &socket );
		outrtp->setPortNum( portnum);
		outrtp->openDevice( DspOut::WriteOnly );
		outrtp->setPayload( call->getResponsePay() );
		if(debug >= 2) cout <<  "=====CallAudio audioIn: RTP " << hostname.latin1() << ":" <<  portnum << outrtp->getPortNum() <<endl;

		if(!intone) intone = new DspOutTone( "default");

		if(!inoss)  {
		    inoss = new DspOutOss( sc->getOSSFilename2() );
		    if( sc->isAudioRW()) {
			if(debug >= 2) cout <<  "=====CallAudio audioIn: OSS device already open (readwrite)\n" ;
			if( !inoss->openDevice( audio_fd ) ) {
				if(debug >= 2) cout <<  "=====CallAudio audioIn:  openDevice Failed.\n" ;
			}
		    } else {
			if(debug >= 2) cout <<  "=====CallAudio audioIn: Opening OSS device " << sc->getOSSFilename2().latin1() << "for Input \n";
			if( !inoss->openDevice( DspOut::ReadOnly ) ) {
				if(debug >= 2) cout <<  "=====CallAudio: audioIn openDevice Failed.\n" ;
			}
		    }
		}
		
		inoss->readBuffer(2);
		
		
		if(debug >= 2) cout <<  "=====CallAudio audioIn: Creating OSS->RTP Diverter";
		if( tone ) {
		    if(debug >= 2) cout <<  " use tone\n";
		    if (input){	
			input->newConn( intone, outrtp); 
		    } else {
			input = new DspAudioIn( intone, outrtp );
		    }
		} else { 
		    if(debug >= 2) cout <<  " use oss\n";
		    if (input) { 
			input->newConn( inoss, outrtp);
		    } else {
			input = new DspAudioIn( inoss, outrtp );
		    }
		}
		output->setCodec(call->getRtpCodec(), call->getRtpCodecNum());
		input->enableAudio();
		input->start();
		if(!outRunning) {
			output->enableAudio();
			output->start();
			if(debug >= 2) cout <<  "=====CallAudio audioIn start output" << endl;
		}
		outRunning = true;
		if(debug >= 2) cout <<  "\n" ;
		return 1;

	} else if( sc->getAudioSys() == SessionControl::isALSA ) {
	//RTP part
		if(debug >= 2) cout << "=====CallAudio ALSA codec = " << call->getRtpCodecNum() << endl;
		outrtp = new DspOutRtp( call->getRtpCodec(), call->getRtpCodecNum(), min, max, sc->getSrtpMode(), hostname, &socket );
		outrtp->setPortNum( portnum);
		outrtp->openDevice( DspOut::WriteOnly );
		outrtp->setPayload( call->getResponsePay() );
	
	//ALSA part

		if (!intone) intone = new DspOutTone( "default");
		if (!inalsa) {
			inalsa = new DspOutAlsa( sc->getALSAFilename() );//("default");
			if(debug >= 2) cout <<  "=====CallAudio audioIn: device = " << sc->getALSAFilename().latin1();
	
			if( !inalsa->openDevice( DspOut::ReadOnly ) ) {
				if(debug >= 2) cout <<  " Failed " << endl ;
			} else {
				if(debug >= 2) cout <<  " Open " << endl ;

			}
		}
	
//Connecting  part
		//inalsa->readBuffer(16);
		if(debug >= 2) cout <<  "=====CallAudio audioIn: Creating ALSA->RTP Diverter ";
		if( tone ) {
			if(debug >= 2) cout <<  " use tone\n";
			if (input){	
				input->newConn( intone , outrtp); 
			} else {
				input = new DspAudioIn( intone, outrtp );
			}
		} else { 
			if(debug >= 2) cout <<  " use alsa\n";
			if (input) { 
				input->newConn( inalsa , outrtp);
			} else {
				input = new DspAudioIn( inalsa, outrtp );
			}
		}
		output->setCodec(call->getRtpCodec(), call->getRtpCodecNum());
		if(debug >= 2) cout <<  "=====CallAudio audioIn start input ";
		input->enableAudio();
		input->start();
		if(!outRunning) {
			output->enableAudio();
			output->start();
			if(debug >= 2) cout <<  "=====CallAudio audioIn start output ";
		}
		outRunning = true;
                //inalsa->readBuffer(2);
		if(debug >= 2) cout <<  "\n" ;
		return 1;
	}
	return 0;
}


int CallAudio::audioOut(  SdpBuild *call,int aU )
{
	audioUser=aU;
	useStun=sc->isStun();
	int min, max, po = 0;
	min =sc-> getMinMediaPort();
	max = sc->getMaxMediaPort();
/*	if( output  ) {
		if(debug >= 2) cout <<  "++++++++++=====CallAudio audioOut: stop sending audio \n" ;
		disableAudio(aU);
		}
*/
	if( sc->getAudioSys() == SessionControl::isOSS ) {
		if(debug >= 2) cout <<  "=====CallAudio audioOut: OSS used codec =" << call->getRtpCodecNum() << " payload size = " << call->getResponsePay() << endl;
		inrtp = new DspOutRtp( call->getRtpCodec(), call->getRtpCodecNum(), min, max, sc->getSrtpMode(), QString::null, &socket );
		inrtp->setPayload(call->getResponsePay() );
		if( useStun ) {
		if(debug >= 2) cout <<  "=====CallAudio audioOut: use stun\n";
		inrtp->setStunSrv( sc->getStunSrv() );
		}
		inrtp->openDevice( DspOut::ReadOnly );
		po=inrtp->getPortNum();
		if(debug >= 2) cout <<  "=====CallAudio audioOut: RTP  " << po << endl;

		if(!outoss) {
		    outoss = new DspOutOss( sc->getOSSFilename() );
		    if( sc->isAudioRW() ) {
			if( !outoss->openDevice( DspOut::ReadWrite ) ) {
				if(debug >= 2) cout << "=====CallAudio audioOut: openDevice Failed.\n" ;
				po=0;
			} else {
				audio_fd = outoss->audio_fd;
			}
		    } else {
			if( !outoss->openDevice( DspOut::WriteOnly )){
				if(debug >= 2) cout << "=====CallAudio audioOut: openDevice Failed.\n" ;
				po=0;
			}
		    }
		}
		
		if(debug >= 2) cout <<  "=====CallAudio audioOut: Creating RTP->OSS Diverter\n";
		
		if (!output) {
			output = new DspAudioOut( inrtp, outoss );
		} else {
			output->newConn( inrtp, outoss );
		}

		return po;
	} else 	if( sc->getAudioSys() == SessionControl::isALSA ) {
	//RTP part
		if(debug >= 2) cout <<  "=====CallAudio audioOut: ALSA codec = " << call->getRtpCodecNum();
		inrtp = new DspOutRtp( call->getRtpCodec(), call->getRtpCodecNum(),  min, max, sc->getSrtpMode(), QString::null, &socket );
		inrtp->setPayload(call->getResponsePay() );
		if( useStun ) {
			if(debug >= 2) cout <<  "=====CallAudio audioOut: use stun\n";
			inrtp->setStunSrv( sc->getStunSrv() );
		}
		inrtp->openDevice( DspOut::ReadOnly );
		po=inrtp->getPortNum();

	//ALSA part		
		if(debug >= 2) cout <<  "=====CallAudio audioOut: open ALSA device = " << sc->getALSAFilename().latin1() << endl;
		if(!outalsa) {
			outalsa = new DspOutAlsa( sc->getALSAFilename() );//default
			if( !outalsa->openDevice( DspOut::WriteOnly )){
			if(debug >= 2) cout << "=====CallAudio audioOut: Failed. \n";
			//    po=0;
			}
		}
	
	//Connection part
		if(debug >= 2) cout <<  "=====CallAudio audioOut: Creating RTP->ALSA Diverter\n";
		if (!output) {
			output = new DspAudioOut( inrtp, outalsa );
		} else {
			output->newConn( inrtp, outalsa );
		}
		return po;
	}
	
return 0;
}

void CallAudio::stopAudio( int aU ) {

if (aU == audioUser) {

	if(debug >= 2) cout << "=====CallAudio: stopSendingAudio " << audioUser << endl;
	if( output ) {
	    if( output->running() ) {
		output->disableAudio();
	    	output->wait();
	    }
	    if(debug >= 2) cout << "=====CallAudio: stopSendingAudio output deleted " << audioUser << endl;
	    delete output; output = 0;
	    outRunning = false;

	    if (inrtp) delete inrtp; inrtp=0;
	    if(outalsa) delete outalsa;outalsa=0;
	    if(outoss) delete outoss;outoss=0;
	    
	}


	if( input ) {
	    if( input->running() ) {
		input->disableAudio();
		input->wait();
	    }
    	    if(debug >= 2) cout << "====CallAudio: stopSendingAudio input deleted " << audioUser << endl;
	    delete input; input = 0; 
	    
	    if (outrtp) delete outrtp; outrtp=0;
	    
	    if(intone) delete intone;intone=0;
	    if(inalsa) delete inalsa;inalsa=0;
	    if(inoss) delete inoss;inoss=0;
	    
	}
	
	clearMe(aU);
}

}
void CallAudio::stopRTPAudio( int aU ) {

if (aU == audioUser) {
	if(debug >= 2) cout << "====CallAudio: stopRTPAudio  " << audioUser << endl;
	if( output ) {
	    if( output->running() ) {
		output->disableAudio();
	    	output->wait();
	    }
	    outRunning = false;
	    if (inrtp) delete inrtp; inrtp=0;
	}


	if( input ) {
	    if( input->running() ) {
		input->disableAudio();
		input->wait();
	    }
	    if (outrtp) delete outrtp; outrtp=0;
	}
	
	clearMe(aU);
}

}
void CallAudio::stopListeningAudio( int aU )
{
    if (aU == audioUser) {
 
	if(debug >= 2) cout << "=====CallAudio: stopListeningAudio " << audioUser << endl;

	if( input ) {
	    if( input->running() ) {
	        input->disableAudio();
	        input->wait();
	    }
	    cout << "=====CallAudio: stopListeningAudio delete input \n";
	    delete input;input = 0;
	    if(intone) delete intone;intone=0;
	    if(inalsa) delete inalsa;inalsa=0;
	    if(inoss) delete inoss;inoss=0;
	}
    }
}

bool CallAudio::isAudioOn( void )
{
	return (output || input );
}

void CallAudio::startDTMF(int tone)
{
	if (outrtp) {

	    outrtp->sendDTMF(tone);
	    if(debug >= 2) cout <<  "tone " << tone << endl;
	}

	if (output) {
		output->startTone(charDTMF[tone]);
	}

	if (input) {
		input->startTone(charDTMF[tone]);
	}
}
void CallAudio::startDTMFSeq(QString string, int size)
{
dtmfCount = 0;
dtmfSize = size;
dtmfString = string;
dtmfTimer->start(500,TRUE);
}
void CallAudio::handleDTMFSeq() {
int tone = 0, id =0;

	if(dtmfCount < dtmfSize) {
		id        = (int)(*dtmfString.mid(dtmfCount,1));
		switch(id) {
			case 0x30: tone = 0; break;
			case 0x31: tone = 1; break;
			case 0x32: tone = 2; break;
			case 0x33: tone = 3; break;
			case 0x34: tone = 4; break;
			case 0x35: tone = 5; break;
			case 0x36: tone = 6; break;
			case 0x37: tone = 7; break;
			case 0x38: tone = 8; break;
			case 0x39: tone = 9;break;
			case 0x2a: tone = 10; break;
			case 0x23: tone = 11;break;
			case 0x2c: dtmfCount ++; dtmfTimer->start(sc->getDTMFWait(),TRUE); return;
			default: tone = 0;
		}
		if (outrtp) outrtp->sendDTMF(tone);
		dtmfCount ++;
	dtmfTimer->start(500,TRUE);
	}

}

void CallAudio::stopDTMF(void)
{
	if (output) {
		output->stopTone();
	}

	if (input) {
		input->stopTone();
	}
}
void CallAudio::ringOnce(int mul) {

	int audio_fd = ::open( sc->getRingtonedevice(), O_WRONLY | O_NONBLOCK );
		if( audio_fd == -1 ) {
			cout <<  "!!!!!ERROR: Open Failed" << endl;
			return;
		}
		int flags = fcntl( audio_fd, F_GETFL );
		flags &= ~O_NONBLOCK;
		fcntl( audio_fd, F_SETFL, flags );
		int format = AFMT_S16_LE;
		if( ioctl( audio_fd, SNDCTL_DSP_SETFMT, &format ) == -1 ) {
			return;
		}
		if( format != AFMT_S16_LE ) {
			return;
		}
		int channels = 1;
		if( ioctl( audio_fd, SNDCTL_DSP_CHANNELS, &channels ) == -1 ) {
			return;
		}
		if( channels != 1 ) {
			return;
		}
		int rate = 8000;
		if( ioctl( audio_fd, SNDCTL_DSP_SPEED, &rate ) == -1 ) {
			return;
		}
		if( rate != 8000) {
			return;
		}
		int size = 1024*16;
		int samp = 1024*(mul-1)*4;
		int ampl = 1024*16;
		unsigned char devbuf[size];
		int buf[samp];
		int i, p=0;
		double arg1, arg2;
		arg1 = (double)2 * (double)M_PI * (double)941 / (double)samp;
		arg2 = (double)2 * (double)M_PI * (double)1336 / (double)samp;
		for(int i = 0; i < samp; i++) {
			buf[i] = (short)((double)(ampl) * sin(arg1 * i) +
				(double)(ampl) * sin(arg2 * i));
		};
		for (i=0; i<samp; i+=2) {
			devbuf[p++] = (unsigned char)(buf[i] & 0xff);
			devbuf[p++] = (unsigned char)((buf[i] >> 8) & 0xff);
		}
		for(;;) {
			if( write( audio_fd, devbuf, samp ) != -1 ) {
				break;
			}
		}
		::close( audio_fd );
}

void CallAudio::clearMe(int cwuser) {

	if(audioUser==cwuser){
 		audioUser=0;
	}
}

bool CallAudio::audioOwner(int cwuser){
	if(audioUser==cwuser) return true; else return false;
}
