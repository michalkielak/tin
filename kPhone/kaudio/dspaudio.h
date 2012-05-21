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
#ifndef DSPTHREAD_H_INCLUDED
#define DSPTHREAD_H_INCLUDED

#include "../config.h"

#ifdef QT_THREAD_SUPPORT
#include <qthread.h>
#else
#include <qtimer.h>
#include <qobject.h>
#endif

#include "audiobuffer.h"
#include "dtmfgenerator.h"
#include "../kphone/sessioncontrol.h"


class DspOut;

#ifdef QT_THREAD_SUPPORT
/**
* @short dsp base class
* 
The base class for audio processing, DspAudioIn and DspAudioOut are derived from this
*/
class DspAudio : public QThread
#else
/**
* @short dsp incoming
The base class for audio processing, DspAudioIn and DspAudioOut are derived from this
*/
class DspAudio : public QObject
#endif
{
public:
	DspAudio( DspOut *in, DspOut *out );
	virtual ~DspAudio( void );
#ifdef QT_THREAD_SUPPORT
/**
*	run forRever
*/
	virtual void run();
#else
	virtual void start( void ) = 0;
	bool running( void ) { return false; }
	void wait( void ) { return; }
	void exit( void ) { return; }
#endif
/**
*	the action of the derived classes
*/
	virtual void timerTick( void ) = 0;
/**
*	start a DTMF tone (not a RFCxxxx signal)
*/
	void startTone(char code);
/**
*	stop a DTMF tone
*/
	void stopTone(void);

/**
 * Generate DTMF if required
 * returns 1 if DTMF was generated
 * and 0 if not
*/
	bool generateDTMF(short* buffer, size_t n);
/**
*	interrupt and disable audio processing
*/
	void disableAudio( void ) { cancel = true; }

/**
*	allow audio processing
*/
	void enableAudio( void ) { cancel = false; }
/**
*	the codec for rtp postprocessing
*/
	void setCodec( const codecType newCodec, int newCodecNum );
/**
*	change the pointer to the processing class
*/
	void newConn (DspOut *in, DspOut *out);
protected:
/**
*	returns true if currently stopped
*/
	bool isStopped( void ) { return cancel; }

	DspOut *input;
	DspOut *output;
	bool broken;
	bool cancel;
	DTMFGenerator dtmf;
	AudioBuffer copybuffer;
	unsigned int dtmfSamples;
	char newTone;      // New DTMF tone to be generated (0 if none)
	char currentTone;  // Tone currently being generated (0 if none)

#ifndef QT_THREAD_SUPPORT
	QTimer *timer;
#endif

};

#endif // DSPTHREAD_H_INCLUDED
