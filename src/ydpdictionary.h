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
	void CloseDictionary(void);

private:
	int  ReadDefinition (int index);
	QString convert_cp1250 (char *tekst, long int size);
	void FillWordList(void);
	QString rtf2html (QString definition);
	void disableTag (int tag_code, int level);
	QString insertTip(QString raw_input);
	QString insertTip2(QString raw_input);

	// configuration holder
	kydpConfig *cnf;

	// GUI element holding index
	QListBox	*dictList;

		// dictionary file variables
	QFile fIndex;               /* index file descriptor */
	QFile fData;                /* data file descriptor */
	int wordCount;          	/* number of all words in dictionary */
	unsigned long *indexes; 	/* indexes to definitions array */
	QString curDefinition;		/* contains current definition (RTF)*/
	QStringList::Iterator it;
	QStringList list;
	int tag_num[16];

};

#endif
