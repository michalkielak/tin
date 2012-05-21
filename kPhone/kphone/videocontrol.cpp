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
#include <qvbox.h>
#include <qlabel.h>
#include <qvbuttongroup.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qsettings.h>
#include <qlayout.h>
#include <qdialog.h>
#include <qhgroupbox.h>

#include "../config.h"
#include "sessioncontrol.h"
#include "videocontrol.h"
#include "../dissipate2/sipcall.h"
#include "kstatics.h"

VideoControl::VideoControl( SessionControl *sessioncontrol,
	QWidget *parent, const char *name )
	: QDialog( parent, QString( name ) + tr("Video Properties"), true )
{
	a = sessioncontrol;
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );
	QSettings settings;
	p = QString::fromAscii( name);
	(void) new QLabel( KStatics::dBase+p+"/Video/video SW:", vbox );
	QString videosw = settings.readEntry(
		KStatics::dBase+p+"/Video/VideoSW", "/usr/local/bin/vic" );
	videoSW = new QLineEdit( videosw, vbox );
	videoSW->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	
	QString pcodec=settings.readEntry( KStatics::dBase+p+"/Video/poscodec0","0");
	QString scodec=settings.readEntry( KStatics::dBase+p+"/Video/poscodec1","1");
	QString tcodec=settings.readEntry( KStatics::dBase+p+"/Video/poscodec2","2");		

	codecBox = new QHGroupBox( tr("Video Codec Position (0-2); -1 disables codec"),vbox);
	(void) new QLabel( "      H261", codecBox );	
	codecEdit0 = new QLineEdit( pcodec, codecBox );
	codecEdit0->setMaxLength(2);
	(void) new QLabel( "      H263", codecBox );	
	codecEdit1 = new QLineEdit( scodec, codecBox );
	codecEdit1->setMaxLength(2);
	(void) new QLabel( "      H263+", codecBox );	
	codecEdit2 = new QLineEdit( tcodec, codecBox );
	codecEdit2->setMaxLength(2);

		
	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 );
	okPushButton = new QPushButton( this, tr("ok button") );
	okPushButton->setText( "OK" );
	okPushButton->setDefault( TRUE );
	buttonBox->addWidget( okPushButton );
	cancelPushButton = new QPushButton( this, tr("cancel button") );
	cancelPushButton->setText( tr("Cancel") );
	cancelPushButton->setAccel( Key_Escape );
	buttonBox->addWidget( cancelPushButton );
	connect( okPushButton, SIGNAL( clicked() ),
		this, SLOT( slotOk() ) );
	connect( cancelPushButton, SIGNAL( clicked() ),
		this, SLOT( slotCancel() ) );

	//The Hold Flag
	holdBG = new QHButtonGroup( tr("Hold Allowed "), vbox );
	QRadioButton *HoldYes = new QRadioButton( tr("Yes"), holdBG );
	holdBG->insert( HoldYes, holdYes );
	QRadioButton *HoldNo = new QRadioButton(tr( "No"), holdBG );
	holdBG->insert( HoldNo, holdNo );
	if (settings.readEntry(	KStatics::dBase+p+"/Video/Hold", "No" ) == "Yes") {
	    holdBG->setButton(holdYes);
	} else {
	    holdBG->setButton(holdNo);
	}
	//The Transfer  Flag
	transferBG = new QHButtonGroup( tr("Transfer Allowed "), vbox );
	QRadioButton *TransferYes = new QRadioButton( tr("Yes"), transferBG );
	transferBG->insert( TransferYes, transferYes );
	QRadioButton *TransferNo = new QRadioButton( tr("No"), transferBG );
	transferBG->insert( TransferNo, transferNo );
	if (settings.readEntry(	KStatics::dBase+p+"/Video/Transfer", "No" ) == "Yes") {
	    transferBG->setButton(transferYes);
	} else {
	    transferBG->setButton(transferNo);
	}
}

VideoControl::~VideoControl( void )
{
}

void VideoControl::slotOk( void )
{
	slotApply();
	QDialog::accept();
}

void VideoControl::slotApply( void )
{
	bool ok;
	a->initCodecVideo( codecEdit0->text().toInt(&ok,10),codecEdit1->text().toInt(&ok,10),codecEdit2->text().toInt(&ok,10));
	a->setVideoSW( videoSW->text() );
	
	QSettings settings;
		settings.writeEntry( KStatics::dBase + p + "/Video/videoSW", videoSW->text() );
		
	switch( holdBG->id(holdBG->selected())) {
	case holdYes:
		settings.writeEntry(KStatics::dBase+p+"/Video/Hold", "Yes");
		a->setHoldAllowed (SipCall::videoCall,true );////
		break;
	default:
		settings.writeEntry(KStatics::dBase+p+"/Video/Hold", "No");
		a->setHoldAllowed (SipCall::videoCall,false);////
		break;
	}


	switch( transferBG->id(transferBG->selected())) {
	case transferYes:
		settings.writeEntry(KStatics::dBase+p+"/Video/Transfer", "Yes");
		a->setTransferAllowed (SipCall::videoCall,true );
		break;
	default:
		settings.writeEntry(KStatics::dBase+p+"/Video/Transfer", "No");
		a->setTransferAllowed (SipCall::videoCall,false);
		break;
	}
	
}	

void VideoControl::slotCancel( void )
{
	QDialog::reject();
}
