/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qapplication.h>
#include <qcursor.h>
#include <qobject.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qimage.h>
#include <qpixmap.h>

#include "dock_widget.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "../icons/babelfish.xpm"

#define kdebug printf

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
//	if (!config.dock)
//		return;

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
	int r1=XChangeProperty(dsp, win, r, r, 32, 0, (uchar *)&data, 1);
	r = XInternAtom(dsp, "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR", false);
	int r2=XChangeProperty(dsp, win, r, XA_WINDOW, 32, 0, (uchar *)&data, 1);

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
	kdebug("TrayIcon::~TrayIcon()\n");
	delete WMakerMasterWidget;
	delete hint;
}

void TrayIcon::setDictWidget(Kydpdict *newDict) {

    dict = newDict;

}

void TrayIcon::setPopupMenu(QPopupMenu *newMenu) {

    popUp = newMenu;

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
	kdebug("show wmakermasterwidget\n");
	WMakerMasterWidget->setGeometry(-20,-20,1,1);
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

//	if (!config.dock)
//		return;

	if (e->button() == MidButton) {
		emit mousePressMidButton();
		kdebug("midbutton\n");
		return;
	}

	if (e->button() == LeftButton) {
		emit mousePressLeftButton();
		kdebug("left button\n");
		switch (dict->isVisible()) {
		    case 0:
			dict->show();
			dict->setFocus();
			dict->setActiveWindow();
			break;
		    case 1:
			dict->hide();
			break;
		    }
		}

	if (e->button() == RightButton) {
		emit mousePressRightButton();
		kdebug("right button\n");
		popUp->exec(QCursor::pos());
		return;
		}
}

/* Hint class below */
//////////////////////////////////////////////

void TrayIcon::showHint(const QString &str, const QString &nick, int index) {
//	if (!config.trayhint || !config.dock)
//		return;

	kdebug("TrayIcon::showHint()\n");
	hint->show_hint(str,nick,index);
}

void TrayIcon::reconfigHint() {
	kdebug("TrayIcon::reconfigHint()\n");
	hint->restart();
}

void TrayIcon::showErrorHint(const QString &str) {
//	if (!config.hinterror)
//		return;
	kdebug("TrayIcon::showErrorHint()\n");
	hint->show_hint(str, tr("Error: "), 1);
}

TrayHint::TrayHint(QWidget *parent, const char *name)
	: QWidget(parent,"TrayHint",WStyle_NoBorder | WStyle_StaysOnTop | WStyle_Tool | WX11BypassWM | WWinOwnDC)
{
	kdebug("TrayHint::TrayHint\n");
	
	hint = new QTextBrowser(this);
	hint->setVScrollBarMode(QScrollView::AlwaysOff);
	hint->setHScrollBarMode(QScrollView::AlwaysOff);
//	hint->setFont(config.fonts.trayhint);
//	hint->setPaletteBackgroundColor(config.colors.trayhintBg);
//	hint->setPaletteForegroundColor(config.colors.trayhintText);

	hint_timer = new QTimer(this);
	
	QObject::connect(hint_timer,SIGNAL(timeout()),this,SLOT(remove_hint()));
}

void TrayHint::set_hint(void) {
	QPoint pos_hint;
	QSize size_hint;
	QPoint pos_tray = trayicon->trayPosition();
	QString text_hint; 
	for (QStringList::Iterator points = hint_list.begin(); points != hint_list.end(); ++points)
		text_hint.append(*points);
//	size_hint = QFontMetrics(config.fonts.trayhint).size(Qt::ExpandTabs, text_hint);
//	size_hint = QSize(size_hint.width()+35,size_hint.height()+10);
//	resize(size_hint);
//	hint->resize(size_hint);
//	QSize size_desk = QApplication::desktop()->size();
//	if (pos_tray.x() < size_desk.width()/2)
//		pos_hint.setX(pos_tray.x()+32);
//	else
//		pos_hint.setX(pos_tray.x()-size_hint.width());
//	if (pos_tray.y() < size_desk.height()/2)
//		pos_hint.setY(pos_tray.y()+32);
//	else
//		pos_hint.setY(pos_tray.y()-size_hint.height());
//	move(pos_hint);
	kdebug("TrayHint::set_hint()\n");
}

void TrayHint::show_hint(const QString &str, const QString &nick, int index) {
	kdebug("TrayHint::show_hint(%s,%s,%d)\n", 
		 (const char *)str.local8Bit(), (const char *)nick.local8Bit(), index);
	if (hint_list.last() == str + nick || hint_list.last() == "\n" + str + nick)
		return;
	QString text;
	text.append("<CENTER>");
	switch(index) {
		case 0:
			text.append(str);
			text.append("<B>");
			text.append(nick);
			text.append("</B>");
			break;

		case 1:
			text.append("<B>");
			text.append(nick);
			text.append("</B>");
			text.append(str);
			break;
	}

	text.append("</CENTER></FONT>");

	if (hint->text()=="") {
		hint->setText(text);
		hint_list.append(str+nick);
		}
	else {
		hint->setText(hint->text()+"\n"+text);
		hint_list.append("\n"+str+nick);
		}
	set_hint();
	show();
//	if (!hint_timer->isActive())
//		hint_timer->start(config.hinttime * 1000);
}

void TrayHint::remove_hint() {
	int len = hint->text().find("\n");
	if ( len > 0) {
		hint->setText(hint->text().remove(0, len + 1));
		hint_list.erase(hint_list.fromLast());
		kdebug("TrayHint::remove_hint() hint_list counts=%d\n",hint_list.count());
		}
	else {
		hide();
		hint->clear();
		hint_timer->stop();
		hint_list.clear();
		kdebug("TrayHint::remove_hint() hint and hint_list is cleared\n");
		return;
	}
	set_hint();
}

void TrayHint::restart() {
	hide();
	hint->clear();
	hint_timer->stop();
	hint_list.clear();
//	hint->setFont(config.fonts.trayhint);
//	hint->setPaletteBackgroundColor(config.colors.trayhintBg);
//	hint->setPaletteForegroundColor(config.colors.trayhintText);
	kdebug("TrayHint::restart()\n");
}

TrayIcon *trayicon = NULL;
