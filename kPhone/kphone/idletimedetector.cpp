//
// C++ Implementation: idletimedetector 
//
// Description: 
//
//
// Author:  <mflacy@verizon.net>, (C) 2007
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
// License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; see the file COPYING.LIB.  If not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.
//
//
#include <stdio.h>
#include "idletimedetector.h"
#include "kstatics.h"

#include <qdatetime.h>
#include <qtimer.h>
#include <iostream>
using namespace std;

IdleTimeDetector::IdleTimeDetector(int maxIdle) : _mit_info(0), _idleDetectionPossible(false), _overAllIdleDetect(true), _maxIdle(maxIdle), _minutesActive(0), _timer(new QTimer(this))
// Trigger a warning after maxIdle minutes
{
	int event_base, error_base;
	_idleDetectionPossible = (XScreenSaverQueryExtension(qt_xdisplay(), &event_base, &error_base) != 0);
	if (KStatics::debugLevel >= 2) cout << "******** IdleTimeDetector" <<  (_idleDetectionPossible ? "can" : "cannot") << " detect keyboard idle.  Idle time is " << _maxIdle << "seconds.\n";
	connect(_timer, SIGNAL(timeout()), this, SLOT(check()));
}

bool IdleTimeDetector::isIdleDetectionPossible()
{
	return (_idleDetectionPossible && _maxIdle > 0);
}

void IdleTimeDetector::check()
{
	if (_idleDetectionPossible)
	{
		_mit_info = XScreenSaverAllocInfo ();
		XScreenSaverQueryInfo(qt_xdisplay(), qt_xrootwin(), _mit_info);
		int idleMinutes = (_mit_info->idle/1000);
		if (KStatics::debugLevel >= 2) cout << "******** IdleTimeDetector::check() idle"<< idleMinutes <<  " interv " << _maxIdle << endl;
		if (idleMinutes >= _maxIdle)
		{
			informOverrun(idleMinutes);
		}
		else
		{
			informActive(idleMinutes);
		}
	}
}

void IdleTimeDetector::setMaxIdle(int maxIdle)
{
	_maxIdle = maxIdle;
}

void IdleTimeDetector::informOverrun(int idleMinutes)
{
	if (!_overAllIdleDetect)
	{
		return;
	}
	if (KStatics::debugLevel >= 2) cout << "******** IdleTimeDetector userIdle\n";
	_timer->stop();
	userIdle( getQDT( idleMinutes ));
	_timer->start(_maxIdle * 1000);
}

void IdleTimeDetector::informActive( int idleMinutes )
{
	if (!_overAllIdleDetect )
	{
		return;
	}
	if (KStatics::debugLevel >= 2) cout << "******** IdleTimeDetector userActive\n";
	userActive( getQDT( idleMinutes ));
}

QDateTime IdleTimeDetector::getQDT( int minusMinutes )
{
	QDateTime now = QDateTime::currentDateTime( Qt::UTC );
	now = now.addSecs(-1 * minusMinutes);
	return now;
}

void IdleTimeDetector::startIdleDetection()
{
	if (KStatics::debugLevel >= 2) cout << "******** startIdleDetector msecs " << _maxIdle * 1000 << endl;
	if (!_timer->isActive())
		_timer->start(_maxIdle * 1000);
}

void IdleTimeDetector::stopIdleDetection()
{
	if (_timer->isActive())
		_timer->stop();
}

void IdleTimeDetector::toggleOverAllIdleDetection(bool on)
{
	_overAllIdleDetect = on;
}

