/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kydpdict.h"

#include <qmime.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qframe.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qtextedit.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qtextbrowser.h>
#include <qsplitter.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qframe.h>
#include <qvaluelist.h>
#include <qapplication.h>

#include "../icons/exit.xpm"
#include "../icons/tux.xpm"
#include "../icons/help.xpm"
#include "../icons/conf.xpm"

#include "../icons/f1.xpm"
#include "../icons/f2.xpm"
#include "../icons/f3.xpm"
#include "../icons/f4.xpm"
#include "../icons/f5.xpm"
#include "../icons/f6.xpm"
#include "../icons/f7.xpm"
#include "../icons/f8.xpm"

#include <qgrid.h>

Kydpdict::Kydpdict(QWidget *parent, const char *name) : QMainWindow(parent, name)
{
	QFrame *centralFrame = new QFrame(this);
	QSplitter *splitter = new QSplitter(centralFrame);
	QVBox *vbox1 = new QVBox(splitter);
	vbox1->setMinimumWidth(QApplication::desktop()->width() / 8);
	QHBox *hbox1 = new QHBox(vbox1);
	wordInput = new QLineEdit( hbox1, "wordInput");
	listclear = new QPushButton(tr("Clear"), hbox1, "Clear");
	vbox1->setSpacing(4);
	wordInput->setMaximumHeight(20);
	listclear->setMaximumWidth(40);
	listclear->setMaximumHeight(20);
	dictList = new QListBox( vbox1, "dictList" );
	RTFOutput = new QTextBrowser (splitter, "RTFOutput");
	listclear->setAccel( QKeySequence( tr("Ctrl+X", "Clear") ) );
	setCentralWidget(centralFrame);

	QValueList<int> splittersizes;
	splittersizes.append(1);
	splittersizes.append(20);
	splitter->setSizes(splittersizes);

	RTFOutput->setTextFormat( RichText );
	RTFOutput->setReadOnly(TRUE);
	RTFOutput->setLineWidth( 0 );
	RTFOutput->setLinkUnderline ( FALSE);

	dictList->setBottomScrollBar(FALSE);
	dictList->setLineWidth( 0 );

	wordInput ->setMaxLength(20);
	wordInput->setLineWidth( 0 );

	cb =  QApplication::clipboard();

	menu = menuBar();

	config = new kydpConfig;
	config->load();

 	myDict = new ydpDictionary(config,dictList);

	setGeometry (config->kGeometryX, config->kGeometryY, config->kGeometryW, config->kGeometryH);

	int a;

	do {
		if(!myDict->CheckDictionary(config)) {
			//jesli nie ma tego co chcemy, to sprawdzamy drugi jezyk
			config->useEnglish = !config->useEnglish;
			config->save();
			if(!myDict->CheckDictionary(config)) {
				// buu, drugiego jezyka tez nie ma; moze user bedzie cos wiedzial na ten temat...
				Configure(TRUE);
			}
		}
		a=myDict->OpenDictionary(config);
	}
	while (a);

	QPopupMenu *file = new QPopupMenu( this );
	Q_CHECK_PTR( file );
	file->insertItem(QPixmap(exit_xpm), tr("&Quit"), qApp, SLOT(quit()), QKeySequence( tr("Ctrl+Q", "File|Quit") ) );

	QPopupMenu *settings = new QPopupMenu( this );
	Q_CHECK_PTR( settings );
	settings->insertItem(QPixmap(conf_xpm), tr("&Settings"), this, SLOT(ConfigureKydpdict()), QKeySequence( tr("Ctrl+S", "Options|Settings") ) );
	settings->insertSeparator();
	if(myDict->CheckDictionary(config->topPath, "dict100.idx", "dict100.dat") && myDict->CheckDictionary(config->topPath, "dict101.idx", "dict101.dat")) {
		polToEng = settings->insertItem("POL --> ENG", this, SLOT(SwapPolToEng()));
		engToPol = settings->insertItem("ENG --> POL", this, SLOT(SwapEngToPol()) );
		settings->insertSeparator();
	}
	if(myDict->CheckDictionary(config->topPath, "dict200.idx", "dict200.dat") && myDict->CheckDictionary(config->topPath, "dict201.idx", "dict201.dat")) {
		polToGer = settings->insertItem("POL --> GER", this, SLOT(SwapPolToGer()));
		gerToPol = settings->insertItem("GER --> POL", this, SLOT(SwapGerToPol()));
		settings->insertSeparator();
	}
	settings->insertItem( tr("&Play sample"), this, SLOT(PlayCurrent()), QKeySequence( tr("Ctrl+P", "Options|Play sample") ) );
	settings->setCheckable(TRUE);

	QPopupMenu *help = new QPopupMenu( this );
	Q_CHECK_PTR( help );
	help->insertItem(QPixmap(tux_xpm), tr("About"), this, SLOT(ShowAbout()));
	help->insertItem(QPixmap(help_xpm), tr("About Qt"), this, SLOT(ShowAboutQt()));

	Q_CHECK_PTR( menu );
	menu->insertItem(  tr("File"), file );
	menu->insertItem(  tr("Options"), settings );
	menu->insertItem(  tr("Help"), help );

 	t = new DynamicTip( this );

 	m_checkTimer = new QTimer(this, "timer");
 	m_checkTimer->start(1000, FALSE);

	connect (dictList, SIGNAL(highlighted(int)), this, SLOT(NewDefinition(int)));
	connect (dictList, SIGNAL(selected(int)), this, SLOT(PlayCurrent()));
	connect (wordInput, SIGNAL(textChanged(const QString&)), this, SLOT(NewFromLine(const QString&)));
	connect (wordInput, SIGNAL(returnPressed()), this, SLOT(PlayCurrent()));
	connect (listclear, SIGNAL(clicked()),wordInput, SLOT(clear()));
 	connect (RTFOutput, SIGNAL( highlighted ( const QString & )), this, SLOT(updateText( const QString & )));
 	connect( cb, SIGNAL( selectionChanged() ), SLOT(slotSelectionChanged()));
 	connect( cb, SIGNAL( dataChanged() ), SLOT( slotClipboardChanged() ));
 	connect(m_checkTimer, SIGNAL(timeout()), this, SLOT(newClipData()));

	QGridLayout *grid = new QGridLayout(centralFrame, 1, 1);
	grid->addWidget(splitter, 0, 0);

	this->show();

	UpdateLook();

 	QMimeSourceFactory::defaultFactory()->setText( "Kydpdict", "Tutaj na razie nic nie ma, ale bêdzie lista wszystkich podpowiedzi" );
}

Kydpdict::~Kydpdict()
{
	myDict->CloseDictionary();
 	delete t;
 	t = 0;
 	delete m_checkTimer;
}

void Kydpdict::resizeEvent(QResizeEvent *)
{
	QSize aRozmiar;

	aRozmiar = this->size();
	config->kGeometryW = aRozmiar.width();
	config->kGeometryH = aRozmiar.height();
	config->save();
}

void Kydpdict::moveEvent(QMoveEvent *)
{
	QPoint aPosition;

	aPosition = this->pos();
	config->kGeometryX = aPosition.x();
	config->kGeometryY = aPosition.y();
	config->save();
}

void Kydpdict::clipboardSignalArrived( bool selectionMode )
{
	cb->setSelectionMode( selectionMode );
	QString text = cb->text();

	checkClipData( text );
}

void Kydpdict::checkClipData( const QString& text )
{
	static QString m_lastString;

	if (text != m_lastString) {
		m_lastString = text;
		PasteClippboard(m_lastString);
	}
}

void Kydpdict::newClipData()
{
	cb->setSelectionMode( true );
	QString clipContents = cb->text().stripWhiteSpace();

	checkClipData( clipContents );
}

void Kydpdict::PasteClippboard(QString haslo)
{
	int index;
	QListBoxItem *result, *tmp_result;

	// do nothing if minimized
	if (this->isMinimized())
		return;

	QString contents = haslo.stripWhiteSpace();

	contents.truncate(20);

	if(!(result = dictList->findItem(contents, Qt::ExactMatch))) {
		int down = 0;
		int up = contents.length() + 1;

		while(up - down > 1) {
			if((tmp_result = dictList->findItem(contents.left((up+down)/2)))) {
				result = tmp_result; //tylko wtedy uaktualniaj gdy jest takie s³owo
				down = (up + down)/2;
			}
			else
				up = (up + down)/2;
		}
	}

	if ((index=dictList->index(result)) >= 0) {
		if (index!=dictList->currentItem())
			dictList->setCurrentItem(index);
		dictList->ensureCurrentVisible();
		this->setActiveWindow ();
		this->raise();
	}

	wordInput->blockSignals( TRUE );
	wordInput->setText(contents);
	wordInput->blockSignals( FALSE );

}

void Kydpdict::NewDefinition (int index)
{
	QString def = myDict->GetDefinition(index);
	RTFOutput->setText(def.fromLocal8Bit(def));
}

void Kydpdict::NewFromLine (const QString &newText)
{
	int index;
	QListBoxItem *result;

	if(!(result = dictList->findItem(newText, Qt::ExactMatch)))
		if(!(result = dictList->findItem(newText)))
			return;

	if ((index=dictList->index(result)) >= 0) {
		if (index!=dictList->currentItem())
			dictList->setCurrentItem(index);
		dictList->ensureCurrentVisible();
	}
}

void Kydpdict::PlaySelected (int index)
{
	if ( (index>=0) && config->useEnglish && config->toPolish )
	    myDict->Play(index,config);
}

void Kydpdict::PlayCurrent ()
{
	PlaySelected(dictList->currentItem());
}

void Kydpdict::SwapEngToPol ()
{
	SwapLang(1, 1);
}

void Kydpdict::SwapPolToEng ()
{
	SwapLang(0, 1);
}

void Kydpdict::SwapGerToPol ()
{
	SwapLang(1, 0);
}

void Kydpdict::SwapPolToGer ()
{
	SwapLang(0, 0);
}

void Kydpdict::SwapLang (bool direction, int lang ) //dir==1 toPolish
{
	int a;
	if(! ((config->toPolish == direction) && (config->useEnglish == lang)) ) {
		myDict->CloseDictionary();
		config->toPolish=direction;
		config->useEnglish = lang;
		config->save();
		do {
			a=myDict->OpenDictionary(config);
			if (a) Configure(TRUE);
		} while (a);

		UpdateLook();
	}
}

void Kydpdict::ShowAbout()
{
	cb->blockSignals( TRUE );
	QMessageBox::about( this, "Kydpdict",
	tr("This is frontend to YDP Collins dictionary.\n"
	"Authors: Andrzej Para , Maciej Witkowiak.\n"
	"Program based on ydpdict by Wojtek Kaniewski"));
	cb->blockSignals( FALSE );
}

void Kydpdict::ShowAboutQt()
{
	cb->blockSignals( TRUE );
	QMessageBox::aboutQt(this, "Kydpdict");
	cb->blockSignals( FALSE);
}

void Kydpdict::ConfigureKydpdict()
{
	Configure(FALSE);
	UpdateLook();
}

void Kydpdict::Configure(bool status)
{
	int result;

	ydpConfigure *myConf = new ydpConfigure(config);

	m_checkTimer->stop();
	cb->blockSignals( TRUE );

	result = myConf->exec();

	if (!(result == QDialog::Accepted) && status ) {
		QMessageBox::critical( this, tr("Error"),
		tr("Kydpdict can't work with incorrect path to dictionary files.\n"
		"Good bye!"));
		if(myConf)
  			delete myConf;
		exit(0);
	}

	if(myConf)
        	delete myConf;

	if (result == QDialog::Accepted)	// just a performance gain
		config->save();
	cb->blockSignals( FALSE);
 m_checkTimer->start(1000, FALSE);
}

void Kydpdict::UpdateLook()
{
	QBrush paper;
	QPixmap pixmap;

    	if(QString::compare(config->kBckgrndPix, "NoBackground") != 0 && pixmap.load(config->kBckgrndPix)) {
		paper.setPixmap(pixmap);
		RTFOutput->setPaper( paper );
		dictList->setPaletteBackgroundPixmap( pixmap );
		dictList->setStaticBackground(TRUE);
	}
	else {
		RTFOutput->setPaper( white );
		dictList->setPaletteBackgroundColor(white);
	}
	dictList->setPaletteForegroundColor ( QColor(config->kFontKolor4) );

	switch (config->useEnglish) { //mozna dopisac kolejne case'y dla kolejnych jezykow
		case 0:
			menu->setItemChecked(gerToPol, config->toPolish);
			menu->setItemChecked(polToGer, !config->toPolish);
			menu->setItemChecked(polToEng, FALSE);
			menu->setItemChecked(engToPol, FALSE);
			if(config->toPolish)
				this->setCaption("De -> Pl - Kydpdict");
			else
				this->setCaption("Pl -> De - Kydpdict");
			break;
		case 1:
			menu->setItemChecked(engToPol, config->toPolish);
			menu->setItemChecked(polToEng, !config->toPolish);
			menu->setItemChecked(gerToPol, FALSE);
			menu->setItemChecked(polToGer, FALSE);
			if(config->toPolish)
				this->setCaption("En -> Pl - Kydpdict");
			else
				this->setCaption("Pl -> En - Kydpdict");
			break;
		default:
			break; //cos jest nie tak
	}

	QString tmp = ".\tc " + config->kFontKolor4;

	char *tst;
	(const char *)tst = tmp;
	f1_xpm[2] = tst;
	f2_xpm[2] = tst;
	f3_xpm[2] = tst;
	f4_xpm[2] = tst;
	f5_xpm[2] = tst;
	f6_xpm[2] = tst;
	f7_xpm[2] = tst;
	f8_xpm[2] = tst;

	QMimeSourceFactory::defaultFactory()->setImage( "f1", QImage(f1_xpm) );
	QMimeSourceFactory::defaultFactory()->setImage( "f2", QImage(f2_xpm) );
	QMimeSourceFactory::defaultFactory()->setImage( "f3", QImage(f3_xpm) );
	QMimeSourceFactory::defaultFactory()->setImage( "f4", QImage(f4_xpm) );
	QMimeSourceFactory::defaultFactory()->setImage( "f5", QImage(f5_xpm) );
	QMimeSourceFactory::defaultFactory()->setImage( "f6", QImage(f6_xpm) );
	QMimeSourceFactory::defaultFactory()->setImage( "f7", QImage(f7_xpm) );
	QMimeSourceFactory::defaultFactory()->setImage( "f8", QImage(f8_xpm) );

	NewDefinition(dictList->currentItem() < 0 ? 0 : dictList->currentItem() );
}

void Kydpdict::updateText( const QString & href )
{
	static QString tab[] = {"", "adj = adjective - przymiotnik", "adv = adverb - przys³ówek", "conj = conjunction - spójnik", \
	"perf = perfective verb - czasownik dokonany", "m(f) = masculine(feminine) - mêski(¿eñski)", "fus = inseparable verb - czasownik nierozdzielny", \
	"inv = invariable - niezmienny", "pron = pronoun - zaimek", "nt = neuter - nijaki", "npl = plural noun - rzeczownik w liczbie mnogiej", \
	"cmp = compound - wyraz z³o¿ony", "pl = plural - liczba mnoga", "vb = verb - czasownik", "vi = intransitive verb - czasownik nieprzechodni", \
	"vt = transitive verb - czasownik przechodni", "sg = singular - liczba pojedyncza", \
	"abbr = abbreviation - skrót", "+nom = nominative - mianownik", "acc = accusative - biernik", "dat = dative - celownik", \
	"gen = genitive - dope³niacz", " infin = infinitiv - bezokolicznik", "instr = instrumental - narzêdnik", "loc = locative - miejscownik", \
	"irreg = irregular - nieregularny", "prep = preposition - przyimek", "aux = auxiliary - pomocniczy", "pt = past tense - czas przesz³y", \
	"pp = past participle - imies³ów czasu przesz³ego", "m = masculine - mêski", "n = noun - rzeczownik", "f = feminine - ¿eñski", \
	"post = postpositiv (does not immediately precede a noun) - nie wystêpuje bezpo¶rednio przed rzeczownikiem", "nvir = nonvirile - niemêskoosobowy", \
	"vir = virile - mêskoosobowy", "num = number - liczba", "decl = declined - odmienny", "excl = exclemation - wykrzyknik", \
	"inf! = offensiv - obra¼liwy, wulgarny", "inf = informal - potocznie", "pej = pejorative - pejoratywny", "perf = perfective verb - czasownik dokonany", \
	"pot! = obra¼liwy - offensiv", "pot = potoczny - informal", "vr = reflexiv verb - czasownik zwrotny", "attr = attribute - przydawka", \
	"part = particle - partyku³a", "fml - formal - formalny", "(im)perf = imperfect tens - czas przesz³y o aspekcie niedokonanym", \
	"dimin = diminutive - zdrobnienie", "ADMIN = administration - administracja", \
	"ANAT = anatomy - anatomia", "AUT = automobiles - motoryzacja", "AVIAT = aviation - lotnictwo",  "BIO = biology - biologia", \
	"BOT = botany - botanika", "BRIT = British English - angielszczyzna brytyjska", "CHEM = chemistry - chemia", "COMM = commerce - handel", \
	"COMPUT = computer - informatyka i komputery", "CULIN = culinary - kulinarny", "ELEC = electronics, eletricity - elektronika, elektryczno¶æ", \
	"FIN = finance finanse", "JUR = law - prawo", "LING = linguistic - jêzykoznawstwo", "MATH = mathematics - matematyka", \
	"MED = medicine - medycyna", "MIL = military - wojskowo¶æ", "MUS = music - muzyka", "NAUT = nautical - ¿egluga", "POL = politics - polityka", \
	"PSYCH = psychology - psychologia", "RAIL = railways - kolej", "REL = religion - religia", "SCOL = school - szko³a", "SPORT = sport - sportowy", \
	"TECH = technology - technika i technologia", "TEL = telecominication - telekomunikacja", "THEAT = theatre - teatr", "TYP = printing - poligrafia", \
	"US = American English - angielszczyzna amerykañska", "ZOOL = zoology - zoologia", "fig = figurative - przeno¶ny", "lit = literal - dos³owny", \
	"GEOG = geography - geografia", "ARCHIT = architecture - architektura", "FIZ = fizyka - physics", "PHYSIOL = physiology - fizjologia", \
	"imp = imperative - tryb rozkazuj±cy", "GEOL = geology - geologia","art = article - rodzajnik" , "indef = indefinite - nieokre¶lony", \
	"def = definite - okre¶lony", "PHOT = photography - fotografia", "ELEKTR = elektryczno¶æ - electricity", "EKON = ekonomia - economy", \
	"FIZ = fizyka - physics", "GEOM = geometria - geometry", "JÊZ = jêzykoznawstwo - linguistic", "KULIN = kulinarny - culinary", \
	"KOMPUT = komputery - computers", "LOT = lotnictwo - aviation", "MAT = matematyka - mathematics", "MOT = motoryzacja - automobiles", \
	"MUZ = muzyka - music", "SZKOL = szko³a - school", "WOJSK = wojskowo¶æ - military", "¯EGL = ¿egluga - nautical", "BUD = budownictwo - construction", \
	"METEO = meteorology - meteorologia", "HIST = history - historia", "DRUK = poligrafia - printing", "ROL - rolnictwo - agriculture", \
	"pref = prefix - przedrostek", "ASTRON = astronomy - astronomia", "ups!"};

	//du¿o tego, mo¿e to jako¶ w pliku umie¶ciæ?

	QString tmp = href;

	tmp.remove(0,9);

	if(tmp.toInt() >= 0 && tmp.toInt() < 117)
	t->tekst = tab[tmp.toInt()];
}
