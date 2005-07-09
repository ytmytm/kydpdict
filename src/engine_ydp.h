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
//#include <qstringlist.h>

//class QListBox;

class QStringList;

#include "kydpconfig.h"
#include "ydpdictionary.h"

class EngineYDP : public ydpDictionary {
	Q_OBJECT
public:
	EngineYDP(kydpConfig *config, QListBox *listBox);
	~EngineYDP();

//	QString GetDefinition(int index);
//	int OpenDictionary(kydpConfig *config);
//	int OpenDictionary(QString path, QString index, QString def);
//	int CheckDictionary(kydpConfig *config);
//	int CheckDictionary(QString path, QString index, QString def);
//	int Play (int index, kydpConfig *config);
	int GetTipNumber(int type);
	QString GetInputTip(int index);
	QString GetOutputTip(int index);
	int OpenDictionary(void);
	int CheckDictionary(void);
	void CloseDictionary(void);
//	void ListScrollUp(int offset);
//	void ListScrollDown(int offset);
//	int FindWord(QString word);
//	int topitem;
//	int wordCount;			/* number of words */

private:
	int ReadDefinition (int index);
	QString convert_cp1250 (char *tekst, int size);
	void FillWordList(void);
	QString rtf2html (QString definition);
	void disableTag (int tag_code, int level);
	QString insertHyperText(QString raw_input, int level);
//	QString stripDelimiters(QString word);
//	int ScoreWord(QString w1, QString w2);
	QString SampleName(QString path, int index);

	// configuration holder
//	kydpConfig *cnf;

	// GUI element holding index
//	QListBox	*dictList;

	// dictionary file variables
	QFile fIndex;			/* index file descriptor */
	QFile fData;			/* data file descriptor */
//	QString curDefinition;		/* contains current definition (RTF)*/
	QStringList::Iterator it;
	QStringList list;
	int tag_num[16];
	int direction_tab[16];

	struct LL_dictionaryCache {
	    unsigned long *indexes;	/* indexes to definitions array */
	} dictCache_LL[4];

	unsigned long *indexes; 	/* indexes to definitions array */
//	char** words;			/* actual words */

	int broken_entry;		/* index of a broken "Provencial" entry */

//public slots:
//	void ListRefresh (int index);
//	void ListScroll1Up();
//	void ListScroll1Down();
//	void ListScrollPageUp();
//	void ListScrollPageDown();

//signals:
//	void dictionaryChanged(const int wordnum = -1, char **words = NULL);
};

#endif
