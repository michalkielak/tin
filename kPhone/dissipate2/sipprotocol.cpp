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
#include <stdio.h>
#include <stdlib.h>
#include <base64.h>
#include <qstringlist.h>
#include <iostream>
using namespace std;

#include "siputil.h"
#include "extern_md5.h"
#include "sipprotocol.h"


QString dissipate_ouraddress;

Sip::Sip( void )
{
}


Sip::~Sip( void )
{
}


Sip::Method Sip::matchMethod( const QString m )
{
	if ( m.compare( getMethodString( MESSAGE ) ) == 0 ) { return MESSAGE; }
	if ( m.compare( getMethodString( INVITE ) ) == 0 ) { return INVITE; }
	if ( m.compare( getMethodString( ACK ) ) == 0 ) { return ACK; }
	if ( m.compare( getMethodString( BYE ) ) == 0 ) { return BYE; }
	if ( m.compare( getMethodString( OPTIONS ) ) == 0 ) { return OPTIONS; }
	if ( m.compare( getMethodString( CANCEL ) ) == 0 ) { return CANCEL; }
	if ( m.compare( getMethodString( REGISTER ) ) == 0 ) { return REGISTER; }
	if ( m.compare( getMethodString( MSG ) ) == 0 ) { return MSG; }
	if ( m.compare( getMethodString( SUBSCRIBE ) ) == 0 ) { return SUBSCRIBE; }
	if ( m.compare( getMethodString( NOTIFY ) ) == 0 ) { return NOTIFY; }
	if ( m.compare( getMethodString( PUBLISH ) ) == 0 ) { return PUBLISH; }
	if ( m.compare( getMethodString( PRACK ) ) == 0 ) { return PRACK; }
	if ( m.compare( getMethodString( INFO ) ) == 0 ) { return INFO; }
	if ( m.compare( getMethodString( REFER ) ) == 0 ) { return REFER; }
	if ( m.compare( getMethodString( RNOTIFY ) ) == 0 ) { return NOTIFY; }
	if ( m.compare( getMethodString( UPDATE ) ) == 0 ) { return UPDATE; }
	return BadMethod;
}


const QString Sip::getMethodString( Method m )
{
	switch ( m ) {
		case MESSAGE: return "MESSAGE";
		case INVITE: return "INVITE";
		case ACK: return "ACK";
		case BYE: return "BYE";
		case OPTIONS: return "OPTIONS";
		case CANCEL: return "CANCEL";
		case REGISTER: return "REGISTER";
		case MSG: return "MSG";
		case SUBSCRIBE: return "SUBSCRIBE";
		case NOTIFY: return "NOTIFY";
		case PUBLISH: return "PUBLISH";
		case PRACK: return  "PRACK";
		case INFO: return "INFO";
		case REFER: return "REFER";
		case RNOTIFY: return "NOTIFY";
		case UPDATE: return "UPDATE";
		case BadMethod: return "BAD";
	};

	return QString::null;
}

void Sip::updateLocalAddresses( void ) 
{
	dissipate_ouraddress = QString::null;
	getLocalAddress();
}
QString Sip::getLocalAddress(void)
{

if( dissipate_ouraddress == QString::null ) dissipate_ouraddress = QString::fromUtf8( SipUtil::getLocalFqdn() );

	return dissipate_ouraddress;
}


void Sip::setLocalAddress( const QString localaddr )
{
	
	dissipate_ouraddress = localaddr;
}


int Sip::parseQop(const QString &qop)
{
	QString tmp;
	int result = NO_QOP;
	if (qop == "") return result;
	QStringList lst(QStringList::split(",", qop));
	QStringList::Iterator it = lst.begin();
	for(; it != lst.end(); ++it) {
		tmp = (*it).stripWhiteSpace();
		if ((tmp) == "auth") result |= AUTH_QOP;
		else if ((tmp) == "auth-int") result |= AUTH_INT_QOP;
//		else if (tmp.contains ("auth-int"))  result |= AUTH_INT_QOP;
		else result |= UNKNOWN_QOP;
	}

	return result;
}


QString Sip::getDigestResponse( const QString &user, const QString &password,
			const QString &method, const QString &requri, const QString &authstr,int nonceCounter )
{
	QString realm = "";
	QString nonce = "";
	QString opaque = "";
	QString algorithm = "";
	QString qop = "";
	QString digest;
	QString cnonce;
	QString noncecount;
	char nc[9];
	const char *p = authstr.latin1();
	HASHHEX HA1;
	HASHHEX HA2 = "";
	HASHHEX response;
	int i = 0;
	int qop_parsed;
	QString trequri = "";
	if (!requri.contains("sip:")) trequri = "sip:" + requri; else trequri = requri;
	
	
	while( *p ) {
		while( *p && (( *p == ' ' ) || ( *p == ',' ) || (*p == '\t') ))
			p++;
		i = 0;
		if( !*p){

/* NOP */
		}
		else if( strncasecmp( p, "realm=\"", 7 ) == 0 ) {
			p += 7;
			while( *p && p[ i ] != '"' ) {
				realm[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "nonce=\"", 7 ) == 0 ) {
			p += 7;
			while( *p && p[ i ] != '"' ) {
				nonce[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "opaque=\"", 8 ) == 0 ) {
			p += 8;
			while( *p && p[ i ] != '"' ) {
				opaque[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "algorith=\"", 10 ) == 0 ) {
			p += 10;
			while( *p && p[ i ] != '"' ) {
				algorithm[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "algorithm=\"", 11 ) == 0 ) {
			p += 11;
			while( *p && p[ i ] != '"' ) {
				algorithm[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "algorithm=", 10 ) == 0 ) {
			p += 10;
			while( *p && p[ i ] != '"' && p[ i ] != ',' &&
					p[ i ] != ' ' && p[ i ] != '\t') {
				algorithm[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "qop=\"", 5 ) == 0 ) {
			p += 5;
			while( *p && p[ i ] != '"' ) {
				qop[ i ] = p[ i ];
				i++;
			}
		} else if( strncasecmp( p, "qop=", 4 ) == 0 ) {
			p += 4;
			while( *p && p[ i ] != '"' && p[ i ] != ',' &&
					p[ i ] != ' ' && p[ i ] != '\t') {
				qop[ i ] = p[ i ];
				i++;
			}
		}
		if(*p) {
		 if( i ) p += i; else p++;
		}
	}
	digest = "Digest username=\"";
	digest += user;
	digest += "\", ";
	digest += "realm=\"";
	digest += realm;
	digest += "\", ";
	digest += "nonce=\"";
	digest += nonce;
	digest += "\", ";
	digest += "uri=\"";
	digest += trequri;
	digest += "\", ";
	cnonce = "abcdefghi";
	sprintf(nc,"%08x",nonceCounter);
	noncecount=QString(nc);
	qop_parsed = parseQop(qop);
	QString pqop = "";
	if(qop_parsed & AUTH_INT_QOP) {
	     pqop= "auth_int"; 
	     digest += "qop=auth-int, ";
	} else if(qop_parsed & AUTH_QOP) { 
	    pqop= "auth";
	    digest += "qop=auth, ";
	} else pqop = ""; 
	//printf("getDigestResponse(): qop parameter: %s\n", pqop.latin1() );
	DigestCalcHA1( "MD5", user.latin1(), realm.latin1(), password.latin1(),
		nonce.latin1(), cnonce.latin1(), HA1 );
	//printf( "WL: SipProtocol: HA1=%s user=%s realm=%s\n",HA1,user.latin1(),realm.latin1() );
	DigestCalcResponse( HA1, nonce.latin1(), noncecount.latin1(),
		cnonce.latin1(),
		pqop,
		method.latin1(), trequri.latin1(), HA2, response );
	digest += "cnonce=\"";
	digest += cnonce;
	digest += "\", ";
	digest += "nc=";
	digest += noncecount;
	digest += ", ";
	digest += "response=\"";
	digest += response;
	digest += "\"";
	if( opaque != QString::null ) {
		digest += ", opaque=\"";
		digest += opaque;
		digest += "\"";
	}
	return digest;
}


QString Sip::getBasicResponse( const QString &user, const QString &password )
{
	QString basic;
	QString userpass;
	basic = "Basic ";
	userpass += user;
	userpass += ":";
	userpass += password;
	basic += base64_encode_line( userpass.latin1() );

	return basic;
}

