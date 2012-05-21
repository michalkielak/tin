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
#include "mimecontenttype.h"

const MimeContentType MimeContentType::null;

MimeContentType::MimeContentType( void )
{
}

MimeContentType::MimeContentType( const QString& initialtype, const QString& initialsubtype )
{	
	m_type = initialtype;
	m_subtype = initialsubtype;
}

MimeContentType::MimeContentType( const QString& parseinput )
{	
	parseContentType( parseinput );
}

MimeContentType::~MimeContentType( void )
{
}

void MimeContentType::setType( QString newtype )
{
	m_type = newtype.lower();
}

void MimeContentType::setSubType( QString newsubtype )
{
	m_subtype = newsubtype.lower();
}

QString MimeContentType::type( void ) const
{
	QString ctype;
	ParameterListConstIterator it;

	ctype = getType() + "/" + getSubType();

	for( it = parameters.begin(); it != parameters.end(); ++it ) {
		ctype += ";" + (*it).getName() + "=" + (*it).getValue();
	}

	return ctype;
}

void MimeContentType::setParameter( const QString &param, const QString& value )
{
	ParameterListIterator it;

	for( it = parameters.begin(); it != parameters.end(); ++it ) {
		if( param == (*it).getName() ) {
			(*it).setValue( value );
			return;
		}
	}

	parameters.append( SimpleParameter( param, value ) );
}

QString MimeContentType::queryParameter( const QString &param )
{
	ParameterListIterator it;

	for( it = parameters.begin(); it != parameters.end(); ++it ) {
		if( param == (*it).getName() ) {
			return (*it).getValue();
		}
	}

	return QString::null;
}

void MimeContentType::parseContentType( const QString& parseinput )
{
	QString inputline;
	QString curparam;
	QString attr;
	QString val;

	inputline = parseinput.simplifyWhiteSpace();

	setType( inputline.left( inputline.find( "/" ) ).stripWhiteSpace().lower() );
	inputline.remove( 0, inputline.find( "/" ) + 1 );

	if ( inputline.contains( ";" ) ) {
		setSubType( inputline.left( inputline.find( ";" ) ).stripWhiteSpace().lower() );
		inputline.remove( 0, inputline.find( ";" ) );
	} else {
		setSubType( inputline.stripWhiteSpace() );
		inputline = QString::null;
	}

	while ( inputline[ 0 ] == ';' ) {
		curparam = inputline.mid( 1, inputline.find( ";", 1 ) - 1 ).stripWhiteSpace();
		inputline.remove( 0, inputline.find( ";", 1 ) );

		if ( curparam.contains( "=" ) ) {
			attr = curparam.left( curparam.find( "=" ) ).stripWhiteSpace();
			val = curparam.mid( curparam.find( "=" ) + 1 ).stripWhiteSpace();

			parameters.append( SimpleParameter( attr, val ) );
		} else {
			parameters.append( SimpleParameter( curparam.simplifyWhiteSpace(), QString::null ) );
		}
	}
}

MimeContentType &MimeContentType::operator=( const MimeContentType &t )
{
	setType( t.getType() );
	setSubType( t.getSubType() );
	parameters = t.parameters;
	return *this;
}


bool MimeContentType::operator==( const MimeContentType &t ) const
{
	if( t.getType() == getType() && t.getSubType() == getSubType() ) {
		return true;
	}

	return false;
}

bool MimeContentType::operator==( const QString &t ) const
{
	MimeContentType mt( t );
	return ( *this == t );
}

bool MimeContentType::operator!=( const MimeContentType &t ) const
{
	return !( *this == t );
}

bool MimeContentType::operator!=( const QString &t ) const
{
	return !( *this == t );
}

