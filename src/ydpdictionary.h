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
//#include <qfile.h>
//#include <qstringlist.h>

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
//	int OpenDictionary(kydpConfig *config);
//	int OpenDictionary(QString path, QString index, QString def);
//	int CheckDictionary(kydpConfig *config);
//	int CheckDictionary(QString path, QString index, QString def);
	int Play (int index, kydpConfig *config);
	virtual int GetTipNumber(int type);
	virtual QString GetInputTip(int index);
	virtual QString GetOutputTip(int index);
//	void CloseDictionary(void);
	void ListScrollUp(int offset);
	void ListScrollDown(int offset);
	virtual int FindWord(QString word);

	int topitem;
	int wordCount;			/* number of words */

private:
	virtual int ReadDefinition (int index);
//	QString convert_cp1250 (char *tekst, int size);
//	void FillWordList(void);
	virtual QString rtf2html (QString definition);
//	void disableTag (int tag_code, int level);
//	QString insertHyperText(QString raw_input, int level);
	virtual QString SampleName(QString path, int index);

	// GUI element holding index
	QListBox	*dictList;

	// dictionary file variables
//	QFile fIndex;			/* index file descriptor */
//	QFile fData;			/* data file descriptor */
protected:
	unsigned short fix16(unsigned short x);
	unsigned long fix32(unsigned long x);
	QString stripDelimiters(QString word);
	int ScoreWord(QString w1, QString w2);

	// configuration holder
	kydpConfig *cnf;
	// text converter
	ydpConverter *cvt;

//	QStringList::Iterator it;
//	QStringList list;
//	int tag_num[16];
//	int direction_tab[16];
	struct dictionaryCache {
	    int wordCount;
	    char **words;
//	    unsigned long *indexes;	//XXX this is ydp-specific
	} dictCache[4];

//	unsigned long *indexes; 	/* indexes to definitions array */
	char** words;			/* actual words */
	QString curDefinition;		/* contains current definition (RTF)*/

//	int broken_entry;		/* index of a broken "Provencial" entry */

public slots:
	void ListRefresh (int index);
	void ListScroll1Up();
	void ListScroll1Down();
	void ListScrollPageUp();
	void ListScrollPageDown();

signals:
	void dictionaryChanged(const int wordnum = -1, char **words = NULL);
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
