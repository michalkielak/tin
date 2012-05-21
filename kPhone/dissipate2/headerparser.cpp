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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>

#include "headerparser.h"

HeaderParser::HeaderParser( void )
{
}

HeaderParser::~HeaderParser( void )
{
}

//Papa
bool HeaderParser::parse( const QString &buf, size_t bufEnd, int *headerend, int *bodystart, int *bodylength )
{
	size_t pos   = 0;
	size_t start = 0;
	size_t len   = 0;

	// Get field name
	while( pos < bufEnd ) {
		if ( buf[ pos ] == ':' ) {
			break;
		}
		pos++;
	}
	len = pos;

	// Remove any white space at end of field-name
	while( len > 0 ) {
		if( !isspace( buf[ len - 1 ] ) ) break;
		len--;
	}

	*headerend = len;

	if( pos < bufEnd && buf[ pos ] == ':') {
		pos++;
	}

	// Skip spaces and tabs (but not newline!)
	while( pos < bufEnd ) {
		if( buf[ pos ] != ' ' && buf[ pos ] != '\t' ) break;
		pos++;
	}

	start = pos;
	len = 0;
	// Get field body
	while( pos < bufEnd ) {
		if( buf[ pos ] == '\n' ) {
			// Are we at the end of the string?
			if( pos == bufEnd - 1 ) {
				pos++;
				break;
			}

			// Is this really the end of the field body, and not just
			// the end of a wrapped line?
			else if( buf[ pos + 1 ] != ' ' && buf[ pos + 1 ] != '\t' ) {
				pos++;
				break;
			}
		}
		pos++;
	}

	// Remove white space at end of field-body
	while( pos > start ) {
		if( !isspace( buf[ pos - 1 ] ) ) break;
			pos--;
	}
	len = pos - start;
	*bodystart = start;
	*bodylength = len;
	return true;
}

