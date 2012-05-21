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
#ifndef DSPOUT_H_INCLUDED
#define DSPOUT_H_INCLUDED

#include <qstring.h>

#include "audiobuffer.h"

/**
 * @short Base class for DSP audio devices.
 * @author Billy Biggs <vektor@div8.net>
 *
 * This is the base class of KPhone's handlers for the audio related devices, such as
 * /dev/dsp for OSS, /dev/audio for Sun audio and hw:x,y for alsa.
 * .
 */ 
class DspOut
{
public:
	/**
	 * DspOut constructor just does simple initialization.
	 */
	DspOut( void );

	/**
	 * DspOut destructor.
	 */
	virtual ~DspOut( void );

	enum DeviceState {
		DeviceOpened,
		DeviceClosed };

	enum DeviceMode {
		ReadOnly,
		WriteOnly,
		ReadWrite };

	/**
	 * This is the call to actually open the device for
	 * audio output.  All setup (samplerate, format) must be
	 * done before the call to openDevice().
	 */
	virtual bool openDevice( DeviceMode mode ) = 0;

	/**
	 * Return the state of the device, opened or closed.
	 */
	DeviceState getDeviceState( void ) const { return devstate; }

	/**
	 * This provides the caller with the next audio buffer to
	 * fill.  A call to @ref #writeBuffer () will cause this
	 * to actually be queued by the soundcard.
	 *
	 * The format of the audio buffer is that of the internal
	 * program format.  The call to @ref #writeBuffer () will
	 * perform the appropriate conversion to the output format,
	 * if any is required.
	 */
	AudioBuffer &getBuffer( void ) { return audio_buf; }

	/**
	 * Send the current audio buffer (available using @ref #getBuffer () )
	 * to the soundcard.  This applies any conversion necessary to go
	 * from the internal format to the format required by the device.
	 */
	virtual bool writeBuffer( void ) = 0;

	/**
	 * Returns the number of currently readable bytes.
	 */
	virtual unsigned int readableBytes( void ) = 0;

	/**
	 * Reads an incoming audio buffer from the soundcard.
	 */
	virtual bool readBuffer( int bytes = 0 ) = 0;

	/**
	 * Returns the name of the current output device.  This is simply
	 * for the user to be able to identify the device, and does not
	 * need to correspond to anything real, such as the filename.
	 */
	QString getDeviceName( void ) const { return devname; }

	/**
	 * Sets the current output device name.  This is simply for the user
	 * to be able to identify the device, and does not need to correspond
	 * to anything real, such as the filename.
	 */
	void setDeviceName( const QString &newDevName );


protected:
	DeviceState devstate;  // Current state
	DeviceMode devmode;    // Current mode
	QString lasterror;     // Informative text output of last error
	QString devname;       // Device Name, (not necessarily filename!)
	AudioBuffer audio_buf; // Buffer that the application fills
	int rate;              // Sampling rate (in fps)
};

#endif  // DSPOUT_H_INCLUDED
