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
#include "kreferdialog.h"
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


KReferDialog::KReferDialog( KPhoneView * phv,int kcwn,QString uPrefix,QWidget *parent, const char *name )
	: QDialog( parent, name, true )
{
	phoneView=phv;
	kcwNumber= kcwn;
	userPrefix=uPrefix;
	
	phoneBook = 0;
	
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );

	(void) new QLabel( tr("Refer-to URI:"), vbox );
	touri = new QLineEdit( vbox );
	touri->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	
	
	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 );
	
	loadUri = new QPushButton( "", this );	
	QIconSet icon;
	icon.setPixmap(SHARE_DIR "/icons/phonebook.png", QIconSet::Automatic );
	loadUri->setIconSet( icon );
	loadUri->setFixedWidth( loadUri->fontMetrics().maxWidth() * 2 );
	buttonBox->addWidget( loadUri );
	
	helpPushButton = new QPushButton( this, tr("help button") );
	helpPushButton->setText( tr("&help...") );
	helpPushButton->setEnabled( FALSE );
	buttonBox->addWidget( helpPushButton );
	
	QSpacerItem *spacer = new QSpacerItem(
		0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
	buttonBox->addItem( spacer );
	okPushButton = new QPushButton( this, tr("ok button") );
	okPushButton->setText( "OK" );
	okPushButton->setDefault( TRUE );
	buttonBox->addWidget( okPushButton );
	
	cancelPushButton = new QPushButton( this,tr( "cancel button") );
	cancelPushButton->setText( tr("Cancel") );
	cancelPushButton->setAccel( Key_Escape );
	buttonBox->addWidget( cancelPushButton );
	
	connect( loadUri, SIGNAL( clicked() ), 
		this, SLOT( getUri() ) );
	connect( okPushButton, SIGNAL( clicked() ),
		this, SLOT( slotOk() ) );
	connect( cancelPushButton, SIGNAL( clicked() ),
		this, SLOT( slotCancel() ) );
}

KReferDialog::~KReferDialog( void )
{
}

void KReferDialog::slotOk( void )
{
	if( touri->text().stripWhiteSpace() != QString::null ) {
	//transfer if your CallWidget is alive
	    if(phoneView->isExistingWidget(kcwNumber))  newSessionRefer( touri->text() );
	}
	touri->clear();
	QDialog::accept();
}

void KReferDialog::slotCancel( void )
{
	touri->clear();
	QDialog::reject();
}


void KReferDialog::getUri( void )
{
	QSettings settings;
	QString p = KStatics::dBase + userPrefix + "/local";
	QString xmlFile = settings.readEntry( p + "/PhoneBook", "" );
	if( xmlFile.isEmpty() ) {
		if( userPrefix.isEmpty() ) {
			xmlFile = QDir::homeDirPath() + "/." + KStatics::xBase + "-phonebook.xml";
		} else {
			xmlFile = QDir::homeDirPath() + "/." + KStatics::xBase + "_" + userPrefix + "phonebook.xml";
		}
	}		

	
	QStringList r;
	if( !phoneBook ) {
		phoneBook = new PhoneBook( xmlFile, this,
			userPrefix + tr("Phone Book"),
			receivedCalls, missedCalls, dialledCalls );
	}
	

	phoneBook->exec();
	
	touri->setText( phoneBook->getUri() );
	//((KPhoneView *)parentWidget())->updateContacts( xmlFile );
}

