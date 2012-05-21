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

#include "watcherlist.h"

#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qpushbutton.h>


WatcherList::WatcherList(SipCall *c,SipCallMember *cm)
    : QWidget( 0, 0 , false)
{
call = c;  
  QGridLayout * g = new QGridLayout( this, 2, 1, 0);

    l = new QListBox( this );
    l->setFocusPolicy( QWidget::StrongFocus );
    g->addWidget( l, 1, 0 );
    QVBoxLayout * v = new QVBoxLayout;
    g->addLayout( v, 2, 0 );
    QPushButton *pb = new QPushButton( "Refresh", this );
    connect( pb, SIGNAL( clicked() ), this, SLOT( refresh() ) );
    v->addWidget( pb );
    refresh();
}

WatcherList::~WatcherList()
{

}

void WatcherList::refresh()
{
	l->clear();
	WatcherInfoIterator it(call->getWIterator() );
	it.toFirst();
	for( it.toFirst(); it.current(); ++it ) {
	//WatcherInfo *wa = call->getWIterator() ;
 	l->insertItem( QString::fromLatin1(it.current()->uri ) + " " + QString::fromLatin1(it.current()->state ));
	}
}

