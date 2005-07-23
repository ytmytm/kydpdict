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

class kydpConfig;
class ydpConverter;

// shortcuts
#define color1 cnf->kFontKolor1
#define color2 cnf->kFontKolor2
#define color3 cnf->kFontKolor3
#define color4 cnf->kFontKolor4

// capabilities bits
#define hasEnglish2Polish	0x00000001
#define hasPolish2English	0x00000002
#define hasGerman2Polish	0x00000004
#define hasPolish2German	0x00000008

class ydpDictionary : public QObject {
	Q_OBJECT
public:
	ydpDictionary(kydpConfig *config, QListBox *listBox, ydpConverter *converter);
	virtual ~ydpDictionary();

	virtual int OpenDictionary(void);
	virtual int CheckDictionary(void);
	virtual void CloseDictionary(void);
	// return capabilities bits
	virtual int GetDictionaryInfo(void);

	QString GetDefinition(int index);
	int Play (int index, kydpConfig *config);
	// return number of all/grammar/category tips
	virtual int GetTipNumber(int type);
	// return abbreviated part of tip
	virtual QString GetInputTip(int index);
	// return description part of tip
	virtual QString GetOutputTip(int index);
	// create a page with tips and their meanings
	// assumed: abbr. with lowercase - grammar, UPPERCASE - category
	virtual QString GetInfoPage(void);
	void ListScrollUp(int offset);
	void ListScrollDown(int offset);
	// find best match
	virtual int FindWord(QString word);

	int topitem;
	int wordCount;			/* number of words */

private:
	// read a definition, convert from local encoding, store in curDefinition
	virtual int ReadDefinition (int index);
	// convert definition into qt-html enriched text
	virtual QString rtf2html (QString definition);
	// return filename of audio sample
	virtual QString SampleName(QString path, int index);
	// update internal dictionary files names w/ respect to language and direction
	virtual void UpdateFName(void);
	// return a tip from tips table according to index, always override this if tips present
	virtual QString GetTip(int index);

	// GUI element holding index
	QListBox	*dictList;

protected:
	unsigned short fix16(unsigned short x);
	unsigned long fix32(unsigned long x);
	QString stripDelimiters(QString word);

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
