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

#ifndef MIMECONTENTTYPE_H_INCLUDED
#define MIMECONTENTTYPE_H_INCLUDED

#include <qstring.h>
#include "parameterlist.h"

/**
*@short Set and retrieve the Mime Type of a SIP-Message
*/
class MimeContentType
{
public:
/**
*	Constructor
*/
	MimeContentType( void );
/**
*	Constructor enter mimetype
*/
	MimeContentType( const QString &initialtype, const QString &initialsubtype );
/**
*	Constructor parses Mimetype given as string
*/
	MimeContentType( const QString &parseinput );
/**
*
*/
	~MimeContentType( void );
/**
*	get the mime type
*/
	QString getType( void ) const { return m_type; }
/**
*	get the mime sub type
*/
	QString getSubType( void ) const { return m_subtype; }
/**
*	set the mime type
*/
	void setType( QString newtype );
/**
*	set the mime type
*/
	void setSubType( QString newsubtype );
/**
*	set a parameter
*/
	void setParameter( const QString &param, const QString &value );
/**
*	get a parameter
*/
	QString queryParameter( const QString &param );
/**
*	parse a strinn, format mime header
*/
	void parseContentType( const QString &param );
/**
*	-
*/
	QString type( void ) const;
/**
*	null m t 
*/
	static const MimeContentType null;
/**
*	set operator
*/
	MimeContentType &operator=( const MimeContentType &t );
/**
*	euql operator
*/
	bool operator==( const MimeContentType &t ) const;
/**
*	not equal operator
*/
	bool operator!=( const MimeContentType &t ) const;
/**
*	equal operator with parsing
*/
	bool operator==( const QString &t ) const;
/**
*	not equal operator with parsing
*/	
	bool operator!=( const QString &t ) const;

private:
	QString m_type;
	QString m_subtype;
	ParameterList parameters;
};

#endif // MIMECONTENTTYPE_H_INCLUDED
