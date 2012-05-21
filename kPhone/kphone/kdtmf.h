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
#ifndef DTMF_H_INCLUDED
#define DTMF_H_INCLUDED

#include <qdialog.h>
#include <qptrdict.h>
#include <qevent.h>
#include <qcombobox.h>
#include "kphonebook.h"
#include "callaudio.h"



class QLabel;


/**
* @short the graphic representation of the DTMF pad
*
The kepad to send DTMF tones
*/
class KDtmf : public QDialog
{
	Q_OBJECT
public:
	KDtmf( CallAudio *au, QWidget *parent = 0, const char *name = 0 );
	~KDtmf( void );
	void createKeypad(void);
signals:

protected slots:
/**
* press the button id
*/  
  void keypadPressed( int id );
/**
* release the button id
*/ 
  void keypadReleased( int id );
 
private: 
QButtonGroup *keypad;
QPushButton *showD;
QPushButton *hideD;
CallAudio *audio;
};

#endif // DTMF_H_INCLUDED
