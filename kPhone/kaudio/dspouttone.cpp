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
#include "dspouttone.h"




DspOutTone::DspOutTone( const QString &devName )
	: DspOut()
{
	rate = 8000;
}

DspOutTone::~DspOutTone( void ) {
//	printf(":::::DspOutTone stop\n");

}

bool DspOutTone::openDevice( DeviceMode mode ) {
	return true;
}
unsigned int DspOutTone::readableBytes( void ) {
        return 64;
}

bool DspOutTone::writeBuffer( void ) {
		usleep(100);
return true;
}

bool DspOutTone::readBuffer( int bytes )
{
		int i=0;
		double pi = 3.1416;
		int frames=32;
		audio_buf.resize( frames );
		void *buf;
		const int samp = frames*4;
		buf = alloca(samp);
//		const int ampl =frames;
		double arg1 = pi * (double)941 / (double)samp;
		double arg2 = pi * (double)1336 / (double)samp;
		int8_t *b1 = reinterpret_cast<int8_t*>(buf);

		for(int i = 0; i < samp; i++) {
			b1[i] = sin(arg1 * i) + sin(arg2 * i);
		}
		int8_t *b2 = reinterpret_cast<int8_t*>(audio_buf.getData());
		for( i=0; i < frames; i++ ) {
			b2[2*i]   = b1[4*i];
			b2[2*i+1] = b1[4*i+1];
		}
		usleep(200);
	        return true;
}

