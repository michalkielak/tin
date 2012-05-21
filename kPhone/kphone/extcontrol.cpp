/*
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

#include "../config.h"
#include "sessioncontrol.h"
#include "extcontrol.h"
#include "../dissipate2/sipcall.h"
#include "kstatics.h"
ExtControl::ExtControl( int lo, SessionControl *sessioncontrol,
	QWidget *parent, const char *name )
	: QDialog( parent, QString( name ) + "Test Properties", true )
{
	a = sessioncontrol;
	load = lo;
	call = SipCall::extCall1 + lo - 1;
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 1 );
	vbox->setSpacing( 1 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );
	QSettings settings;
	pFix = QString::fromAscii( name) + "/SessionType" + QString::number(load);
	(void) new QLabel( KStatics::dBase+pFix, vbox );
	(void) new QLabel( tr("The Program"), vbox );
	QString extsw  = settings.readEntry(
		KStatics::dBase+pFix+"/exec", "kedit" );
	extSW  = new QLineEdit( extsw, vbox );
	extSW->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	(void) new QLabel( tr("The Parameters"), vbox );
	QString extparam  = settings.readEntry(
		KStatics::dBase+pFix+"/param", "" );
	extPR  = new QLineEdit( extparam, vbox );
	extPR->setMinimumWidth( fontMetrics().maxWidth() * 20 );

	(void) new QLabel( tr("The Icon"), vbox );
	QString extico  = settings.readEntry(
		KStatics::dBase+pFix+"/icon", "question" );
	extIco  = new QLineEdit( extico, vbox );
	extIco->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	
	//The Remote Startable   Flag
	rstBG = new QHButtonGroup( tr( "Remote Startable"), vbox );
	QRadioButton *RstYes = new QRadioButton(tr( "Yes"), rstBG );
	rstBG->insert( RstYes, rstYes );
	QRadioButton *RstNo = new QRadioButton(  tr("No"), rstBG );
	rstBG->insert( RstNo, rstNo );
	if (settings.readEntry(	KStatics::dBase+pFix+"/Rst", "No" ) == "Yes") {
	    rstBG->setButton(rstYes);
	} else {
	    rstBG->setButton(rstNo);
	}
	
	(void) new QLabel( tr("The VectorId"), vbox );
	QString vectorid  = settings.readEntry(
		KStatics::dBase+pFix+"/VectorId", "" );
	vecId  = new QLineEdit( vectorid, vbox );
	vecId->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	
	//The Hold Flag
	holdBG = new QHButtonGroup( tr("Hold Allowed "), vbox );
	QRadioButton *HoldYes = new QRadioButton( tr("Yes"), holdBG );
	holdBG->insert( HoldYes, holdYes );
	QRadioButton *HoldNo = new QRadioButton(tr("No"), holdBG );
	holdBG->insert( HoldNo, holdNo );
	if (settings.readEntry(	KStatics::dBase+pFix+"/Hold", "No" ) == "Yes") {
	    holdBG->setButton(holdYes);
	} else {
	    holdBG->setButton(holdNo);
	}
	
	
	//The Transfer  Flag
	transferBG = new QHButtonGroup(tr( "Transfer Allowed "), vbox );
	QRadioButton *TransferYes = new QRadioButton(tr( "Yes"), transferBG );
	transferBG->insert( TransferYes, transferYes );
	QRadioButton *TransferNo = new QRadioButton( tr("No"), transferBG );
	transferBG->insert( TransferNo, transferNo );
	if (settings.readEntry(	KStatics::dBase+pFix+"/Transfer", "No" ) == "Yes") {
	    transferBG->setButton(transferYes);
	} else {
	    transferBG->setButton(transferNo);
	}

	//The Usesound  Flag
	usesoundBG = new QHButtonGroup(tr( "Sound Card Used"), vbox );
	QRadioButton *UsesoundYes = new QRadioButton( tr("Yes"), usesoundBG );
	usesoundBG->insert( UsesoundYes, usesoundYes );
	QRadioButton *UsesoundNo = new QRadioButton( tr("No"), usesoundBG );
	usesoundBG->insert( UsesoundNo, usesoundNo );
	if (settings.readEntry(	KStatics::dBase+pFix+"/UseSound", "No" ) == "Yes") {
	    usesoundBG->setButton(usesoundYes);
	} else {
	    usesoundBG->setButton(usesoundNo);
	}
	//The UseL4  Flag
	useL4BG = new QHButtonGroup(tr( "SDP Transport"), vbox );
	QRadioButton *Useudp = new QRadioButton( "UDP", useL4BG );
	useL4BG->insert( Useudp, useudp );
	QRadioButton *Usertp = new QRadioButton( "RTP", useL4BG );
	useL4BG->insert( Usertp, usertp );

	if (settings.readEntry(	KStatics::dBase+pFix+"/UseL4", "UDP" ) == "UDP") {
	    useL4BG->setButton(useudp);
	} else {
		useL4BG->setButton(usertp);
	}
	
	
	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 );
	okPushButton = new QPushButton( this, "ok button" );
	okPushButton->setText( "OK" );
	okPushButton->setDefault( TRUE );
	buttonBox->addWidget( okPushButton );
	cancelPushButton = new QPushButton( this, "cancel button" );
	cancelPushButton->setText( tr("Cancel") );
	cancelPushButton->setAccel( Key_Escape );
	buttonBox->addWidget( cancelPushButton );
	connect( okPushButton, SIGNAL( clicked() ),
		this, SLOT( slotOk() ) );
	connect( cancelPushButton, SIGNAL( clicked() ),
		this, SLOT( slotCancel() ) );
}

ExtControl::~ExtControl( void )
{
}

void ExtControl::slotOk( void )
{
	slotApply();
	QDialog::accept();
}

void ExtControl::slotApply( void )
{
	QSettings settings;

	settings.writeEntry( KStatics::dBase+pFix+"/exec", extSW->text() );
	a->setExtSW ( call, extSW->text() );


	a->setExtIco(call, extIco->text() );
	settings.writeEntry( KStatics::dBase+pFix+"/icon", extIco->text() );

	a->setExtPR (call, extPR->text() );
	settings.writeEntry( KStatics::dBase+pFix+"/param", extPR->text() );
	a->setVectorId (call, vecId->text() );
	settings.writeEntry( KStatics::dBase+pFix+"/VectorId", vecId->text() );
	switch( rstBG->id(rstBG->selected())) {
	case rstYes:
		settings.writeEntry(KStatics::dBase+pFix+"/Rst", "Yes");
		a->allowRemoteStart (call,true );//
		break;
	default:
		settings.writeEntry(KStatics::dBase+pFix+"/Rst", "No");
		a->allowRemoteStart (call,false );//
		break;
	}

	switch( holdBG->id(holdBG->selected())) {
	case holdYes:
		settings.writeEntry(KStatics::dBase+pFix+"/Hold", "Yes");
		a->setHoldAllowed (call,true );//
		break;
	default:
		settings.writeEntry(KStatics::dBase+pFix+"/Hold", "No");
		a->setHoldAllowed (call,false );//
		break;
	}
	
	switch( useL4BG->id(useL4BG->selected())) {

	case useudp:
		settings.writeEntry(KStatics::dBase+pFix+"/UseL4","UDP");
		a->setUseL4 (call,useudp );//
		break;
	default:
		settings.writeEntry(KStatics::dBase+pFix+"/UseL4","RTP");
		a->setUseL4 (call,usertp );//
		break;
	}

	switch( usesoundBG->id(usesoundBG->selected())) {
	case usesoundYes:
		settings.writeEntry(KStatics::dBase+pFix+"/UseSound","Yes");
		a->setActiveOnly (call,true );//
		break;
	default:
		settings.writeEntry(KStatics::dBase+pFix+"/UseSound","No");
		a->setActiveOnly (call,false );//
		break;
	}

	switch( transferBG->id(transferBG->selected())) {
	case transferYes:
		settings.writeEntry(KStatics::dBase+pFix+"/Transfer", "Yes");
		a->setTransferAllowed (call,true );//
		break;
	default:
		settings.writeEntry(KStatics::dBase+pFix+"/Transfer", "No");
		a->setTransferAllowed (call,false );//
		break;
	}
	
	
}

void ExtControl::slotCancel( void )
{
	QDialog::reject();
}
