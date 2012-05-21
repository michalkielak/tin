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
#include <assert.h>
#include <unistd.h>
#include "../config.h"
#include "audiobuffer.h"
#include "dspout.h"
#include "dspoutrtp.h"
#include "dspaudio.h"
#include <iostream>
using namespace std;

DspAudio::DspAudio( DspOut *in, DspOut *out )
	: input( in ), output( out )
{
//	cout  << ":::::DspAudio start\n");
	broken = false;
	cancel = false;
	copybuffer.resize( out->getBuffer().getSize() );
	newTone = 0;
	currentTone = 0;
	input->getBuffer().resize( 0 );
	dtmfSamples = 0;
#ifndef QT_THREAD_SUPPORT
cout  << "?????????????????????????????????????????????TICK\n";
	timer = 0;
#endif
}

DspAudio::~DspAudio( void )
{
//	cout  << ":::::DspAudio stop\n");
//	if( input ) delete input;
//	if( output ) delete output;
#ifndef QT_THREAD_SUPPORT
	if( timer ) delete timer;
#endif
}

void  DspAudio::newConn ( DspOut *in, DspOut *out) {
      cout  <<  "=====DspAudio::newConn\n" ;
      input  = in;
      output = out;
}
#ifdef QT_THREAD_SUPPORT
void DspAudio::run( void )
{
	for(;;) {
		if( isStopped() ) {
			usleep( 5 );
			break;
		}
		timerTick();
//		usleep( 5 );
	}
#ifdef QTHREAD_TERMINATE
	terminate();  // use this function only if absolutely necessary. (e.g. Redhat9)
#else
	QThread::exit();
#endif
}
#endif

void DspAudio::startTone(char code)
{
	newTone = code;
}

void DspAudio::stopTone(void)
{
	newTone = 0;
}

bool DspAudio::generateDTMF(short* buffer, size_t n)
{
	if (!buffer) return false;
	if (currentTone != 0) {
		// Currently generating a DTMF tone
		if (currentTone == newTone) {
			// Continue generating the same tone
			dtmf.getNextSamples(buffer, n);
			return true;
		} else if (newTone != 0) {
			// New tone requested
			dtmf.getSamples(buffer, n, newTone);
			currentTone = newTone;
			return true;
		} else {
			// Stop requested
			currentTone = newTone;
			return false;
		}
	} else {
		// Not generating any DTMF tone
		if (newTone) {
			// Requested to generate a DTMF tone
			dtmf.getSamples(buffer, n, newTone);
			currentTone = newTone;
			return true;
		}
		return false;
	}
}

void DspAudio::setCodec( const codecType newCodec, int newCodecNum )
{
	((DspOutRtp *)input)->setCodec( newCodec, newCodecNum );
}
