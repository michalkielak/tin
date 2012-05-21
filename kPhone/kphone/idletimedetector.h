// C++ Interface: idletimedetector
//
// Description: Ask the screensaver extension if the user has been idle. 
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

#ifndef IDLETIMEDETECTOR_H
#define IDLETIMEDETECTOR_H

#include <qobject.h>
#include <qdatetime.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>

//
class QTimer;

// Seconds per minute
const int secsPerMinute = 60;

// Minutes between each idle overrun test.
// QTimer takes microseconds.  This is a 1 sec test interval.
const int testInterval= 1000;

/**
*@short activity check for the phone
*
*The class calculates user activities on the phone, used for presence
*/
class IdleTimeDetector: public QObject
{
Q_OBJECT

public:
/**
*the constructor
*/
	IdleTimeDetector(int maxIdle);
/**
*	we are able to detect  the idle time
*/
	bool isIdleDetectionPossible();
signals:
/**
*	really used?
*/
//	void extractTime(int minutes);
/**
*	really used?
*/
//	void stopAllTimers();
/**
*	really used?
*/
//	void stopAllTimersAt(QDateTime qdt);
signals:

/**
*	emits if the user is seen as idle
*/
	void userIdle(const QDateTime qdt);
/**
*	emits if the user is seen as active
*/
	void userActive(const QDateTime qdt);

public slots:

/**
*	set the time intervall for the detection algoritm
*/
	void setMaxIdle(int maxIdle);
/**
*	start the thing
*/
	void startIdleDetection();
/**
*	stop it
*/
	void stopIdleDetection();
/**
*	pause detection
*/
	void toggleOverAllIdleDetection(bool on);

protected:
/**
*
*/
	void informOverrun(int idle);
/**
*
*/
	void informActive(int idle);

protected slots:

/**
*
*/
	void check();

private:

/**
*
*/
	QDateTime getQDT(int offset);
	XScreenSaverInfo *_mit_info;
	bool _idleDetectionPossible;
	bool _overAllIdleDetect;
	int _maxIdle;
	int _minutesActive;
	QTimer *_timer;
};
#endif
