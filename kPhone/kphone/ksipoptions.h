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
#ifndef KSIPOPTIONS_H_INCLUDED
#define KSIPOPTIONS_H_INCLUDED

#include <qdialog.h>

#include "../dissipate2/sipclient.h"

class SipUser;
class SipCall;
class SipCallMember;
class QLineEdit;
class QMultiLineEdit;
/**
* @short Experimental stuff 
*/
class KSipOptions : public QDialog
{
	Q_OBJECT
public:
	KSipOptions( SipUser *user, QWidget *parent = 0, const char *name = 0 );
	~KSipOptions( void );

private slots:
	void queryOptions( void );
	void optionsCallStatusUpdated( void );

private:
	SipUser *u;
	SipCall *call;
	SipCallMember *member;
	QLineEdit *queryuri;
	QMultiLineEdit *qresults;
};

/**
* @short Experimental stuff 
*/
class KTestOptions : public QDialog
{
	Q_OBJECT
public:
	KTestOptions( SipClient *client );
	~KTestOptions( void );

public slots:
	void testSendMessage( void );

protected slots:
	void slotOk( void );

private slots:
	void testClean( void );
	void testFile( void );
	void testSend( void );

private:
	SipClient *c;
	QLineEdit *addr;
	QLineEdit *port;
	QMultiLineEdit *msg;
	QString testStr;
};


#endif // KSIPOPTIONS_H_INCLUDED
