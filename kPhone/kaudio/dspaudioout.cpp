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
#include "dspaudioout.h"
#include <iostream>
using namespace std;

DspAudioOut::DspAudioOut( DspOut *in, DspOut *out )
	: DspAudio( in, out )
{
	curpos = 0;
}

DspAudioOut::~DspAudioOut( void )
{
}

void DspAudioOut::timerTick( void )
{
	if( broken ){
		cout  << "(b)";
		fflush(stdout);
		return;
	}

	short *curbuf;
	unsigned int outputsize = output->getBuffer().getSize() / sizeof( short );

	if ( !broken ) {
		int numBytes;
		if( input->getDeviceState() == DspOut::DeviceClosed ) {
			return;
		}
		numBytes = input->readableBytes();
		unsigned int samplesleft = input->getBuffer().getSize() / sizeof( short );
		short *inbuf = (short *)input->getBuffer().getData();

		if (numBytes > 0) {
			input->readBuffer( numBytes );
		} 
		if (!numBytes) {
			if (!currentTone && !newTone) {
				return;
			}
		} else if (dtmfSamples > 0) { 
			if (dtmfSamples - samplesleft > 0) {
				dtmfSamples -= 80;
				return;
			} else {
				inbuf += dtmfSamples;
				samplesleft -= dtmfSamples;
				dtmfSamples = 0;
			}
		}


		while( samplesleft > 0 ) {
			if( isStopped() ) break;
			if( samplesleft >= ( outputsize - curpos ) ) {
				curbuf = (short *) copybuffer.getData();
				curbuf += curpos;
				if (!generateDTMF(curbuf, outputsize - curpos)) {
					memcpy( curbuf, inbuf, ( outputsize - curpos ) * sizeof( short ) );
				}
				memcpy( output->getBuffer().getData(), copybuffer.getData(),
					output->getBuffer().getSize() );

				// If the write fails, mark the pipe as broken and stop sending!
				if( !output->writeBuffer() ) {
					cout  <<  QObject::tr("DspAudio: Broken pipe") << endl ;
					broken = true;
				}
				samplesleft -= ( outputsize - curpos );
				inbuf += ( outputsize - curpos );
				curpos = 0;

			} else {
				curbuf = (short *) copybuffer.getData();
				curbuf += curpos;
				if (!generateDTMF(curbuf, samplesleft)) {
					memcpy( curbuf, inbuf, samplesleft * sizeof( short ) );
				}
				curpos += samplesleft;
				samplesleft = 0;
			}
		}
	}
}


#ifndef QT_THREAD_SUPPORT
void DspAudioOut::start( void )
{
	timer = new QTimer();
	connect( timer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
	timer->start( 1 );
}

void DspAudioOut::timeout()
{
	timerTick();
}

#endif

