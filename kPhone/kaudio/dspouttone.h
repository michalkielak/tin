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
#ifndef DSPOUTTONE_H_INCLUDED
#define DSPOUTTONE_H_INCLUDED

#include "dspout.h"
/**
*	The class generates output buffers with noice, 
*	used for early audio, when the media gateway needs a rtp steam
*/
class DspOutTone : public DspOut
{
public:
	/**
	 * Constructs a DspOutTone object representing the given
	 * filename.  Default is default.
	 */
	DspOutTone( const QString &fileName = "default" ); //Changed by bobosch

	/**
	 * Destructor.  Will close the device if it is open.
	 */
	virtual ~DspOutTone( void );
/**
*	always returns true
*/
	bool openDevice( DeviceMode mode );
/**
*	sleeps a 100 msec and returns true
*/
	bool writeBuffer( void );
/**
*	returns 64
*/
	unsigned int readableBytes( void );
/**
*	fills a buffer with incredible noice
*/
	bool readBuffer( int bytes );

private:
	int err;
	unsigned int rate;
};

#endif  // DSPOUTTONE_H_INCLUDED
