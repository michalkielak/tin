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

#ifndef SIPUSER_H_INCLUDED
#define SIPUSER_H_INCLUDED

#include <qstring.h>
#include <qptrlist.h>

#include "sipuri.h"
#include "sipregister.h"
#include "../kphone/sessioncontrol.h"

class SipClient;
class SipCall;

/**
 * 
 * @shortClass for describing a user of a SipClient.
 * Class for describing a user of a SipClient.
 *Creates a new SipUser associated with the given parent SipClient.
 */
class SipUser
{
public:
	/**
	 * Creates a new SipUser associated with the given parent @ref SipClient.
	 * Constructs the URI using the given fullname username, and hostname.
	 */
	SipUser( SipClient *parent, SessionControl *sc,  QString fullname, QString username,
		QString athostname );

	/**
	 * Creates a new SipUser associated with the given parent @ref SipClient for
	 * the given URI.
	 */
	SipUser( SipClient *parent, SessionControl *sc, const SipUri &inituri );

	/**
	 * SipUser destructor.  Deletes this user from the parent SipClient.
	 * All active calls made using this user will remain active.
	 */
	~SipUser( void );

	/**
	 * Creates a new SIP register server object given the hostname and port
	 * and returns a pointer to the register object.  The register server
	 * object should be removed from this SipUser by calling
	 * @ref removeServer() before deleting.
	 */
	void addServer( SipRegister *server );

	/**
	 * Removes the given server from the list of register servers.
	 *
	 * @see addServer()
	 */
	void removeServer( SipRegister *server );


	/**
	 * Sets the URI this user will be known as.  Any modifications to the
	 * URI will not affect existing calls, but will be used for new
	 * incoming calls or locally created calls.
	 */
	void setUri( const SipUri &newuri );

	/**
	 * Returns the URI this user is known as.
	 */
	const SipUri &getUri( void ) { return myuri; }

	SipUri *getMyUri( void ) { return &myuri; }

	/**
	 * Returns the SipClient parent of this user.
	 */
	SipClient *parent( void ) { return client; }

	/**
	 * Returns the SessionControl instance for  this user. 
	 */
	SessionControl *getSC( void ) { return sessionC; }



	/**
	 * Returns an iterator for the list of registration servers for this
	 * user.
	 */
	SipRegisterIterator getSipRegisterList( void ) const { return SipRegisterIterator( servers ); }

private:
	SipClient *client;
	SessionControl *sessionC;
	SipUri myuri;
	QPtrList<SipRegister> servers;
};

typedef QListIterator<SipUser> SipUserIterator;

#endif // SIPUSER_H_INCLUDED
