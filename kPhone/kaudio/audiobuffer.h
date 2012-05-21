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
#ifndef AUDIOBUFFER_H_INCLUDED
#define AUDIOBUFFER_H_INCLUDED

#include <stdlib.h>

/**
* @short an audio buffer
 *
 * Small class for passing around buffers of audio data. Does not
 * specify a format, so the responsibility is on the programmer to
 * know what the datatype of the buffer really is.
 */
class AudioBuffer
{
public:
	/**
	 * Creates an audio buffer of @param length bytes.
	 */
	AudioBuffer( size_t length = 4096 );

	/**
	 * Deletes the audio buffer, freeing the data.
	 */
	~AudioBuffer( void );

	/**
	 * Returns a pointer to the audio data.
	 */
	void *getData( void ) const { return data; }

	/**
	 * Returns the size of the buffer.
	 */
	size_t getSize( void ) const { return size; }

	/**
	 * Resizes the buffer to size newlength. Will only allocate new memory
	 * if the size is larger than what has been previously allocated.
	 */
	void resize( size_t newsize );

private:
	void *data;
	size_t size;
	size_t realsize;
};

#endif // AUDIOBUFFER_H_INCLUDED
