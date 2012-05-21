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
#ifndef CALLLISTVIEW_H_INCLUDED
#define CALLLISTVIEW_H_INCLUDED

#include <qptrlist.h>
#include <qlistview.h>

#include "../dissipate2/sipcall.h"
#include "menulistview.h"
#include "qcombobox.h"

class SipCall;
class SipClient;
class SessionControl;
class KPhoneView;
class QPainter;
class QColorGroup;

/**
* @short used for presence visualisation 
*
* visualizes the buddy states 
*/
class ContactsListViewItem : public QListViewItem
{
public:
	enum painter {
	    noPaint,
	    paintRed,
	    paintGreen,
	    paintBlack
	} Painter;
	ContactsListViewItem(enum painter subscribe, SipCall *c, QListView *parent );
	~ContactsListViewItem( void );

	virtual QString text( int col ) const;
	virtual void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );

	SipCall *getCall( void ) { return call; }


private:
	int  isSubscribe;
	SipCall *call;
};

/**
* @short used for presence visualisation 
*
  Sub window of KPhoneView, each line presents the state of one buddy
*
*/
class ContactsListView : public MenuListView
{
	Q_OBJECT
public:
	ContactsListView( SipClient *client, QWidget *parent = 0, const char *name = 0 );
	~ContactsListView( void );

public slots:
	void auditList( void );

private:
	SipClient *c;
	KPhoneView *v;
};


#endif // CALLLISTVIEW_H_INCLUDED
