/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpushbutton.h>
#include <qtextcodec.h>
#include <qlabel.h>

#include "ydpfuzzysearch.h"
#include "kydpdict.h"

ydpFuzzySearch::ydpFuzzySearch (QWidget *parent, const char* name, bool modal)
 : QDialog(parent, name, modal)
{
    setFixedSize(400,260);
    setCaption(tr("Fuzzy search"));

    QPushButton *searchButton = new QPushButton(this,"search");
    searchButton->setText(tr("Search"));
    searchButton->setGeometry(10,220,81,30);
    connect(searchButton, SIGNAL(clicked()), this, SLOT(doSearch()));

    wordEdit = new QLineEdit(this, "wordEdit");
    wordEdit->setFrameStyle( QFrame::Sunken | QFrame::Panel);
    wordEdit->setLineWidth( 1 );
    wordEdit->setGeometry( 10, 17, 290, 22 );
    wordEdit->setText(tr("Search for..."));
    wordEdit->selectAll();
    wordEdit->setFocus();

    listBox = new QListBox(this, "listBox");
    listBox->setGeometry(10, 40, 290, 160);
    connect(listBox, SIGNAL(clicked(QListBoxItem*)), this, SLOT(newFromClick(QListBoxItem*)));
    connect(listBox, SIGNAL(selected(QListBoxItem*)), this, SLOT(newFromClick(QListBoxItem*)));

    distSlider = new QSlider(1, 10, 1, 2, Qt::Vertical, this, "distSlider");
    distSlider->setGeometry(320, 40, 22, 160);
    distSlider->setTracking(FALSE);
    distSlider->setTickmarks(QSlider::Left);
    distSlider->setTickInterval(1);

    QLabel *maxV = new QLabel(tr("high"), this, "maxV");
    QLabel *minV = new QLabel(tr("low"), this, "minV");
    QLabel *similar = new QLabel(tr("Similarity"), this, "similar");
    similar->setGeometry(310, 17, 90, 22);
    maxV->setGeometry(345, 36, 60, 22);
    minV->setGeometry(345, 180, 60, 22);
}

ydpFuzzySearch::~ydpFuzzySearch()
{

}

void ydpFuzzySearch::updateDictionary(const int wordnum, char **words)
{
    if ((wordnum<0) || (words == NULL))
	return;

    wordCount = wordnum;
    wordList = words;
}

void ydpFuzzySearch::doSearch()
{
    if ((wordCount<0) || (wordList == NULL))
	return;

    static QTextCodec *codec = QTextCodec::codecForName("CP1250");
    listBox->clear();
    int i;
    int distance = distSlider->value();
    for (i=0;i<wordCount;i++)
	if (editDistance(codec->fromUnicode(wordEdit->text()),wordList[i]) < distance)
	    listBox->insertItem(codec->toUnicode(wordList[i]));
}

void ydpFuzzySearch::newFromClick(QListBoxItem *lbi)
{
    QString result = listBox->text(listBox->currentItem());
    emit textChanged(result);
}

int ydpFuzzySearch::min3(const int a, const int b, const int c)
{
    int min=a;
    if (b<min) min = b;
    if (c<min) min = c;
    return min;
}

int ydpFuzzySearch::editDistance(const char*slowo1, const char*slowo2)
{
    int *row0, *row1, *row;
    int cost,i,j,m,n;

    n = strlen(slowo1);
    m = strlen(slowo2);

    row0 = new int[n+1];
    row1 = new int[n+1];

    for (i=0;i<=n;i++)
	row0[i] = i;

    for (j=1;j<=m;j++) {
	row1[0] = j;
	for (i=1;i<=n;i++) {
	    cost = (slowo1[i-1]==slowo2[j-1]) ? 0 : 1;
	    row1[i] = min3(row0[i]+1,row1[i-1]+1,row0[i-1]+cost);
	}
	row = row0;
	row0 = row1;
	row1 = row;
    }
    cost = row0[n];
    delete [] row0;
    delete [] row1;
    return cost;
}