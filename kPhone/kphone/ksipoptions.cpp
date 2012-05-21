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
#include <iostream>
#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qlayout.h>

#include "../dissipate2/sipcall.h"
#include "../dissipate2/sipclient.h"
#include "ksipoptions.h"
#include "kstatics.h"
using namespace std;
KSipOptions::KSipOptions( SipUser *user, QWidget *parent, const char *name )
	: QDialog( parent, name, false )
{
	u = user;
	QHBox *hbox = new QHBox( this );
	hbox->setMargin( 3 );
	hbox->setSpacing( 3 );
	QHBoxLayout *hboxl = new QHBoxLayout( this, 5 );
	hboxl->addWidget( hbox );
	QVBox *lvbox = new QVBox( hbox );
	(void) new QLabel( "URI to Request Options For:", lvbox );
	queryuri = new QLineEdit( lvbox );
	(void) new QLabel( "Results:", lvbox );
	qresults = new QMultiLineEdit( lvbox );

	QVBox *rvbox = new QVBox( hbox );

	QPushButton *query = new QPushButton( "Make Query", rvbox );
	connect( query, SIGNAL( clicked() ), this, SLOT( queryOptions() ) );

	call = 0;
}

KSipOptions::~KSipOptions( void )
{
}

void KSipOptions::queryOptions( void )
{
	if( call ) {
		delete call;
		call = 0;
	}

	if( ( queryuri->text() != QString::null ) && ( !call ) ) {
		call = new SipCall( u, QString::null, SipCall::OptionsCall );
		SipUri remoteuri( queryuri->text() );
		member = new SipCallMember( call, remoteuri );
		connect( member, SIGNAL( statusUpdated( SipCallMember * ) ), this, SLOT( optionsCallStatusUpdated() ) );
		member->requestOptions();
	}
}

void KSipOptions::optionsCallStatusUpdated( void )
{
	int status = member->getLocalStatusCode();
	if( (status == 401) || (status == 407) ) {
			QString u = KStatics::myID; 
			QString p = KStatics::myPW;
			member->clearLocalStatusCode();
			member->setAuthState(SipCallMember::authState_AuthenticationOK);
			member->sendRequestOptions( u, p );
	} else {
	qresults->setText( member->getSessionDescription() );
	}
}

KTestOptions::KTestOptions( SipClient *client )
	: QDialog( 0, 0, false )
{
	c = client;
	connect( c, SIGNAL( incomingTestMessage() ), this, SLOT( testSendMessage() ) );

	QHBox *hbox = new QHBox( this );
	hbox->setMargin( 3 );
	hbox->setSpacing( 3 );
	QHBoxLayout *hboxl = new QHBoxLayout( this, 5 );
	hboxl->addWidget( hbox );

	QVBox *lvbox = new QVBox( hbox );
	(void) new QLabel( "Address:", lvbox );
	addr = new QLineEdit( lvbox );
	(void) new QLabel( "Port:", lvbox );
	port = new QLineEdit( lvbox );
	(void) new QLabel( "Message:", lvbox );
	msg = new QMultiLineEdit( lvbox );

	QVBox *rvbox = new QVBox( hbox );

	QPushButton *clean = new QPushButton( "Clean", rvbox );
	connect( clean, SIGNAL( clicked() ), this, SLOT( testClean() ) );
	QPushButton *file = new QPushButton( "File", rvbox );
	connect( file, SIGNAL( clicked() ), this, SLOT( testFile() ) );
	QPushButton *send = new QPushButton( "Send", rvbox );
	connect( send, SIGNAL( clicked() ), this, SLOT( testSend() ) );

	addr->setText("127.0.0.1");
	port->setText("5060");
}

KTestOptions::~KTestOptions( void )
{
}

void KTestOptions::slotOk( void )
{
	c->setTest( false );
	QDialog::accept();
}

void KTestOptions::testClean( void )
{
	msg->setText("");
}

void KTestOptions::testFile( void )
{
	QString s = QFileDialog::getOpenFileName(
		"",
		"Text files (*.txt)",
		this,
		"open file dialog"
		"Choose a file" );
	QFile file( s );
	msg->setText("");
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		while ( !stream.eof() ) {
			msg->insertLine( stream.readLine() );
		}
		file.close();
	}
}

void KTestOptions::testSend( void )
{
	testStr = msg->text();
	testSendMessage();
}

void KTestOptions::testSendMessage( void )
{
	QString m;
	if ( testStr.contains("#TEST#" ) ) {
		m = testStr.left( testStr.find( "#TEST#" ) );
		testStr = testStr.mid( testStr.find( "#TEST#" ) + 7 );
	} else {
		m = testStr;
		testStr = "";
	}
	if( !addr->text().isEmpty() && !port->text().isEmpty() && !m.isEmpty() ) {
		c->sendTestMessage( addr->text(), port->text().toUInt(), m );
	}
}
