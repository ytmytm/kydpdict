/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINE_DICT_H
#define ENGINE_DICT_H

#include <qobject.h>
#include <qfile.h>
#include <zlib.h>

class QStringList;

class kydpConfig;

#include "ydpdictionary.h"
#include "ydpconverter.h"

class EngineDICT : public ydpDictionary {
	Q_OBJECT
public:
	EngineDICT(kydpConfig *config, QListBox *listBox, ydpConverter *converter);
	~EngineDICT();

	int OpenDictionary(void);
	int CheckDictionary(void);
	void CloseDictionary(void);

private:
	int ReadDefinition (int index);
	void FillWordList(void);
	QString rtf2html (QString definition);
	void UpdateFName(void);

	int Base64Decode(const char *input);

	// dictionary file variables
	QFile fData;			/* index file descriptor */
	QFile fIndex;
	gzFile zData;

	unsigned int *offsets;
	unsigned int *lengths;
	struct LL_dictionaryCache {
	    unsigned int *offsets;
	    unsigned int *lengths;
	} dictCache_LL[4];

};

/* converter class */

class ConvertDICT : public ydpConverter {
public:
	ConvertDICT(void);
	~ConvertDICT();
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
