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

#include "sipuri.h"
#include "sipvia.h"
#include <stdlib.h>

SipVia::SipVia( void )
{
	clear();
}

SipVia::SipVia( const QString& parseinput )
{
	clear();
	parseVia( parseinput );
}

SipVia::~SipVia( void )
{
}

void SipVia::clear( void )
{
	setTransport( UDP );
	port = SipUri::getSipPort();
	hasport = false;
	hasreceived = false;
	hasrport = false;
	ishidden = false;
	hasttl = false;
	hasmaddr = false;
	hasbranch = false;
	isvalid = true;
	protocolname = "SIP";
	protocolver = "2.0";
}

const QString SipVia::getTransportString( Transport t )
{
	switch ( t ) {
		case UDP: return "UDP";
		case TCP: return "TCP";
		case TLS: return "BAD";
		case BadTransport: return "BAD";
	}
	return QString::null;
}

SipVia::Transport SipVia::matchTransport( const QString t )
{
	if ( t.compare( getTransportString( UDP ) ) == 0 ) { return UDP; }
	if ( t.compare( getTransportString( TCP ) ) == 0 ) { return TCP; };

	return BadTransport;
}

const QString SipVia::getViaParamString( ViaParam v )
{
	switch ( v ) {
		case Hidden: return "hidden";
		case Ttl: return "ttl";
		case Maddr: return "maddr";
		case Received: return "received";
		case Rport: return "rport";
		case Branch: return "branch";
		case BadViaParam: return "BAD";
	}
	return QString::null;
}

SipVia::ViaParam SipVia::matchViaParam( const QString v )
{
	if ( v.compare( getViaParamString( Hidden ) ) == 0 ) { return Hidden; }
	if ( v.compare( getViaParamString( Ttl ) ) == 0 ) { return Ttl; }
	if ( v.compare( getViaParamString( Maddr ) ) == 0 ) { return Maddr; }
	if ( v.compare( getViaParamString( Received ) ) == 0 ) { return Received; }
	if ( v.compare( getViaParamString( Branch ) ) == 0 ) { return Branch; }
	if ( v.compare( getViaParamString( Rport ) ) == 0 ) { return Rport; }

	return BadViaParam;
}

void SipVia::parseVia( const QString& parseinput )
{
	QString inputline;
	QString hostport;
	QString curparm;
	QString parmname;
	QString parmvalue;
	QString transport;
	SipVia::ViaParam parm;

	isvalid = false;

	inputline = parseinput.simplifyWhiteSpace();

	protocolname = inputline.left( inputline.find( "/" ) ).upper();
	inputline.remove( 0, inputline.find( "/" ) + 1 );

	protocolver = inputline.left( inputline.find( "/" ) );
	inputline.remove( 0, inputline.find( "/" ) + 1 );

	// fixed for notel
	transport = inputline.left( inputline.find( " " ) ).upper();
	inputline.remove( 0, inputline.find( " " ) + 1 );

	setTransport( matchTransport( transport ) );
	if( getTransport() == BadTransport ) { return; }

	if ( inputline.contains( ";" ) ) {
		hostport = inputline.left( inputline.find( ";" ) );
		inputline.remove( 0, inputline.find( ";" ) );
	} else if ( inputline.contains( " " ) ) {
		hostport = inputline.left( inputline.find( " " ) );
		inputline.remove( 0, inputline.find( " " ) + 1 );
	} else {
		hostport = inputline;
		inputline = QString::null;
	}
	// if hostport contains more than 1 colon, there is an IPv6 address involved
	if ( hostport.contains(':') >= 2 ) { // So it is an IPv6 address involved
		QString ipv6addr;
		QString ipv6port;
		// According to RFC3261 (SIP) there MUST be brackets around the IPv6 address here.
		if ( ! hostport.contains('[') ) { return; }
		ipv6addr=hostport.mid(hostport.find('['));
		if ( ! ipv6addr.contains(']') ) { return; }  // oops, broken IPv6 reference
		ipv6port=ipv6addr.mid(ipv6addr.find(']')+1);
		ipv6addr=ipv6addr.left(ipv6addr.find(']')+1);
		if ( ! ipv6port.contains(':') ) { // no colon behind ']', so no port specified
			setPortNumber( 5060 );
			hasport = true;
		} else {
			setPortNumber( ipv6port.mid( ipv6port.find(':')+1 ).toUInt() );
			hasport = true;
		}
		setHostname(ipv6addr);
	} else { // Check for hostname:port or IP:port
		if ( hostport.contains( ":" ) ) {
			setHostname( hostport.left( hostport.find( ":" ) ) );
			setPortNumber( hostport.mid( hostport.find( ":" ) + 1 ).toUInt() );
			hasport = true;
		} else {
			setHostname( hostport );
			setPortNumber( SipUri::getSipPort() );
			hasport = true;

		}
	}

	while ( inputline[ 0 ] == ';' ) {
		curparm = inputline.mid( 1, inputline.find( ";", 1 ) - 1 ).stripWhiteSpace();
		inputline.remove( 0, inputline.find( ";", 1 ) );

		if( curparm.contains( '=' ) ) {
			parmname = curparm.left( curparm.find( '=' ) ).stripWhiteSpace().lower();
			parmvalue = curparm.mid( curparm.find( '=' ) + 1 ).stripWhiteSpace();
		} else {
			parmname = curparm;
			parmvalue = QString::null;
		}

		parm = matchViaParam( parmname );

		if( parm == Received ) {
			hasreceived = true;
			received = parmvalue;
		} else if( parm == Rport ) {
			hasrport = true;
			rport = parmvalue;
		} else if( parm == Hidden ) {
			ishidden = true;
		} else if( parm == Ttl ) {
			hasttl = true;
			ttl = parmvalue;
		} else if( parm == Maddr ) {
			hasmaddr = true;
			maddr = parmvalue;
		} else if( parm == Branch ) {
			hasbranch = true;
			branch = parmvalue;

//if different of the above , save not ignore it
		} else {
			restOfParams.append( SimpleParameter(parmname,parmvalue));

		}
	}

	isvalid = true;
}

void SipVia::setHostname( const QString &hname )
{
	if( (hname.contains(':') >= 2) && (hname.contains('[') < 1) ) {
		hostname = "[" + hname + "]";
	} else {
	    hostname = hname;
	}
}

void SipVia::setPortNumber( unsigned int p )
{
	port = p;
	hasport=true;
}

QString SipVia::via( void )
{
	QString viatext;
	viatext += protocolname;
	viatext += "/";
	viatext += protocolver;
	viatext += "/";
	viatext += getTransportString( trans );
	viatext += " ";
	viatext += hostname;
if( (hostname.contains(':') >=2) && (hostname.contains('[') < 1) ) {
	hostname = "[" + hostname + "]";
	}
	if( hasport) { // || port != SipUri::getSipPort() ) {
		viatext += ":";
		viatext += QString::number( port );
	}

	if( hasreceived ) {
		viatext += ";received=";
		viatext += received;
	}

	if( hasrport ) {
		viatext += ";rport";
		if (rport != QString::null) {
			viatext += "=";
			viatext += rport;
		}
	}

	if( ishidden ) {
		viatext += ";hidden";
	}

	if( hasttl ) {
		viatext += ";ttl=";
		viatext += ttl;
	}

	if( hasmaddr ) {
		viatext += ";maddr=";
		viatext += maddr;
	}

	if( hasbranch ) {
		viatext += ";branch=";
		viatext += branch;
	}

	//add the rest of the params
	ParameterListIterator it;
	for(it = restOfParams.begin();it != restOfParams.end();++it) {
		viatext +=";";
		viatext +=(*it).getName();
		if( !(*it).getValue().isEmpty() ) {
			viatext +="=";
			viatext +=(*it).getValue();
		}
	}

	return viatext;
}

void SipVia::setTransport( Transport t )
{
	trans = t;
}

void SipVia::setReceivedParam( const QString &newreceived )
{
	if( newreceived == QString::null ) {
		hasreceived = false;
	} else {
		hasreceived = true;
		received = newreceived.lower();
	}
}

void SipVia::setRportParam( const QString &newport )
{
	rport = newport.lower();
//	if (rport != QString::null) {
		hasrport = true;
//	} else {
//		hasrport = false;
//	}
}

void SipVia::setHidden( bool hidden )
{
	ishidden = hidden;
}

void SipVia::setTtlParam( const QString &newttl )
{
	if( newttl == QString::null ) {
		hasttl = false;
	} else {
		hasttl = true;
		ttl = newttl;
	}
}

void SipVia::setMaddrParam( const QString &newmaddr )
{
	if( newmaddr == QString::null ) {
		hasmaddr = false;
	} else {
		hasmaddr = true;
		maddr = newmaddr;
	}
}

void SipVia::setBranchParam( const QString &newbranch )
{
	if( newbranch == QString::null ) {
		hasbranch = false;
	} else {
		hasbranch = true;
		branch = newbranch;
	}
}

void SipVia::generateBranchParam( void )
{
	int uniqid = rand();
	branch.sprintf( "z9hG4bK%X", uniqid );
	hasbranch = true;
}

SipVia &SipVia::operator=( const SipVia &v )
{
	setHostname( v.getHostname() );
	setTransport( v.getTransport() );
	setPortNumber( v.getPortNumber() );
	setReceivedParam( v.getReceivedParam() );
	setRportParam( v.getRportParam() );
	setHidden( v.isHidden() );
	setTtlParam( v.getTtlParam() );
	setMaddrParam( v.getMaddrParam() );
	setBranchParam( v.getBranchParam() );

	//copy the rest of params
	restOfParams = v.restOfParams;

	return *this;
}

bool SipVia::operator==( const SipVia &v ) const
{
	if( v.getHostname() == getHostname() && v.getTransport() == getTransport() &&
	    v.getPortNumber() == getPortNumber() && v.getReceivedParam() == getReceivedParam() &&
	    v.isHidden() == isHidden() && v.getTtlParam() == getTtlParam() &&
	    v.getMaddrParam() == getMaddrParam() && v.getBranchParam() == getBranchParam() &&
	    v.getRportParam() == getRportParam() ) {
		return true;
	}

	return false;
}

bool SipVia::operator!=( const SipVia &v ) const
{
	return !( *this == v );
}

bool SipVia::operator==( const QString &v ) const
{
	SipVia via( v );
	return ( *this == v );
}

bool SipVia::operator!=( const QString &v ) const
{
	return !( *this == v );
}

