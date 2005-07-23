/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENGINE_SAP_H
#define ENGINE_SAP_H

#include <qobject.h>
#include <qfile.h>

class QStringList;

class kydpConfig;

#include "ydpdictionary.h"
#include "ydpconverter.h"

class EngineSAP : public ydpDictionary {
	Q_OBJECT
public:
	EngineSAP(kydpConfig *config, QListBox *listBox, ydpConverter *converter);
	~EngineSAP();

	int OpenDictionary(void);
	int CheckDictionary(void);
	void CloseDictionary(void);

private:
	int ReadDefinition (int index);
	void FillWordList(void);
	QString rtf2html (QString definition);
	void UpdateFName(void);

	// dictionary file variables
	QFile fData;			/* index file descriptor */

	char **definitions;
	struct LL_dictionaryCache {
	    char **definitions;
	} dictCache_LL[4];

	QString curWord;
};

/* converter class */

class ConvertSAP : public ydpConverter {
public:
	ConvertSAP(void);
	~ConvertSAP();
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
