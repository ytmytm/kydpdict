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

class kydpConfig;

#include "ydpdictionary.h"
#include "ydpconverter.h"

class EnginePWN : public ydpDictionary {
	Q_OBJECT
public:
	EnginePWN(kydpConfig *config, QListBox *listBox, ydpConverter *converter);
	~EnginePWN();

	int OpenDictionary(void);
	int CheckDictionary(void);
	void CloseDictionary(void);
	int GetDictionaryInfo(void);

private:
	int ReadDefinition (int index);
	void FillWordList(void);
	QString rtf2html (QString definition);
	void disableTag (int tag_code, int level);
	QString insertHyperText(QString raw_input, int level);
	void UpdateFName(void);

	//
	void DoToolTips(const QString regex, QString *tmp, const QString color, const int n);
	QString MatchToolTips(const QString input);
	QString GetTip(int index);

	// dictionary file variables
	QFile fIndex;			/* index file descriptor */

	struct LL_dictionaryCache {
	    unsigned int *offsets;	/* offsets in data file to word def's */
	    unsigned int words_base;
	} dictCache_LL[4];

	// dictionary loader stuff
	unsigned int index_base, words_base, end_last_word, maxlength;
	unsigned int *offsets;
	char *wordbuffer;
};

/* converter class */

class ConvertPWN : public ydpConverter {
public:
	ConvertPWN(void);
	~ConvertPWN();
	char toLower(const char c);
	char *toLocal(const char *input);
	int charIndex(const char c);
	QString toUnicode(const char *input);
	QCString fromUnicode(QString input);
	QString convertChunk(const char *inp, int size = 0, bool unicodeFont = false);

private:
	QTextCodec *codec;
};

#endif
