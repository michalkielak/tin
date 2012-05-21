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

#include "sipheader.h"
#include "sipvialist.h"

SipViaList::SipViaList( void )
{
}

SipViaList::~SipViaList( void )
{
}

QString SipViaList::getViaList( void )
{
	QValueList<SipVia>::Iterator it;
	QString list;

	for( it = vialist.begin(); it != vialist.end(); ++it ) {
		list += SipHeader::getHeaderString( SipHeader::Via ) + ": " + (*it).via() + "\r\n";
	}

	return list;
}

void SipViaList::insertTopmostVia( const SipVia &newtop )
{
	vialist.prepend( newtop );
}

void SipViaList::parseVia( const QString &via )
{
	QString inputline;
	QString curvia;

	inputline = via.simplifyWhiteSpace();

	while( inputline != QString::null ) {
		if( inputline.contains( ',' ) ) {
			curvia = inputline.left( inputline.find( ',' ) );
			vialist.append( SipVia( curvia ) );
			inputline.remove( 0, inputline.find( ',' ) + 1 );
			inputline.simplifyWhiteSpace();
		} else {
			vialist.append( SipVia( inputline ) );
			inputline = QString::null;
		}
	}
}

const SipVia &SipViaList::getTopmostVia( void )
{
	return vialist[ 0 ];
}

const SipVia &SipViaList::getBottommostVia( void )
{
	return vialist.last();
}

bool SipViaList::isValid( void )
{
	QValueList<SipVia>::Iterator it;
	for( it = vialist.begin(); it != vialist.end(); ++it ) {
		if( !(*it).isValid() ) {
			return false;
		}
	}
	return true;
}

SipViaList &SipViaList::operator=( const SipViaList &v )
{
	vialist = v.vialist;
	return *this;
}

