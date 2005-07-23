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

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "kydpconfig.h"
#include "engine_ydp.h"
#include "engine_ydp.moc"

#define T_PL 1
#define T_CM 2
#define T_PN_L 4
#define T_PN_R 8
#define T_SL_L 16
#define T_SL_R 32

#define _native 0
#define _foreign 1

QString EngineYDP::GetTip(int index) {
    #include "ydp-tooltips.h"
    return tab[index];
}

EngineYDP::EngineYDP(kydpConfig *config, QListBox *listBox, ydpConverter *converter) : ydpDictionary(config, listBox, converter)
{
    for (int i=0;i<4;i++) {
        dictCache_LL[i].indexes = NULL;
    }
}

EngineYDP::~EngineYDP()
{
    int i;

    for (i=0;i<4;i++) {
	if (dictCache[i].wordCount>0) {
	    if (dictCache_LL[i].indexes) delete [] dictCache_LL[i].indexes;
	}
    }
}

int EngineYDP::OpenDictionary(void)
{
    int i;

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

    /* 0-english, eng->pol; 1-english, pol->eng; 2-german, ger->pol; 3-german, pol->ger */
    i = 0;
    if (cnf->language == LANG_DEUTSCH)
	i+=2;
    if (!(cnf->toPolish))
	i++;
    if (dictCache[i].wordCount>0) {
	indexes = dictCache_LL[i].indexes;
    } else {
	FillWordList();
	dictCache_LL[i].indexes = indexes;
    }
    return ydpDictionary::OpenDictionary();	// required call
}

int EngineYDP::CheckDictionary(void)
{
    QFile f;
    unsigned int test = 0;

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
    if (fix32(test) != fix32(0x8d4e11d5))	// magic test
	return 0;
    f.setName( cnf->topPath + "/" + cnf->dataFName );
    if ( !(f.exists()) ) {
	f.setName( cnf->topPath + "/" + cnf->dataFName.upper() );
	if ( !(f.exists()) )
	    return 0;
    }
    return 1;
}

void EngineYDP::CloseDictionary()
{
    fIndex.close();
    fData.close();
    // free some stuff allocated elsewhere???
    ydpDictionary::CloseDictionary();	// required call
}

void EngineYDP::UpdateFName(void) {
    switch (cnf->language) {
	case LANG_ENGLISH:
	    cnf->indexFName= cnf->toPolish ? "dict100.idx" : "dict101.idx";
	    cnf->dataFName = cnf->toPolish ? "dict100.dat" : "dict101.dat";
	    break;
	case LANG_DEUTSCH:
	    cnf->indexFName= cnf->toPolish ? "dict200.idx" : "dict201.idx";
	    cnf->dataFName = cnf->toPolish ? "dict200.dat" : "dict201.dat";
	    break;
	}
}

void EngineYDP::FillWordList()
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

int EngineYDP::ReadDefinition(int index)
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

QString EngineYDP::SampleName(QString path, int index)
{
    QFile fd;
    QString name, myname;

    name.sprintf("/s%.3d/%.6d.", index/1000, index+1);

    myname = name+"wav";
    fd.setName(path+myname);
    if (fd.exists())
	return fd.name();
    fd.setName(path+myname.upper());
    if (fd.exists())
	return fd.name();

    myname = name+"mp3";
    fd.setName(path+myname);
    if (fd.exists())
	return fd.name();
    fd.setName(path+myname.upper());
    if (fd.exists())
	return fd.name();

    myname = name+"ogg";
    fd.setName(path+myname);
    if (fd.exists())
	return fd.name();
    fd.setName(path+myname.upper());
    if (fd.exists())
	return fd.name();

    return QString("");
}

QString EngineYDP::rtf2html (QString definition)
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
				br_tag = TRUE; //nie ≥am linii za wcieciem bo qt samo to robi, czemu?
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
			QString tmp = cvt->convertChunk(token, tp, cnf->unicodeFont);

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

void EngineYDP::disableTag (int tag_code, int level)
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

QString EngineYDP::insertHyperText(QString raw_input, int level)
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
			tmp = tmp.left(tmp.length()-1);
		}

		if(tmp.startsWith("+")) {
			tags |= T_PL;
			tmp = tmp.right(tmp.length()-1);
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

			for (int i=0; i<GetTipNumber(0); i++) {
				if (!QString::compare(tmp2, GetInputTip(i))) {
					proposition = "<a href=\""+GetInputTip(i)+"\">"+tmp2+"</a>";
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

/* converter class */

ConvertYDP::ConvertYDP(void) {
    codec = QTextCodec::codecForName("CP1250");
}

ConvertYDP::~ConvertYDP() {

}

char ConvertYDP::toLower(const char c) {
    const static char upper_cp[] = "A•BC∆DE FGHIJKL£MN—O”PQRSåTUVWXYZèØ";
    const static char lower_cp[] = "aπbcÊdeÍfghijkl≥mnÒoÛpqrsútuvwxyzüø";

    unsigned int i;
    for (i=0;i<sizeof(upper_cp);i++)
	if (c == upper_cp[i])
	    return lower_cp[i];
    return c;
}

int ConvertYDP::charIndex(const char c) {
    const static char lower_cp[] = "aπbcÊdeÍfghijkl≥mnÒoÛpqrsútuvwxyzüø";
    unsigned int i;
    for (i=0;i<sizeof(lower_cp);i++)
	if (c == lower_cp[i])
	    return i;
    return sizeof(lower_cp)+1;
}

char *ConvertYDP::toLocal(const char *input) {
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

QString ConvertYDP::toUnicode(const char *input) {
    return codec->toUnicode(input);
}

QCString ConvertYDP::fromUnicode(QString input) {
    return codec->fromUnicode(input);
}

#define TABLE_PHONETIC_ISO { \
  ".", ".", "<img src=\"f2\">", "<img src=\"f3\">", ".", "<img src=\"f5\">", "e", "<img src=\"f6\">", \
  "<img src=\"f1\">", "<img src=\"f8\">", "<img src=\"f4\">", "<img src=\"f7\">", "¶", ":", "¥", "¨", \
  "&#331;", ".", ".", ".", ".", ".", ".", "&#240;", \
  "&#230;", ".", ".", ".", "∂", ".", ".", "º", \
  "†", "°", "¢", "£", "§", "•", "¶", "ß", \
  "®", "©", "™", "´", "¨", "≠", "Æ", "Ø", \
  "∞", "±", "≤", "≥", "¥", "µ", "∂", "∑", \
  "∏", "±", "∫", "ª", "º", "Ω", "æ", "ø", \
  "¿", "¡", "¬", "√", "ƒ", "≈", "∆", "«", \
  "»", "…", " ", "À", "Ã", "Õ", "Œ", "œ", \
  "–", "—", "“", "”", "‘", "’", "÷", "◊", \
  "ÿ", "Ÿ", "⁄", "€", "‹", "›", "ﬁ", "ﬂ", \
  "‡", "·", "‚", "„", "‰", "Â", "Ê", "Á", \
  "Ë", "È", "Í", "Î", "Ï", "Ì", "Ó", "Ô", \
  "", "Ò", "Ú", "Û", "Ù", "ı", "ˆ", "˜", \
  "¯", "˘", "˙", "˚", "¸", "˝", "˛", "ˇ" } \

QString ConvertYDP::convertChunk(const char *inp, int size, bool unicodeFont)
{
    QString out;
    unsigned char *input = (unsigned char *)inp;
    unsigned char ch;
    char a;
    int i;

    if (unicodeFont) {
	// prawdopodobnie jesli to jest wlaczone, to czesc zabaw z lokalami jest zbedna
	const static int table_cp_unicode[] = {
	    0x002e, 0x002e, 0x0254, 0x0292, 0x002e, 0x0283, 0x0065, 0x028c,	// 80-87
	    0x0259, 0x03b8, 0x026a, 0x0251, 0x015a, 0x02d0, 0x00b4, 0x0179,	// 88-8f
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
