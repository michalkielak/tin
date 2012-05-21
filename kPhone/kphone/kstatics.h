/*
 * Copyright (c) 2000 Billy Biggs <bbiggs@div8.net>
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
//
// C++ Interface: kstatics
//
// Description: 
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KSTATICS_H
#define KSTATICS_H
#include <qdialog.h>
/**
* @short static variables 
* @author kfl
*
* static variables used by KPhone
*/
class KStatics{
public:
    KStatics();

    ~KStatics();
/**
*	the debuglevel: 1: short, 2: contact tracer , 3: long
*/
static int debugLevel;
/**
*	to supervise the network's responsiveness
*/
static int noLine;
/**
*	the subscr.db file
*/
static QString dBase;
/**
*	the subscr.db file
*/
static QString xBase;
/**
*	the call log file
*/
static QString cBase;
/**
*	the call log file
*/
static QString cxBase;
/**
*	the IP I'm listening on
*/
static QString myIP;
/**
*	my auth password
*/
static QString myPW;
/**
*	my auth. user id
*/
static QString myID;
/**
*	the MinExpires Header	
*/
static QString minExT;
/**
*	the entry in the kpsirc database
*/
static QString myInd;
/**
*	the version string	
*/
static QString me;
/**
*	hold the serviceRoute header	
*/
static QString serviceRoute;
/**
*	we are registered
*/
static bool isReg;
/**
*	we are taking to anouther UA directrly
*/
static bool isP2P;
/**
*	We use IPv6
*/
static bool haveIPv6;
};
#endif

