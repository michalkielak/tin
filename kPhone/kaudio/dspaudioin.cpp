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
#include <unistd.h>

#include "../config.h"
#include "audiobuffer.h"
#include "dspout.h"
#include "dspoutrtp.h"
#include "dspaudioin.h"
#include <iostream>
using namespace std;

DspAudioIn::DspAudioIn( DspOut *in, DspOut *out )
	: DspAudio( in, out )
{
	udp_failures = 0;
//	cout  << ":::::DSPAudioIn start\n";
}

DspAudioIn::~DspAudioIn( void ) {

//	cout  << ":::::DSPAdioIn stop\n";
}

void DspAudioIn::timerTick( void )
{
	if( broken ) {
		cout  << "(B)";
		fflush(stdout);
		return;
	}

	int bytes = input->readableBytes();
	
	// Fixed problem with ALSA or the OSS emulation layer
	bytes=bytes&0xfff0;
	
	if( bytes > 0 ) {
		// Read in a buffer
		input->readBuffer( bytes );

		// Make sure there's room in the output
		output->getBuffer().resize( input->getBuffer().getSize() );
		if (!generateDTMF((short*)output->getBuffer().getData(),
			input->getBuffer().getSize() / sizeof(short))) {

			// Copy the data
			memcpy( output->getBuffer().getData(),
				input->getBuffer().getData(),
				input->getBuffer().getSize() );
		}

		// If the write fails, mark the pipe as broken and stop sending!
		if( !output->writeBuffer() ) {
			udp_failures++;
			if(udp_failures > 5){
				broken = true;
				cout  <<  QObject::tr("DspAudioIn: Broken pipe") << endl ;
			} else {
				cout  <<  QObject::tr("DspAudioIn: UDP-failure ") <<  udp_failures << endl;
			}
		} else {
			udp_failures = 0;
		}
	} 
}

#ifndef QT_THREAD_SUPPORT
void DspAudioIn::start( void )
{
	timer = new QTimer();
	connect( timer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
	timer->start( 1 );
}

void DspAudioIn::timeout()
{
	timerTick();
}

#endif
