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

#ifndef PARAMETERLIST_H_INCLUDED
#define PARAMETERLIST_H_INCLUDED

#include <qvaluelist.h>
/**
*
*	Container for the  parameters of SIP headers
* 	They have the form (name,value)
*/
class SimpleParameter
{
public:
/**
*	Constructor
*/
	SimpleParameter( void );
/**
*	Constructor enters another class
*/
	SimpleParameter( const SimpleParameter &p );
/**
*	Constructor, enters name,value couple
*/
	SimpleParameter( QString name, QString value );
/**
*	Destructor
*/
	~SimpleParameter( void );
/**
*	get the name of a parameter
*/
	QString getName( void ) const { return nam; }
/**
*	get the value of a parameter
*/
	QString getValue( void ) const { return val; }
/**
*	set the name of a parameter
*/
	void setName( const QString &newname );
/**
*	set the value of a parameter
*/
	void setValue( const QString &newval );

/**
*	set operator
*/
	SimpleParameter &operator=( const SimpleParameter &p );
private:
	QString nam;
	QString val;
};

typedef QValueList<SimpleParameter> ParameterList;
typedef ParameterList::Iterator ParameterListIterator;
typedef ParameterList::ConstIterator ParameterListConstIterator;

#endif // PARAMETERLIST_H_INCLUDED
