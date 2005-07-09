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

#include <qobject.h>

class QListBox;
class QString;

#include "kydpconfig.h"
#include "ydpconverter.h"

#define color1 cnf->kFontKolor1
#define color2 cnf->kFontKolor2
#define color3 cnf->kFontKolor3
#define color4 cnf->kFontKolor4

class ydpDictionary : public QObject {
	Q_OBJECT
public:
	ydpDictionary(kydpConfig *config, QListBox *listBox, ydpConverter *converter);
	virtual ~ydpDictionary();

	virtual int OpenDictionary(void);
	virtual int CheckDictionary(void);
	virtual void CloseDictionary(void);

	QString GetDefinition(int index);
	int Play (int index, kydpConfig *config);
	virtual int GetTipNumber(int type);
	virtual QString GetInputTip(int index);
	virtual QString GetOutputTip(int index);
	void ListScrollUp(int offset);
	void ListScrollDown(int offset);
	virtual int FindWord(QString word);

	int topitem;
	int wordCount;			/* number of words */

private:
	virtual int ReadDefinition (int index);
	virtual QString rtf2html (QString definition);
	virtual QString SampleName(QString path, int index);

	// GUI element holding index
	QListBox	*dictList;

protected:
	unsigned short fix16(unsigned short x);
	unsigned long fix32(unsigned long x);
	QString stripDelimiters(QString word);
	int ScoreWord(QString w1, QString w2);

	// configuration holder
	kydpConfig *cnf;
	// text converter
	ydpConverter *cvt;

	struct dictionaryCache {
	    int wordCount;
	    char **words;
	} dictCache[4];

	char** words;			/* actual words */
	QString curDefinition;		/* contains current definition (RTF)*/

public slots:
	void ListRefresh (int index);
	void ListScroll1Up();
	void ListScroll1Down();
	void ListScrollPageUp();
	void ListScrollPageDown();

signals:
	void dictionaryChanged(const int wordnum = -1, char **words = NULL, ydpConverter *converter = NULL);
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
