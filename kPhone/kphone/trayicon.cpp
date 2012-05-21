/****************************************************************************
   Copyright (C) 2003-2004, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*****************************************************************************/

#include "trayicon.h"

#include <qtooltip.h>
#include <qbitmap.h>
#include <qimage.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
// System Tray Protocol Specification opcodes.
#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2


//----------------------------------------------------------------------------
// TrayIcon -- Custom system tray widget.

// Constructor.
TrayIcon::TrayIcon ( QWidget *pParent, const char *pszName, const QPixmap &pm, const char *pszLabel = NULL)
    : QLabel(pParent, pszName, WMouseNoMask | WRepaintNoErase | WType_TopLevel | WStyle_Customize | WStyle_NoBorder | WStyle_StaysOnTop)
{
    QLabel::setMinimumSize(22, 22);
    QLabel::setBackgroundMode(Qt::X11ParentRelative);
    QLabel::setBackgroundOrigin(QWidget::WindowOrigin);

    Display *dpy = qt_xdisplay();
    WId trayWin  = winId();

    // System Tray Protocol Specification.
    Screen *screen = XDefaultScreenOfDisplay(dpy);
    int iScreen = XScreenNumberOfScreen(screen);
    char szAtom[32];
    snprintf(szAtom, sizeof(szAtom), "_NET_SYSTEM_TRAY_S%d", iScreen);
    Atom selectionAtom = XInternAtom(dpy, szAtom, false);
    XGrabServer(dpy);
    Window managerWin = XGetSelectionOwner(dpy, selectionAtom);
    if (managerWin != None)
        XSelectInput(dpy, managerWin, StructureNotifyMask);
    XUngrabServer(dpy);
    XFlush(dpy);
    if (managerWin != None) {
        XEvent ev;
        memset(&ev, 0, sizeof(ev));
        ev.xclient.type = ClientMessage;
        ev.xclient.window = managerWin;
        ev.xclient.message_type = XInternAtom(dpy, "_NET_SYSTEM_TRAY_OPCODE", false);
        ev.xclient.format = 32;
        ev.xclient.data.l[0] = CurrentTime;
        ev.xclient.data.l[1] = SYSTEM_TRAY_REQUEST_DOCK;
        ev.xclient.data.l[2] = trayWin;
        ev.xclient.data.l[3] = 0;
        ev.xclient.data.l[4] = 0;
        XSendEvent(dpy, managerWin, false, NoEventMask, &ev);
        XSync(dpy, false);
    }

    Atom trayAtom;
    // For older KDE's (hopefully)...
    int data = 1;
    trayAtom = XInternAtom(dpy, "KWM_DOCKWINDOW", false);
    XChangeProperty(dpy, trayWin, trayAtom, trayAtom, 32, PropModeReplace, (unsigned char *) &data, 1);
    // For not so older KDE's...
    WId forWin = pParent ? pParent->topLevelWidget()->winId() : qt_xrootwin();
    trayAtom = XInternAtom(dpy, "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR", false);
    XChangeProperty(dpy, trayWin, trayAtom, XA_WINDOW, 32, PropModeReplace, (unsigned char *) &forWin, 1);

    setPixmap(pm);
    if (pszLabel) {
        QToolTip::add(this, pszLabel);
    }
}


// Default destructor.
TrayIcon::~TrayIcon (void)
{
}

// Inherited mouse event.
void TrayIcon::mousePressEvent ( QMouseEvent *pMouseEvent )
{
    if (!QLabel::rect().contains(pMouseEvent->pos()))
        return;

    switch (pMouseEvent->button()) {

      case LeftButton:
        // Toggle parent widget visibility.
        emit clicked();
        break;

      case RightButton:
        // Just signal we're on to context menu.
        emit contextMenuRequested(pMouseEvent->globalPos());
        break;

      default:
        break;
    }
}
