/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qmime.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qframe.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qtextbrowser.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qvaluelist.h>
#include <qapplication.h>
#include <qtextcodec.h>
#include <qgrid.h>

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

#include "kydpdict.h"
#include "kydpconfig.h"

Kydpdict::Kydpdict(QWidget *parent, const char *name) : QMainWindow(parent, name)
{
	QFrame *centralFrame = new QFrame(this);
	splitterH = new QSplitter(centralFrame, "splitter");
	splitterV = new QSplitter(Qt::Vertical, splitterH);
	QHBox *hbox1 = new QHBox(splitterV);
	wordInput = new QLineEdit( hbox1, "wordInput");
	listclear = new QPushButton(tr("Clear"), hbox1, "Clear");
	listclear->setMaximumWidth(40);
	hbox1->setMinimumHeight(20);
	dictList = new QListBox( splitterV, "dictList" );
	RTFOutput = new QTextBrowser (splitterH, "RTFOutput");
	listclear->setAccel( QKeySequence( tr("Ctrl+X", "Clear") ) );
	setCentralWidget(centralFrame);

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

	QValueList<int> splittersizes;
	splittersizes.append(config->spH1);
	splittersizes.append(config->spH2);
	splitterH->setSizes(splittersizes);

	QValueList<int> splittersizesV;
	splittersizesV.append(config->spV1);
	splittersizesV.append(config->spV2);
	splitterV->setSizes(splittersizesV);

 	m_checkTimer = new QTimer(this, "timer");
	m_checkTimer->start(1000, FALSE);

	int a;

	do {
		if(!myDict->CheckDictionary(config)) {
			//jesli nie ma tego co chcemy, to sprawdzamy drugi jezyk
			config->language++;
			if (config->language >= LANG_LAST)
				config->language = LANG_ENGLISH;
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
	grid->addWidget(splitterH, 0, 0);

	this->show();

	UpdateLook();

	RTFOutput->mimeSourceFactory()->setFilePath( config->tipsPath );
	RTFOutput->mimeSourceFactory()->setExtensionType("html", "text/html;charset=iso8859-2");
}

Kydpdict::~Kydpdict()
{
    	myDict->CloseDictionary();
 	delete t;
 	t = 0;
 	delete m_checkTimer;
	delete splitterH;
}

void Kydpdict::flushConfig(void)
{
	QValueList<int> list = splitterH->sizes();
    	QValueList<int>::Iterator it = list.begin();
	config->spH1 = *it;
	it++;
	config->spH2 = *it;

	QValueList<int> list2 = splitterV->sizes();
    	QValueList<int>::Iterator it2 = list2.begin();
	config->spV1 = *it2;
	it2++;
	config->spV2 = *it2;

	config->save();
}

void Kydpdict::resizeEvent(QResizeEvent *)
{
 	QSize aRozmiar;

	aRozmiar = this->size();
	config->kGeometryW = aRozmiar.width();
	config->kGeometryH = aRozmiar.height();
}

void Kydpdict::moveEvent(QMoveEvent *)
{
	QPoint aPosition;

	aPosition = this->pos();
	config->kGeometryX = aPosition.x();
	config->kGeometryY = aPosition.y();
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
		if(config->clipTracking)
			PasteClipboard(m_lastString);
	}
}

void Kydpdict::newClipData()
{
	cb->setSelectionMode( true );
	QString clipContents = cb->text().stripWhiteSpace();

	checkClipData( clipContents );
}

void Kydpdict::PasteClipboard(QString haslo)
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
	
	if(config->autoPlay)
		PlaySelected (index);

}

void Kydpdict::NewDefinition (int index)
{
	QString def = myDict->GetDefinition(index);
/* to jest wlasciwie potrzebne tylko dla prawidlowego pokazania pierwszej def. */
	QTextCodec *codec = QTextCodec::codecForName("ISO8859-2");
	def = codec->toUnicode(def.fromAscii(def));
/* z wyj±tkiem tej definicji dzia³a prawid³owo */
	RTFOutput->setText(def);
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
	if ( (index>=0) && config->toPolish )
	    myDict->Play(index,config);
}

void Kydpdict::PlayCurrent ()
{
	PlaySelected(dictList->currentItem());
}

void Kydpdict::SwapEngToPol ()
{
	SwapLang(1, LANG_ENGLISH);
}

void Kydpdict::SwapPolToEng ()
{
	SwapLang(0, LANG_ENGLISH);
}

void Kydpdict::SwapGerToPol ()
{
	SwapLang(1, LANG_DEUTSCH);
}

void Kydpdict::SwapPolToGer ()
{
	SwapLang(0, LANG_DEUTSCH);
}

void Kydpdict::SwapLang (bool direction, int language ) //dir==1 toPolish
{
	int a;
	if(! ((config->toPolish == direction) && (config->language == language)) ) {
		myDict->CloseDictionary();
		config->toPolish=direction;
		config->language = language;
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
		exit(1);
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

	switch (config->language) { //mozna dopisac kolejne case'y dla kolejnych jezykow
		case LANG_DEUTSCH:
			menu->setItemChecked(gerToPol, config->toPolish);
			menu->setItemChecked(polToGer, !config->toPolish);
			menu->setItemChecked(polToEng, FALSE);
			menu->setItemChecked(engToPol, FALSE);
			if(config->toPolish)
				this->setCaption("De -> Pl - Kydpdict");
			else
				this->setCaption("Pl -> De - Kydpdict");
			break;
		case LANG_ENGLISH:
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
	static QString tab[] = {"", "adjective - przymiotnik", "adverb - przys³ówek", "conjunction - spójnik", \
	"perfective verb - czasownik dokonany", "masculine(feminine) - mêski(¿eñski)", "inseparable verb - czasownik nierozdzielny", \
	"invariable - niezmienny", "pronoun - zaimek", "neuter - nijaki", "plural noun - rzeczownik w liczbie mnogiej", \
	"compound - wyraz z³o¿ony", "plural - liczba mnoga", "verb - czasownik", "intransitive verb - czasownik nieprzechodni", \
	"transitive verb - czasownik przechodni", "singular - liczba pojedyncza", \
	"abbreviation - skrót", "nominative - mianownik", "accusative - biernik", "dative - celownik", \
	"genitive - dope³niacz", "infinitive - bezokolicznik", "instrumental - narzêdnik", "locative - miejscownik", \
	"irregular - nieregularny", "preposition - przyimek", "auxiliary - pomocniczy", "past tense - czas przesz³y", \
	"past participle - imies³ów czasu przesz³ego", "masculine - mêski", "noun - rzeczownik", "feminine - ¿eñski", \
	"postpositiv (does not immediately precede a noun) - nie wystêpuje bezpo¶rednio przed rzeczownikiem", "nonvirile - niemêskoosobowy", \
	"virile - mêskoosobowy", "number - liczba", "declined - odmienny", "exclamation - wykrzyknik", \
	"offensiv - obra¼liwy, wulgarny", "informal - potocznie", "pejorative - pejoratywny", "compaund - wyraz z³o¿ony", \
	"obra¼liwy - offensiv", "potoczny - informal", "reflexiv verb - czasownik zwrotny", "attribute - przydawka", \
	"particle - partyku³a", "formal - formalny", "imperfect tense - czas przesz³y o aspekcie niedokonanym", \
	"diminutive - zdrobnienie", "administration - administracja", \
	"anatomy - anatomia", "automobiles - motoryzacja", "aviation - lotnictwo",  "biology - biologia", \
	"botany - botanika", "British English - angielszczyzna brytyjska", "chemistry - chemia", "commerce - handel", \
	"computer - informatyka i komputery", "culinary - kulinarny", "electronics, eletricity - elektronika, elektryczno¶æ", \
	"finance finanse", "law - prawo", "linguistic - jêzykoznawstwo", "mathematics - matematyka", \
	"medicine - medycyna", "military - wojskowo¶æ", "music - muzyka", "nautical - ¿egluga", "politics - polityka", \
	"psychology - psychologia", "railways - kolej", "religion - religia", "school - szko³a", "sport - sportowy", \
	"technology - technika i technologia", "telecominication - telekomunikacja", "theatre - teatr", "printing - poligrafia", \
	"American English - angielszczyzna amerykañska", "zoology - zoologia", "figurative - przeno¶ny", "literal - dos³owny", \
	"geography - geografia", "architecture - architektura", "fizyka - physics", "physiology - fizjologia", \
	"imperative - tryb rozkazuj±cy", "geology - geologia","article - rodzajnik", "indefinite - nieokre¶lony", \
	"definite - okre¶lony", "photography - fotografia", "elektryczno¶æ - electricity", "ekonomia - economy", \
	"economy - ekonomia", "geometria - geometry", "jêzykoznawstwo - linguistic", "kulinarny - culinary", \
	"komputery - computers", "lotnictwo - aviation", "matematyka - mathematics", "motoryzacja - automobiles", \
	"muzyka - music", "szko³a - school", "wojskowo¶æ - military", "¿egluga - nautical", "budownictwo - construction", \
	"meteorology - meteorologia", "history - historia", "poligrafia - printing", "rolnictwo - agriculture", \
	"prefix - przedrostek", "astronomy - astronomia", "physics - fizyka", "et cetera - itd.", "agriculture - rolnictwo", \
	"construction - budownictwo", "ups!"};

	QString tmp = href;

	tmp.remove(0, 10);

	if(tmp.toInt() >= 0 && tmp.toInt() < 121)
	t->tekst = tab[tmp.toInt()];
}
