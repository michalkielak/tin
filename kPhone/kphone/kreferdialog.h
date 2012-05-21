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
#ifndef REFERDIALOG_H_INCLUDED
#define REFERDIALOG_H_INCLUDED

#include <qdialog.h>
#include <qptrdict.h>
#include <qevent.h>
#include <qcombobox.h>
#include <time.h>

#include "../dissipate2/sipuri.h"
#include "../dissipate2/sdpbuild.h"
#include "../dissipate2/sipmessage.h"
#include "../dissipate2/sipcall.h"
#include "kphonebook.h"




class QLabel;
class QLineEdit;
class KComboBox;
class QPushButton;
class QTextEdit;
class SipUser;
class SipCall;
class SipCallMember;
class SipTransaction;
class SessionControl;
class IncomingCall;
class SipMessage;
class KSipAuthentication;
class KPhoneView;


/**
* @short the graphic representation of the CT Widget
*
* the graphic representation of the CT dialog
*/
class KReferDialog : public QDialog
{
	Q_OBJECT
public:
	KReferDialog( KPhoneView * phv, int kcwn ,QString uPrefix,QWidget *parent = 0, const char *name = 0 );
	~KReferDialog( void );

signals:
	void newSessionRefer( const QString & );

protected slots:
	void slotOk( void );
	void slotCancel( void );
	void getUri( void );
private:
	QPtrList<IncomingCall> receivedCalls;
	QPtrList<IncomingCall> missedCalls;
	QPtrList<IncomingCall> dialledCalls;
	IncomingCall *incomingCall;
	QString userPrefix;
	KPhoneView *phoneView;
	QLineEdit *touri;
	QPushButton *helpPushButton;
	QPushButton *okPushButton;
	QPushButton *cancelPushButton;
	QPushButton *loadUri;
	int kcwNumber;
	PhoneBook *phoneBook;
};

#endif // REFERDIALOG_H_INCLUDED
