/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KYDPDICT_H
#define KYDPDICT_H

#include <qwidget.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qclipboard.h>
#include <qmainwindow.h>
#include <qtimer.h>
#include <qsplitter.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qscrollbar.h>

#include "ydpdictionary.h"
#include "kydpconfig.h"
#include "ydpconfigure.h"
#include "kdynamictip.h"

class Kydpdict : public QMainWindow
{
  Q_OBJECT
    public:
    /** construtor */
    Kydpdict(QWidget* parent=0, const char *name=0);
    /** destructor */
    ~Kydpdict();
    void flushConfig(void);
    void showMinimized();

    private:
		void resizeEvent (QResizeEvent *);
		void moveEvent (QMoveEvent *);
		void Configure(bool status);
		void UpdateLook ();
		void SwapLang(bool direction, int lang);
		void PasteClipboard (QString haslo);
		void showEvent(QShowEvent *ashowEvent);
		void updateUserTimestamp(void);

		// global configuration
		kydpConfig *config;

		// GUI widgets
		QListBox *dictList;
		QComboBox *wordInput;
		QTextBrowser *RTFOutput;
		QPushButton *listclear;
		QMenuBar *menu;
		QScrollBar *scrollBar;
		QPopupMenu *trayMenu;
		int polToEng,engToPol, gerToPol, polToGer;
		int toolBarMenuItem, trayMenuItemClipTrack;

		// dictionary class
		ydpDictionary *myDict;

		// and others
		DynamicTip * t;
		QSplitter *splitterH, *splitterV;
		QToolBar *toolBar;
		QToolButton *but_EnPl, *but_PlEn, *but_DePl, *but_PlDe, *but_Play;
		QToolButton *but_SwapLang, *but_Clipboard, *but_Settings;

		// clipboard shit
		QClipboard  *cb;
		QTimer *m_checkTimer, *escTimer;
		QString slastSelection;
		QString slastClipboard;

    private slots:
		void NewDefinition (int index);       		/* word list highlited */
		void NewFromLine (const QString& newText);	/* new word entered */
		void PlaySelected (int index);			/* dbl click or return to play */
		void UpdateHistory (void);
		void PlayCurrent ();
		void SwapLanguages ();
		void SwapPolToEng ();
		void SwapEngToPol ();
		void SwapPolToGer ();
		void SwapGerToPol ();
		void ShowAbout ();
		void ShowAboutQt ();
		void ConfigureKydpdict ();
		void ToggleClipTracking ();
		void ToolbarShowHide (bool visible);
		void ShowToolbar ();
		void updateText( const QString & href );
		void newClipData();
		void onEscaped();

    protected:
		void windowActivationChange( bool oldActive );

};

#endif
