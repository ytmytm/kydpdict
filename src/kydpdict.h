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
#include <qlineedit.h>
#include <qmenubar.h>
#include <qclipboard.h>
#include <qmainwindow.h>

#include "ydpdictionary.h"
#include "kydpconfig.h"
#include "ydpconfigure.h"

class Kydpdict : public QMainWindow
{
  Q_OBJECT
    public:
    /** construtor */
    Kydpdict(QWidget* parent=0, const char *name=0);
    /** destructor */
    ~Kydpdict();

    private:
		void resizeEvent (QResizeEvent *);
		void moveEvent (QMoveEvent *);
		void ShowDefinition (QString def);
		void Configure(bool status);
		void UpdateLook ();
		void SwapLang(bool direction, int lang);

		// global configuration
		kydpConfig *config;

		QClipboard  *cb;
		// GUI widgets
		QListBox *dictList;
		QLineEdit *wordInput;
		QTextEdit *RTFOutput;
		QPushButton *listclear;
		QMenuBar  *menu;
		int polToEng,engToPol, gerToPol, polToGer;

		// dictionary class
		ydpDictionary *myDict;

    private slots:
		void NewDefinition (int index);       		/* word list highlited */
		void NewFromLine (const QString& newText);	/* new word entered */
		void PlaySelected (int index);			/* dbl click or return to play */
		void PlayCurrent ();
		void SwapPolToEng ();
		void SwapEngToPol ();
		void SwapPolToGer ();
		void SwapGerToPol ();
		void ShowAbout ();
		void ShowAboutQt ();
		void PasteClippboard ();
		void ConfigureKydpdict();

};

extern QApplication *a;

#endif
