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
#ifndef DSPOUTALSA_H_INCLUDED
#define DSPOUTALSA_H_INCLUDED

#include <alsa/asoundlib.h>
#include "dspout.h"
/**
* @short ALSA implementation of DspOut.
*
* DspOutAlsa, interaction between the ALSA sound system and KPhone
* derived from DspOut
*/
class DspOutAlsa : public DspOut
{
public:
	/**
	 * Constructs a DspOutAlsa object representing the given
	 * filename.  Default is default.
	 */
	DspOutAlsa( const QString &fileName = "default" ); //Changed by bobosch

	/**
	 * Destructor.  Will close the device if it is open.
	 */
	virtual ~DspOutAlsa( void );

/**
*	opens and initialises the ALSA soundsystem for us
	returns false if something goes wrong
*/
	bool openDevice( DeviceMode mode );
/**
*	Send the current audio buffer (available using @ref #getBuffer () )
*	to the soundcard
*/
	bool writeBuffer( void );
/**
*	Returns the number of currently readable bytes.
*/
	unsigned int readableBytes( void );
/**
*	Reads an incoming audio buffer from the soundcard.
*/
	bool readBuffer( int bytes );

	int audio_fd;

private:
	int err;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	snd_pcm_uframes_t buffer_size;
	QString devname;         // device filename
	unsigned int rate;
	sigset_t set;  
	int debug;

/**
*	drops the buffer and reinitialises ALSA for further use
*/
	bool alsaReset(snd_pcm_t *handle);
};

#endif  // DSPOUTALSA_H_INCLUDED
