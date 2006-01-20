/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINE_YDP_H
#define ENGINE_YDP_H

#include <qobject.h>
#include <qfile.h>

class QStringList;

class kydpConfig;

#include "ydpdictionary.h"
#include "ydpconverter.h"

class EngineYDP : public ydpDictionary {
	Q_OBJECT
public:
	EngineYDP(kydpConfig *config, QListBox *listBox, ydpConverter *converter);
	~EngineYDP();

	int OpenDictionary(void);
	int CheckDictionary(void);
	void CloseDictionary(void);

private:
	int ReadDefinition (int index);
	void FillWordList(void);
	QString rtf2html (QString definition);
	void disableTag (int tag_code, int level);
	QString insertHyperText(QString raw_input, int level);
	QString SampleName(QString path, int index);
	void UpdateFName(void);
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
	    unsigned int *indexes;	/* indexes to definitions array */
	} dictCache_LL[4];

	unsigned int *indexes; 	/* indexes to definitions array */

	int broken_entry;		/* index of a broken "Provencial" entry */
};

/* converter class */

class ConvertYDP : public ydpConverter {
public:
	ConvertYDP(void);
	~ConvertYDP();
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
