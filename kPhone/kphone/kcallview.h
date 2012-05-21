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
#ifndef KCALLVIEW_H_INCLUDED
#define KCALLVIEW_H_INCLUDED

#include <qptrlist.h>
#include <qlistview.h>
#include "kcallwidget.h"
#include "menulistview.h"
#include "qcombobox.h"
#include "../dissipate2/sipcall.h"
#include "kphoneview.h"
class SessionControl;
class KPhoneView;
class QPainter;
class QColorGroup;

/**
* @short used for call visualisation 
*
* Paints a line with call properties into KCallView 
*/
class KCallViewItem : public QListViewItem
{
public:
	KCallViewItem(  KCallWidget *c, QListView *parent );
	~KCallViewItem( void );
	virtual QString text( int col ) const;
	virtual void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
KCallWidget *call;
private:
};
/**
* @short used for call visualisation
*
* Sub window of KPhoneView, each line presents a ongoing call
*/
class KCallView : public MenuListView
{
	Q_OBJECT
public:
	KCallView( KPhoneView *theParent, QWidget *parent = 0, const char *name = 0 );
	~KCallView( void );

public slots:
	void auditList( void);

private:
KPhoneView *papa;
};


#endif // KCALLVIEW_H_INCLUDED
