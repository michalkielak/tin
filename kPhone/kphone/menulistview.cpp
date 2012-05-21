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
#include <menulistview.h>

MenuListView::MenuListView( QWidget *parent, const char *name )
	: QListView( parent, name )
{
	connect( this, SIGNAL( rightButtonPressed( QListViewItem *, const QPoint&, int ) ),
		this, SLOT( showMenu( QListViewItem *, const QPoint&, int ) ) );

	ops = new QPopupMenu( this );
}

MenuListView::~MenuListView( void )
{
}

void MenuListView::addMenuItem( QString desc, const QObject *receiver, const char *member )
{
	ops->insertItem( desc, receiver, member );
}

void MenuListView::addMenuSeparator( void )
{
	ops->insertSeparator();
}

void MenuListView::showMenu( QListViewItem *curi, const QPoint& pos, int )
{
	setCurrentItem( curi );
	setSelected( curi, TRUE );
	ops->popup( pos );
}
