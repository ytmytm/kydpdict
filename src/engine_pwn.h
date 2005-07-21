/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINE_PWN_H
#define ENGINE_PWN_H

#include <qobject.h>
#include <qfile.h>

class QStringList;

#include "kydpconfig.h"
#include "ydpdictionary.h"

class EnginePWN : public ydpDictionary {
	Q_OBJECT
public:
	EnginePWN(kydpConfig *config, QListBox *listBox, ydpConverter *converter);
	~EnginePWN();

	int GetTipNumber(int type);
	QString GetInputTip(int index);
	QString GetOutputTip(int index);
	int OpenDictionary(void);
	int CheckDictionary(void);
	void CloseDictionary(void);
//	int FindWord(QString word);

private:
	int ReadDefinition (int index);
	void FillWordList(void);
//	QString rtf2html (QString definition);
	QString pwnhtml2qthtml(char *definition);
	void disableTag (int tag_code, int level);
	QString insertHyperText(QString raw_input, int level);
//	QString SampleName(QString path, int index);
	void UpdateFName(void);

	//
	void DoToolTips(const QString regex, QString *tmp, const QString color, const int n);
	QString MatchToolTips(const QString input);
	QString GetTip(int index);

	// dictionary file variables
	QFile fIndex;			/* index file descriptor */
	QFile fData;			/* data file descriptor */

	// shared between parsing routines
	QStringList::Iterator it;
	QStringList list;
	int tag_num[16];
	int direction_tab[16];

	struct LL_dictionaryCache {
	    unsigned int *offsets;	/* offsets in data file to word def's */
	    unsigned int words_base;
	} dictCache_LL[4];

	unsigned long *indexes; 	/* indexes to definitions array */

	int broken_entry;		/* index of a broken "Provencial" entry */
};

/* converter class */

class ConvertPWN : public ydpConverter {
public:
	ConvertPWN(void);
	~ConvertPWN();
//	char toLower(const char c);
	QString toUnicode(const char *input);
	QCString fromUnicode(QString input);
//	QString convertChunk(char *input, int size, bool unicodeFont);

private:
	QTextCodec *codec;
};

#endif
