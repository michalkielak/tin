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
#ifndef EXTCONTROL_H_INCLUDED
#define EXTCONTROL_H_INCLUDED
#include <qdialog.h>
#include <qbutton.h>
#include <qbuttongroup.h>

class QLineEdit;
class QRadioButton;
class SessionControl;
class DspOutOss;

/**
* @short Provisioning for the private Payload 
*
* Provisioning for the private calls' payload 
*/
class ExtControl : public QDialog
{
	Q_OBJECT
public:
	ExtControl( int lo,SessionControl *sessioncontrol, QWidget *parent = 0, const char *name = 0 );
	~ExtControl( void );

protected slots:
	void slotOk( void );
	void slotApply( void );
	void slotCancel( void );

private:
	enum Rsttoggle { rstYes , rstNo};
	enum Holdtoggle { holdYes , holdNo};
	enum Usesoundtoggle { usesoundYes , usesoundNo};
	enum Transfertoggle { transferYes , transferNo};
	enum UseL4toggle { usenone,useudp, usertp, };
	int load;
	int call;
	SessionControl *a;
	QButtonGroup *rstBG;
	QButtonGroup *holdBG;
	QButtonGroup *transferBG;
	QButtonGroup *usesoundBG;
	QButtonGroup *useL4BG;
	QLineEdit *extSW;
	QLineEdit *extPR;
	QLineEdit *extIco;
	QLineEdit *vecId;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
	QString pFix;
};

#endif // EXTCONTROL_H_INCLUDED
