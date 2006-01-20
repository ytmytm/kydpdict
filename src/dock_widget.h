
#ifndef DOCKAPP_H
#define DOCKAPP_H

#include <qlabel.h>

class QPixmap;

/**
	Klasa reprezentuj±ca ikonkê w obszarze powiadamiania
**/
class TrayIcon : public QLabel
{
	Q_OBJECT

	private:
		QWidget* WMakerMasterWidget;

	protected:
		virtual void resizeEvent(QResizeEvent* e);
		virtual void enterEvent(QEvent* e);
		virtual void mousePressEvent(QMouseEvent*);

	public:
		TrayIcon(QWidget *parent = 0, const char *name = 0);
		~TrayIcon();
		QPoint trayPosition();
		void show();
		void setPixmap(const QPixmap& pixmap);

	signals:
		void mousePressMidButton();
		void mousePressLeftButton();
		void mousePressRightButton();
};

extern TrayIcon *trayicon;

#endif
