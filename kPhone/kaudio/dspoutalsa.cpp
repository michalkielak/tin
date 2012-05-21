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
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include "../config.h"
#define ALSA_PCM_NEW_HW_PARAMS_API
//#define DBGR
//#define DBGW
#include "dspoutalsa.h"
#include <iostream>
#include <../kphone/kstatics.h>

using namespace std;

static snd_pcm_uframes_t chunk_size = 0;
static snd_pcm_uframes_t period_frames = 0;


DspOutAlsa::DspOutAlsa( const QString &devName )
	: DspOut()
{
	debug=KStatics::debugLevel;
	devname = devName;
	setDeviceName( "alsa" );
	rate = 8000;
//	cout  << ":::::DspOutAlsa start\n");
}

DspOutAlsa::~DspOutAlsa( void )
{
	if(devstate == DeviceOpened) {
		snd_pcm_drain( handle);
		snd_pcm_close( handle);
	}
//	cout  << ":::::DspOutAlsa stop\n");

}

bool  DspOutAlsa::alsaReset( snd_pcm_t *handle ) {
		snd_pcm_drop(handle);
		if( snd_pcm_prepare(handle) < 0) {
			cout  <<  "prep ";
			return false;
		}
		snd_pcm_start(handle);
		return true;
}

bool DspOutAlsa::openDevice( DeviceMode mode ){

	int dir;
	//static snd_pcm_sframes_t period_size;	
// set nonblock and leave, if device blocked
	if( mode == ReadOnly ) {
		if (debug >= 2) cout  <<  "=====DspOutAlsa::openDevice CAPTURE "  <<  devname.latin1() << endl;
		if ((err = snd_pcm_open( &handle, devname, SND_PCM_STREAM_CAPTURE,1)) < 0) { 
			cout  <<  "=====DspOutAlsa::openDevice cannot open audio device "  <<  snd_strerror (err) << endl;
			
			return false;
		}
	} else {
		if (debug >= 2) cout  <<  "=====DspOutAlsa::openDevice PLAYBACK " <<  devname.latin1();
		if ((err = snd_pcm_open( &handle, devname, SND_PCM_STREAM_PLAYBACK,1)) < 0) { 
			cout  <<  "=====DspOutAlsa::openDevice cannot open audio device "  <<   snd_strerror (err) << endl;
			return false;
		}
	} 

// set blocking mode
	snd_pcm_nonblock(handle,0);
	
	snd_pcm_hw_params_alloca( &hw_params);

	
	if ((err = snd_pcm_hw_params_any( handle, hw_params)) < 0) {
		cout  <<  "=====DspOutAlsa::openDevice cannot initialize hardware parameter structure "  << 
			 snd_strerror (err) << endl;
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access( handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		cout  <<  "=====DspOutAlsa::openDevice cannot set access type "  << 
			 snd_strerror (err) << endl;
		exit (1);
	}
#ifdef ENDIAN
	if ((err = snd_pcm_hw_params_set_format( handle, hw_params, SND_PCM_FORMAT_S16_BE)) < 0) {
		cout  <<  "=====DspOutAlsa::openDevice cannot set sample format " << 
			 snd_strerror (err) << endl;
		exit (1);
	}
#else
	if ((err = snd_pcm_hw_params_set_format( handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		cout  <<  "=====DspOutAlsa::openDevice cannot set sample format " << 
			 snd_strerror (err) << endl;
		exit (1);
	}
#endif

	if ((err = snd_pcm_hw_params_set_channels( handle, hw_params, 2 )) < 0) {
		cout  <<  "=====DspOutAlsa::openDevice cannot set channel count " << 
			 snd_strerror (err) << endl;
		exit (1);
	}
	unsigned int exact_rate = rate;
	dir = 0;
	if ((err = snd_pcm_hw_params_set_rate_near( handle, hw_params, &exact_rate, &dir )) < 0) {
		cout  <<  "=====DspOutAlsa::openDevice cannot set rate " << 
			 snd_strerror (err) << endl;
		exit (1);
	}
	rate = exact_rate;

	period_frames = 32; 
	if ((err = snd_pcm_hw_params_set_period_size_near( handle, hw_params, &period_frames, &dir)) < 0) {
		cout  <<  "=====DspOutAlsa::openDevice cannot set sample format " << 
			 snd_strerror (err) << endl;
		exit (1);
	}
	

	if ((err = snd_pcm_hw_params( handle, hw_params )) < 0) {
		cout  <<  "=====DspOutAlsa::openDevice cannot set parameters " << 
			 snd_strerror (err) << endl;
		exit (1);
	}
	snd_pcm_hw_params_get_period_size( hw_params, &chunk_size, &dir );
	snd_pcm_hw_params_get_buffer_size( hw_params, &buffer_size );

	audio_buf.resize( chunk_size*2 );
	if (chunk_size == buffer_size) {
		cout  <<  "=====DspOutAlsa::openDevice Can't use period equal to buffer size " <<  chunk_size << "==" << buffer_size;
		exit (1);
	}

	if ((err = snd_pcm_prepare (handle)) < 0) {
		cout  <<  "cannot prepare audio interface for use" << snd_strerror (err) << endl;
		exit (1);
	}

	lasterror = QString::null;
	devstate = DeviceOpened;

  
	return true;
}



bool DspOutAlsa::writeBuffer( void )
{
	snd_pcm_sframes_t delay;
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set,SIGALRM);
	sigprocmask(SIG_BLOCK,&set,NULL);

	int t = 0;	
	if( devstate != DeviceOpened ) {
		lasterror = "=====DspOutAlsa::writeBuffer Device Not Open";
		sigprocmask(SIG_UNBLOCK,&set,NULL);
		return false;
	}

	int rc;
	size_t count = audio_buf.getSize();
	void *buf;
	int8_t x0 = 0;
	int8_t x1 = 0;
	buf = alloca(2*(count+20));
	int8_t *b1 = reinterpret_cast<int8_t*>(audio_buf.getData());
	int8_t *b2 = reinterpret_cast<int8_t*>(buf);
	for( size_t i=0; i < count/2; i++ ) {
		x0 = b1[2*i];
		x1 = b1[2*i+1];
		b2[4*i]   = x0;
		b2[4*i+1] = x1;
		b2[4*i+2] = x0;
		b2[4*i+3] = x1;
	}
    int state = snd_pcm_state(handle);

    /* check for idle states, which need to be handled as delay=0 */
    if(state == SND_PCM_STATE_PREPARED || state == SND_PCM_STATE_PAUSED ||  state == SND_PCM_STATE_OPEN || SND_PCM_STATE_XRUN) { 

	if (snd_pcm_delay(handle, &delay) < 0) { 
		#ifdef DBGW
		cout  <<  "=====DspOutAlsa w:snd_pcm_delay failed, we  set delay " << delay << " to  0\n");
		#endif
		// this probably means a buffer underrund, therefore delay = 0 
		delay = 0; 
	}
	
    } else {
	#ifdef DBGW
	cout  << "=====DspOutAlsa w:pcm state set delay=0\n";
	#endif
	return true;
    }
    
    if (delay > (int)rate/10) { // more than 0.1 s delay 
	#ifdef DBGW
	cout  << "=====DspOutAlsa w:too high delay: " << delay << ">" << (int)rate/10) << endl;
	#endif
	delay = 0;
	return alsaReset( handle );
    } else if ((delay + count/2) < (rate/200)) { // after writing samples less than 5 ms delay 
	#ifdef DBGW
	cout  << "=====DspOutAlsa w:too little delay=" << delay << "count=" << count/2 << "rate=" << rate/200) << " underrun likely, inserting additional samples\n";
	#endif

	for( size_t i = count/2; i < count/2 + 10; i++ ) {
		b2[4*i]   = 0;//x0;
		b2[4*i+1] = 0;//x1;
		b2[4*i+2] = 0;//x0;
		b2[4*i+3] = 0;//x1;
	}
	count += 20;t=1;
    } 
	
    while ((rc = snd_pcm_writei(handle, buf, count/2)) != (int)count/2) {
		if (rc >= 0) {
		/* This should never happen, because we opened the device in blocking mode */
		#ifdef DBGW
			cout  <<   "=====DspOutAlsa w:write " << rc << " instead of " << (int)count/2 << " frames ! " << t << endl;
		#endif
			int8_t *buff = (int8_t*) buf;
			buff += rc*4;
			count -= rc*2;
			continue;
  		} else if (rc == -EPIPE) {
		//buffer underrun
		 snd_pcm_drop(handle);
		 #ifdef DBGW
			cout  <<  "=====DspOutAlsa w:buffer underrun\n";
		#endif			
		if (snd_pcm_prepare(handle) < 0) {
		#ifdef DBGW
			cout  <<  "=====DspOutAlsa w:snd_pcm_prepare failed\n";
		#endif
			return false;

		}
		snd_pcm_start(handle);
		return true;

		} else if ( (rc == - ESTRPIPE) ) {
		    int err;
		    #ifdef DBGW
		    cout  <<  "=====DspOutAlsa w:estrpipe\n";
		    #endif
		    while ((err = snd_pcm_resume(handle)) == -EAGAIN) {
			#ifdef DBGW
			cout  <<  "=====DspOutAlsa w:resume failed, prepare\n";
			#endif
			usleep(100); 
		    }
		    if (err < 0) {
		    #ifdef DBGW
			cout  <<  "=====DspOutAlsa w:resume failed not EGAIN, prepare\n";
		    #endif
		        if (snd_pcm_prepare(handle) < 0) {
				#ifdef DBGW
			        cout  <<  "w:snd_pcm_prepare failed finally\n";
				#endif
			}
		    }
		    continue;
		} else {
		snd_pcm_drop(handle);
		#ifdef DBGW
			cout  <<  "=====DspOutAlsa w:other problem\n";
		#endif
			if (snd_pcm_prepare(handle) < 0) {
			    #ifdef DBGW
				cout  <<  "=====DspOutAlsa w:snd_pcm_prepare failed\n");
			    #endif
				return false;
			}
			 snd_pcm_start(handle);
		}
	}
	sigprocmask(SIG_UNBLOCK,&set,NULL);
	return true;
}

unsigned int DspOutAlsa::readableBytes( void ) {
    if( devstate == DeviceOpened ) {
	return 128;//64;
    }
    return 0;
}

bool DspOutAlsa::readBuffer( int bytes ) {
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set,SIGALRM);
	sigprocmask(SIG_BLOCK,&set,NULL);
	if( devstate != DeviceOpened ) {
		lasterror = "=====DspOutAlsa::readBufferr: Device Not Open";
		sigprocmask(SIG_UNBLOCK,&set,NULL);
		return false;
	}

	int rc;//,i;
	size_t frames = audio_buf.getSize()/2;
	frames=32;
	audio_buf.resize( frames*2 );
	void *buf;
	buf = alloca(frames*4);
        do {
            rc = snd_pcm_readi(handle, buf, frames);
        } while (rc == -EAGAIN);
	
	if (rc == -EPIPE) {
	    if (snd_pcm_prepare(handle) < 0) {
		#ifdef DBGR 
		cout  <<  "=====DspOutAlsa r:EPIPE snd_pcm_prepare failed\n";
		#endif
		return false;
	    }
	    snd_pcm_start(handle);
	    #ifdef DBGR 
	    cout  <<  "=====DspOutAlsa r:EPIPE\n";
	    #endif
	    return true;
	} else if ( (rc == - ESTRPIPE) ) {
		int err;
		#ifdef DBGR 
		cout  <<  "=====DspOutAlsa r:strpipe \n";
		#endif
		while ((err = snd_pcm_resume(handle)) == -EAGAIN) {
		    #ifdef DBGR 
		    cout  <<  "=====DspOutAlsa r:resume failed\n";
		    #endif
		    usleep(100); 
		}
	} else if (rc < 0) {
	    #ifdef DBGR 
	    cout  <<  "=====DspOutAlsa r error: " << snd_strerror(rc) << endl;
	    #endif
	    return false;
	} else if (rc != (int)frames) {
	    #ifdef DBGR 
	    cout  << "=====DspOutAlsa r: warning, asked microphone for " << (unsigned long)frames << " frames but got " << (unsigned long)rc) << endl;
	    #endif
	    frames = rc;
	    audio_buf.resize( frames*2 );
	}
	int8_t *b1 = reinterpret_cast<int8_t*>(buf);
	int8_t *b2 = reinterpret_cast<int8_t*>(audio_buf.getData());
	for( size_t i=0; i < frames; i++ ) {
		b2[2*i]   = b1[4*i];
		b2[2*i+1] = b1[4*i+1];
	}
	sigprocmask(SIG_UNBLOCK,&set,NULL);
	return true;
}
