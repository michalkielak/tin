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

#include <stdlib.h>
#include <qregexp.h>

#include "sipuri.h"

const SipUri SipUri::null;

SipUri::SipUri( void )
{
clear();
}

SipUri::SipUri( const QString &parseinput )
{
	clear();
	parseUri( parseinput );
}

SipUri::~SipUri( void )
{
}

void SipUri::clear( void )
{
	protocolname = "sip";
	isvalid = true;
	islooserouting = false;
	haslr = false;
	hasftag = false;
	hasuserinfo = false;
	hasproxyusername = false;
	haspassword = false;
	port = getSipPort();
	hastransparam = false;
	hasuserparam = false;
	hasmethodparam = false;
	hasttlparam = false;
	ttl = 0;
	hasmaddrparam = false;
	hastag = false;
	hasport = false;
	hasq = false;
}
void SipUri::parseUri( const QString &parseinput )
{
	QString inputline;
	QString userinfo;
	QString hostport;
	QString parameters;

	isvalid = false;

	inputline = parseinput.simplifyWhiteSpace();

	// Squeeze out blah<uri>blah
	if( inputline.contains( '<' ) ) {

		// Validity check (possibly bogus)
		if( !inputline.contains( '>' ) ) {
			return;
		}

		// Parse out the squeezed uri
		QString beforeangle = inputline.left( inputline.find( '>' ) );
		parseUri( beforeangle.mid( beforeangle.find( '<' ) + 1 ) );

		// Parse the fullname, if any
		setFullname( inputline.left( inputline.find( '<' ) ) );

		// Parse the parameters, if any
		parseParameters( inputline.mid( inputline.find( '>' ) + 1 ), false );

		return;
	}
	if( inputline.left( 4 ).lower() == "sip:" ) {
		setProtocolName( "sip" );
		inputline.remove( 0, 4 );
	}
	if( inputline.left( 4 ).lower() == "tel:" ) {
		setProtocolName( "tel" );
		inputline.remove( 0, 4 );
	}

	// If we have user info
	if( inputline.contains( '@' ) ) {
		userinfo = inputline.left( inputline.find( '@' ) );
		if( userinfo.contains( ':' ) ) {
			setUsername( userinfo.left( userinfo.find( ':' ) ) );
			setPassword( userinfo.mid( userinfo.find( ':' ) + 1 ) );
		} else {
			setUsername( userinfo );
		}
		inputline.remove( 0, inputline.find( '@' ) + 1 );
	}

	// If we have any parameters
	if( inputline.contains( ';' ) ) {
		hostport = inputline.left( inputline.find( ';' ) ).stripWhiteSpace();
		inputline.remove( 0, inputline.find( ';' ) );
	} else if( inputline.contains( '?' ) ) {
		hostport = inputline.left( inputline.find( '?' ) ).stripWhiteSpace();
		inputline.remove( 0, inputline.find( '?' ) );
	} else {
		hostport = inputline;
		inputline = QString::null;
	}

// if hostport contains more than 1 colon, there is an IPv6 address involved
	if ( hostport.contains(':') >= 2 ) { // So it is an IPv6 address involved
		QString ipv6addr;
		QString ipv6port;
		// According to RFC3261 (SIP) there MUST be brackets around the IPv6 address here.
		if ( ! hostport.contains("[") ) { return; }
		if ( ! hostport.contains("]") ) { return; }  // oops, broken IPv6 reference

		ipv6addr=hostport.mid(hostport.find("["),hostport.find("]")+1);
		ipv6port=hostport.mid(hostport.find("]")+1);
		if ( ! ipv6port.contains(':') ) { // no colon behind "]", so no port specified
			setPortNumber( getSipPort() );
		} else {
			setPortNumber( ipv6port.mid( ipv6port.find(':')+1 ).toUInt() );
			hasport=true;
		}
		setHostname(ipv6addr);
	} else { // Check for hostname:port or IP:port
		if( hostport.contains( ':' ) ) {
			setHostname( hostport.left( hostport.find( ':' ) ) );
			setPortNumber( hostport.mid( hostport.find( ':' ) + 1 ).toUInt() );
			hasport = true;
		} else {
			setHostname( hostport );
			setPortNumber( getSipPort() );
		}

	}
	if( inputline.contains( '?' ) ) {
		parameters = inputline.left( inputline.find( '?' ) );
		inputline.remove( 0, inputline.find( '?' ) + 1 );
	} else {
		parameters = inputline;
		inputline = QString::null;
	}

	// Parse the parameters at the end
	parseParameters( parameters, true );

	// If we make it this far
	isvalid = true;
}

void SipUri::parseParameters( const QString &parseinput, bool beforeangle )
{
	QString curparm;
	QString parmname;
	QString parmvalue;
	QString parameters;

	// NOTE: Calling this function will never change validity of the URI

	parameters = parseinput.simplifyWhiteSpace();

	while( parameters[ 0 ] == ';' ) {
		curparm = parameters.mid( 1, parameters.find( ';', 1 ) - 1 ).stripWhiteSpace();
		parameters.remove( 0, parameters.find( ';', 1 ) );
		parmname = curparm.left( curparm.find( '=' ) ).lower().stripWhiteSpace();
		if( curparm.contains( '=' ) ) {
			parmvalue = curparm.mid( curparm.find( '=' ) + 1 ).stripWhiteSpace();
		} else {
			parmvalue = QString::null;
		}

		// Check if the URI has a tag
		if( parmname.compare( "tag" ) == 0 ) {
			hastag = true;
			tag = parmvalue;
		}

		// maddr param
		else if( parmname.compare( "maddr" ) == 0 ) {
			hasmaddrparam = true;
			maddrhostname = parmvalue.lower();
		}

		// ttl param
		else if( parmname.compare( "ttl" ) == 0 ) {
			hasttlparam = true;
			ttl = parmvalue.toUInt();
		}

		// transport param
		else if( parmname.compare( "transport" ) == 0 ) {
			if( parmvalue.lower() == "udp" ) {
				transparam = UDP;
				hastransparam = true;
			} else if( parmvalue.lower() == "tcp" ) {
				transparam = TCP;
				hastransparam = true;
			}
		}

		// Loose Routing param
		else if( parmname.compare( "lr" ) == 0 ) {
			islooserouting = true;
			lr = parmvalue;
			haslr = !lr.isEmpty();
		}

		// ftag
		else if( parmname.compare( "ftag" ) == 0 ) {
			hasftag = true;
			ftag = parmvalue;
		}

		// q
		else if( parmname.compare( "q" ) == 0 ) {
			hasq = true;
			qValue = parmvalue;
		}

		//save the rest of params.
		else if( beforeangle ) {
			rOfParamsUri.append( SimpleParameter(parmname,parmvalue));
		} else {
			rOfParams.append( SimpleParameter(parmname,parmvalue));
		}
	}
}

void SipUri::setHostname( const QString &hname )
{
// set brackets if necessary
	if ( hname.contains(':') >=2 ) {
	
	    if(!hname.contains("[") ) { // encode IPv6 reference
		hostname = "[" ;
	    }
	    hostname += hname.lower().stripWhiteSpace();
	    if(!hname.contains("]") ) { // encode IPv6 reference
		hostname += "]"  ;
	    }
	} else {
	    hostname = hname.lower().stripWhiteSpace();
	}
}

const QString SipUri::getUserParamString( UserParam u )
{
	switch ( u ) {
		case Phone: return "phone";
		case IP: return "ip";
		case NoUserParam: return "BAD";
	};
	return QString::null;
}

const QString SipUri::getTransportParamString( TransportParam t )
{
	switch ( t ) {
		case UDP: return "udp";
		case TCP: return "tcp";
		case NoTransportParam: return "BAD"; // Should return udp
	};
	return QString::null;
}

QString SipUri::user( void ) const
{
	QString usertext;
	if ( hasUserInfo() ) {
		usertext += username + "@";
		if ( hasPassword() ) {
			usertext += password;
		}
	}
	usertext += hostname;
	return usertext;
}

QString SipUri::nameAddr( void ) const
{
	QString nameaddr;
	if( fullname.length() > 0 ) {
		nameaddr = "\"" + fullname + "\" ";
	}
	nameaddr += "<" + uri() + ">";
	if( hasTag() ) {
		nameaddr += ";tag=" + getTag();
	}
	ParameterListConstIterator it;
	for(it=rOfParams.begin();it != rOfParams.end();++it) {
		nameaddr += ";";
		nameaddr += (*it).getName();
		nameaddr += "=";
		nameaddr += (*it).getValue();
	}
	return nameaddr;
}

QString SipUri::nameAddr_noTag( void ) const
{
	QString nameaddr;

	if( fullname.length() > 0 ) {
		nameaddr = "\"" + fullname + "\" ";
	}
	nameaddr += "<" + uri() + ">";
	ParameterListConstIterator it;
	for(it=rOfParams.begin();it != rOfParams.end();++it) {
		nameaddr += ";";
		nameaddr += (*it).getName();
		nameaddr += "=";
		nameaddr += (*it).getValue();
	}
	return nameaddr;
}

QString SipUri::proxyUri( void ) const
{
	QString uritext;
// set brackets if necessary
	if ( hostname.contains(':') >=2 ) {
	    if(!hostname.contains("[") ) { // encode IPv6 reference
		uritext += "[" ;
	    }
	    uritext+= hostname;
	    if(!hostname.contains("]") ) { // encode IPv6 reference
		uritext += "]"  ;
	    }
	} else {
	    uritext = hostname;
	}
	if( hasport || port != getSipPort() ) {
		uritext += ":" + QString::number( port );
	}

	return uritext;
}

QString SipUri::uri( void ) const
{
	QString uritext;

// set brackets if necessary

    if( hasUserInfo() ) {
		uritext = username;
		uritext += "@";
	}
	if ( hostname.contains(':') >=2 ) {
	    if(!hostname.contains("[") ) { // encode IPv6 reference
		uritext += "[" ;
	    }
	    uritext+= hostname;
	    if(!hostname.contains("]") ) { // encode IPv6 reference
		uritext += "]"  ;
	    }
	} else {
		uritext += hostname;
	}

	if( hasport || port != getSipPort() ) {
		uritext += ":" + QString::number( port );
	}
	if( hasTransportParam() ) {
		uritext += ";transport=" + getTransportParamString( transparam );
	}
	if( hasUserParam() ) {
		uritext += ";user=" + getUserParamString( userparam );
	}
	if( hasMethodParam() ) {
		uritext += ";method=" + Sip::getMethodString( meth );
	}
	if( hasTtlParam() ) {
		uritext += ";ttl=" + QString::number( (int) ttl );
	}
	if( hasMaddrParam() ) {
		uritext += ";maddr=" + maddrhostname;
	}
	if( hasFtag() ) {
		uritext += ";ftag=" + ftag;
	}
	if( isLooseRouting() ) {
		uritext += ";lr";
		if( haslrparam() ) {
			uritext += "=" + lr;
		}
	}

	if( !uritext.isEmpty() ) {
		uritext = getProtocolName() + ":" + uritext;
	}

	//add the rest of params to the generated uri
	ParameterListConstIterator it;
	for(it=rOfParamsUri.begin();it != rOfParamsUri.end();++it) {
		uritext += ";";
		uritext += (*it).getName();
		if( !(*it).getValue().isEmpty() ) {
			uritext += "=";
			uritext += (*it).getValue();
		}
	}

	return uritext;
}

//
QString SipUri::theUri( void ) const
{
	QString uritext;

// set brackets if necessary

    if( hasUserInfo() ) {
		uritext  = getProtocolName() + ":";
		uritext += username;
		uritext += "@";
	}
	if ( hostname.contains(':') >=2 ) {
	    if(!hostname.contains("[") ) { // encode IPv6 reference
		uritext += "[" ;
	    }
	    uritext+= hostname;
	    if(!hostname.contains("]") ) { // encode IPv6 reference
		uritext += "]"  ;
	    }
	} else {
		uritext += hostname;
	}


	return uritext;
}



QString SipUri::reqUri( void ) const
{
	QString uritext;
	uritext = getProtocolName() + ":";
	if( hasUserInfo() ) {
		uritext += username + "@";
	}
// set brackets if necessary
	if ( hostname.contains(':') >=2 ) {
	
	    if(!hostname.contains("[") ) { // encode IPv6 reference
		uritext += "[" ;
	    }
	    
	    uritext+= hostname;
	    
	    if(!hostname.contains("]") ) { // encode IPv6 reference
		uritext += "]"  ;
	    }
	} else {
		uritext += hostname;
	}
	if( hasport || port != getSipPort() ) {
		uritext += ":" + QString::number( port );
	}
	if( hasMaddrParam() ) {
		uritext += ";maddr=" + maddrhostname;
	}
	if( hasTransportParam() ) {
		uritext += ";transport=" + getTransportParamString( transparam );
	}

	//add the rest of params to the generated uri
	ParameterListConstIterator it;
	for(it=rOfParamsUri.begin();it != rOfParamsUri.end();++it) {
		uritext += ";";
		uritext += (*it).getName();
		if( !(*it).getValue().isEmpty() ) {
			uritext += "=";
			uritext += (*it).getValue();
		}
	}
	return uritext;
}

QString SipUri::getRouteUri( void ) const
{
	QString uritext;
// set brackets if necessary
	uritext = "<" + getProtocolName() + ":";
	if ( hostname.contains(':') >=2 ) {
	
	    if(!hostname.contains("[") ) { // encode IPv6 reference
		uritext += "[" ;
	    }
	    
	    uritext+= hostname;
	    
	    if(!hostname.contains("]") ) { // encode IPv6 reference
		uritext += "]"  ;
	    }
	} else {
		uritext += hostname;
	}
	uritext += ">";
	return uritext;
}

QString SipUri::getRegisterUri( void ) const
{
	QString uritext;

// set brackets if necessary
	uritext = getProtocolName() + ":";
	
	if ( hostname.contains(':') >=2 ) {
	
	    if(!hostname.contains("[") ) { // encode IPv6 reference
		uritext += "[" ;
	    }
	    
	    uritext+= hostname;
	    
	    if(!hostname.contains("]") ) { // encode IPv6 reference
		uritext += "]"  ;
	    }
	} else {
		uritext += hostname;
	}
	return uritext;
}

void SipUri::setUsername( const QString &u )
{
	if ( u.isNull() ) {
		hasuserinfo = false;
	} else {
		hasuserinfo = true;
		username = u;
	}
}

void SipUri::setProxyUsername( const QString &u )
{
	if ( u.isNull() ) {
		hasproxyusername = false;
	} else {
		hasproxyusername = true;
		proxyusername = u;
	}
}

void SipUri::setPassword( const QString &p )
{
	if ( p.isNull() ) {
		haspassword = false;
	} else {
		haspassword = true;
		password = p;
	}
}

void SipUri::setPortNumber( unsigned int p )
{
	port = p;
}

void SipUri::setTransportParam( TransportParam t )
{
	if ( t == NoTransportParam ) {
		hastransparam = false;
	} else {
		hastransparam = true;
		transparam = t;
	}
}

void SipUri::setUserParam( UserParam u )
{
	if ( u == NoUserParam ) {
		hasuserparam = false;
	} else {
		hasuserparam = true;
		userparam = u;
	}
}

void SipUri::setMethodParam( Sip::Method m )
{
	if ( m == Sip::BadMethod ) {
		hasmethodparam = false;
	} else {
		hasmethodparam = true;
		meth = m;
	}
}

void SipUri::setTtl( unsigned char t )
{
	ttl = t;
}

void SipUri::setFullname( const QString &newfname )
{
	fullname = newfname;
	fullname = fullname.replace( QRegExp( "\"" ), QString::null ).simplifyWhiteSpace();
}

void SipUri::generateTag( void )
{
	int uniqid = rand();
	tag.sprintf( "%X", uniqid );
	hastag = true;
}

bool SipUri::operator==( const SipUri &uri ) const
{

	if( !isValid() || !uri.isValid() ) {
		return false;
	}
	if( (username == uri.username) && /* password == uri.password && */
		(hostname == uri.hostname) && (port == uri.port) ) {
		return true;
	}
	return false;
}

bool SipUri::operator==( const QString &url ) const
{
	SipUri u( url );
	return ( *this == u );
}

bool SipUri::operator!=( const SipUri &uri ) const
{
	return !( *this == uri );
}

bool SipUri::operator!=( const QString &url ) const
{
	return !( *this == url );
}

SipUri &SipUri::operator=( const SipUri &uri )
{
	clear();
	protocolname = uri.protocolname;
	fullname = uri.fullname;
	hasuserinfo = uri.hasuserinfo;
	username = uri.username;
	hasproxyusername = uri.hasproxyusername;
	proxyusername = uri.proxyusername;
	haspassword = uri.haspassword;
	password = uri.password;
	port = uri.port;
	isvalid = uri.isvalid;
	hastransparam = uri.hastransparam;
	transparam = uri.transparam;
	hasuserparam = uri.hasuserparam;
	userparam = uri.userparam;
	hasmethodparam = uri.hasmethodparam;
	meth = uri.meth;
	hasttlparam = uri.hasttlparam;
	maddrhostname = uri.maddrhostname;
	hasmaddrparam = uri.hasmaddrparam;
	hastag = uri.hastag;
	tag = uri.tag;
	hostname = uri.hostname;
	hasport = uri.hasport;
	ftag = uri.ftag;

//copy rest of params
	rOfParams = uri.rOfParams;
	rOfParamsUri = uri.rOfParamsUri;

	return *this;
}

void SipUri::setTag( const QString &newtag )
{
	tag = newtag;
	if( tag == QString::null ) {
		hastag = false;
	} else {
		hastag = true;
	}
}


void SipUri::setMaddrParam( const QString &newmaddr )
{
	QString nm=newmaddr;
// set brackets if necessary
	if( nm != QString::null ) {
		if ( nm.contains(':') >=2 ) { // IPv6 address
			if (!nm.contains("[")) { // no brackets -> add some
				nm= "[" + nm; 
			}
			if (!nm.contains("]")) { // no brackets -> add some
				nm+= "]"; 
			}
		}
		maddrhostname = nm;
		hasmaddrparam = true;
	} else {
		hasmaddrparam = false;
		maddrhostname = QString::null;
	}
}

void SipUri::updateUri( const QString &u )
{
	parseUri( u );
}

QString SipUri::getHostname(void) const {
// just do it
QString h = hostname+" ";

// set brackets if necessary
	if ( h.contains(':') >=2 ) {
	    if(h.contains("[") < 1) { // encode IPv6 reference
		h = "[" + h ;
	    }
	    	if(h.contains("]") < 1 ) { // encode IPv6 reference
		h += "]"  ;
	    }
	}
return h.stripWhiteSpace();
}
