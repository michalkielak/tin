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
#include <qsettings.h>
#include <qdialog.h>

#include "../dissipate2/sipclient.h"
#include "sessioncontrol.h"
#include "kcallview.h"
#include "kphoneview.h"
#include "kcallwidget.h"

KCallViewItem::KCallViewItem( KCallWidget *c, QListView *parent)
	: QListViewItem( parent ), call( c )
{

}

KCallViewItem::~KCallViewItem( void )
{
}

void KCallViewItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
{

	QColorGroup _cg( cg );
	QColor c = _cg.text();
	
	    if(call->getCS()== "Trying") {
		    _cg.setColor( QColorGroup::Text, Qt::green ); 
	    } else   if(call->getCS()== "Invitation received"){
		    _cg.setColor( QColorGroup::Text, Qt::red ); 
	    } else {
		    _cg.setColor( QColorGroup::Text, Qt::black ); 
	    }

	QListViewItem::paintCell( p, _cg, column, width, alignment );
	 _cg.setColor( QColorGroup::Text, c );
}

QString KCallViewItem::text( int col ) const
{
	switch( col ) {
		case 0: 

			return (call->getCon() + "  CS:<" +call->getCS() + ">  LT:<" + call->getLT() +  "> Refer Notification: " + call->refnoti);
			break;
		default:
			break;
	}
	return "-";
	
}


KCallView::KCallView( KPhoneView *theParent, QWidget *parent, const char *name )
	: MenuListView( parent, name )
{
	papa=theParent;
	addColumn( tr("Session Info:         CallState   LoadType     Refer Notification"),640 );
	setAllColumnsShowFocus( TRUE );
	setMultiSelection( FALSE );
	auditList();

}

KCallView::~KCallView( void )
{
}

void KCallView::auditList( void )
{
	clear();
	widgetIterator itw (papa->getWList());
	KCallWidget *kcw;
	while ( (kcw = itw.current()) != 0 ) {
		if (!kcw->isHided()) {
		    insertItem( new KCallViewItem(kcw, this) );
		}
		++itw;
	}
}

