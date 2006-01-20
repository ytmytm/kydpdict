/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qtooltip.h>
#include <qimage.h>
#include <qpixmap.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "../icons/babelfish.xpm"

#include "dock_widget.h"
#include "dock_widget.moc"

extern Time qt_x_time;

static XErrorHandler old_handler = 0;
static int dock_xerror = 0;

static int dock_xerrhandler(Display* dpy, XErrorEvent* err)
{
    dock_xerror = err->error_code;
    return old_handler(dpy, err);
}

static void trap_errors()
{
    dock_xerror = 0;
    old_handler = XSetErrorHandler(dock_xerrhandler);
}

static bool untrap_errors()
{
    XSetErrorHandler(old_handler);
    return (dock_xerror == 0);
}

static bool send_message(
    Display* dpy, /* display */
    Window w,     /* sender (tray icon window) */
    long message, /* message opcode */
    long data1,   /* message data 1 */
    long data2,   /* message data 2 */
    long data3    /* message data 3 */
)
{
    XEvent ev;

    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = w;
    ev.xclient.message_type = XInternAtom (dpy, "_NET_SYSTEM_TRAY_OPCODE", False );
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = CurrentTime;
    ev.xclient.data.l[1] = message;
    ev.xclient.data.l[2] = data1;
    ev.xclient.data.l[3] = data2;
    ev.xclient.data.l[4] = data3;

    trap_errors();
    XSendEvent(dpy, w, False, NoEventMask, &ev);
    XSync(dpy, False);
    return untrap_errors();
};

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

TrayIcon::TrayIcon(QWidget *parent, const char *name)
	: QLabel(0,"TrayIcon", WMouseNoMask | WRepaintNoErase | WType_TopLevel | WStyle_Customize | WStyle_NoBorder | WStyle_StaysOnTop)
{

	QPixmap pix = QPixmap(babelfish_xpm);
	setBackgroundMode(X11ParentRelative);
	setMinimumSize(pix.size());
	QLabel::setPixmap(pix);
	resize(pix.size());
	setMouseTracking(true);
	QToolTip::add(this, tr("Left click - hide/show window\nRight click - open menu"));
	update();

	Display *dsp = x11Display();
	WId win = winId();

	// Okno potrzebne dla WindowMakera, ale przydaje
	// si� te� w KDE 3.1, gdzie je�li chocia� na chwil�
	// nie poka�emy okna g��wnego na ekranie wyst�puje
	// "efekt klepsydry"
	WMakerMasterWidget=new QWidget(0,"WMakerMasterWidget");
	WMakerMasterWidget->setGeometry(-20,-20,1,1);

	// SPOS�B PIERWSZY
	// System Tray Protocol Specification
	// Dzia�a pod KDE 3.1 i GNOME 2.x
	Screen *screen = XDefaultScreenOfDisplay(dsp);
	int screen_id = XScreenNumberOfScreen(screen);
	char buf[32];
	snprintf(buf, sizeof(buf), "_NET_SYSTEM_TRAY_S%d", screen_id);
	Atom selection_atom = XInternAtom(dsp, buf, false);
	XGrabServer(dsp);
	Window manager_window = XGetSelectionOwner(dsp, selection_atom);
	if (manager_window != None)
		XSelectInput(dsp, manager_window, StructureNotifyMask);
	XUngrabServer(dsp);
	XFlush(dsp);
	if (manager_window != None)
		send_message(dsp, manager_window, SYSTEM_TRAY_REQUEST_DOCK, win, 0, 0);

	// SPOS�B DRUGI
	// Dzia�a na KDE 3.0.x i pewnie na starszych
	// oraz pod GNOME 1.x
	int r;
	int data = 1;
	r = XInternAtom(dsp, "KWM_DOCKWINDOW", false);
	XChangeProperty(dsp, win, r, r, 32, 0, (uchar *)&data, 1);
	r = XInternAtom(dsp, "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR", false);
	XChangeProperty(dsp, win, r, XA_WINDOW, 32, 0, (uchar *)&data, 1);

	// SPOS�B TRZECI
	// Dzia�a pod Window Makerem
	WId w_id = WMakerMasterWidget->winId();
	XWMHints *hints;
	hints = XGetWMHints(dsp, w_id);
	hints->icon_window = win;
	hints->window_group = w_id;
	hints->flags |= WindowGroupHint | IconWindowHint;
	XSetWMHints(dsp, w_id, hints);
	XFree( hints );

	setPixmap(pix);
};

TrayIcon::~TrayIcon()
{
	delete WMakerMasterWidget;
}

QPoint TrayIcon::trayPosition()
{
	return mapToGlobal(QPoint(0,0));
};

void TrayIcon::show()
{
	QLabel::show();
	// Je�li WindowMaker nie jest aktywny okno
	// nie powinno zosta� widoczne
	if(XInternAtom(x11Display(),"_WINDOWMAKER_WM_PROTOCOLS",true)==0) {
		WMakerMasterWidget->hide();
		return;
	}
//	WMakerMasterWidget->setGeometry(-20,-20,1,1);
	WMakerMasterWidget->show();
	WMakerMasterWidget->setGeometry(-20,-20,1,1);
};

void TrayIcon::setPixmap(const QPixmap& pixmap)
{
	QLabel::setPixmap(pixmap);
	WMakerMasterWidget->setIcon(pixmap);
	repaint();
};

void TrayIcon::resizeEvent(QResizeEvent* e)
{
	resize(size());
};

void TrayIcon::enterEvent(QEvent* e)
{
	if (!qApp->focusWidget())
	{
		XEvent ev;
		memset(&ev, 0, sizeof(ev));
		ev.xfocus.display = qt_xdisplay();
		ev.xfocus.type = FocusIn;
		ev.xfocus.window = winId();
		ev.xfocus.mode = NotifyNormal;
		ev.xfocus.detail = NotifyAncestor;
		Time time = qt_x_time;
		qt_x_time = 1;
		qApp->x11ProcessEvent( &ev );
		qt_x_time = time;
	};
	QWidget::enterEvent(e);
};

void TrayIcon::mousePressEvent(QMouseEvent * e) {

	if (e->button() == MidButton) {
		emit mousePressMidButton();
		return;
	}

	if (e->button() == LeftButton) {
		emit mousePressLeftButton();
		return;
	}

	if (e->button() == RightButton) {
		emit mousePressRightButton();
		return;
	}
}

TrayIcon *trayicon = NULL;
