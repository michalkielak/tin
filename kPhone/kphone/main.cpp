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
#include "../config.h"
#include <qapplication.h>
#ifdef LIPO
#include <qtranslator.h>
#endif
#include <qtextcodec.h>
#include "kphone.h"
#include <stdlib.h>
#include <signal.h>
#include "kstatics.h"

int main( int argc, char **argv )
{
	QString port = "";
	QString dhost= "";
	QString user = "default";
	bool looseRoute = false;
	bool strictRoute = false;
	bool direct2URI = false;
	bool dontRegister = false;
	int p2p = 0;
	int debug = 0;
	QApplication a( argc, argv );
	
	//translation file for application strings
#ifdef LIPO
	QTranslator myapp( 0 );
	myapp.load( QString( "kphone_" ) + QTextCodec::locale(), PO_DIR );
	a.installTranslator( &myapp );
#endif	
	//the command line
	for ( int i = 0; i < a.argc(); i++ ) {
		// -u <user> user identification in database 
		if( QString( a.argv()[i] ) == "-u" ) {
			if( i+1 < a.argc() ) {
				user = QString( a.argv()[++i] );
			}
		// -a <host> dissipate-host 
		} else if( QString( a.argv()[i] ) == "-a" ) {
			if( i+1 < a.argc() ) {
				dhost = QString( a.argv()[++i] );
			}
		// -p <port>[:n] portnumber to partner and repetition counter for free port search
		} else if( QString( a.argv()[i] ) == "-p" ) {
			if( i+1 < a.argc() ) {
				port = QString( a.argv()[++i] );
			}
		// -t n  UA point to point 4=IPv4 6=IPv6 (i.e. dissipate-host=host part of sip URI of the UA)
		} else if( QString( a.argv()[i] ) == "-t" ) {
			if( i+1 < a.argc() ) {
				p2p =  atoi(a.argv()[++i]) ;
			}
		// -v n  n=1 SIP messages  n=2 debug info n=3 both
		} else if( QString( a.argv()[i] ) == "-v" ) {
			if( i+1 < a.argc() ) {
				debug =  atoi(a.argv()[++i]) ;
			}
		// -l loose route
		} else if( QString( a.argv()[i] ) == "-l" ) {
			looseRoute = true;
		// -s strict route
		} else if( QString( a.argv()[i] ) == "-s" ) {
			strictRoute = true;
		} else if( QString( a.argv()[i] ) == "-r" ) {
		// -r send requests to the To-URI directly
			direct2URI = true;
		// -n NO registration
		} else if( QString( a.argv()[i] ) == "-n") {
			dontRegister = true;
		} else if( QString( a.argv()[i] ) == "-h" ) {
			printf (" -u <user> user identification in database \n -a <host> dissipate-host \n -p <port>[:n] portnumber to partner and repetition counter for free port search\n -t n  UA point to point 4=IPv4 6=IPv6 (i.e. dissipate-host=host part of sip URI of the UA)\n -v n  n=1 SIP messages  n=2 debug info n=3 both\n -l loose route\n -s strict route\n -r send requests to the To-URI directly\n -n do NOT register after startup\n");
			exit ( 1);
		}
	}
	/* prevent zombie process when the video child is killed */
	if (signal(SIGCHLD, SIG_IGN)) perror("signal(SIGCHLD, SIG_IGN)");
	KPhone *phone = new KPhone( port, dhost, user, p2p, looseRoute, strictRoute, debug, direct2URI, dontRegister );
	a.setMainWidget( phone );
	phone->show();
	return 	a.exec();

}
