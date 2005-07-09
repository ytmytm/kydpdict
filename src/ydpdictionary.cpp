/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlistbox.h>
#include <qmessagebox.h>
#include <qprocess.h>
#include <qtextcodec.h>
//#include <qregexp.h>

//#include <sys/mman.h>
//#include <fcntl.h>
//#include <unistd.h>
//#include <ctype.h>

#include <locale.h>

#include "kydpdict.h"

#include "ydpdictionary.h"
#include "ydpdictionary.moc"

KeyEater* keyEater;

int ydpDictionary::GetTipNumber(int type) {
    return -1;
}

QString ydpDictionary::GetInputTip(int index) {
	return QString("");
}

QString ydpDictionary::GetOutputTip(int index) {
	return QString("");
}

unsigned long ydpDictionary::fix32(unsigned long x)
{
#ifndef WORDS_BIGENDIAN
	return x;
#else
	return (unsigned long)
		(((x & (unsigned long) 0x000000ffU) << 24) |
                 ((x & (unsigned long) 0x0000ff00U) << 8) |
                 ((x & (unsigned long) 0x00ff0000U) >> 8) |
                 ((x & (unsigned long) 0xff000000U) >> 24));
#endif		
}

unsigned short ydpDictionary::fix16(unsigned short x)
{
#ifndef WORDS_BIGENDIAN
	return x;
#else
	return (unsigned short)
		(((x & (unsigned short) 0x00ffU) << 8) |
                 ((x & (unsigned short) 0xff00U) >> 8));
#endif
}

// XXX add convertor here
ydpDictionary::ydpDictionary(kydpConfig *config, QListBox *listBox)
{
    int i;

    dictList = listBox;
    cnf = config;
//    cvt = converter;
    keyEater = new KeyEater(this);
    dictList->installEventFilter(keyEater);
    for (i=0;i<4;i++) {
        dictCache[i].wordCount = -1;
//        dictCache[i].indexes = NULL;
        dictCache[i].words = NULL;
    }
}

ydpDictionary::~ydpDictionary()
{
    int i,j;

    for (i=0;i<4;i++) {
	if (dictCache[i].wordCount>0) {
//	    if (dictCache[i].indexes) delete [] dictCache[i].indexes;
	    if (dictCache[i].words) {
		for (j=0;j<dictCache[i].wordCount;j++)
		    delete [] dictCache[i].words[j];
		delete [] dictCache[i].words;
	    }
	}
    }
}

QString ydpDictionary::GetDefinition (int index)
{
    if (ReadDefinition(index) == 0)
	return rtf2html(curDefinition);
    else
	return (tr("Error reading data file!"));
}

int ydpDictionary::OpenDictionary(void)
{
    /* 0-english, eng->pol; 1-english, pol->eng; 2-german, ger->pol; 3-german, pol->ger */
    int i = 0;

    if (cnf->language == LANG_DEUTSCH)
	i+=2;
    if (!(cnf->toPolish))
	i++;
    if (dictCache[i].wordCount>0) {
	wordCount = dictCache[i].wordCount;
//	indexes = dictCache[i].indexes;
	words = dictCache[i].words;
    } else {
//	FillWordList();
	dictCache[i].wordCount = wordCount;
//	dictCache[i].indexes = indexes;
	dictCache[i].words = words;
    }
    /* don't want to do ListRefresh yet and dictList can't be empty */
    dictList->blockSignals(TRUE);
    dictList->insertItem(QString("dummy"));
    dictList->insertItem(QString("dummy"));
    ListRefresh(0);
    dictList->blockSignals(FALSE);
    emit dictionaryChanged(wordCount,words);
    return 0;
}

int ydpDictionary::CheckDictionary(void) {
    return 1;	// always fail
}

void ydpDictionary::CloseDictionary()
{
    dictList->clear();
//    fIndex.close();
//    fData.close();
}

KeyEater::KeyEater(ydpDictionary *parent)
{
    myDict = parent;
}

KeyEater::~KeyEater()
{

}

bool KeyEater::eventFilter(QObject *o, QEvent *e) {
    QListBox *dictList = (QListBox*)o;
    if ( e->type() == QEvent::Wheel ) {
	QWheelEvent *k = (QWheelEvent *)e;
	if (k->orientation() == Qt::Vertical) {
	    if (k->delta() < 0)
		myDict->ListScrollPageUp();
	    else
		myDict->ListScrollPageDown();
	}
    }
    if ( e->type() == QEvent::KeyPress ) {
	QKeyEvent *k = (QKeyEvent *)e;
	if ((dictList->currentItem()==0)&&((k->key()==Qt::Key_Up)||(k->key()==Qt::Key_Prior))) {
	    if (k->key()==Qt::Key_Up)
		myDict->ListScroll1Down();
	    if (k->key()==Qt::Key_Prior)
		myDict->ListScrollPageDown();
	    return true;	// don't pass this down
	}
	if ((dictList->currentItem()+1==(int)dictList->count())&&((k->key()==Qt::Key_Down)||(k->key()==Qt::Key_Next))) {
	    if (k->key()==Qt::Key_Down)
		myDict->ListScroll1Up();
	    if (k->key()==Qt::Key_Next)
		myDict->ListScrollPageUp();
	    return true;	// don't pass this down
	}
	return false;	// process event
    } else
	return false;	// process event
}

// offset == 1 - o linie, !=1 - o strone widoczna
void ydpDictionary::ListScrollUp(int offset) {
    int i,spacefor,itemh;

    itemh = dictList->itemHeight();
    if (itemh==0) {
	printf("codename: upper larch!!! mail this to ytm@elysium.pl!!!\n");
	itemh=10;
    }
    spacefor=dictList->height() / itemh;

    if (topitem+spacefor == wordCount)
	return;

    if (offset != 1) {
	ListRefresh(topitem+spacefor);
	dictList->blockSignals(TRUE);
	dictList->setCurrentItem(spacefor-2);	// prepare to force highlight signal
	dictList->blockSignals(FALSE);
	dictList->setCurrentItem(spacefor-1);	// restore correct item
    } else {
	static QTextCodec *codec = QTextCodec::codecForName("CP1250");
	dictList->setAutoUpdate(FALSE);
	dictList->blockSignals(TRUE);
	// przepisac o 1 w gore, w ostatni wpisac nowy, current ustawic na przedostatni
	for (i=1;i<spacefor;i++)
	    dictList->changeItem(dictList->text(i),i-1);

	dictList->changeItem(codec->toUnicode(words[topitem+spacefor]),i-1);
	topitem++;

	dictList->setCurrentItem(spacefor-2);	// prepare to force highlight signal
	dictList->setAutoUpdate(TRUE);
	dictList->blockSignals(FALSE);
	dictList->setCurrentItem(spacefor-1);	// restore correct item
    }
}

void ydpDictionary::ListScrollDown(int offset) {
    int i, spacefor, itemh;

    if (topitem==0)
	return;

    itemh = dictList->itemHeight();
    if (itemh == 0) {
	printf("codename: lower larch!!! mail this to ytm@elysium.pl!!!\n");
	itemh=10;
    }
    spacefor=dictList->height()/itemh;

    if (offset != 1) {
	ListRefresh(topitem-spacefor);
	dictList->setCurrentItem(0);
    } else {
	static QTextCodec *codec = QTextCodec::codecForName("CP1250");
	dictList->setAutoUpdate(FALSE);
	dictList->blockSignals(TRUE);
	for (i=spacefor-1;i!=0;i--)
	    dictList->changeItem(dictList->text(i-1),i);
	topitem--;
	dictList->changeItem(codec->toUnicode(words[topitem]),0);

	dictList->setCurrentItem(1);	// prepare to force highlight signal
	dictList->setAutoUpdate(TRUE);
	dictList->blockSignals(FALSE);
	dictList->setCurrentItem(0);	// restore correct index
    }
}

void ydpDictionary::ListScroll1Up() 
{
    if (dictList->currentItem()+1==(int)dictList->count()) {
	ListScrollUp(1);
    } else {
	dictList->setCurrentItem(dictList->currentItem()+1);
    }
}

void ydpDictionary::ListScrollPageUp()
{
    ListScrollUp(20);	// anything but 1 for page
}

void ydpDictionary::ListScroll1Down()
{
    if (dictList->currentItem()==0) {
	ListScrollDown(1);
    } else {
	dictList->setCurrentItem(dictList->currentItem()-1);
    }
}

void ydpDictionary::ListScrollPageDown()
{
    ListScrollDown(20);	// anything but 1 for page
}

void ydpDictionary::ListRefresh(int index)
{
/* refresh list so index is visible and dictList length is correct */
    static QTextCodec *codec = QTextCodec::codecForName("CP1250");
    unsigned int spacefor,itemh;
    bool needRefresh=false;
    static unsigned int lastspacefor;
    int i;

    if (index<0)
	index=0;

    /* now, before showing QMessageBox with bad paths error, ListRefresh gots called
       and we have nothing yet, so we must quit. Moving connect signal lower should help,
       but it wouldn't be obvious why one connect signal is in a different place */
    itemh = dictList->itemHeight();
    if (itemh==0)
	return;
    /* handle size change */
    spacefor=dictList->height()/itemh;
    dictList->setAutoUpdate(FALSE);
    if (spacefor!=lastspacefor) {
	needRefresh=true;
	if (dictList->count()<spacefor) {
	    int i,j;
	    j = spacefor-dictList->count();
	    for (i=0;i<j;i++)
		dictList->insertItem(QString("dummy"));
	} else {
	    int i,j;
	    j = dictList->count()-spacefor;
	    for (i=0;i<j;i++)
		dictList->removeItem(dictList->count()-1);
	}
    }
    dictList->setAutoUpdate(TRUE);

    /* if currently visible - do nothing */
    if (((index>topitem)&&(index<(int)(topitem+dictList->count())))&&(!needRefresh))
	return;

    /* fix for out of bounds */
    if ((int)((index+dictList->count()))>wordCount)
	index=wordCount-dictList->count();

    topitem = index;

    dictList->blockSignals(TRUE);
    for (i=0;i<(int)dictList->count();i++)
	dictList->changeItem(codec->toUnicode(words[index+i]),i);
    dictList->blockSignals(FALSE);
}

int ydpDictionary::ScoreWord(QString w1, QString w2)
{
    unsigned int i = 0;
    unsigned int len1 = w1.length();
    unsigned int len2 = w2.length();
    for (; (i<len1) && (i<len2); i++)
	if (w1.at(i) != w2.at(i))	// cvt->tolower e.g.
	    break;
    return i;
}

QString ydpDictionary::stripDelimiters(QString word)
{
    QString lstring;
    unsigned int i;
    unsigned int wlen = word.length();

    for (i=0; i<wlen; i++) {
	if (word.at(i).isLetterOrNumber())
	    lstring += word.at(i);
    }

    return lstring;
}

int ydpDictionary::FindWord(QString word)
{
    static QTextCodec *codec = QTextCodec::codecForName("CP1250");
    QString midstring, wordorig;
    int a,b;
    int middle,result;
    int lwordorig;
    bool same;

    lwordorig = word.length();

    if (lwordorig == 0)
	return 0;

    wordorig = word.lower();
    word = stripDelimiters(word.lower());
    word.truncate(20);

    same = (word.compare(wordorig) == 0);

    a = 0; b = wordCount;

    setlocale(LC_COLLATE,"pl_PL");	// otherwise '¶win'->'z...' on at least one system :(
					// required for that localeAwareCompare below

    /* najpierw wyszukiwanie binarne zawê¿a zakres do 30 hase³ (warto¶æ wziêta z g³owy (mojej - MW)) */

    while (b-a >= 30) {	/* bez tego marginesu s± problemy np. z 'for' */
	middle = a + (b-a)/2;
	midstring = stripDelimiters(codec->toUnicode(words[middle])).lower();
	result = word.localeAwareCompare(midstring);
	if (result==0) {
	    a = middle; b = middle;
	} else {
	    if (result<0)
		b = middle;
	    else
		a = middle;
	}
    }

    /* poniewa¿ indeksy nie s± idealnie posortowane - teraz wyszukujemy naiwnie;
       nastêpuje cofniêcie o 35 hase³ i w¶ród 150 nastêpnych znalezienie tego z najwy¿szym
       wynikiem ze ScoreWord (najwiêcej wspólnych znaków z wyszukiwanym has³em) */

    int i;
    int max, maxs;	// index of max scored and max scored with strip delimiters
    int smax, smaxs;	// scores of - '' -
    int score, scores;
    QString bstring;

    if (a!=b) {		// what was not a direct match
	a=a-35; if (a<0) a=0;
	max=a; maxs=a; smax=0; smaxs=0; score=0; scores=0;
	i=1;
	while ((i<150) && (a+i<wordCount) && (smax<100)) {
	    bstring = codec->toUnicode(words[a+i]).lower();
	    score  = ScoreWord(wordorig,bstring);
	    if (!same) scores = ScoreWord(word,stripDelimiters(bstring));
	    if (score > smax) {
		smax = score; max = a+i;
		if (lwordorig == smax)
		    smax +=100;				// extra bonus for direct match - break while
	    }
	    if ((!same)&&(scores > smaxs)) {
		smaxs = scores; maxs = a+i;
	    }
	    ++i;
	}
	a = max;	// raw match preferred
	if ((!same)&&(smaxs > smax))
	    a = maxs;	// unless stripped has better score
    }

    return a;
}

int ydpDictionary::ReadDefinition(int index)
{
    return 0;
}

QString ydpDictionary::SampleName(QString path, int index)
{
    return QString("");
}

int ydpDictionary::Play(int index, kydpConfig *config)
{
    QFile fd;
    QString pathdir, samplename;
    static QProcess proc;

    switch (config->language) {
	case LANG_DEUTSCH:
	    pathdir = config->cd2Path;
	    break;
	case LANG_ENGLISH:
	    pathdir = config->cdPath;
	    break;
	default:
	    break;		// cos poszlo nie tak?
    }

    fd.setName(pathdir.latin1());
    if (!(fd.exists()))
	return 0;		// nie ma co tracic czasu jesli katalogu nie ma

    samplename = SampleName(pathdir,index);
    if (samplename.length() == 0)
	return 0;		// pliku równie¿ mo¿e nie byæ

    fd.setName(config->player);
    if (!(fd.exists()))
	return 0;		// ani odtwarzacza...

    if (proc.isRunning())
	proc.kill();		// nie ma litosci dla skurwysynow! BUM! BUM!

    proc.clearArguments();
    proc.addArgument( config->player );
    proc.addArgument( samplename );
    proc.start();

    return 1;
}

QString ydpDictionary::rtf2html (QString definition)
{
  return QString("");
}
