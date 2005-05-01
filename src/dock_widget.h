
#ifndef DOCKAPP_H
#define DOCKAPP_H

#include <qlabel.h>

class QTextBrowser;
class QStringList;
class QPixmap;

/**
	Klasa reprezentuj±ca ikonkê w obszarze powiadamiania
**/
class TrayIcon : public QLabel
{
	Q_OBJECT

	private:
		friend class TrayHint;
		QWidget* WMakerMasterWidget;
		TrayHint *hint;

	protected:
		virtual void resizeEvent(QResizeEvent* e);
		virtual void enterEvent(QEvent* e);
		virtual void mousePressEvent(QMouseEvent*);

	public:
		TrayIcon(QWidget *parent = 0, const char *name = 0);
		~TrayIcon();
		QPoint trayPosition();
		void show();
		void showHint(const QString&, const QString&, int index);
		void showErrorHint(const QString&);
		void reconfigHint();
		void setPixmap(const QPixmap& pixmap);

	signals:
		void mousePressMidButton();
		void mousePressLeftButton();
		void mousePressRightButton();
};

class TrayHint : public QWidget
{
	Q_OBJECT
	
	private:
		QTimer *hint_timer;
		QTextBrowser *hint;
		QStringList hint_list;

	public:
		TrayHint(QWidget *parent=0, const char *name = 0);
		void show_hint(const QString&, const QString&, int index);
		void restart();
		
	public slots:
		void remove_hint();

	private slots:
		void set_hint();
};

extern TrayIcon *trayicon;

#endif
