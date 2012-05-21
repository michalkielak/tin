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
#ifndef DSPOUTOSS_H_INCLUDED
#define DSPOUTOSS_H_INCLUDED

#include <sys/soundcard.h>
#include "dspout.h"

/**
 * @short OSS implementation of DspOut.
 * @author Billy Biggs <vektor@div8.net>
 *
 * This is the OSS implementation of DspOut for the /dev/dsp*
 * devices.  Note that you cannot change how many fragments
 * this class requests, yet.
 */
class DspOutOss : public DspOut
{
public:
	/**
	 * Constructs a DspOutOss object representing the given
	 * filename.  Default is /dev/dsp.
	 */
	DspOutOss( const QString &fileName = "/dev/dsp" );

	/**
	 * Destructor.  Will close the device if it is open.
	 */
	virtual ~DspOutOss( void );

	bool openDevice( DeviceMode mode );
	bool openDevice( int exist_fs );
	bool writeBuffer( void );
	unsigned int readableBytes( void );
	bool readBuffer( int bytes );
	int audio_fd;

private:
	QString filename;         // device filename
	bool audio_stereo;
};

#endif  // DSPOUTOSS_H_INCLUDED
