/***************************************************************************
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kydpdict.h"
#include "ydpdictionary.h"
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qprocess.h>

#include <qregexp.h>

#define color1 cnf->kFontKolor1
#define color2 cnf->kFontKolor2
#define color3 cnf->kFontKolor3
#define color4 cnf->kFontKolor4

#define TABLE_CP_ISO { \
  128, 129, 130, 131, 132, 133, 134, 135, \
  136, 137, 138, 139, 166, 141, 142, 172, \
  144, 145, 146, 147, 148, 149, 150, 151, \
  152, 153, 154, 155, 182, 157, 158, 188, \
  160, 161, 162, 163, 164, 161, 166, 167, \
  168, 169, 170, 171, 172, 173, 174, 175, \
  176, 177, 178, 179, 180, 181, 182, 183, \
  184, 177, 186, 187, 188, 189, 190, 191, \
  192, 193, 194, 195, 196, 197, 198, 199, \
  200, 201, 202, 203, 204, 205, 206, 207, \
  208, 209, 210, 211, 212, 213, 214, 215, \
  216, 217, 218, 219, 220, 221, 222, 223, \
  224, 225, 226, 227, 228, 229, 230, 231, \
  232, 233, 234, 235, 236, 237, 238, 239, \
  240, 241, 242, 243, 244, 245, 246, 247, \
  248, 249, 250, 251, 252, 253, 254, 255 }

#define TABLE_PHONETIC_ISO { \
  ".", ".", "<img src=\"f2\">", "<img src=\"f3\">", ".", "<img src=\"f5\">", "e", "<img src=\"f6\">", \
  "<img src=\"f1\">", "<img src=\"f8\">", "<img src=\"f4\">", "<img src=\"f7\">", ".", ":", "´", ".", \
  "&#331;", ".", ".", ".", ".", ".", ".", "&#240;", \
  "&#230;", ".", ".", ".", "¶", ".", ".", "¼", \
  " ", "¡", "¢", "£", "¤", "¥", "¦", "§", \
  "¨", "©", "ª", "«", "¬", "­", "®", "¯", \
  "°", "±", "²", "³", "´", "µ", "¶", "·", \
  "¸", "¹", "º", "»", "¼", "½", "¾", "¿", \
  "À", "Á", "Â", "Ã", "Ä", "Å", "Æ", "Ç", \
  "È", "É", "Ê", "Ë", "Ì", "Í", "Î", "Ï", \
  "Ð", "Ñ", "Ò", "Ó", "Ô", "Õ", "Ö", "×", \
  "Ø", "Ù", "Ú", "Û", "Ü", "Ý", "Þ", "ß", \
  "à", "á", "â", "ã", "ä", "å", "æ", "ç", \
  "è", "é", "ê", "ë", "ì", "í", "î", "ï", \
  "ð", "ñ", "ò", "ó", "ô", "õ", "ö", "÷", \
  "ø", "ù", "ú", "û", "ü", "ý", "þ", "ÿ" } \


ydpDictionary::ydpDictionary(kydpConfig *config, QListBox *listBox) {
	dictList = listBox;
	cnf = config;
}

ydpDictionary::~ydpDictionary(){
	// free all objects here...
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
    QString p;

  /* open index and definition files */
    p = path + "/" + index;
    fIndex.setName(p);
    if (!(fIndex.open(IO_ReadOnly))) {
	  	p = tr( "Can't open index file!\n" ) + p;
		QMessageBox::critical(0, "kydpdict", p );
		return 1; };

    p = path + "/" + def;
    fData.setName(p);

    if (!(fData.open(IO_ReadOnly))) {
	  	p = tr( "Can't open data file!\n" ) + p;
		QMessageBox::critical(0, "kydpdict", p );
		return 1; };

    FillWordList();

    return 0;
}

int ydpDictionary::CheckDictionary(kydpConfig *config)
{
	return CheckDictionary(config->topPath,config->indexFName,config->dataFName);
}

int ydpDictionary::CheckDictionary(QString path, QString index, QString def)
{
    QFile fi, fd;

    fi.setName( path + "/" + index);
    fd.setName(path + "/" + def);

    if ( !(fi.open(IO_ReadOnly)) )
    	return 0;
    fi.close();
    if( !(fd.open(IO_ReadOnly)) )
    	return 0;
    fd.close();
    return 1;
}

void ydpDictionary::CloseDictionary()
{
	dictList->clear();
	fIndex.close();
	fData.close();
	if (indexes) delete (indexes);
}

QString ydpDictionary::convert_cp1250(char *tekst, long int size)
{
    const char *table_cp_phonetic[]=TABLE_PHONETIC_ISO;
    unsigned const char table_cp_iso[]=TABLE_CP_ISO;
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
			if (ch > 127) ch=table_cp_iso[ch-128];
			out +=ch;
			j++;
		    } while (table_cp_phonetic[*input-128][j]!='\0');
		} else {
		    a = *input;
		    out += a; };
		input++;
    };

    return out;
}


void ydpDictionary::FillWordList()
{
  QString p;
  QProgressDialog *g;

  unsigned long pos;
  int current=0, bp;
  char buf[256], ch;

  g = new QProgressDialog;

    /* read # of words */
    wordCount=0;
    fIndex.at(0x08);
    fIndex.readBlock((char*)&wordCount,2);

    indexes = new unsigned long [wordCount];

  g->setTotalSteps(wordCount / 200);
  g->setMinimumDuration(500);
  g->show();

    /* read index table offset */
    pos=0;
    fIndex.at(0x10);
    fIndex.readBlock((char*)&pos, sizeof(unsigned long));
    fIndex.at(pos);

    dictList->setAutoUpdate(FALSE);
  /* read index table */
  do {
	if ((current % 200)==0) { g->setProgress(current / 200); };
	fIndex.at(fIndex.at()+4);
	fIndex.readBlock((char*)&indexes[current], sizeof(unsigned long));

    bp=0;
    do {
	ch = fIndex.getch();
	buf[bp < 255 ? bp++ : bp] = ch;
    } while(ch);
    buf[bp]=0;

    p = convert_cp1250(buf,bp-1);
    p = p.fromLocal8Bit(p.latin1());
    dictList->insertItem(p);
  } while (++current < wordCount);

  delete g;

  QListBoxItem *result;
  if((result = dictList->findItem("Proven<")))
  	dictList->changeItem(QString("Provencial"), dictList->index(result));
  dictList->setCurrentItem(0);

  dictList->setAutoUpdate(TRUE);
  dictList->repaint();
}

int ydpDictionary::ReadDefinition(int index)
{
  unsigned long dsize, size;
  char *def;

    dsize=0;
    fData.at(indexes[index]);
    fData.readBlock((char*)&dsize, sizeof(unsigned long));

    def = new char[dsize+1];
    if ((size = fData.readBlock(def,dsize)) !=dsize) return -1;
    def[size] = 0;
    curDefinition=QString(def);

    delete def;
    return 0;
}

int ydpDictionary::Play (int index, kydpConfig *config) {
	QFile fd;
	QString name, ext;
	static QProcess proc;

	fd.setName(config->cdPath.latin1());
	if (!(fd.exists()))
		return 0;	// nie ma co tracic czasu jesli katalogu nie ma

	name.sprintf("%s/s%.3d/%.6d.", config->cdPath.latin1(), index/1000, index+1);

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
	
	if (proc.isRunning()) {
		proc.kill();		// nie ma litosci dla skurwysynow! BUM! BUM!
	}

	proc.clearArguments();
	proc.addArgument( config->player );
	proc.addArgument( name+ext );
	proc.start();

	return 1;
}

QString ydpDictionary::rtf2html (QString definition) {

char token[128];
int tp, level = 0;
bool sa_tag = FALSE,br_tag = FALSE;
QString tag_on,tag_off;
char *def;
(const char*)def=definition;
list.clear();
list += "<qt title Kypdict><font face=Arial, Helvetica, sans-serif color=" + color4 +">";
list += "</font></qt>";
for(level=15;level>=0;level--)
	tag_num[level] = 0;
level = 0;

it = list.begin();
it++;

  while (*def) {
  	switch(*def) {
  		case '{':
				if (level < 16) tag_num[++level] = 0;
  			break;
  		case '\\':
			def++; tp = 0;
			while((*def >= 'a' && *def <= 'z') || (*def >='0' && *def <= '9'))
				token[tp++] = *def++;
			token[tp] = 0;
			if (*def == ' ') def++;

			if (!strcmp(token, "i")) { //	109891 -ilosc wystapien -  optymalizacja pod plik ang->pol :)))))))
				tag_on = "<i>";
				tag_off = "</i>";
				tag_num[level]++;
			} else

			if (!strcmp(token, "cf2")) { //	79850
				tag_on = "<font color=" + color2 + ">";
				tag_off = "</font>";
				tag_num[level]++;
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
			} else

			if (!strcmp(token, "cf5")) { // 21478
				tag_on = "<font color=" + color4 + ">";
				tag_off = "</font>";
				tag_num[level]++;
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
			} else

			if (!strcmp(token, "i0")) { // 69
				tag_on = "";
				tag_off = "";
				disableTag (2, level);
			} else

			if (!strcmp(token, "cf4")) {
				tag_on = "<font color=" + color3 + ">";
				tag_off = "</font>";
				tag_num[level]++;
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
			it = list.insert(it, convert_cp1250(token, (long int) tp)); it++;
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
