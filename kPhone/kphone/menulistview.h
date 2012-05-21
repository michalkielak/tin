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
#ifndef MENULISTVIEW_H_INCLUDED
#define MENULISTVIEW_H_INCLUDED

#include <qlistview.h>
#include <qpopupmenu.h>

/**
 * @short ListView with a right-click menu.
 * @author Billy Biggs <vektor@div8.net>
 *
 * MenuListView is an extension of QListView which provides a right-click
 * popup menu. The click handling and menu are provided by MenuListView.
 */
class MenuListView : public QListView
{
	Q_OBJECT
public:

	/**
	 * Constructs a MenuListView.
	 */
	MenuListView( QWidget *parent = 0, const char *name = 0 );

	/**
	 * Destructor for MenuListView.
	 */
	~MenuListView( void );

	/**
	 * Add an item to the menu with description desc
	 * and connect it to the provided slot.
	 */
	void addMenuItem( QString desc, const QObject *receiver,
		const char *member );

	/**
	 * Inserts a separator into the menu.
	 */
	void addMenuSeparator( void );

private slots:
	void showMenu( QListViewItem *curi, const QPoint& pos, int );

private:
	QPopupMenu *ops;
};

#endif // MENULISTVIEW_H_INCLUDED
