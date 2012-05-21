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
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <iostream>
#include <qbuttongroup.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qgrid.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qtextedit.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qsound.h>
#include "kcallwidget.h"
#include "kphone.h"
#include "kphoneview.h"
#include "ksipauthentication.h"
#include "sessioncontrol.h"
#include "callaudio.h"
#include "../dissipate2/mimecontenttype.h"
#include "../dissipate2/sdpbuild.h"
#include "../dissipate2/sipcall.h"
#include "../dissipate2/sipmessage.h"
#include "../dissipate2/siptransaction.h"
#include "../dissipate2/sipuser.h"
#include "../dissipate2/udpmessagesocket.h"
#include "kstatics.h"
using namespace std;
KInstantMessageWidget::KInstantMessageWidget( KSipAuthentication *auth,
		SipCall *initcall, QWidget *parent, const char *name )
	: QDialog( parent, name ), imcall( initcall )
{
	sipauthentication = auth;
	phoneBook = 0;
	setCaption( getUserPrefix() + " Message - KPhone" );
	QVBoxLayout *vbox = new QVBoxLayout( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QGridLayout *headergrid = new QGridLayout( 6, 3 );
	headergrid->setSpacing( 3 );
	vbox->insertLayout( 0, headergrid );
	
	QLabel *remotelabel = new QLabel( "Remote: ", this );
	remotelabel->setFixedWidth( remotelabel->fontMetrics().width( "  Remote:  "  ) );
	headergrid->addWidget( remotelabel, 0, 0 );
	remote = new QComboBox( true, this );
	remote->setAutoCompletion( true );
	remote->setEditText( "" );
	headergrid->addWidget( remote, 0, 1 );
	QPushButton *loadUri = new QPushButton( "", this );
	
	QIconSet icon;
	icon.setPixmap(SHARE_DIR "/icons/phonebook.png", QIconSet::Automatic );
	loadUri->setIconSet( icon );
	loadUri->setFixedWidth( loadUri->fontMetrics().maxWidth() * 2 );
	connect( loadUri, SIGNAL( clicked() ), this, SLOT( getUri() ) );
	headergrid->addWidget( loadUri, 0, 2 );
	QLabel *slabel = new QLabel( "Status: ", this );
	vbox->insertWidget( 1, slabel );
	status = new QTextEdit( this );
	status->setReadOnly( true );
	vbox->insertWidget( 2, status );
	QLabel *mlabel = new QLabel( "Message: ", this );
	vbox->insertWidget( 3, mlabel );
	QGridLayout *mgrid = new QGridLayout( 1, 3 );
	mgrid->setSpacing( 3 );
	vbox->insertLayout( 4, mgrid );
	mEdit = new QTextEdit( this );
	mgrid->addWidget( mEdit, 0,0 );
	mEdit->setFocus();
	mbutton = new QPushButton( "Send", this );
	mbutton->setFixedWidth( mbutton->fontMetrics().maxWidth() * 4 );
	connect( mbutton, SIGNAL( clicked() ), this, SLOT( mClicked() ) );
	mgrid->addWidget( mbutton, 0, 2 );
	QGrid *buttonbox = new QGrid(2, this );
	buttonbox->setSpacing( 5 );
	vbox->insertWidget( 5, buttonbox );
	(void) new QWidget( buttonbox );
	hidebutton = new QPushButton( "Hide", buttonbox );
	connect( hidebutton, SIGNAL( clicked() ), this, SLOT( hide() ) );
	resize( 400, 200 );
}

KInstantMessageWidget::~KInstantMessageWidget( void )
{
	if( phoneBook ) {
		delete phoneBook;
	}
}

void KInstantMessageWidget::instantMessage( SipMessage *message )
{
	QString s = message->getHeaderData( SipHeader::From );
	SipUri u = SipUri( s );
	remote->setEditText( u.getUsername() + "@" + u.getHostname() + ":" + QString::number(u.getPortNumber()) );
	QString m = message->messageBody();
	status->setText( status->text() + "\nMessage received from " + remote->currentText() );
	status->setText( status->text() + " (" + QTime::currentTime().toString().latin1() + ")" );
	status->setText( status->text() + "\n > " + m );
	status->scrollToBottom();
}

SipCall *KInstantMessageWidget::getCall()
{
	return imcall;
}

QString KInstantMessageWidget::getRemote()
{
	return remote->currentText();
}

void KInstantMessageWidget::setRemote( QString newremote )
{
	remote->setEditText( newremote );
}

void KInstantMessageWidget::mClicked( void )
{
	if( remote->currentText().length() == 0 ) {
		QMessageBox::critical( this, "Error: No Destination",
			"You must specify someone to send message." );
		return;
	}
	QString strRemoteUri;
	QString s = remote->currentText();
	if( s.contains( '[' ) && s.contains( ']' ) ) {
		strRemoteUri = s.mid( s.find( '[' ) + 1, s.find( ']' ) - s.find( '[' ) - 1 );
	} else {
		if( s.left( 4 ).lower() != "tel:" ) {
			if( s.left( 4 ).lower() != "sip:" ) {
				s = "sip:" + s;
			}
			if( !s.contains( '@' ) ) {
				s = s + "@" + imcall->getHostname();
			}
		}
		strRemoteUri = s;
	}
	for( int i = 0; i < remote->count(); i++ ) {
		if( remote->text( i ).compare( s ) == 0 ) {
			remote->removeItem( i );
		}
	}
	remote->insertItem( s, 0 );
	remote->setCurrentItem( 0 );
	SipUri remoteuri( strRemoteUri );
	member = new SipCallMember( imcall, remoteuri );
	connect( member, SIGNAL( statusUpdated( SipCallMember * ) ),
		sipauthentication, SLOT( authRequest( SipCallMember * ) ) );
	status->setText( status->text() + "\nMessage sent to " + remote->currentText() );
	status->setText( status->text() + " (" + QTime::currentTime().toString().latin1() + ")" );
	status->setText( status->text() + "\n > " + mEdit->text() );
	status->scrollToBottom();

	member->requestMessage( mEdit->text(),
		MimeContentType( "text/plain; charset=UTF-8" ) );
	mEdit->clear();
}

void KInstantMessageWidget::getUri( void )
{
	QSettings settings;
	QString p = KStatics::dBase + getUserPrefix() + "/local";
	QString xmlFile = settings.readEntry( p + "/PhoneBook", "" );
	if( xmlFile.isEmpty() ) {
		if( getUserPrefix().isEmpty() ) {
			xmlFile = QDir::homeDirPath() +
				"/." + KStatics::xBase + "-phonebook.xml";
		} else {
			xmlFile = QDir::homeDirPath() + "/." + KStatics::xBase + "_" + getUserPrefix() + "phonebook.xml";
		}
	}
	QStringList r;
	if( !phoneBook ) {
		phoneBook = new PhoneBook( xmlFile, this,
			getUserPrefix() + "Phone Book",
			receivedCalls, missedCalls, dialledCalls );
	}
	phoneBook->exec();
	remote->setEditText( phoneBook->getUri() );
	((KPhoneView *)parentWidget())->updateContacts( xmlFile );
}


QString KInstantMessageWidget::getUserPrefix( void )
{
	return ((KPhone *)parentWidget()->parentWidget())->getUserPrefix();
}
