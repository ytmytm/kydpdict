/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef YDPFUZZYSEARCH_H
#define YDPFUZZYSEARCH_H

#include <qdialog.h>

class QLineEdit;
class QListBox;
class QListBoxItem;
class QSlider;

class ydpConverter;
class kydpConfig;

class ydpFuzzySearch : public QDialog
{
	Q_OBJECT

public:
    ydpFuzzySearch(kydpConfig *config, QWidget *parent = 0, const char* name = 0, bool modal = FALSE);
    ~ydpFuzzySearch();

private:
    int wordCount;
    char **wordList;
    ydpConverter *cvt;
    kydpConfig *cfg;
    QLineEdit *wordEdit;
    QListBox *listBox;
    QSlider *distSlider;
    int min3(const int a, const int b, const int c);
    int editDistance(const char*slowo1, const char*slowo2);

private slots:
    void doSearch();
    void newFromClick(QListBoxItem *lbi);

public slots:
    void updateDictionary(const int wordnum = -1, char **words = NULL, ydpConverter *converter = NULL);

signals:
    void textChanged(const QString &);
};

#endif	// YDPFUZZYSEARCH_H
