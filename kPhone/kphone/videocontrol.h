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
#ifndef VIDEOCONTROL_H_INCLUDED
#define VIDEOCONTROL_H_INCLUDED
#include <qdialog.h>
#include <qbutton.h>
#include <qbuttongroup.h>
#include <qhgroupbox.h>
class QLineEdit;
class QRadioButton;
class SessionControl;
class DspOutOss;

/**
* @short Provisioning for the Video Payload 
*
* Provisioning for the Video Payload 
*/
class VideoControl : public QDialog
{
	Q_OBJECT
public:
	VideoControl( SessionControl *sessioncontrol, QWidget *parent = 0, const char *name = 0 );
	~VideoControl( void );

protected slots:
	void slotOk( void );
	void slotApply( void );
	void slotCancel( void );

private:
	enum Codecs { h261, h263, h263p };
	QHGroupBox *codecBox;
	QLineEdit *codecEdit0;
	QLineEdit *codecEdit1;
	QLineEdit *codecEdit2;
	SessionControl *a;
	QLineEdit *videoSW;
	enum Holdtoggle { holdYes , holdNo};
	enum Usesoundtoggle { usesoundYes , usesoundNo};
	enum Transfertoggle { transferYes , transferNo};
	QButtonGroup *holdBG;
	QButtonGroup *transferBG;
	QButtonGroup *usesoundBG;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
	QString p;
};

#endif // VIDEOCONTROL_H_INCLUDED
