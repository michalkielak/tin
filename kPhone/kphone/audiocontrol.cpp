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
#include <qhgroupbox.h>
#include <qhbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qsettings.h>
#include <qlayout.h>
#include <qdialog.h>

#include "../config.h"
#include "sessioncontrol.h"
#include "../kaudio/dspoutoss.h"
#include "audiocontrol.h"
#include "kstatics.h"


AudioControl::AudioControl( SessionControl *sessioncontrol, QWidget *parent, const char *name )
	: QDialog( parent, QString( name ) + tr("Audio Properties"), true )
{
	a = sessioncontrol;
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 1 );
	vbox->setSpacing( 1 );
	QVBoxLayout *vboxl = new QVBoxLayout( this);// , 5 );
	vboxl->addWidget( vbox );
	pFix = QString::fromAscii( name);
	debug=KStatics::debugLevel;
	bg4 = new QHButtonGroup( tr("Audio Mode"), vbox );
	
	alsaButton = new QRadioButton( tr("ALSA"), bg4 );
	bg4->insert( alsaButton, alsa );
	connect( alsaButton, SIGNAL( clicked() ), this, SLOT( alsaButtonClicked() ) );

	ossButton = new QRadioButton( tr("OSS"), bg4 );
	bg4->insert( ossButton, oss );
	connect( ossButton, SIGNAL( clicked() ), this, SLOT( ossButtonClicked() ) );


	bg2 = new QHButtonGroup( tr("OSS Device Mode"), vbox );
	rwButton = new QRadioButton( tr("ReadWrite"), bg2 );
	bg2->insert( rwButton, rw );
	rowoButton = new QRadioButton( tr("ReadOnly / WriteOnly"), bg2 );
	bg2->insert( rowoButton, rowo );
	connect( rwButton, SIGNAL( clicked() ), this, SLOT( rwButtonClicked() ) );
	connect( rowoButton, SIGNAL( clicked() ), this, SLOT( rowoButtonClicked() ) );

	(void) new QLabel( tr("Device for WriteOnly or ReadWrite:"), vbox );
	dspdev = new QLineEdit( a->getOSSFilename(), vbox );
	dspdev->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	dspdev->setText( a->getOSSFilename() );

	(void) new QLabel( tr("Device for ReadOnly:"), vbox );
	dspdev2 = new QLineEdit( a->getOSSFilename2(), vbox );
	dspdev2->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	dspdev2->setText( a->getOSSFilename2() );
        QSettings settings;
	audioMode = settings.readEntry( KStatics::dBase+pFix+"/Audio/audio", "alsa" );
	if( audioMode == "alsa" ) {
		a->setAudioSys( SessionControl::isALSA );
		alsaButton->setChecked(TRUE);
		rwButton->setEnabled(FALSE);
		rowoButton->setEnabled(FALSE);
		dspdev->setEnabled(TRUE);
		dspdev->setText(a->getALSAFilename());
		dspdev2->setEnabled(FALSE);
	} else {
		a->setAudioSys( SessionControl::isOSS );
		ossButton->setChecked(TRUE);
		rowoButton->setEnabled(TRUE);
		rwButton->setEnabled(TRUE);
		QString mode = settings.readEntry( KStatics::dBase+pFix+"/Audio/mode", "readonly_writeonly" );
		if( mode == "readwrite" ) {
			rwButton->setChecked(TRUE);
			rowoButton->setChecked(FALSE);
			dspdev2->setEnabled(FALSE);
		} else {
			rwButton->setChecked(FALSE);
			rowoButton->setChecked(TRUE);
			dspdev2->setEnabled(TRUE);
		}
	}

	QString pcodec=settings.readEntry( KStatics::dBase+pFix+"/Audio/poscodec0","0");
	QString scodec=settings.readEntry( KStatics::dBase+pFix+"/Audio/poscodec1","1");
	QString tcodec=settings.readEntry( KStatics::dBase+pFix+"/Audio/poscodec2","2");
	QString qcodec=settings.readEntry( KStatics::dBase+pFix+"/Audio/poscodec3","3");

#ifdef SPEEX
	codecBox = new QHGroupBox( "Audio Codec Position (values: 0 highest to 4 lowest; -1 disables codec)",vbox);
	QString fcodec=settings.readEntry( KStatics::dBase+pFix+"/Audio/poscodec4","4");
#else 
	QString fcodec=settings.readEntry( KStatics::dBase+pFix+"/Audio/poscodec4","-1");
	codecBox = new QHGroupBox( "Audio Codec Position (values: 0 highest to 3 lowest; -1 disables codec)",vbox);
#endif
	(void) new QLabel( "        ULAW", codecBox );	
	codecEdit0 = new QLineEdit( pcodec, codecBox );
	codecEdit0->setMaxLength(2);
	(void) new QLabel( "        ALAW", codecBox );	
	codecEdit1 = new QLineEdit( scodec, codecBox );
	codecEdit1->setMaxLength(2);
	(void) new QLabel("        GSM", codecBox );	
	codecEdit2 = new QLineEdit( tcodec, codecBox );
	codecEdit2->setMaxLength(2);
	(void) new QLabel( "        ILBC", codecBox );	
	codecEdit3 = new QLineEdit( qcodec, codecBox );
	codecEdit3->setMaxLength(2);
#ifdef SPEEX
	(void) new QLabel( "       Speex", codecBox );	
	codecEdit4 = new QLineEdit( fcodec, codecBox );
	codecEdit4->setMaxLength(2);
#endif	
	// Size of Payload
	bg3 = new QHButtonGroup( tr("Size of Payload, ILBC: 160,240; others: 160 (default), 80"), vbox );
	QRadioButton *s80 = new QRadioButton( "80 (10 ms Sample)", bg3 );
	bg3->insert( s80, size80 );
	QRadioButton *s160 = new QRadioButton( "160 (20 ms Sample)", bg3 );
	bg3->insert( s160, size160 );
	QRadioButton *s240 = new QRadioButton( "240 (30 ms Sample)", bg3 );
	bg3->insert( s240, size240 );
	int s=  settings.readNumEntry( KStatics::dBase+pFix+"/dsp/SizeOfPayload", 160 );
	switch(s ) {
		case 80: 
			s80->setChecked(TRUE); 
			s160->setChecked(FALSE); 
			s240->setChecked(FALSE); 
			break;
		case 160: 
			s80->setChecked(FALSE); 
			s160->setChecked(TRUE); 
			s240->setChecked(FALSE); 
			break;
		case 240: 
			s80->setChecked(FALSE); 
			s160->setChecked(FALSE); 
			s240->setChecked(TRUE); 
			break;
	}

	// Ringing tone
	rtb = new QHButtonGroup( tr("Ringtone"), vbox );
	QRadioButton *rtb0 = new QRadioButton( "No", rtb );
	rtb->insert( rtb0, rtbS0 );
	QRadioButton *rtb1 = new QRadioButton( "Yes", rtb );
				
	switch( settings.readNumEntry( KStatics::dBase+pFix+"/General/Ringtone", 0 ) ) {
		case 0: rtb0->setChecked(TRUE); break;
		case 1: rtb1->setChecked(TRUE); break;
		
	}

	//Ring devices
	
	(void) new QLabel(tr("Ringtone device for local ringing:"),  vbox );
	ringtonedevice = new QLineEdit( vbox );
	ringtonedevice->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	ringtonedevice->setText( a->getRingtonedevice() );
	(void) new QLabel(tr("application for ringing:"),  vbox );
	ringtoneapp = new QLineEdit( vbox );
	ringtoneapp->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	ringtoneapp->setText( a->getRingtoneapp() );
	(void) new QLabel(tr("Ringtone parameters:"),  vbox );
	ringtoneP = new QLineEdit( vbox );
	ringtoneP->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	ringtoneP->setText( a->getRingtoneP() );
	(void) new QLabel (tr("Ringtone sound file:"),  vbox );
	ringtonefile = new QLineEdit( vbox );
	ringtonefile->setMinimumWidth( fontMetrics().maxWidth() * 20 );
	ringtonefile->setText( a->getRingtonefile() );
	
	QHBoxLayout *buttonBox;
	buttonBox = new QHBoxLayout( vboxl, 6 ); 
	okPushButton = new QPushButton( this, tr("ok button") );
	okPushButton->setText( "OK" );
	okPushButton->setDefault( TRUE );
	buttonBox->addWidget( okPushButton );
	cancelPushButton = new QPushButton( this, tr("cancel button") );
	cancelPushButton->setText( "Cancel" );
	cancelPushButton->setAccel( Key_Escape );
	buttonBox->addWidget( cancelPushButton );
	connect( okPushButton, SIGNAL( clicked() ), this, SLOT( slotOk() ) );
	connect( cancelPushButton, SIGNAL( clicked() ), this, SLOT( slotCancel() ) );
}

AudioControl::~AudioControl( void )
{
}

void AudioControl::ossButtonClicked( void )
{
        QSettings settings;
	a->setAudioSys( SessionControl::isOSS );
	ossButton->setChecked(TRUE);
	rowoButton->setEnabled(TRUE);
	rwButton->setEnabled(TRUE);
	bg2->setEnabled(TRUE);
	dspdev->setEnabled(TRUE);
	dspdev->setText( a->getOSSFilename() );
	QString mode = settings.readEntry( KStatics::dBase+pFix+"/Audio/mode", "readonly_writeonly" );
	if( mode == "readwrite" ) {
		rwButton->setChecked(TRUE);
		rowoButton->setChecked(FALSE);
		dspdev2->setEnabled(FALSE);
	} else {
		rwButton->setChecked(FALSE);
		rowoButton->setChecked(TRUE);
		dspdev2->setEnabled(TRUE);
		dspdev2->setText( a->getOSSFilename2() );
	}
}

void AudioControl::alsaButtonClicked( void )
{
	bg2->setEnabled(FALSE);
	dspdev->setEnabled(TRUE);
	dspdev2->setEnabled(FALSE);
	dspdev->setText( a->getALSAFilename() );
	
}





void AudioControl::rwButtonClicked( void )
{
	dspdev2->setEnabled(FALSE);
	
}

void AudioControl::rowoButtonClicked( void )
{
	dspdev2->setEnabled(TRUE);
}


void AudioControl::slotOk( void )
{
	slotApply();
	QDialog::accept();
}

void AudioControl::slotApply( void )
{
	bool ok;
	QSettings settings;
#ifdef SPEEX
	a->initCodecAudio( codecEdit0->text().toInt(&ok,10),codecEdit1->text().toInt(&ok,10),codecEdit2->text().toInt(&ok,10),codecEdit3->text().toInt(&ok,10),codecEdit4->text().toInt(&ok,10) );
#else	
	a->initCodecAudio( codecEdit0->text().toInt(&ok,10),codecEdit1->text().toInt(&ok,10),codecEdit2->text().toInt(&ok,10),codecEdit3->text().toInt(&ok,10), -1 );
#endif
	switch( bg2->id( bg2->selected() ) ) {
		case rw:
			settings.writeEntry( KStatics::dBase+pFix+"/Audio/mode", "readwrite");
			a->setAudioRW(true);
			break;
		case rowo:
			settings.writeEntry( KStatics::dBase+pFix+"/Audio/mode", "readonly_writeonly");
			a->setAudioRW(false);
			break;
	}
	
	switch( bg4->id( bg4->selected() ) ) {
		case alsa:
			settings.writeEntry( KStatics::dBase+pFix+"/Audio/audio", "alsa");
			a->setAudioSys( SessionControl::isALSA );
			audioMode = "alsa";
			break;
		default:
			settings.writeEntry( KStatics::dBase+pFix+"/Audio/audio", "oss");
			a->setAudioSys( SessionControl::isOSS );
			audioMode = "oss";

			break;
	}
	
	int payload;
	switch( bg3->id( bg3->selected() ) ) {
		case size80:
			payload = 80;
			break;
		case size160:
			payload = 160;
			break;
		case size240:
			payload = 240;
			break;
		default:
			payload = 240;
	}
	a->setPayload( payload );
	settings.writeEntry( KStatics::dBase+pFix+"/dsp/SizeOfPayload", payload );
	int rtbSel;
switch( rtb->id( rtb->selected() ) ) {	
	case 0:
		rtbSel=0;
		break;
	case 1:
		rtbSel=1;
		break;
	default:
		rtbSel=1;
		break;
}	
	a->setRinging( rtbSel );
	settings.writeEntry( KStatics::dBase+pFix+"/General/Ringtone",rtbSel );	
	QString of1 = dspdev->text();
	QString of2 = dspdev2->text();
	QString rf  = ringtonefile->text();
	QString rp  = ringtoneP->text();
	QString ra  = ringtoneapp->text();
	QString rd  = ringtonedevice->text();
	
	if(audioMode == "alsa") {
		a->setALSAFilename( of1 );
		settings.writeEntry( KStatics::dBase+pFix+"/Audio/alsa-filename", of1 );
	} else {
		a->setOSSFilename( of1 );
		settings.writeEntry( KStatics::dBase+pFix+"/Audio/oss-filename", of1 );
		a->setOSSFilename2( of2);
		settings.writeEntry( KStatics::dBase+pFix+"/Audio/oss-filename2", of2);
	}
	a->setRingtonefile( rf );
	settings.writeEntry( KStatics::dBase+pFix+"/Audio/ringtone-file", rf );
	a->setRingtoneapp( ra );
	settings.writeEntry( KStatics::dBase+pFix+"/Audio/ringtone-application", ra );	
	a->setRingtonedevice(rd );
	settings.writeEntry( KStatics::dBase+pFix+"/Audio/ringtone-device",rd);	
	a->setRingtoneP( rp );
	settings.writeEntry( KStatics::dBase+pFix+"/Audio/ringtone-parameters" ,rp );	
}

void AudioControl::slotCancel( void )
{
	dspdev->setText( a->getOSSFilename() );
	dspdev2->setText( a->getOSSFilename2() );
	ringtoneP->setText( a->getRingtoneP() );
	ringtoneapp->setText(a->getRingtoneapp() );
	ringtonedevice->setText( a->getRingtonedevice() );
	ringtonefile->setText( a->getRingtonefile() );
	
	QDialog::reject();
}

