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
#include <qclipboard.h>
#include <qmainwindow.h>

#include <qtimer.h>
#include <qsplitter.h>

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

    private:
		void resizeEvent (QResizeEvent *);
		void moveEvent (QMoveEvent *);
		void Configure(bool status);
		void UpdateLook ();
		void SwapLang(bool direction, int lang);
		void PasteClipboard (QString haslo);

		// global configuration
		kydpConfig *config;

		// GUI widgets
		QListBox *dictList;
		QComboBox *wordInput;
		QTextBrowser *RTFOutput;
		QPushButton *listclear;
		QMenuBar  *menu;
		int polToEng,engToPol, gerToPol, polToGer;

		// dictionary class
		ydpDictionary *myDict;

		//and others
		QClipboard  *cb;
		QTimer *m_checkTimer;
		DynamicTip * t;
		QSplitter *splitterH, *splitterV;

    private slots:
		void NewDefinition (int index);       		/* word list highlited */
		void NewFromLine (const QString& newText);	/* new word entered */
		void PlaySelected (int index);			/* dbl click or return to play */
		void UpdateHistory (int index);
		void PlayCurrent ();
		void SwapPolToEng ();
		void SwapEngToPol ();
		void SwapPolToGer ();
		void SwapGerToPol ();
		void ShowAbout ();
		void ShowAboutQt ();
		void ConfigureKydpdict();
		void updateText( const QString & href );

		void slotSelectionChanged() {
			clipboardSignalArrived( true );
		}
		void slotClipboardChanged() {
			clipboardSignalArrived( false );
		}
		void newClipData();

    protected:
    		void clipboardSignalArrived( bool selectionMode );
		void checkClipData( const QString& text );

};

#endif
