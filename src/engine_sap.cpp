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
#include <qtextcodec.h>

// for memset
#include <string.h>

#include "kydpconfig.h"
#include "engine_sap.h"
#include "engine_sap.moc"

#define A_BOLD 		0x00000001
#define A_ITALIC	0x00000002
#define A_COLOR1	0x00000004
#define A_COLOR2	0x00000008
#define A_COLOR3	0x00000010
#define A_COLOR4	0x00000020
#define A_SUPER		0x00000040

EngineSAP::EngineSAP(kydpConfig *config, QListBox *listBox, ydpConverter *converter) : ydpDictionary(config, listBox, converter)
{
    for (int i=0; i<2; i++)
    	dictCache_LL[i].definitions = NULL;
}

EngineSAP::~EngineSAP()
{
    for (int i=0; i<2; i++) {
	if (this->dictCache[i].wordCount>0) {
	    if (this->dictCache[i].words) {
		for (int j=0; j<this->dictCache[i].wordCount; j++) {
		    delete [] dictCache_LL[j].definitions;
		}
	    }
	}
    }
}

int EngineSAP::OpenDictionary(void)
{
    int i;

    /* open index and definition files */
    UpdateFName();
    fData.setName(cnf->topPath + "/" + cnf->indexFName);

    if (!(fData.open(IO_ReadOnly))) {
		QMessageBox::critical(0, "Kydpdict",
		    tr( "Can't open dictionary data file!\n"
		    "Make sure your installation is OK!"));
		return 1;
    }

    i = 0;
    if (!(this->cnf->toPolish)) i++;
    if (this->dictCache[i].wordCount>0) {
    	definitions = dictCache_LL[i].definitions;
    } else {
    	FillWordList();
    	dictCache_LL[i].definitions = definitions;
    }
    return ydpDictionary::OpenDictionary();	// required call
}

int EngineSAP::CheckDictionary(void)
{
    QFile f;
    unsigned int test = 0;

    if (cnf->language != LANG_ENGLISH)
	return 0;

    UpdateFName();
    f.setName( cnf->topPath + "/" + cnf->indexFName );
    if ( !(f.exists()) ) {
    	f.setName( cnf->topPath + "/" + cnf->indexFName.upper() );
	if ( !(f.exists()) )
    	    return 0;
    }
    f.open(IO_ReadOnly);
    f.readBlock((char*)&test, 4);		// read magic
    f.close();
    if (fix32(test)!=fix32(0xfadeabba))
	return 0;
    return 1;
}

void EngineSAP::CloseDictionary()
{
	fData.close();
	ydpDictionary::CloseDictionary();	// required call
}

void EngineSAP::UpdateFName()
{
	cnf->indexFName = cnf->toPolish ? "dvp_1.dic" : "dvp_2.dic";
}

void EngineSAP::FillWordList()
{
	int magic;
	int npages;
	int *pages_offsets;
	int wordspp, dsize, dvoffset;
	char *body;
	int curpage, curword, i;
	short wordlen, wordspps, dsizes, dvoffsets;

	this->wordCount = 0;
	fData.readBlock((char*)&magic,4);
	fData.readBlock((char*)&this->wordCount,4);
	fData.readBlock((char*)&npages,4);
	this->wordCount = this->fix32(this->wordCount);
	npages = this->fix32(npages);
	magic = this->fix32(magic);
	this->words = new char* [this->wordCount];
	definitions = new char* [this->wordCount+1];
	pages_offsets = new int [4*npages];
	fData.readBlock((char*)pages_offsets,4*npages);

	curword = 0;
	body = new char [16384];
	for (curpage=0;curpage<npages;curpage++) {
		fData.at(pages_offsets[curpage]);
		wordspp = dsize = dvoffset = 0;
		wordspps = dsizes = dvoffsets = 0;
		fData.readBlock((char*)&wordspps,2); wordspp = fix16(wordspps);
		fData.readBlock((char*)&dsizes,2); dsize = fix16(dsizes);
		fData.readBlock((char*)&dvoffsets,2); dvoffset = fix16(dvoffsets);
		fData.readBlock((char*)body,dsize);
		char *current = &body[2*wordspp];
		for (i=0;i<wordspp;i++) {	
			wordlen = strlen(current);
			this->words[curword+i] = new char [wordlen+1];
			strcpy(this->words[curword+i],current);
			this->words[curword+i][wordlen] = '\0';
			current += wordlen +1;
		}
		for (i=0;i<wordspp;i++) {
			wordlen = *(short*)&body[i*2];
			definitions[curword] = new char [wordlen+1];
			memcpy(definitions[curword],current,wordlen);
			definitions[curword][wordlen] = '\0';
			current += wordlen;
			++curword;
		}
	}
	delete body;
}

int EngineSAP::ReadDefinition(int index)
{
    curWord = cvt->toUnicode(words[index]);
    curIndex = index;
    return 0;
}

struct okreslenia { char *nazwa; int flagi; int maska; } okreslenia[]={
{"przymiotnik",1,15},
{"przys³ówek",2,15},
{"spójnik",3,15},
{"liczebnik",4,15},
{"partyku³a",5,15},
{"przedrostek",6,15},
{"przyimek",7,15},
{"zaimek",8,15},
{"rzeczownik",9,15},
{"czasownik posi³kowy",10,15},
{"czasownik nieprzechodni",11,15},
{"czasownik nieosobowy",12,15},
{"czasownik zwrotny",13,15},
{"czasownik przechodni",14,15},	/* 16 - 4 bajty */
{"czasownik",15,15},

{"rodzaj ¿eñski",16,0x30},		/* 4 bajty */
{"rodzaj mêski",32,0x30},
{"rodzaj nijaki",16+32,0x30},

{"liczba pojedyncza",64,0xc0},
{"liczba mnoga",128,0xc0},
{"tylko liczba mnoga",128+64,0xc0},

{"czas przesz³y",2048,2048|4096|8192|16384},
{"czas tera¼niejszy",4096,2048|4096|8192|16384},
{"czas przysz³y",2048+4096,2048|4096|8192|16384},
{"bezokolicznik",8192,2048|4096|8192|16384},

{"stopieñ najwy¿szy",16384,2048|4096|8192|16384},
{"regularny",256,256},	/* 3 bajty */
{"wyraz potoczny",1024,1024},
{"skrót",512,512},
{"stopieñ wy¿szy",16384|8192,2048|4096|8192|16384},
{0,0}};

QString EngineSAP::UpdateAttr(int newattr) {

    if (curLine.length() == 0)
	return QString("");

    QString tmp = cvt->convertChunk(curLine);
    QString tmp2;
    curLine = "";

    if (newattr & A_BOLD)
	tmp = "<b>" + tmp + "</b>";
    if ((newattr & A_ITALIC) && cnf->italicFont)
	tmp = "<i>" + tmp + "</i>";
    if (newattr & A_SUPER)
	tmp = "<sup>" + tmp + "</sup>";
    if (newattr & A_COLOR1)
	tmp = "<font color=" + color1 + ">" + tmp + "</font>";
    if (newattr & A_COLOR2)
	tmp = "<font color=" + color2 + ">" + tmp + "</font>";
    if (newattr & A_COLOR3)
	tmp = "<font color=" + color3 + ">" + tmp + "</font>";
    if (newattr & A_COLOR4)
	tmp = "<font color=" + color4 + ">" + tmp + "</font>";

    return tmp;
}

QString EngineSAP::rtf2html(QString definition) {
    QString tmp;
    QCString num;
    char *c = definitions[curIndex];
    int cdoll = 0;
    int level = 0;
    int attr[16];

    attr[level] = 0;
    curLine = curWord;
    tmp += UpdateAttr(A_BOLD|A_COLOR4);
    tmp += " - ";

    curLine = "";
    while ((*c)&&(c!=definitions[curIndex+1])) {
	switch (*c) {
	    case ',':
	    case '.':
	    case ')':
		curLine += *c++;
		if (!strchr(".,",*c)) curLine += ' ';
		continue;
	    case '(':
		curLine += *c++;
		tmp += UpdateAttr(attr[level]);
		continue;
	    case '$':
		if (cdoll) curLine += "; "; else curLine += "<br>";
		tmp += UpdateAttr(attr[level]);
		curLine += "<br>";
		cdoll++;
		num.setNum(cdoll);
		curLine += num;
		curLine += ")<br>&nbsp;";
		tmp += UpdateAttr(A_ITALIC | A_BOLD | A_COLOR3);
		c++;
		continue;
	    case '-':
	    	curLine += " - ";
		c++;
		tmp += UpdateAttr(0);	// color1
		continue;
	    case '{':
		level++;
		attr[level]=attr[level-1];
		curLine += "<br>&nbsp;";
		c++;
		continue;
	    case '}':
		cdoll = 0;
		tmp += UpdateAttr(0);	// color3
		curLine += " - ";
		level--;
		tmp += UpdateAttr(attr[level]);
		c++;
		continue;
	    case '*':
		curLine += curWord;
		tmp += UpdateAttr(attr[level]);
		c++;
		continue;
	    case '=':
		curLine += "&asymp;";
		tmp += UpdateAttr(A_COLOR3);
		c++;
		continue;
	    case '#':
		c++;
		tmp += UpdateAttr(0);
		int v, mc, i;
		v = ((*c++)<<8) & 0xff00;
		v |= (*c++)&255;
		for (i=mc=0; okreslenia[i].nazwa; i++)
		    if ((v & okreslenia[i].maska)==okreslenia[i].flagi) {
			if (mc) curLine+= ',';
			curLine += okreslenia[i].nazwa;
			curLine += "<br>&nbsp;";
			mc = 1;
		    }
		tmp += UpdateAttr(A_COLOR2);
		continue;
	}
	curLine += *c++;
    }
    tmp += UpdateAttr(0);
    tmp = "<qt type=\"page\"><font color=" + color4 + ">" + tmp + "</font></qt>";
    return tmp;
}

////////////

ConvertSAP::ConvertSAP(void) {
    codec = QTextCodec::codecForName("ISO8859-2");
}

ConvertSAP::~ConvertSAP() {

}

char ConvertSAP::toLower(const char c) {
    const static char upper_cp[] = "A¡BCÆDEÊFGHIJKL£MNÑOÓPQRS¦TUVWXYZ¬¯";
    const static char lower_cp[] = "a±bcædeêfghijkl³mnñoópqrs¶tuvwxyz¼¿";

    unsigned int i;
    for (i=0;i<sizeof(upper_cp);i++)
	if (c == upper_cp[i])
	    return lower_cp[i];
    return c;
}

int ConvertSAP::charIndex(const char c) {
    const static char lower_cp[] = "a±bcædeêfghijkl³mnñoópqrs¶tuvwxyz¼¿";
    unsigned int i;
    for (i=0;i<sizeof(lower_cp);i++)
	if (c == lower_cp[i])
	    return i;
    return sizeof(lower_cp)+1;
}

char *ConvertSAP::toLocal(const char *input) {
    int i, j = 0;
    int len = strlen(input);
    static char buffer[256];		// ugly? maybe

    memset(buffer,0,sizeof(buffer));

    for (i=0;i!=len;) {
	if ((input[i] != ' ') && (input[i] != '\t') && (input[i] != '-') && (input[i] !=','))
	    buffer[j++] = toLower(input[i++]);
	else
	    ++i;
    }
    return buffer;
}

QString ConvertSAP::toUnicode(const char *input) {
    return codec->toUnicode(input);
}

QCString ConvertSAP::fromUnicode(QString input) {
    return codec->fromUnicode(input);
}

QString ConvertSAP::convertChunk(const char *inp, int size, bool unicodeFont) {
    return codec->toUnicode(inp);
}
