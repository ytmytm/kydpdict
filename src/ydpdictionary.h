/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef YDPDICTIONARY_H
#define YDPDICTIONARY_H

#include <qapplication.h>
#include <qlistbox.h>
#include <qfile.h>
#include "kydpconfig.h"

class ydpDictionary : public QObject {
	Q_OBJECT
public:
	ydpDictionary(kydpConfig *config, QListBox *listBox);
	~ydpDictionary();

	QString GetDefinition(int index);
	int OpenDictionary(kydpConfig *config);
	int OpenDictionary(QString path, QString index, QString def);
	int CheckDictionary(kydpConfig *config);
	int CheckDictionary(QString path, QString index, QString def);
	int Play (int index, kydpConfig *config);
	QString GetInputTip(int index);
	QString GetOutputTip(int index);
	void CloseDictionary(void);
	void ListScrollUp(int offset);
	void ListScrollDown(int offset);
	int FindWord(QString word);
	int topitem;
	int wordCount;			/* number of words */

private:
	int  ReadDefinition (int index);
	QString convert_cp1250 (char *tekst, long int size);
	void FillWordList(void);
	QString rtf2html (QString definition);
	void disableTag (int tag_code, int level);
	QString insertHyperText(QString raw_input, int level);
	int ScoreWord(const char *w1, const char *w2);

	// configuration holder
	kydpConfig *cnf;

	// GUI element holding index
	QListBox	*dictList;

	// dictionary file variables
	QFile fIndex;			/* index file descriptor */
	QFile fData;			/* data file descriptor */
	QString curDefinition;		/* contains current definition (RTF)*/
	QStringList::Iterator it;
	QStringList list;
	int tag_num[16];
	int direction_tab[16];
	static QString input_tip[];

	unsigned long *indexes; 	/* indexes to definitions array */
	char** words;			/* actual words */

public slots:
	void ListRefresh (int index);
	void ListScroll1Up();
	void ListScroll1Down();
	void ListScrollPageUp();
	void ListScrollPageDown();
};

class KeyEater : public QObject {
    public:
	KeyEater(ydpDictionary* parent);
	~KeyEater();
    protected:
	bool eventFilter( QObject *o, QEvent *e );
    private:
	ydpDictionary *myDict;
};

#endif
