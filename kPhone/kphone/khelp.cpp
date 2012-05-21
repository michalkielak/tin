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
#include <qsound.h>
#include "khelp.h"
#include "kphone.h"
#include "kstatics.h"


KHelp::KHelp( QWidget *parent, const char *name )
	: QDialog( parent, name, false )
{
	QVBox *vbox = new QVBox( this );
	vbox->setMargin( 3 );
	vbox->setSpacing( 3 );
	QVBoxLayout *vboxl = new QVBoxLayout( this, 5 );
	vboxl->addWidget( vbox );

	(void) new QLabel("When starting KPhoneSI for the first time, you should enter your identity information in the window appearing.\n", vbox );
	(void) new QLabel("The information required for successful sipping are 'Full Name', 'User part of SIP URL' and 'Host part of SIP URL'.\n", vbox );
	(void) new QLabel("Filling the 'Outbound Proxy' Field is recommended, possibly necessary. Study you provider's documentation for the \n", vbox );
	(void) new QLabel("details. The field should contain your proxy's IP address or that of a Media Gateway if used.\n", vbox );	
	(void) new QLabel("Mark the auto register selection if you wish KPhone SI to automatically register your identity on application start.\n", vbox );
	(void) new QLabel("If you want to use multiple identities, start KPhone SI with the '-u' option and the username of your selection.\n", vbox );
	(void) new QLabel("\n", vbox );
	(void) new QLabel("For example:  'kpsi -u jsmith'.\n" , vbox );
	(void) new QLabel("The -v <n> (n= 1, 2, 3) commandline option produces different forms of logging. \n" , vbox );
	(void) new QLabel("\n", vbox );
	(void) new QLabel("If you are behind a NAT (Network Address Translation) device, you should go to Preferences -> SIP Preferences -> Socket and\n", vbox );
	(void) new QLabel("check the 'Use STUN Server' and specify the STUN server's IP address\n", vbox );
	(void) new QLabel("\n", vbox );
	(void) new QLabel("To start a call you type the SIP-URI and then select the call type (audio, video, ...)\n", vbox );
	(void) new QLabel("A started call is displayed in the 'Session Info' window, right clicking an entry pops up a window with the controls for that call\n", vbox );
	(void) new QLabel("An incoming call shows up in the 'Session Info' window, too. It is painted red. To accept the call, double click that window \n", vbox );
	(void) new QLabel("\n", vbox );
	(void) new QLabel("You should edit the Phone Book and enter all your favorite contacts there. Of course this is not mandatory but it increases your\n", vbox );
	(void) new QLabel("efficiency with KPhoneSI.When adding contacts to your Phone Book, simply click Add and enter the person's real name, SIP identity\n", vbox );
	(void) new QLabel("and the optional description. If you double click a existing contact, the URI appears in the dial window\n", vbox );
	(void) new QLabel("\n", vbox );
	(void) new QLabel("More detailed documentation plus graphic material can be found in the html written user manual. Its available in the document \n", vbox );
	(void) new QLabel("subdirectory  of the source code.\n", vbox );

	(void) new QLabel("\n", vbox );


}

KHelp::~KHelp( void )
{
}



