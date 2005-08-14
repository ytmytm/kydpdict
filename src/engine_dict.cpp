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
// for gzfile
#include <zlib.h>

#include "kydpconfig.h"
#include "engine_dict.h"
#include "engine_dict.moc"

EngineDICT::EngineDICT(kydpConfig *config, QListBox *listBox, ydpConverter *converter) : ydpDictionary(config, listBox, converter)
{
    for (int i=0; i<2; i++) {
    	dictCache_LL[i].offsets = NULL;
	dictCache_LL[i].lengths = NULL;
    }
}

EngineDICT::~EngineDICT()
{
    for (int i=0; i<2; i++) {
	if (this->dictCache[i].wordCount>0) {
	    if (this->dictCache[i].words) {
		delete [] dictCache_LL[i].offsets;
		delete [] dictCache_LL[i].lengths;
	    }
	}
    }
}

int EngineDICT::OpenDictionary(void)
{
    int i;
    QFile fData;

    /* open index and definition files */
    UpdateFName();
    fIndex.setName( cnf->topPath + "/" + cnf->indexFName );
    if (!(fIndex.open(IO_ReadOnly))) {
	fIndex.setName( cnf->topPath + "/" + cnf->indexFName.upper());
	if (!(fIndex.open(IO_ReadOnly))) {
	    QMessageBox::critical(0, "Kydpdict", tr( "Can't open index file!\n" ) + fIndex.name() );
	    return 1;
	}
    }

    fData.setName( cnf->topPath + "/" + cnf->dataFName );
    if (!(fData.open(IO_ReadOnly))) {
	fData.setName( cnf->topPath + "/" + cnf->dataFName.upper());
	if (!(fData.open(IO_ReadOnly))) {
	    QMessageBox::critical(0, "Kydpdict", tr( "Can't open data file!\n" ) + fData.name() );
	    return 1;
	}
    }
    fData.close();

    zData = gzopen(fData.name(), "r");
    if (zData == NULL)
	return 1;

    i = 0;
//    if (!(this->cnf->toPolish)) i++;
    if (this->dictCache[i].wordCount>0) {
	offsets = dictCache_LL[i].offsets;
	lengths = dictCache_LL[i].lengths;
    } else {
    	FillWordList();
	dictCache_LL[i].offsets = offsets;
	dictCache_LL[i].lengths = lengths;
    }
    return ydpDictionary::OpenDictionary();	// required call
}

int EngineDICT::CheckDictionary(void)
{
    QFile f;

    if (cnf->language != LANG_ENGLISH)
	return 0;
    if (! cnf->toPolish)
	return 0;

    UpdateFName();
    f.setName( cnf->topPath + "/" + cnf->indexFName );
    if ( !(f.exists()) ) {
    	f.setName( cnf->topPath + "/" + cnf->indexFName.upper() );
	if ( !(f.exists()) )
    	    return 0;
    }
    f.setName( cnf->topPath + "/" + cnf->dataFName );
    if ( !(f.exists()) ) {
	f.setName( cnf->topPath + "/" + cnf->dataFName.upper() );
	if ( !(f.exists()) )
    	    return 0;
    }
    return 1;
}

void EngineDICT::CloseDictionary()
{
	fIndex.close();
	gzclose(zData);
	ydpDictionary::CloseDictionary();	// required call
}

void EngineDICT::UpdateFName()
{
//	cnf->indexFName = cnf->toPolish ? "dvp_1.dic" : "dvp_2.dic";
	cnf->topPath = "/usr/share/dictd/";
	cnf->indexFName = "sap_en-pl.index";
	cnf->dataFName = "sap_en-pl.dict.dz";
}

int EngineDICT::Base64Decode(const char *input) {

const static char base64tab[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 			/* 00-0f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,			/* 10-1f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 63,			/* 20-2f */
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0,		/* 30-3f */
    0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,		/* 40-4f */
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0,		/* 50-5f */
    0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,	/* 60-6f */
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0 };	/* 70-7f */

    int i;
    int result = 0;
    int c;
    for (i=0;(input[i]) && (input[i]!='\n');i++) {
	result *= 64;
	c = input[i];
	if ( (c<0) || (c>0x7F) )
	    c = 0;
	result += base64tab[c];
    }

    return result;
}

void EngineDICT::FillWordList()
{
    int i = 0;
    int j, l;
    char buf[256];		// XXX arbitrary
    char *start;
    char *len;

    wordCount = 40000;		// XXX arbitrary
    words = new char* [wordCount];
    offsets = new unsigned int [wordCount];
    lengths = new unsigned int [wordCount];
    fIndex.at(0);
    while ( fIndex.readLine(buf,255) > 0 ) {
	// analyze this
	l = strlen(buf);
	j = 0;
	while ( (j<l) && (buf[j]!='\t') ) { j++; };	// skip word part
	buf[j] = '\0'; j++; start = &buf[j];		// get offset b64
	words[i] = new char [j];			// copy word
	strcpy(words[i], buf);
	while ( (j<l) && (buf[j]!='\t') ) { j++; };	// skip start part
	buf[j] = '\0'; j++; len = &buf[j];		// get length b64
	offsets[i] = Base64Decode(start);
	lengths[i] = Base64Decode(len);
	i++;
    }
    wordCount = i;
}

int EngineDICT::ReadDefinition(int index)
{
    char *buf = new char [lengths[index]+10];		// safeguard at the end

    if (gzseek(zData, offsets[index], SEEK_SET) < 0)
	return 1;
    if (gzread(zData, buf, lengths[index]) < 0)
	return 1;
//    gzrewind(zData);	// unneeded I presume

    buf[lengths[index]] = '\0';	// sometimes junk at the end
    curDefinition = buf;
    delete buf;

    return 0;
}

QString EngineDICT::rtf2html(QString definition) {
    QString tmp = definition;
    tmp.replace("\n","<br>");
    tmp = "<qt type=\"page\"><font color=" + color4 + ">" + tmp + "</font></qt>";
    return tmp;
}

////////////

ConvertDICT::ConvertDICT(void) {
    codec = QTextCodec::codecForName("ISO8859-2");
}

ConvertDICT::~ConvertDICT() {

}

char ConvertDICT::toLower(const char c) {
    const static char upper_cp[] = "A¡BCÆDEÊFGHIJKL£MNÑOÓPQRS¦TUVWXYZ¬¯";
    const static char lower_cp[] = "a±bcædeêfghijkl³mnñoópqrs¶tuvwxyz¼¿";

    unsigned int i;
    for (i=0;i<sizeof(upper_cp);i++)
	if (c == upper_cp[i])
	    return lower_cp[i];
    return c;
}

int ConvertDICT::charIndex(const char c) {
    const static char lower_cp[] = "a±bcædeêfghijkl³mnñoópqrs¶tuvwxyz¼¿";
    unsigned int i;
    for (i=0;i<sizeof(lower_cp);i++)
	if (c == lower_cp[i])
	    return i;
    return sizeof(lower_cp)+1;
}

char *ConvertDICT::toLocal(const char *input) {
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

QString ConvertDICT::toUnicode(const char *input) {
    return codec->toUnicode(input);
}

QCString ConvertDICT::fromUnicode(QString input) {
    return codec->fromUnicode(input);
}

QString ConvertDICT::convertChunk(const char *inp, int size, bool unicodeFont) {
    return codec->toUnicode(inp);
}
