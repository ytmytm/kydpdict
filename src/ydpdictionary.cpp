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

#include "kydpdict.h"
#include "kydpconfig.h"
#include "ydpconverter.h"
#include "ydpdictionary.h"
#include "ydpdictionary.moc"

KeyEater* keyEater;

QString ydpDictionary::GetTip(int index) {
    return QString("XXXYYYZZZ");
}

int ydpDictionary::GetTipNumber(int type) {

static int gram,dom,flag=-1;
int i;

    if (flag<0) {
	gram = 0; dom = 0;
	for (i=0;!(GetTip(i).startsWith("XXXYYYZZZ"));i++) {
	    if (GetInputTip(i)[0].category() == QChar::Letter_Uppercase)
		dom++;
	    else
		gram++;
	}
	flag = 0;
    }
    switch (type) {
	case 0:
	    return gram+dom;
	    break;
	case 1:
	    return gram;
	    break;
	case 2:
	    return dom;
	    break;
	default:
	    return -1;
	    break;
    }
}

QString ydpDictionary::GetInputTip(int index) {
    QString tmp = GetTip(index);
    return tmp.mid(0,tmp.find(':'));
}

QString ydpDictionary::GetOutputTip(int index) {
    QString tmp = GetTip(index);
    return tmp.mid(tmp.find(':')+1);
}

QString ydpDictionary::GetInfoPage(void) {
    QString gram, dom;

    for (int i=0; i<GetTipNumber(0); i++) {
	if (GetInputTip(i)[0].category() == QChar::Letter_Uppercase) {
	    dom  += "<a name=\""+ GetInputTip(i) + "\"></a><h4><font color=\"red\">"+ GetInputTip(i) + "</font></h4>" + GetOutputTip(i) + "<hr>";
	} else {
	    gram += "<a name=\""+ GetInputTip(i) + "\"></a><h4><font color=\"red\">"+ GetInputTip(i) + "</font></h4>" + GetOutputTip(i) + "<hr>";
	}
    }

    QString output = "<h2>Skróty wystêpuj±ce w t³umaczeniach</h2>";
    output += "<h3>Czê¶æ I - GRAMATYKA</h3>";
    output += gram;
    output += "<h3>Czê¶æ II - DZIEDZINY</h3>";
    output += dom;

    return output;
}

unsigned int ydpDictionary::fix32(unsigned int x)
{
#ifndef WORDS_BIGENDIAN
	return x;
#else
	return (unsigned int)
		(((x & (unsigned int) 0x000000ffU) << 24) |
                 ((x & (unsigned int) 0x0000ff00U) << 8) |
                 ((x & (unsigned int) 0x00ff0000U) >> 8) |
                 ((x & (unsigned int) 0xff000000U) >> 24));
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

ydpDictionary::ydpDictionary(kydpConfig *config, QListBox *listBox, ydpConverter *converter)
{
    int i;

    dictList = listBox;
    cnf = config;
    cvt = converter;
    keyEater = new KeyEater(this);
    dictList->installEventFilter(keyEater);
    for (i=0;i<4;i++) {
        dictCache[i].wordCount = -1;
        dictCache[i].words = NULL;
    }
}

ydpDictionary::~ydpDictionary()
{
    int i,j;

    for (i=0;i<4;i++) {
	if (dictCache[i].wordCount>0) {
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
	words = dictCache[i].words;
    } else {
	dictCache[i].wordCount = wordCount;
	dictCache[i].words = words;
    }
    /* don't want to do ListRefresh yet and dictList can't be empty */
    dictList->blockSignals(TRUE);
    dictList->insertItem(QString("dummy"));
    dictList->insertItem(QString("dummy"));
    ListRefresh(0);
    dictList->blockSignals(FALSE);
    emit dictionaryChanged(wordCount,words,cvt);
    return 0;
}

int ydpDictionary::CheckDictionary(void) {
    return 1;	// always fail
}

int ydpDictionary::GetDictionaryInfo(void) {
    bool toPolish = cnf->toPolish;
    int language = cnf->language;
    int bits = 0;

    cnf->language = LANG_ENGLISH;
    cnf->toPolish = true;
    if (CheckDictionary())
	bits |= hasEnglish2Polish;
    cnf->toPolish = false;
    if (CheckDictionary())
	bits |= hasPolish2English;
    cnf->language = LANG_DEUTSCH;
    cnf->toPolish = true;
    if (CheckDictionary())
	bits |= hasGerman2Polish;
    cnf->toPolish = false;
    if (CheckDictionary())
	bits |= hasPolish2German;
    cnf->language = language; cnf->toPolish = toPolish;
    return bits;
}

void ydpDictionary::CloseDictionary()
{
    dictList->clear();
}

void ydpDictionary::UpdateFName(void)
{

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
	    return true;	// don't pass this down
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
	dictList->setAutoUpdate(FALSE);
	dictList->blockSignals(TRUE);
	// przepisac o 1 w gore, w ostatni wpisac nowy, current ustawic na przedostatni
	for (i=1;i<spacefor;i++)
	    dictList->changeItem(dictList->text(i),i-1);

	dictList->changeItem(cvt->toUnicode(words[topitem+spacefor]),i-1);
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
	dictList->setAutoUpdate(FALSE);
	dictList->blockSignals(TRUE);
	for (i=spacefor-1;i!=0;i--)
	    dictList->changeItem(dictList->text(i-1),i);
	topitem--;
	dictList->changeItem(cvt->toUnicode(words[topitem]),0);

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
	dictList->changeItem(cvt->toUnicode(words[index+i]),i);
    dictList->blockSignals(FALSE);
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

/*
// naive search, moderately fast if convertion is fast
int ydpDictionary::FindWord(QString word)
{
    int i, score, maxscore=0, maxitem=0;

    if (word.length() == 0)
	return 0;

    word = stripDelimiters(word.lower());
    word.truncate(20);

    for (i=0;i<wordCount;i++) {
	score = ScoreWord(word, cvt->toUnicode(words[i]));
	if (score>maxscore) {
    	    maxscore = score;
	    maxitem = i;
	}
    }
    return maxitem;
}
*/
int ydpDictionary::FindWord(QString word) {

    int a, b, middle, result;
    int i, score, maxscore=0, maxitem=0;

    if (word.length() == 0)
	return 0;

    word = stripDelimiters(word.lower());
    word.truncate(20);
    QCString newWord = cvt->fromUnicode(word);

    a = 0; b = wordCount;

    // binary search to narrow range to 30 (arbitrary)
    while (b-a >= 30) {	/* there are problems with 'for', 'f.o.r.' etc. without margin */
	middle = a + (b-a)/2;
	result = cvt->localeCompare(newWord, cvt->toLocal(words[middle]));
	if (result==0) {
	    a = middle; b = middle;
	} else {
	    if (result<0)
		b = middle;
	    else
		a = middle;
	}
    }

    // lower and upper bounds for naive search
    a = a - 150; if (a<0) a = 0;
    b = a + 2000; if (b>wordCount) b = wordCount;

    // now a naive search
    for (i=a;i<b;i++) {
	score = cvt->scoreWord(newWord, cvt->toLocal(words[i]));
	if (score>maxscore) {
    	    maxscore = score;
	    maxitem = i;
	}
	if ((maxscore>1) && (i>maxitem+2000))
	    break;
    }
    return maxitem;
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
    return definition;
}
