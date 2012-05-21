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

#ifndef SIPUTIL_H_INCLUDED
#define SIPUTIL_H_INCLUDED

/**
 * Container class for useful functions needed by the stack in general.
 * Rewritten for IPv6
 */
class SipUtil
{
public:
	SipUtil( void ) {}
	~SipUtil( void ) {}
	/**
	* get our local IP for a given interface
	*/
	static char *getLocalFqdn(void );
	/**
	* get our local IP for a given interface (V6 version)
	*/
	static char *getLocalFqdn6( void );
	/**
	* does the file exist
	*/
	static bool checkFilename( const char *filename );

	//static int checkForAddressChange (int AF );
	/**
	* did our registrar's IP change?
	*/
	static int checkForAddressChange2 (int AF, const char *registrarIP );

	static int found;
};
#endif // SIPUTIL_H_INCLUDED
