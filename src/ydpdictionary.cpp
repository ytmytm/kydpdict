/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qmessagebox.h>
#include <qprocess.h>
#include <qtextcodec.h>
#include <qregexp.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include <locale.h>

#include "ydpdictionary.h"
#include "kydpdict.h"

KeyEater* keyEater;

struct dictionaryCache {
    int wordCount;
    char **words;
    unsigned long *indexes;
} dictCache[3];

#define color1 cnf->kFontKolor1
#define color2 cnf->kFontKolor2
#define color3 cnf->kFontKolor3
#define color4 cnf->kFontKolor4

#define T_PL 1
#define T_CM 2
#define T_PN_L 4
#define T_PN_R 8
#define T_SL_L 16
#define T_SL_R 32

#define _native 0
#define _foreign 1

#define TABLE_PHONETIC_ISO { \
  ".", ".", "<img src=\"f2\">", "<img src=\"f3\">", ".", "<img src=\"f5\">", "e", "<img src=\"f6\">", \
  "<img src=\"f1\">", "<img src=\"f8\">", "<img src=\"f4\">", "<img src=\"f7\">", "¦", ":", "´", ".", \
  "&#331;", ".", ".", ".", ".", ".", ".", "&#240;", \
  "&#230;", ".", ".", ".", "¶", ".", ".", "¼", \
  " ", "¡", "¢", "£", "¤", "¥", "¦", "§", \
  "¨", "©", "ª", "«", "¬", "­", "®", "¯", \
  "°", "±", "²", "³", "´", "µ", "¶", "·", \
  "¸", "±", "º", "»", "¼", "½", "¾", "¿", \
  "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "Ç", \
  "È", "É", "Ê", "Ë", "Ì", "Í", "Î", "Ï", \
  "Ð", "Ñ", "Ò", "Ó", "Ô", "Õ", "Ö", "×", \
  "Ø", "Ù", "Ú", "Û", "Ü", "Ý", "Þ", "ß", \
  "à", "á", "â", "ã", "ä", "å", "æ", "ç", \
  "è", "é", "ê", "ë", "ì", "í", "î", "ï", \
  "ð", "ñ", "ò", "ó", "ô", "õ", "ö", "÷", \
  "ø", "ù", "ú", "û", "ü", "ý", "þ", "ÿ" } \

#include "tips.h"

int ydpDictionary::GetTipNumber(int type) {
    switch (type) {
	case 0:
	    return I_size+II_size;
	    break;
	case 1:
	    return I_size;
	    break;
	case 2:
	    return II_size;
	    break;
	default:
	    return -1;
	    break;
    }
}

QString ydpDictionary::GetInputTip(int index) {
static QString input_tip[] = INPUT_TIP;
    if (index <= GetTipNumber(0))
	return input_tip[index];
    else
	return QString("");
}

QString ydpDictionary::GetOutputTip(int index) {
static QString output_tip[] = OUTPUT_TIP;	// guess what happens with 'ñ' when this is not private...
    if (index <= GetTipNumber(0))
	return output_tip[index];
    else
	return QString("");
}

unsigned long fix32(unsigned long x)
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

unsigned short fix16(unsigned short x)
{
#ifndef WORDS_BIGENDIAN
	return x;
#else
	return (unsigned short)
		(((x & (unsigned short) 0x00ffU) << 8) |
                 ((x & (unsigned short) 0xff00U) >> 8));
#endif
}

ydpDictionary::ydpDictionary(kydpConfig *config, QListBox *listBox)
{
    int i;

    dictList = listBox;
    cnf = config;
    keyEater = new KeyEater(this);
    dictList->installEventFilter(keyEater);
    for (i=0;i<4;i++) {
        dictCache[i].wordCount = -1;
        dictCache[i].indexes = NULL;
        dictCache[i].words = NULL;
    }
}

ydpDictionary::~ydpDictionary()
{
    int i,j;
    
    for (i=0;i<4;i++) {
	if (dictCache[i].wordCount>0) {
	    if (dictCache[i].indexes) delete [] dictCache[i].indexes;
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

int ydpDictionary::OpenDictionary(kydpConfig *config)
{
    return OpenDictionary(config->topPath,config->indexFName,config->dataFName);
}

int ydpDictionary::OpenDictionary(QString path, QString index, QString def)
{
    int i;

    /* open index and definition files */
    fIndex.setName( path + "/" + index );
    if (!(fIndex.open(IO_ReadOnly))) {
	fIndex.setName(path + "/" + index.upper());
	if (!(fIndex.open(IO_ReadOnly))) {
	    QMessageBox::critical(0, "kydpdict", tr( "Can't open index file!\n" ) + fIndex.name() );
	    return 1;
	}
    }

    fData.setName( path + "/" + def );
    if (!(fData.open(IO_ReadOnly))) {
	fData.setName(path + "/" + def.upper());
	if (!(fData.open(IO_ReadOnly))) {
	    QMessageBox::critical(0, "kydpdict", tr( "Can't open data file!\n" ) + fData.name() );
	    return 1;
	}
     }

    /* 0-english, eng->pol; 1-english, pol->eng; 2-german, ger->pol; 3-german, pol->ger */
    i = 0;
    if (cnf->language == LANG_DEUTSCH)
	i+=2;
    if (!(cnf->toPolish))
	i++;
    if (dictCache[i].wordCount>0) {
	wordCount = dictCache[i].wordCount;
	indexes = dictCache[i].indexes;
	words = dictCache[i].words;
    } else {
	FillWordList();
	dictCache[i].wordCount = wordCount;
	dictCache[i].indexes = indexes;
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

int ydpDictionary::CheckDictionary(kydpConfig *config)
{
    return CheckDictionary(config->topPath,config->indexFName,config->dataFName);
}

int ydpDictionary::CheckDictionary(QString path, QString index, QString def)
{
    QFile f;

    f.setName( path + "/" + index);
    if ( !(f.exists()) ) {
	f.setName( path + "/" + index.upper() );
        if ( !(f.exists()) )
    	    return 0;
    }
    f.setName( path + "/" + def);
    if ( !(f.exists()) ) {
	f.setName( path + "/" + index.upper() );
	if ( !(f.exists()) )
	    return 0;
    }
    return 1;
}

void ydpDictionary::CloseDictionary()
{
    dictList->clear();
    fIndex.close();
    fData.close();
}

QString ydpDictionary::convert_cp1250(char *tekst, int size)
{
    QString out;
    unsigned char *input = (unsigned char *)tekst;
    unsigned char ch;
    char a;
    int i;

    if (cnf->unicodeFont) {
	// prawdopodobnie jesli to jest wlaczone, to czesc zabaw z lokalami jest zbedna
	const static int table_cp_unicode[] = {
	    0x002e, 0x002e, 0x0254, 0x0292, 0x002e, 0x0283, 0x0065, 0x028c,	// 80-87
	    0x0259, 0x03b8, 0x026a, 0x0251, 0x015a, 0x02d0, 0x00b4, 0x002e,	// 88-8f
	    0x014b, 0x002e, 0x002e, 0x002e, 0x002e, 0x002e, 0x002e, 0x00f0,	// 90-97
	    0x00e6, 0x002e, 0x002e, 0x002e, 0x015b, 0x002e, 0x002e, 0x017a,	// 98-9f
	    0x002e, 0x0104, 0x002e, 0x0141, 0x00a4, 0x013d, 0x015a, 0x00a7,	// a0-a7
	    0x00a8, 0x0160, 0x015e, 0x0164, 0x0179, 0x00ad, 0x017d, 0x017b,	// a8-af
	    0x00b0, 0x0105, 0x002e, 0x0142, 0x00b4, 0x013e, 0x015b, 0x002e,	// b0-b7
	    0x00b8, 0x0105, 0x015f, 0x0165, 0x017a, 0x002e, 0x017e, 0x017c,	// b8-bf
	    0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x013d, 0x0106, 0x00c7,	// c0-c7
	    0x010c, 0x00c9, 0x0118, 0x00cb, 0x0114, 0x00cd, 0x00ce, 0x010e,	// c8-cf
	    0x00d0, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,	// d0-d7
	    0x0158, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x0162, 0x00df,	// d8-df
	    0x0155, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x013e, 0x0107, 0x00e7,	// e0-e7
	    0x010d, 0x00e9, 0x0119, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x010f,	// e8-ef
	    0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,	// f0-f7
	    0x0159, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x0163, 0x002e,	// f8-ff
        };

	for (i=0; i!=size; i++) {
	    if (*input > 127) {
		// bierzemy z tabeli
		ch = *input;
	        out += QChar(table_cp_unicode[*input-128]);
	    } else {
		a = *input;
		out += a;
	    }
	    input++;
	}
    } else {
	const static char *table_cp_phonetic[]=TABLE_PHONETIC_ISO;
	int j;

	for (i=0; i!=size; i++) {
	    if (*input > 127) {
		j=0;
		do {
		    ch = table_cp_phonetic[*input-128][j];
		    out += ch;
		    j++;
		} while (table_cp_phonetic[*input-128][j]!='\0');
	    } else {
		a = *input;
		out += a;
	    }
	    input++;
	}
    }
    return out;
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

void ydpDictionary::FillWordList()
{
    unsigned long pos;
    unsigned long index[2];
    unsigned short wcount;
    int current=0;
    /* for mmap */
    int f;
    char *filedata;
    size_t page_size;
    unsigned int length;

    /* read # of words */
    wordCount=0;
    fIndex.at(0x08);
    fIndex.readBlock((char*)&wcount,2);
    wordCount = (int)fix16(wcount);

    indexes = new unsigned long [wordCount+2];

    words = new char* [wordCount+1];
    words[wordCount] = 0;

    /* read index table offset */
    pos=0;
    fIndex.at(0x10);
    fIndex.readBlock((char*)&pos, 4);
    pos=fix32(pos);

    /* prepare mmap stuff */
    f = open(fIndex.name(), O_RDONLY);
    page_size = (size_t)sysconf(_SC_PAGESIZE);
    if (page_size==0) {
	printf("codename: yellow page!!! mail this to ytm@elysium.pl\n");
	page_size = 4096;
    }
    length = ((fIndex.size() / page_size)+1)*page_size;
    filedata = (char*)mmap(NULL, length, PROT_READ, MAP_PRIVATE, f, 0);

    if ((int)filedata > 0) {
	do {
	    indexes[current] = *(int*)&filedata[pos+4];
	    indexes[current] = fix32(indexes[current]);
	    words[current] = new char [(filedata[pos])];
	    strcpy(words[current], &filedata[pos+8]);
	    pos += 8+1+strlen(words[current]);
	} while (++current < wordCount);
    } else {
	fIndex.at(pos);
	do {
	//  trick - instead of fssek(cur+4), read ulong we read ulong twice
	//  and throw out first 4 bytes
	    fIndex.readBlock((char*)&index[0], 8);
	    indexes[current]=fix32(index[1]);
	//  and another trick
	//  we don't throw out first 4 bytes :)
	    words[current] = new char [fix32(index[0]) & 0xff];
	    fIndex.readBlock(words[current], fix32(index[0]) & 0xff);
	} while (++current < wordCount);
    }
    munmap((void*)filedata, length);
    close(f);

    topitem = 0;

    /* omijanie bledow w slowniku... */
    if ((cnf->language==LANG_ENGLISH) && (cnf->toPolish)) {
	current = FindWord(QString("Proven"));
	delete [] words[current];
	words[current] = new char [strlen("Provencial")];
	strcpy(words[current],"Provencial");
	broken_entry = current;
    }
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
    for (; (i<w1.length()) && (i<w2.length()); i++)
	if (w1.at(i) != w2.at(i))
	    break;
    return i;
}

QString ydpDictionary::stripDelimiters(QString word)
{
    QString lstring;

    unsigned int i;
    for (i=0; i<word.length(); i++) {
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
    unsigned long dsize, size;
    char *def;

    dsize=0;
    fData.at(indexes[index]);
    fData.readBlock((char*)&dsize, sizeof(unsigned long));
    dsize=fix32(dsize);

    def = new char[dsize+1];
    if ((size = fData.readBlock(def,dsize)) !=dsize) return -1;
    def[size] = 0;
    curDefinition=QString(def);
    if (index == broken_entry) {
	curDefinition.replace(QRegExp("Proven.al"),"Provencial");
    }
    delete [] def;
    return 0;
}

int ydpDictionary::Play (int index, kydpConfig *config)
{
    QFile fd;
    QString pathdir, name, ext;
    static QProcess proc;

    switch (config->language) {
	case LANG_DEUTSCH:
	    pathdir = config->cd2Path;
	    break;
	case LANG_ENGLISH:
	    pathdir = config->cdPath;
	    break;
	default:
	    break;	// cos poszlo nie tak?
    }

    fd.setName(pathdir.latin1());
    if (!(fd.exists()))
	return 0;	// nie ma co tracic czasu jesli katalogu nie ma

    name.sprintf("%s/s%.3d/%.6d.", pathdir.latin1(), index/1000, index+1);

    ext = "wav";
    fd.setName(name+ext);
    if (!(fd.exists())) {
	ext = "mp3";
	fd.setName(name+ext);
	if(!(fd.exists())) {
	    ext = "ogg";
	    fd.setName(name+ext);
	    if(!(fd.exists()))
		return 0;
	}
    }

    fd.setName(config->player);
    if (!(fd.exists()))
	return 0;		// oh well...

    if (proc.isRunning())
	proc.kill();		// nie ma litosci dla skurwysynow! BUM! BUM!

    proc.clearArguments();
    proc.addArgument( config->player );
    proc.addArgument( name+ext );
    proc.start();

    return 1;
}

QString ydpDictionary::rtf2html (QString definition)
{
char token[1024];
int tp, level = 0;
bool sa_tag = FALSE,br_tag = FALSE, italic_tag = FALSE, link_tag = TRUE;
QString tag_on,tag_off;
char *def = const_cast<char*>((const char*)definition);
list.clear();
list += "<qt><font color=\"" + color4 +"\" link=\"" + color4 + "\">";
list += "</font></qt>";
for(level=15;level>=0;level--) {
	tag_num[level] = 0;
	direction_tab[level] = 0;	
}
level = 0;
direction_tab[level] = _native;
it = list.begin();
it++;


  while (*def) {
  	switch(*def) {
  		case '{':
			if (level < 16) tag_num[++level] = 0;
			direction_tab[level] = _native;
  				break;
  		case '\\':
			def++; tp = 0;
			while((*def >= 'a' && *def <= 'z') || (*def >='0' && *def <= '9'))
				token[tp++] = *def++;
			token[tp] = 0;
			if (*def == ' ') def++;

			if (!strcmp(token, "i")) { //	109891 -ilosc wystapien -  optymalizacja pod plik ang->pol :)))))))
				if(cnf->italicFont) {
					tag_on = "<i>";
					tag_off = "</i>";
					tag_num[level]++;
				}
				direction_tab[level] = _native;
			//	italic_tag = TRUE;
			} else

			if (!strcmp(token, "cf2")) { //	79850
				tag_on = "<font color=" + color2 + ">";
				tag_off = "</font>";
				tag_num[level]++;
				direction_tab[level] = _native; // vi - trzeba to przemyslec jeszcze
				link_tag = TRUE;
			} else

			if (!strcmp(token, "par")) { //74442
				tag_on = "<br>";
				tag_off =  "";
			} else

			if (!strcmp(token, "line")) { //	61703
				tag_on = "<br>";
				tag_off = "";
				if(br_tag)
					tag_on="";
				br_tag = FALSE;
			} else

			if (!strcmp(token, "cf1")) { //	55929
				tag_on = "<font color=" + color1 + ">";
				tag_off = "</font>";
				tag_num[level]++;
				direction_tab[level] = _foreign;
			} else

			if (!strcmp(token, "b")) { //	43737
				tag_on = "<b>";
				tag_off = "</b>";
				tag_num[level]++;
			} else

			if (!strcmp(token, "cf0")) { // 22607
				tag_on = "<font color=" + color3 + ">";
				tag_off = "</font>";
				tag_num[level]++;
				direction_tab[level] = _native;
			} else

			if (!strcmp(token, "cf5")) { // 21478
				tag_on = "<font color=" + color4 + ">";
				tag_off = "</font>";
				tag_num[level]++;
//				direction_tab[level] = _native;
				link_tag = FALSE;
			} else
			
			if (!strcmp(token, "f1")) { // ????
				tag_on = "";
				tag_off = "";
				link_tag = FALSE;
			} else
			
			if (!strcmp(token, "f2")) { // ????
				tag_on = "";
				tag_off = "";
				link_tag = TRUE;
			} else

			if (!strncmp(token, "sa", 2)) { // 19969
				tag_on = "<table cellspacing=0><tr><td width=25></td><td>";
				tag_off =  "</td></tr></table>";
				sa_tag = TRUE;
				tag_num[level]++;
			} else

			if (!strcmp(token, "b0")) { // 2130
				tag_on = "";
				tag_off = "";
				disableTag (1, level);

			} else

			if (!strcmp(token, "super")) { // 92
				tag_on = "<small>";
				tag_off = "</small>";
				tag_num[level]++;
//				direction_tab[level] = _off;
			} else

			if (!strcmp(token, "i0")) { // 69
				tag_on = "";
				tag_off = "";
				italic_tag = FALSE;
				disableTag (2, level);
			} else

			if (!strcmp(token, "cf4")) {
				tag_on = "<font color=" + color3 + ">";
				tag_off = "</font>";
				tag_num[level]++;
				direction_tab[level] = _native;
			}

			def--;

			if(!tag_on.isEmpty()) {
				it = list.insert(it, tag_on); it++;
				tag_on = "";
				if (!tag_off.isEmpty()) {
					it = list.insert(it, tag_off);
					tag_off = "";
				}
  			}
			break;
		case '}':
			while(tag_num[level] > 0) {
				it++;
				tag_num[level]--;
			}
			level--;
			italic_tag = FALSE;
			link_tag = TRUE;
			if(sa_tag) {
				it++; //tabela ma tylko obejmowac 1 pozycje a nie do konca tekstu
				sa_tag = FALSE;
				br_tag = TRUE; //nie ³am linii za wcieciem bo qt samo to robi, czemu?
			}
			break;
		default:
			tp = 0;
			while(!(*def == '{' || *def == '}' || *def == '\\') && *def) {
				if (*def == 0x7f) (*def )--;
				if (*def == '<') {
					token[tp++]='&'; //// paskudnie to wyglada - jak chcesz to zmien zapis :)
					token[tp++]='l'; // takie sa uroki uzywania C w C++ :-P
					token[tp++]='t';
					*def=';';
				}
				if (*def == '>') {
					token[tp++]='&';
					token[tp++]='g';
					token[tp++]='t';
					*def = ';';
				}
 	    			token[tp++] = *def++;
			}
    			token[tp] = '\0';
			def--;
			QString tmp = convert_cp1250(token, tp);

			if((cnf->language == LANG_ENGLISH) && cnf->toolTips && link_tag) {
				tmp = insertHyperText(tmp, level);
			}
			it = list.insert(it, tmp); it++;
			break;
	}
  	def++;
  }
  return list.join("");
}

void ydpDictionary::disableTag (int tag_code, int level)
{

QString a_on, a_off, p_on, p_off;
int color_off=0, italic_off=0;
QStringList::Iterator wsk;
int wstawiono=0;

	if(tag_code == 1) {
		a_on = "<b>";
		a_off = "</b>";
		p_on = "<i>";
		p_off = "</i>";
	} else if(tag_code == 2) {
		a_on = "<i>";
		a_off = "</i>";
		p_on = "<b>";
		p_off = "</b>";
	} else return;

	for(wsk = it, wsk--; wsk != list.begin(); wsk--) {
		if(!strcmp(*wsk, a_on)) {
			it = list.insert(it, a_off); it++;
			break;
		} else if(!strcmp(*wsk, a_off)) {
			break;
		} else if(!strcmp(*wsk, "</font>")) {
			color_off++;
		} else if(!strcmp(*wsk, p_off)) {
			italic_off++;
		} else if(!strcmp((*wsk).left(5), "<font")) {
			if(color_off > 0) color_off--;
			else {
				it = list.insert(it, *wsk);
				it = list.insert(it, "</font>"); it++;
				wstawiono++;
			}
		} else if(!strcmp(*wsk, p_on)) {
			if(italic_off > 0) italic_off--;
			else {
				it = list.insert(it, *wsk);
				it = list.insert(it, p_off); it++;
				wstawiono++;
			}
		}
	}

	int j = wstawiono;

	while(j > 0) {
		it++;
		j--;
	}

	for(wsk = it; wsk != list.end(); wsk++) {
		if(!strcmp(*wsk, a_off)) {
			if(wstawiono > 0) {
				j = wstawiono;
				wsk = it;
				wsk++;
				while (j-- > 0 && strcmp(*wsk, a_off)) {
					wsk ++;
					tag_num[level]++;
				}
				list.insert(wsk, a_on);
			} else
				it = list.insert(it, a_on);

			tag_num[level]++;

			break;
		} else if(!strcmp(*wsk, a_on)) {
			break;
		}
	}
}

QString ydpDictionary::insertHyperText(QString raw_input, int level)
{

	int tags = 0, pos = 0, pos2 = 0;
	bool result;
	QString output, output2, fullOutput, number, proposition;

	QString input = raw_input.simplifyWhiteSpace();

	while(TRUE) {
		QString tmp = input.section(' ', pos, pos);

		if(tmp.startsWith("(")) {
			tags |= T_PN_L;
			tmp = tmp.right(tmp.length()-1);
		}

		if(tmp.endsWith(")")) {
			tags |= T_PN_R;
			tmp = tmp.left(tmp.length()-1);
		}

		if(tmp.endsWith(",")) {
			tags |= T_CM;
			tmp  = tmp.left(tmp.length()-1);
		}

		if(tmp.startsWith("+")) {
			tags |= T_PL;
			tmp  = tmp.right(tmp.length()-1);
		}

		pos2 = 0;
		output2 = "";
		QString direction;
		direction_tab[level] == _native ? direction = "0" : direction = "1"; 
		result = FALSE;

		if(tmp.startsWith("/")) {
			tags |= T_SL_L;
			tmp = tmp.right(tmp.length()-1);
		}

		if(tmp.endsWith("/")) {
			tags |= T_SL_R;
			tmp = tmp.left(tmp.length()-1);
		}

		while(TRUE) {
			QString tmp2 = tmp.section('/', pos2, pos2);

			proposition = tmp2;

			for(int i = 0; i < I_size+II_size; i++) {
				if (!QString::compare(tmp2, GetInputTip(i))) {
					number.sprintf("%d", i);
					proposition = "<a href=2" + number +">" + tmp2 + "</a>";
					result = TRUE;
					break;
				}
			}

			if(!result) {
				proposition = "<a href="+ direction + tmp2+">"+tmp2+"</a>";
			}

			output2 += proposition;

			pos2++;

			if(tmp.section('/', pos2, pos2).isEmpty())
				break;
			output2 += '/';
		}

		if(tags & T_SL_L)
			output2 = '/' + output2;

		if(tags & T_SL_R)
			output2 += '/';

		output += output2;
		output2 = "";

		if(tags & T_CM)
			output += ",";

		if(tags & T_PL)
			output = "+" + output;

		if(tags & T_PN_L)
			fullOutput += "(";

		fullOutput += output;
		output = "";

		if(tags & T_PN_R)
			fullOutput += ")";

		tags = 0;
		pos++;

		if( input.section(' ', pos, pos).isEmpty() )
			break;
		fullOutput +=" ";
	}

	if(raw_input.startsWith(" "))
		fullOutput = " " + fullOutput;
	if(raw_input.endsWith(" "))
		fullOutput += " ";
	return fullOutput;
}
