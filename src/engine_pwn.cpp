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
#include <qregexp.h>

// for memset
#include <string.h>
// for uncompress
#include <zlib.h>

// for mmap
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "kydpconfig.h"
#include "engine_pwn.h"
#include "engine_pwn.moc"

EnginePWN::EnginePWN(kydpConfig *config, QListBox *listBox, ydpConverter *converter) : ydpDictionary(config, listBox, converter)
{

}

EnginePWN::~EnginePWN()
{

}

int EnginePWN::OpenDictionary(void)
{
    QString p;
    int i;

    /* open index and definition files */
    UpdateFName();
    p = cnf->topPath + "/" + cnf->indexFName;
    fIndex.setName(p);
    if (!(fIndex.open(IO_ReadOnly))) {
	  	p = tr( "Can't open dictionary data file!\n"
			"Make sure you have copied the following file\n"
			"from Windows installation:\n" ) + cnf->indexFName;
		QMessageBox::critical(0, "kpwndict", p );
		return 1; };

    i = 0;
    if (dictCache[i].wordCount>0) {
	offsets = dictCache_LL[i].offsets;
	words_base = dictCache_LL[i].words_base;
    } else {
	FillWordList();
	dictCache_LL[i].offsets = offsets;
	dictCache_LL[i].words_base = words_base;
    }
    return ydpDictionary::OpenDictionary();	// required call
}

int EnginePWN::CheckDictionary(void)
{
    QFile f;

    UpdateFName();
    f.setName( cnf->topPath + "/" + cnf->indexFName );
    if ( !(f.exists()) )
    	return 0;
    return 1;
}

void EnginePWN::CloseDictionary()
{
	fIndex.close();
// this should be here???
	delete [] offsets;
	delete [] wordbuffer;
}

void EnginePWN::UpdateFName()
{
	cnf->indexFName= cnf->toPolish ? "angpol.win" : "polang.win";
}

void EnginePWN::FillWordList()
{

  int i;
  // for mmap
  int f;
  char *filedata;
  size_t page_size;
  unsigned int inioffset,mmapbase,endbase;

  fIndex.at(0x18);
  fIndex.readBlock((char*)&wordCount,4);	wordCount=fix32(wordCount);
  fIndex.readBlock((char*)&index_base,4);	index_base=fix32(index_base);
  fIndex.readBlock((char*)&words_base,4);	words_base=fix32(words_base);
  fIndex.readBlock((char*)&end_last_word,4);	end_last_word=fix32(end_last_word);

  // use nonalphabetical index to get 1st offset (->words_base, max length)
  offsets = new unsigned int[wordCount+1];
  words = new char* [wordCount+1];
  words[wordCount] = 0;
  fIndex.at(index_base+wordCount*4);
  fIndex.readBlock((char*)offsets,4*wordCount);
  words_base = words_base + 1 - fix32(offsets[0]);

  // maxlength
  maxlength = 0;
  for (i=0;i<wordCount-2;i++)
	  if (fix32(offsets[i+1])-fix32(offsets[i]) > maxlength)
	  	maxlength = fix32(offsets[i+1])-fix32(offsets[i]);

  // now read alphabetical index
  fIndex.at(index_base);
  fIndex.readBlock((char*)offsets,4*wordCount);
  for (i=0;i<wordCount;i++)
	  offsets[i] = fix32(offsets[i]) & 0x00ffffff;

  wordbuffer = new char[maxlength+1];

  /* read index table */
  f = open(fIndex.name(), O_RDONLY);
  page_size = (size_t)sysconf(_SC_PAGESIZE);
  mmapbase  = (words_base/page_size)*page_size;
  inioffset = words_base - mmapbase;
  endbase   = ((end_last_word/page_size)+1)*page_size;
  filedata  = (char*)mmap(NULL, endbase, PROT_READ, MAP_PRIVATE, f, mmapbase);

  if ((int)filedata != -1) {
    for (i=0;i<wordCount;i++) {
	words[i] = new char [strlen(&filedata[inioffset+fix32(offsets[i])+2+4+11])+1];
	strcpy(words[i],&filedata[inioffset+fix32(offsets[i])+2+4+11]);
    }
  } else {
//	XXX THIS IS BROKEN!!!
//    printf("no mmap: %i\n",filedata);	// XXX
//    for (i=0;i<wordCount-1;i++) {
//	fIndex.at(words_base+fix32(offsets[i]));
//	fIndex.readBlock((char*)wordbuffer,maxlength);
////	dictList->insertItem(ListItemConvert(QCString(&wordbuffer[2+4+11])));
//	words[i] = new char [strlen(wordbuffer)+1];
//	strcpy(words[i],wordbuffer);
//	printf("%i:%s:%s:%s\n",i,words[i],wordbuffer,&filedata[inioffset+fix32(offsets[i])+2+4+11]);
//    }
  }
  munmap((void*)filedata, endbase);
  close(f);
}

int EnginePWN::ReadDefinition(int index)
{
    int i;
    static char *outbuffer = NULL;

    if (outbuffer==NULL)
	outbuffer = new char [5*maxlength];	// XXX is this memleak?
    else
	memset(outbuffer,0,5*maxlength);	// 5*maxlength is arbitrary, based on zip performance

	fIndex.at(words_base+fix32(offsets[index]));
	fIndex.readBlock(wordbuffer, maxlength);

	i = 2+4+11+strlen(&wordbuffer[2+4+11])+2;
	if (wordbuffer[i]<20) {			// not a character -> must be compressed
		int unzipresult;
		uLongf destlen;
		i += wordbuffer[i]+1;
		destlen = 5*maxlength;
		unzipresult = uncompress((Bytef*)outbuffer,&destlen,(const Bytef*)&wordbuffer[i],maxlength);
		curDefinition=cvt->convertChunk(outbuffer);
	} else {
		curDefinition=cvt->convertChunk(&wordbuffer[i]);
	}
    return 0;
}

QString EnginePWN::GetTip(int index) {
    #include "pwn-tooltips.h"
    return tab[index];
}

QString EnginePWN::MatchToolTips(const QString input) {

    QString output = input;
    int i=0, tpos=0, pos=0;

    QRegExp rx("[\\.\\w]+\\s[\\.\\w]+");
    while (pos>=0) {
        pos = rx.search(output,pos);
	if (pos>-1) {
	    for (i=0; i<GetTipNumber(0); i++) {
		tpos = GetTip(i).find(rx.cap(0));
		if (tpos==0) {
		    output.replace(pos,rx.matchedLength(),"<a href=\""+rx.cap(0)+"\">"+rx.cap(0)+"</a>");
		    return output;		// optymistycznie!
		}
	    }
	    pos+=rx.matchedLength();
        }
    }
    return output.replace(QRegExp("(([.\\w]+)[,\\s]*)"),"<a href=\"\\2\">\\1</a>");
}

void EnginePWN::DoToolTips(const QString regex, QString *tmp, const QString color, const int n) {

    QString itOn = ""; QString itOff = "";

    /* prepare defaults */
    if (cnf->italicFont) {
	itOn = "<I>";
	itOff = "</I> ";
    }

    QRegExp rx(regex);
    QString newval;
    int pos = 0;

    while (pos>=0) {
	pos = rx.search(*tmp,pos);
	if (pos>-1) {
	    newval = MatchToolTips(rx.cap(n));
	    if (n==1)
		newval = itOn+"<font color="+color+">"+newval+"</font>"+itOff;
	    if (n==2)
		newval = rx.cap(1)+"<font color="+color+">"+newval+"</font>";
	    tmp->replace(pos,rx.matchedLength(),newval);
	    pos+=rx.matchedLength();
	}
    }
}

QString EnginePWN::rtf2html(QString definition) {

    QString itOn = ""; QString itOff = "";
    QString tmp = definition;

    /* prepare defaults */
    if (cnf->italicFont) {
	itOn = "<I>";
	itOff = "</I> ";
    }

    /* first, convert phonetic and other symbols */
    if (cnf->unicodeFont) {
	tmp.replace("<IMG SRC=\"IPA503.JPG\">",QChar(720));	// lengthtened, :
	tmp.replace("&IPA502;",QChar(716));	// secondary stress (headquaters), ,
	tmp.replace("&idot;",QChar(618));			// high front lax (bit), f4
	tmp.replace("<IMG SRC=\"schwa.JPG\">",QChar(601));	// schwa (about), f1
	tmp.replace("<IMG SRC=\"IPA306.JPG\">",QChar(596));	// mid back lax (boy), f2
	tmp.replace("<IMG SRC=\"IPA313.JPG\">",QChar(594));	// (dog,about), f9
	tmp.replace("<IMG SRC=\"IPA314.JPG\">",QChar(652));	// central low (but), f6
	tmp.replace("<IMG SRC=\"IPA321.JPG\">",QChar(650));	// high back lax (put), f10
	tmp.replace("<IMG SRC=\"IPA305.JPG\">",QChar(593));	// low back (father), f7
	tmp.replace("<IMG SRC=\"IPA326.JPG\">",QChar(604));	// (bird), f3
	tmp.replace("&#952;",QChar(952));	// voiceless interdental fricative (thing), f8
	tmp.replace("&##952;",QChar(952));	// voiceless interdental fricative (thing), f8
	tmp.replace("&##8747;",QChar(643));	// voiceless palatal fricative (ship), f5
	tmp.replace("<SUB><IMG SRC=\"IPA135.JPG\"></SUB>",QChar(658));	// voiced palatal fricative (pleasure), f11
	tmp.replace("&eng;",QChar(331));	// velar nasal (sung), &#331;
// these are handled by Qt anyway
//	tmp.replace("&aelig;",QChar(230));	// low front (cat)
//	tmp.replace("&eth;",QChar(240));	// voiced interdental fricative (then)
    } else {
// old stuff - no Unicode font
	tmp.replace("<IMG SRC=\"IPA503.JPG\">",":");
	tmp.replace("&IPA502;",",");
	tmp.replace("&idot;","<IMG SRC=\"f4\">");
	tmp.replace("schwa.JPG","f1");
	tmp.replace("IPA306.JPG","f2");
	tmp.replace("IPA313.JPG","f9");
	tmp.replace("IPA314.JPG","f6");
	tmp.replace("IPA321.JPG","f10");
	tmp.replace("IPA305.JPG","f7");
	tmp.replace("IPA326.JPG","f3");
	tmp.replace("&#952;","<IMG SRC=\"f8\">");
	tmp.replace("&##952;","<IMG SRC=\"f8\">");
	tmp.replace("&##8747;","<IMG SRC=\"f5\">");
	tmp.replace("IPA135.JPG","f11");
	tmp.replace("&eng;","&#331;");
    }
    tmp.replace("<IMG SRC=\"idioms.JPG\">","IDIOMS:");
    tmp.replace("&ap;~","&asymp;");
    tmp.replace("&ap;","&asymp;");
    tmp.replace("&squareb;","&bull;");
    tmp.replace("&hfpause;","-");
    tmp.replace("&tilde;","~");

    /* now beautification */
    // english examples - bolds
    tmp.replace(QRegExp(";<B>([^<]*)</B>"),";<B><font color="+color3+">\\1</font></B>");
    tmp.replace(QRegExp("/I><B>([^<]*)</B>"),"/I><B><font color="+color3+">\\1</font></B>");
    tmp.replace(QRegExp("<TEXTSECTION ID=0><B>([^<]*)</B>"),"<TEXTSECTION ID=0><B><font color="+color3+">\\1</font></B>");
    // english examples - italics with [ ]
    tmp.replace(QRegExp("<I>( [^<]*)</I>"),itOn+"<font color="+color3+">\\1</font>"+itOff);
    if (cnf->toolTips) {
	// qualifiers - italics, but not those with [ .. ]
	DoToolTips("<I>([^<\[]*)</I>",&tmp,color2,1);
	// qualifier - SMALL not in ( )
	DoToolTips("([^(])<SMALL>([^<]*)</SMALL>",&tmp,color1,2);
    } else {
    	tmp.replace(QRegExp("<I>([^<\[]*)</I>"),itOn+"<font color="+color2+">\\1</font>"+itOff);
	tmp.replace(QRegExp("([^(])<SMALL>([^<]*)</SMALL>"),"\\1<font color="+color1+">\\2</font>");
    }
    // additional context info - (<SMALL...SMALL> - (letter))

    /* finally qml wrap */
    tmp = "<qt type=\"page\"><font color=" + color4 + ">" + tmp + "</font></qt>";

    return tmp;
}

////////////

ConvertPWN::ConvertPWN(void) {
    codec = QTextCodec::codecForName("ISO8859-2");
}

ConvertPWN::~ConvertPWN() {

}

char ConvertPWN::toLower(const char c) {
    const static char upper_cp[] = "A¡BCÆDEÊFGHIJKL£MNÑOÓPQRS¦TUVWXYZ¬¯";
    const static char lower_cp[] = "a±bcædeêfghijkl³mnñoópqrs¶tuvwxyz¼¿";

    unsigned int i;
    for (i=0;i<sizeof(upper_cp);i++)
	if (c == upper_cp[i])
	    return lower_cp[i];
    return c;
}

int ConvertPWN::charIndex(const char c) {
    const static char lower_cp[] = "a±bcædeêfghijkl³mnñoópqrs¶tuvwxyz¼¿";
    unsigned int i;
    for (i=0;i<sizeof(lower_cp);i++)
	if (c == lower_cp[i])
	    return i;
    return sizeof(lower_cp)+1;
}

char *ConvertPWN::toLocal(const char *input) {
    // convert input to encoding suitable for plain text matching
    // (remove spaces, tags, replace accents by letters etc.)
    // tolowercase
    int i, j, len;
    static char buffer[256];		// ugly? maybe
    #define issubstr(a,b) (!(memcmp(a,b,strlen(a))))

    len = strlen(input);
    j = 0;
    memset(buffer,0,sizeof(buffer));

    for (i=0;i!=len;) {
	if (input[i] == '<') {		// opening tag
	    while (input[i++] != '>');	// hit closing brace of open tag
	    while (input[i++] != '>');	// hit closing brace of close tag
	} else
	if (input[i] == '&') {		// entity
	    i++;
	    if (issubstr("rsquo;",&input[i]))
		buffer[j++] = '\'';
	    else
	    if (issubstr("amp;",&input[i]))
		buffer[j++] = '&';
	    else
	    if (issubstr("agrave;",&input[i]))
		buffer[j++] = 'a';
	    else
	    if ((issubstr("eacute;",&input[i])) || (issubstr("egrave;",&input[i])) || (issubstr("ecirc;",&input[i])))
		buffer[j++] = 'e';
	    while (input[i++] != ';');	// hit end of entity
	} else
	if ((input[i] != ' ') && (input[i] != '\t') && (input[i] != '-') && (input[i] !=','))
	    buffer[j++] = toLower(input[i++]);
	else
	    ++i;
    }
    #undef issubstr
    return buffer;
}

QString ConvertPWN::toUnicode(const char *input) {
    QString stmp;
    bool mark=false;
    stmp = codec->toUnicode(input);

    if (stmp.find('&')>=0) {
	mark = true;
	stmp.replace("&amp;","&");
	stmp.replace("&rsquo;","'");
    }
    if (stmp.find('<')>=0) {
	stmp.replace(QRegExp("<SUP>([^<]*)</SUP>")," \\1");
    }
    if (mark) {
	stmp.replace("&agrave;",QChar(224));	// tetchy, tete-a-tete, a`
	stmp.replace("&egrave;",QChar(232));	// finder, fin de siecle, e`
	stmp.replace("&eacute;",QChar(233));	// eclair, e'
	stmp.replace("&ecirc;", QChar(234));	// tetchy, tete-a-tete, e^
	stmp.replace("&reg;",   QChar(174));	// Alar, (R)
    }
    return stmp;
}

QCString ConvertPWN::fromUnicode(QString input) {
    return codec->fromUnicode(input);
}

QString ConvertPWN::convertChunk(const char *inp, int size, bool unicodeFont)
{
    return codec->toUnicode(inp);
}
