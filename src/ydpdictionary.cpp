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

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include <locale.h>

#include "ydpdictionary.h"

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
  "<img src=\"f1\">", "<img src=\"f8\">", "<img src=\"f4\">", "<img src=\"f7\">", "�", ":", "�", ".", \
  "&#331;", ".", ".", ".", ".", ".", ".", "&#240;", \
  "&#230;", ".", ".", ".", "�", ".", ".", "�", \
  "�", "�", "�", "�", "�", "�", "�", "�", \
  "�", "�", "�", "�", "�", "�", "�", "�", \
  "�", "�", "�", "�", "�", "�", "�", "�", \
  "�", "�", "�", "�", "�", "�", "�", "�", \
  "�", "�", "�", "�", "�", "�", "�", "�", \
  "�", "�", "�", "�", "�", "�", "�", "�", \
  "�", "�", "�", "�", "�", "�", "�", "�", \
  "�", "�", "�", "�", "�", "�", "�", "�", \
  "�", "�", "�", "�", "�", "�", "�", "�", \
  "�", "�", "�", "�", "�", "�", "�", "�", \
  "�", "�", "�", "�", "�", "�", "�", "�", \
  "�", "�", "�", "�", "�", "�", "�", "�" } \

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
static QString output_tip[] = OUTPUT_TIP;	// guess what happens with '�' when this is not private...
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

QString ydpDictionary::convert_cp1250(char *tekst, long int size)
{
    const char *table_cp_phonetic[]=TABLE_PHONETIC_ISO;
    unsigned char *input;
    unsigned char ch;
    long int i,j;
    char a;
    QString out;

    input = (unsigned char *)tekst;

    for (i=0; i!=size; i++) {
	if (*input > 127) {
	    j=0;
	    do {
		ch=table_cp_phonetic[*input-128][j];
		out +=ch;
		j++;
	    } while (table_cp_phonetic[*input-128][j]!='\0');
	} else {
	    a = *input;
	    out += a;
	}
	input++;
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
	QTextCodec *codec = QTextCodec::codecForName("CP1250");
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
	QTextCodec *codec = QTextCodec::codecForName("CP1250");
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
    QTextCodec *codec = QTextCodec::codecForName("CP1250");
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

// odpowiednik tego chyba jest w QString - ile znakow z poczatku w1 jest w2
int ydpDictionary::ScoreWord(const char *w1, const char *w2)
{
    int i=0;
    int score=0;
    while (w1[i] && w2[i] && (score <=0)) {
	if (tolower(w1[i]) == tolower(w2[i]))
	    score--;
	else {
	    if (score == 0) return score;
	    score=-score;
	}
	i++;
    }
    if (score<0) score=-score;

    return score;
}

int ydpDictionary::FindWord(QString word)
{
    QTextCodec *toCodec = QTextCodec::codecForName("CP1250");
    QTextCodec *fromCodec = QTextCodec::codecForName("ISO8859-2");
    QString midstring;
    QCString qcword, qmword, qaword, qbword;
    int a,b,i,sa,sb,lword;
    int middle,result;

    if (word.length() == 0)
	return 0;

    a = 0; b = wordCount;

    word = word.lower();

    setlocale(LC_COLLATE,"pl_PL");	// otherwise '�win'->'z...' on at least one system :(

    while (b-a >= 30) {	/* bez tego marginesu s� problemy np. z 'for' */
	middle = a + (b-a)/2;
	midstring = toCodec->toUnicode(words[middle]);
	result = word.localeAwareCompare(midstring.lower());
	if (result==0) {
	    a = middle; b = middle;
	} else {
	    if (result<0)
		b = middle;
	    else
		a = middle;
	}
    }

    if (a!=b) {
	a=a-35; if (a<0) a=0; i=1;
	b=a;
	qcword = fromCodec->fromUnicode(word);
	lword = word.length();
	while ((i<70) && (a+i<wordCount)) {
	/* co tu si� dzieje: ScoreWord u�ywa tolower w pl_PL, wi�c musimy kodowa� na iso
	    - kodujemy word na iso
	    - kodujemy words[a+i] na unicode, unicode na iso (jak od razu?)
	    - kodujemy words[b] na unicode, unicode na iso (jak od razu?) */
	    qaword = fromCodec->fromUnicode(toCodec->toUnicode(words[a+i]));
	    qbword = fromCodec->fromUnicode(toCodec->toUnicode(words[b]));
	    sa = ScoreWord(qcword,qaword);
	    sb = ScoreWord(qcword,qbword);
	    if ((sa==lword)&&((int)strlen(words[a+i])<=sa)) {
		b=a+i; i=260;
	    } else {
		if (sa>sb) {
		    b=a+i;
		    if (!strcmp(words[b],qcword))
			i=260;
		}
	    }
	    i++;
	}
	a=b;
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
				br_tag = TRUE; //nie �am linii za wcieciem bo qt samo to robi, czemu?
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
			QString tmp = convert_cp1250(token, (long int) tp);

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
