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
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <qbuttongroup.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qgrid.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qtimer.h>
#include <qvbox.h>
#include "kdtmf.h"
#include "kphone.h"
#include "kstatics.h"

KDtmf::KDtmf( CallAudio *au, QWidget *parent, const char *name )
	: QDialog( parent, name, false )
{
	audio=au;
	QVBox *vboxb = new QVBox( this );
	vboxb->setMargin( 3 );
	vboxb->setSpacing( 3 );
	QVBoxLayout *vbox = new QVBoxLayout( this, 3 );
	vbox->addWidget( vboxb );
	
	QHBoxLayout *buttonbox;
	buttonbox = new QHBoxLayout( vbox, 3 );
	
	showD = new QPushButton(this, "Show DTMF" );
	showD->setText( "Show DTMF" );
	buttonbox->addWidget(showD );
	showD->setToggleButton(true);
	showD->setOn( true );
	showD->setEnabled( true );
	connect(showD, SIGNAL( toggled(bool) ),
		this, SLOT( showExtension(bool)));	

	createKeypad();
	
	showExtension(true);

}
void KDtmf::createKeypad(void)
{	
	QGrid* grid;
	setOrientation(Qt::Vertical);
	grid = new QGrid(3);
	grid->setMargin(5);//30
	grid->setLineWidth(1);
	grid->setFrameStyle(3);
	
	keypad = new QButtonGroup(3,Qt::Vertical);
	keypad->insert(new QPushButton("1 ", grid));
	keypad->insert(new QPushButton("2 ABC", grid));
	keypad->insert(new QPushButton("3 DEF", grid));
	keypad->insert(new QPushButton("4 GHI", grid));
	keypad->insert(new QPushButton("5 JKL", grid));
	keypad->insert(new QPushButton("6 MNO", grid));
	keypad->insert(new QPushButton("7 PQRS", grid));
	keypad->insert(new QPushButton("8 TUV", grid));
	keypad->insert(new QPushButton("9 WXYZ", grid));
	keypad->insert(new QPushButton("* ", grid));
	keypad->insert(new QPushButton("0 ", grid));
	keypad->insert(new QPushButton("# ", grid));

	connect(keypad, SIGNAL( pressed(int) ), this, SLOT( keypadPressed(int) ));
	connect(keypad, SIGNAL( released(int) ), this, SLOT( keypadReleased(int) ));
	
	setExtension (grid);

}




void KDtmf::keypadPressed( int id )
{
	int tone = 0;
	switch(id) {
		case 0: tone = 1; break;
		case 1: tone = 2; break;
		case 2: tone = 3; break;
		case 3: tone = 4; break;
		case 4: tone = 5; break;
		case 5: tone = 6; break;
		case 6: tone = 7; break;
		case 7: tone = 8; break;
		case 8: tone = 9; break;
		case 9: tone = 10;break;
		case 10:tone = 0; break;
		case 11:tone = 11;break;
	}
	if (audio) {
		audio->startDTMF(tone);
	}
}

void KDtmf::keypadReleased( int id )
{
	if (audio) audio->stopDTMF();
}


KDtmf::~KDtmf( void )
{
}



