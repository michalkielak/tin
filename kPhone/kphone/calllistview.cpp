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
#include "../dissipate2/sipclient.h"
#include "sessioncontrol.h"
#include "calllistview.h"
#include "../config.h"
ContactsListViewItem::ContactsListViewItem( enum painter subscribe, SipCall *c, QListView *parent )
	: QListViewItem( parent ), call( c )
{
	isSubscribe = subscribe;
	QObject::connect( call, SIGNAL( callStatusUpdated() ), listView(), SLOT( triggerUpdate() ) );
}

ContactsListViewItem::~ContactsListViewItem( void )
{
}

QString ContactsListViewItem::text( int col ) const
{
	switch( col ) {
		case 0: // Status
			return call->getTrPresenceStatus();
			break;
	#ifdef PRESACT
		case 1: // Activity
			return call->getActivityStatus(); 
			break;
		case 2: // Subject
			return call->getSubject() ;
			break;
		case 3: // Description
			return call->getContactStr();
			break;
		default:
			break;
	#else
		case 1: // Subject
			return call->getSubject() ;
			break;
		case 2: // Description
			return call->getContactStr();
			break;
		default:
			break;
	
	#endif
	}
	return QString::null;
}

void ContactsListViewItem::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment )
{
	QColorGroup _cg( cg );
	QColor c = _cg.text();
	

	if( column == 0 ) { //PRESENCE != Offline
	    if(isSubscribe == paintRed) {
		    _cg.setColor( QColorGroup::Text, Qt::red ); 
	    } else if (isSubscribe == paintGreen) {
		    _cg.setColor( QColorGroup::Text, Qt::darkGreen ); 
	    } else if (isSubscribe == paintBlack) {
		    _cg.setColor( QColorGroup::Text, Qt::black ); 
	    }
	}
	QListViewItem::paintCell( p, _cg, column, width, alignment );
	 _cg.setColor( QColorGroup::Text, c );
}

ContactsListView::ContactsListView( SipClient *client, QWidget *parent, const char *name )
	: MenuListView( parent, name )
{
	c = client;
	addColumn( tr("Status"),90 );
	#ifdef PRESACT
	addColumn( tr("Activity"),40 );
	#endif
	addColumn( tr("Contact"),150 );
	addColumn( tr("Description"),248 );
//	setColumnWidth( 1, fontMetrics().maxWidth() * 9 );
	setAllColumnsShowFocus( TRUE );
	setMultiSelection( FALSE );
	auditList();
}

ContactsListView::~ContactsListView( void )
{
}

void ContactsListView::auditList( void )
{
	enum ContactsListViewItem::painter isSubscribed = ContactsListViewItem::noPaint;
	clear();
	SipCallIterator it( c->getCallList() );
	it.toFirst();
	for( it.toFirst(); it.current(); ++it ) {
		isSubscribed = ContactsListViewItem::noPaint;
		if( it.current()->getCallType() == SipCall::outSubscribeCall ) {
			if( (it.current()->getTrPresenceStatus().contains( "Offline") ) || (it.current()->getTrPresenceStatus() == ""))  isSubscribed = ContactsListViewItem::paintBlack; 
			 else isSubscribed = ContactsListViewItem::paintGreen;
			if( it.current()->getCallStatus() != SipCall::callDead ) {
				insertItem( new ContactsListViewItem(isSubscribed, it.current(), this ) );
			}
		}
	}
}
