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

#include "sipurilist.h"

const SipUriList SipUriList::null;

/**
* @short the list of URIs 
* the list of URIs
*/
SipUriList::SipUriList( void )
{
}

SipUriList::SipUriList( const QString &parseinput )
{
	parseList( parseinput );
}

SipUriList::~SipUriList( void )
{
}

QString SipUriList::getUriList( void )
{
	QValueList<SipUri>::Iterator it;
	QString list;
	bool first = true;

	for( it = urilist.begin(); it != urilist.end(); ++it ) {
		if( !first ) {
			list += ", ";
		}

		list += (*it).nameAddr();
		first = false;
	}
	return list;
}

QString SipUriList::getReverseOrderList( void )
{
	QValueList<SipUri>::Iterator it;
	QString list;
	bool first = true;

	for( it = urilist.end(); it != urilist.begin(); --it ) {
		if( !first ) {
			list += ", ";
		}

		list += (*it).nameAddr();
		first = false;
	}
	return list;
}

void SipUriList::parseList( const QString &input )
{
	QString inputline;
	QString cururi;
	unsigned int i = 0;

	inputline = input.simplifyWhiteSpace();

	while( i < inputline.length() ) {

		if( inputline[ i ] == QChar::null ) {
			break;
		} else if( inputline[ i ] == ',' ) {
			cururi = inputline.left( i );
			urilist.append( SipUri( cururi ) );
			inputline.remove( 0, i + 1 );
			inputline.simplifyWhiteSpace();
			i = 0;
		} else if( inputline[ i ] == '<' ) {
			while( i < inputline.length() && inputline[ i ] != '>' ) i++;
		} else if( inputline[ i ] == '\"' ) {
			i++;
			while( i < inputline.length() && inputline[ i ] != '\"' ) i++;
			i++;
		} else {
			i++;
		}
	}

	if( i > 0 ) urilist.append( SipUri( inputline ) );
}

void SipUriList::addToHead( const SipUri &uri )
{
	urilist.prepend( uri );
}

void SipUriList::addToEnd( const SipUri &uri )
{
	urilist.append( uri );
}

void SipUriList::removeHead( void )
{
	urilist.remove( urilist.begin() );
}

SipUri SipUriList::getHead( void ) const
{
	return urilist.first();
}

SipUriList &SipUriList::operator=( const SipUriList &u )
{
	urilist = u.urilist;
	return *this;
}

SipUriList &SipUriList::reverseList( void )
{
	QValueList<SipUri>::Iterator it;
	QValueList<SipUri> templist;

	for( it = urilist.begin(); it != urilist.end(); ++it ) {
		templist.prepend( (*it) );
	}

	urilist = templist;
	return *this;
}


SipUri SipUriList::getPriorContact( void )
{
	QValueList<SipUri>::Iterator it;
	SipUri uri = urilist.first();
	int q = 0;
	for( it = urilist.begin(); it != urilist.end(); ++it ) {
		SipUri contact = SipUri(*it);
		if( q < contact.getPrior().toInt() ) {
			q = contact.getPrior().toInt();
			uri = contact;
		}
	}
	removeContact( uri );
	
	return uri;
}

void SipUriList::removeContact( SipUri uri )
{
	QValueList<SipUri>::Iterator it;
	for( it = urilist.begin(); it != urilist.end(); ++it ) {
		if( uri == SipUri(*it) ) {
			urilist.remove( it );
			break;
		}
	}
}
