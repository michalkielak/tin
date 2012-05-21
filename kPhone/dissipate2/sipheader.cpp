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

SipHeader::SipHeader( SipHeaderId newid, QString newdata )
{
	id = newid;
	data = newdata;
}

SipHeader::~SipHeader( void )
{
}

SipHeader::SipHeaderId SipHeader::matchHeader( const QString& h )
{
	QString u = h.upper();
	// Check short forms first
	if ( u.compare( getHeaderString( Via, true ).upper() ) == 0 ) { return Via; }
	if ( u.compare( getHeaderString( Content_Type, true ).upper() ) == 0 ) { return Content_Type; }
	if ( u.compare( getHeaderString( Content_Encoding, true ).upper() ) == 0 ) { return Content_Encoding; }
	if ( u.compare( getHeaderString( From, true ).upper() ) == 0 ) { return From; }
	if ( u.compare( getHeaderString( Call_ID, true ).upper() ) == 0 ) { return Call_ID; }
	if ( u.compare( getHeaderString( Contact, true ).upper() ) == 0 ) { return Contact; }
	if ( u.compare( getHeaderString( Content_Length, true ).upper() ) == 0 ) { return Content_Length; }
	if ( u.compare( getHeaderString( Subject, true ).upper() ) == 0 ) { return Subject; }
	if ( u.compare( getHeaderString( To, true ).upper() ) == 0 ) { return To; }

	if ( u.compare( getHeaderString( Accept ).upper() ) == 0 ) { return Accept; }
	if ( u.compare( getHeaderString( Accept_Encoding).upper() ) == 0 ) { return Accept_Encoding; }
	if ( u.compare( getHeaderString( Accept_Language).upper() ) == 0 ) { return Accept_Language; }
	if ( u.compare( getHeaderString( Allow ).upper() ) == 0 ) { return Allow; }
	if ( u.compare( getHeaderString( Also ).upper() ) == 0 ) { return Also; }
	if ( u.compare( getHeaderString( Authorization ).upper() ) == 0 ) { return Authorization; }
	if ( u.compare( getHeaderString( Call_ID ).upper() ) == 0 ) { return Call_ID; }
	if ( u.compare( getHeaderString( Contact ).upper() ) == 0 ) { return Contact; }
	if ( u.compare( getHeaderString( Content_Encoding ).upper() ) == 0 ) { return Content_Encoding; }
	if ( u.compare( getHeaderString( Content_Length ).upper() ) == 0 ) { return Content_Length; }
	if ( u.compare( getHeaderString( Content_Type ).upper() ) == 0 ) { return Content_Type; }
	if ( u.compare( getHeaderString( CSeq ).upper() ) == 0 ) { return CSeq; }
	if ( u.compare( getHeaderString( Date ).upper() ) == 0 ) { return Date; }
	if ( u.compare( getHeaderString( Encryption ).upper() ) == 0 ) { return Encryption; }
	if ( u.compare( getHeaderString( Expires ).upper() ) == 0 ) { return Expires; }
	if ( u.compare( getHeaderString( From ).upper() ) == 0 ) { return From; }
	if ( u.compare( getHeaderString( Hide ).upper() ) == 0 ) { return Hide; }
	if ( u.compare( getHeaderString( Max_Forwards ).upper() ) == 0 ) { return Max_Forwards; }
	if ( u.compare( getHeaderString( Min_Expires ).upper() ) == 0 ) { return Min_Expires; }
	if ( u.compare( getHeaderString( Organization ).upper() ) == 0 ) { return Organization; }
	if ( u.compare( getHeaderString( Proxy_Authenticate ).upper() ) == 0 ) { return Proxy_Authenticate; }
	if ( u.compare( getHeaderString( Proxy_Authorization ).upper() ) == 0 ) { return Proxy_Authorization; }
	if ( u.compare( getHeaderString( Proxy_Require ).upper() ) == 0 ) { return Proxy_Require; }
	if ( u.compare( getHeaderString( Priority ).upper() ) == 0 ) { return Priority; }
	if ( u.compare( getHeaderString( Require ).upper() ) == 0 ) { return Require; }
	if ( u.compare( getHeaderString( Retry_After ).upper() ) == 0 ) { return Retry_After; }
	if ( u.compare( getHeaderString( Response_Key ).upper() ) == 0 ) { return Response_Key; }
	if ( u.compare( getHeaderString( Record_Route ).upper() ) == 0 ) { return Record_Route; }
	if ( u.compare( getHeaderString( Refer_To ).upper() ) == 0 ) { return Refer_To; }
	if ( u.compare( getHeaderString( Referred_By ).upper() ) == 0 ) { return Referred_By; }
	if ( u.compare( getHeaderString( Refer_Sub ).upper() ) == 0 ) { return Refer_Sub; }
	if ( u.compare( getHeaderString( Route ).upper() ) == 0 ) { return Route; }
	if ( u.compare( getHeaderString( RAck  ).upper() ) == 0 ) { return RAck; }
	if ( u.compare( getHeaderString( RSeq ).upper() ) == 0 ) { return RSeq; }
	if ( u.compare( getHeaderString( Server ).upper() ) == 0 ) { return Server; }
	if ( u.compare( getHeaderString( Service_Route ).upper() ) == 0 ) { return Service_Route; }
	if ( h.compare( getHeaderString( SIP_ETag ) ) == 0 ) { return SIP_ETag ; }
	if ( h.compare( getHeaderString( SIP_If_Match ) ) == 0 ) { return SIP_If_Match; }
	if ( u.compare( getHeaderString( Subject ).upper() ) == 0 ) { return Subject; }
	if ( h.compare( getHeaderString( Subscription_State)) == 0 ) { return Subscription_State; } 
	if ( h.compare( getHeaderString( Subscription_State,true).upper() ) == 0 ) { return Subscription_State; }
 	if ( u.compare( getHeaderString( Supported ).upper() ) == 0 ) { return Supported; }
	if ( u.compare( getHeaderString( Timestamp ).upper() ) == 0 ) { return Timestamp; }
	if ( u.compare( getHeaderString( To ).upper() ) == 0 ) { return To; }
	if ( u.compare( getHeaderString( Unsupported ).upper() ) == 0 ) { return Unsupported; }
	if ( u.compare( getHeaderString( User_Agent ).upper() ) == 0 ) { return User_Agent; }
	if ( u.compare( getHeaderString( Via ).upper() ) == 0 ) { return Via; }
	if ( u.compare( getHeaderString( Warning ).upper() ) == 0 ) { return Warning; }
	if ( u.compare( getHeaderString( WWW_Authenticate ).upper() ) == 0 ) { return WWW_Authenticate; }
	if ( u.compare( getHeaderString( Event ).upper() ) == 0 ) { return Event; }
	if ( u.compare( getHeaderString( Allow_Events ).upper() ) == 0 ) { return Allow_Events; }

	return BadHeader;
}

const QString SipHeader::getHeaderString( SipHeaderId id, bool shortform )
{
	if ( shortform ) {
		switch ( id ) {
			case Accept: return "Accept";
			case Accept_Encoding: return "Accept-Encoding";
			case Accept_Language: return "Accept-Language";
			case Allow: return "Allow";
			case Allow_Events: return "Allow-Events";
			case Also: return "Also";
			case Authorization: return "Authorization";
			case Call_ID: return "i";
			case Contact: return "m";
			case Content_Encoding: return "e";
			case Content_Length: return "l";
			case Content_Type: return "c";
			case CSeq: return "CSeq";
			case Date: return "Date";
			case Encryption: return "Encryption";
			case Event: return "Event";
			case Expires: return "Expires";
			case From: return "f";
			case Hide: return "Hide";
			case Max_Forwards: return "Max-Forwards";
			case Min_Expires: return "Min-Expires";
			case Organization: return "Organization";
			case Proxy_Authenticate: return "Proxy-Authenticate";
			case Proxy_Authorization: return "Proxy-Authorization";
			case Proxy_Require: return "Proxy-Require";
			case Priority: return "Priority";
			case Require: return "Require";
			case Retry_After: return "Retry-After";
			case Response_Key: return "Response-Key";
			case Record_Route: return "Record-Route";
			case Refer_To: return "Refer-To";
			case Referred_By: return "Referred-By";
			case Refer_Sub: return "Refer-Sub";
			case Route: return "Route";
			case Server: return "Server";
			case Service_Route: return "Service-Route";
			case RAck: return "RAck";
			case RSeq: return "RSeq";
			case Subject: return "s";
			case SIP_ETag: return "SIP-ETag";
			case SIP_If_Match: return "SIP-If-Match";
			case Subscription_State: return "Subscription-State";
	                case Supported: return "k";
			case Timestamp: return "Timestamp";
			case To: return "t";
			case Unsupported: return "Unsupported";
			case User_Agent: return "User-Agent";
			case Via: return "v";
			case Warning: return "Warning";
			case WWW_Authenticate: return "WWW-Authenticate";
			case BadHeader: return "BadHeader";
		}
	} else {
		switch ( id ) {
			case Accept: return "Accept";
			case Accept_Encoding: return "Accept-Encoding";
			case Accept_Language: return "Accept-Language";
			case Allow: return "Allow";
			case Allow_Events: return "Allow-Events";
			case Also: return "Also";
			case Authorization: return "Authorization";
			case Call_ID: return "Call-ID";
			case Contact: return "Contact";
			case Content_Encoding: return "Content-Encoding";
			case Content_Length: return "Content-Length";
			case Content_Type: return "Content-Type";
			case CSeq: return "CSeq";
			case Date: return "Date";
			case Encryption: return "Encryption";
			case Event: return "Event";
			case Expires: return "Expires";
			case From: return "From";
			case Hide: return "Hide";
			case Min_Expires: return "Min-Expires"; 
			case Max_Forwards: return "Max-Forwards";
			case Organization: return "Organization";
			case Proxy_Authenticate: return "Proxy-Authenticate";
			case Proxy_Authorization: return "Proxy-Authorization";
			case Proxy_Require: return "Proxy-Require";
			case Priority: return "Priority";
			case Require: return "Require";
			case Retry_After: return "Retry-After";
			case Response_Key: return "Response-Key";
			case Record_Route: return "Record-Route";
			case Refer_To: return "Refer-To";
			case Refer_Sub: return "Refer-Sub";
			case Referred_By: return "Referred-By";
			case Route: return "Route";
			case RAck: return "RAck";
			case RSeq: return "RSeq";
			case Server: return "Server";
			case Service_Route: return "Service-Route";
			case Subject: return "Subject";
			case SIP_ETag: return "SIP-ETag";
			case SIP_If_Match: return "SIP-If-Match";
			case Subscription_State: return "Subscription-State";
	                case Supported: return "Supported";
			case Timestamp: return "Timestamp";
			case To: return "To";
			case Unsupported: return "Unsupported";
			case User_Agent: return "User-Agent";
			case Via: return "Via";
			case Warning: return "Warning";
			case WWW_Authenticate: return "WWW-Authenticate";
			case BadHeader: return "BadHeader";
		}
	}
	return QString::null;
}

