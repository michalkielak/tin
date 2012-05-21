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
#ifndef AUDIOCONTROL_H_INCLUDED
#define AUDIOCONTROL_H_INCLUDED
#include <qdialog.h>
#include <qbutton.h>
#include <qbuttongroup.h>
#include <qhgroupbox.h>

class QLineEdit;
class QRadioButton;
class SessionControl;
class DspOutOss;

/**
* @short Provisioning for the Audio Payload 
*
* Provisioning for the audio call's payload 
*/
class AudioControl : public QDialog
{
	Q_OBJECT
public:
	AudioControl( SessionControl *sessioncontrol, QWidget *parent = 0, const char *name = 0 );
	~AudioControl( void );

protected slots:
	void ossButtonClicked( void );
	void alsaButtonClicked( void );
	void rwButtonClicked( void );
	void rowoButtonClicked( void );
	void slotOk( void );
	void slotApply( void );
	void slotCancel( void );

private:
	QString audioMode;
	enum audiomode { oss, alsa };
	enum rdwr { rw, rowo };
	enum Codecs { ulaw,alaw, gsm, ilbc };
	enum SizeOfPayload { size80, size160,size240 };
	enum RingToneList {rtbS0, rtbS1, rtbS2, rtbS3, rtbS4, rtbS5 };
	enum Ringingtone { ringingtoneYes, ringingtoneNo };
	QButtonGroup *bg0;
	QButtonGroup *bg2;
	QButtonGroup *bg3;
	QButtonGroup *bg4;
	QButtonGroup *rtb;
	QHGroupBox *codecBox;
	QLineEdit *codecEdit0;
	QLineEdit *codecEdit1;
	QLineEdit *codecEdit2;
	QLineEdit *codecEdit3;
	QLineEdit *codecEdit4;
	SessionControl *a;
	QLineEdit *dspdev;
	QLineEdit *dspdev2;
	QLineEdit *ringtonedevice;
	QLineEdit *ringtoneapp;
	QLineEdit *ringtonefile;
	QLineEdit *ringtoneP;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
	QRadioButton *rwButton;
	QRadioButton *rowoButton;
	QRadioButton *ossButton;
	QRadioButton *alsaButton;
	QString pFix;
	int debug;
};

#endif // AUDIOCONTROL_H_INCLUDED
