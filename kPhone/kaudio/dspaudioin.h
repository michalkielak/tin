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
#ifndef DSPAUDIOIN_H_INCLUDED
#define DSPAUDIOIN_H_INCLUDED

#include "dspaudio.h"

#include "../config.h"

class DspOut;

/**
* @short dsp incoming
*
This class scans the audio handlers (rtp, alsa, oss, tone)
and transports the data between them.
Direction: micro->network
*/
class DspAudioIn : public DspAudio
{
#ifndef QT_THREAD_SUPPORT
	Q_OBJECT
#endif
public:
/**
*Constructor: sets up the devices to be connected
*/
	DspAudioIn( DspOut *in, DspOut *out );
	~DspAudioIn( void );

	/**
	 * Just do one scan and transport
	   depending if timer or threads are used, timerTick is called by timeout or run
	 */
	virtual void timerTick( void );

#ifndef QT_THREAD_SUPPORT
	virtual void start( void );

private slots:
	virtual void timeout( void );

#endif

private:
	int udp_failures;
};

#endif // DSPAUDIOIN_H_INCLUDED
