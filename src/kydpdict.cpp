/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <qmime.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qframe.h>
#include <qmessagebox.h>
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
#include <qaccel.h>

#define COMBO_HISTORY_SIZE	25
#define TIMER_PERIOD		1000

/* 16x16 */
#include "../icons/conf.xpm"
#include "../icons/exit.xpm"
#include "../icons/babelfish_small.xpm"
#include "../icons/help.xpm"
/* glyphs */
#include "../icons/f1.xpm"
#include "../icons/f2.xpm"
#include "../icons/f3.xpm"
#include "../icons/f4.xpm"
#include "../icons/f5.xpm"
#include "../icons/f6.xpm"
#include "../icons/f7.xpm"
#include "../icons/f8.xpm"
/* 22x22 */
#include "../icons/en_pl.xpm"
#include "../icons/pl_en.xpm"
#include "../icons/de_pl.xpm"
#include "../icons/pl_de.xpm"
#include "../icons/configure.xpm"
#include "../icons/loop.xpm"
#include "../icons/player_play.xpm"
#include "../icons/clear_left.xpm"
#include "../icons/clipboard.xpm"
#include "../icons/babelfish.xpm"

#include "kydpdict.h"
#include "kydpconfig.h"
#include "dock_widget.h"

Kydpdict::Kydpdict(QWidget *parent, const char *name) : QMainWindow(parent, name)
{

	toolBar = new QToolBar(this, "toolbar");
	toolBar->setLabel(tr("Kydpdict toolbar"));
	toolBar->setCloseMode(QToolBar::Undocked);

	QFrame *centralFrame = new QFrame(this);
	splitterH = new QSplitter(centralFrame, "splitter");
	splitterV = new QSplitter(Qt::Vertical, splitterH);
	QHBox *hbox1 = new QHBox(splitterV);
	QHBox *hbox2 = new QHBox(splitterV);
	wordInput = new QComboBox( hbox1, "wordInput");
	listclear = new QPushButton(QIconSet(QPixmap(clear_left_xpm)), QString::null, hbox1, "clear");
	listclear->setMaximumWidth(40);
	listclear->setFlat(TRUE);
	hbox1->setMinimumHeight(20);
	dictList = new QListBox( hbox2, "dictList" );
	scrollBar = new QScrollBar( Qt::Vertical, hbox2, "scrollBar");
	scrollBar->setMaximumWidth(20);
	RTFOutput = new QTextBrowser (splitterH, "RTFOutput");
	setCentralWidget(centralFrame);

	RTFOutput->setTextFormat( RichText );
	RTFOutput->setReadOnly(TRUE);
	RTFOutput->setLineWidth( 0 );
	RTFOutput->setLinkUnderline (FALSE);

	dictList->setBottomScrollBar(FALSE);
	dictList->setAutoScrollBar(FALSE);
	dictList->setLineWidth( 0 );

	wordInput->setMaxCount(20);
	wordInput->setDuplicatesEnabled(false);
	wordInput->setEditable(true);
	wordInput->setInsertionPolicy(QComboBox::AtTop);

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
	m_checkTimer->start(TIMER_PERIOD, FALSE);
	slastSelection = "";
	slastClipboard = "";

	int a;

	do {
		testagain:
		if(!myDict->CheckDictionary(config)) {
		//jesli nie ma tego co chcemy, to sprawdzamy drugi jezyk
			config->language++;
			if (config->language >= LANG_LAST)
				config->language = LANG_ENGLISH;
			config->save();
			if(!myDict->CheckDictionary(config)) {
			// buu, drugiego jezyka tez nie ma; moze user bedzie cos wiedzial na ten temat...
			    QMessageBox::critical( this, tr("Error"),
				tr( "Kydpdict can't work with incorrect path to dictionary files.\n"
				    "In order to use this program you have to have data files from Windows\n"
				    "dictionary installation. For more information please read README.\n"
				    "A configuration window will be opened now, so you can set the path."));
			    Configure(TRUE);
			    goto testagain;	// z³o¿y³em ofiarê z jagniêcia i wolno mi u¿yæ goto
			}
		}
		a=myDict->OpenDictionary(config);
	}
	while (a);
	wordInput->clear();

	QPopupMenu *file = new QPopupMenu( this );
	Q_CHECK_PTR( file );

	file->insertItem(QPixmap(exit_xpm), tr("&Quit"), qApp, SLOT(quit()), QKeySequence( tr("Ctrl+Q", "File|Quit") ) );

	QPopupMenu *settings = new QPopupMenu( this );
	Q_CHECK_PTR( settings );
	settings->insertItem(QPixmap(conf_xpm), tr("&Settings"), this, SLOT(ConfigureKydpdict()), QKeySequence( tr("Ctrl+S", "Options|Settings") ) );
	toolBarMenuItem = settings->insertItem(tr("Show &toolbar"), this, SLOT(ShowToolbar()), QKeySequence( tr("Ctrl+T", "Options|Settings") ) );
	settings->insertSeparator();
	but_PlEn = NULL; but_PlDe = NULL;
	but_EnPl = NULL; but_DePl = NULL;
	if(myDict->CheckDictionary(config->topPath, "dict100.idx", "dict100.dat") && myDict->CheckDictionary(config->topPath, "dict101.idx", "dict101.dat")) {
		polToEng = settings->insertItem("POL --> ENG", this, SLOT(SwapPolToEng()), QKeySequence( tr("Ctrl+;", "Options|POL --> ENG") ) );
		engToPol = settings->insertItem("ENG --> POL", this, SLOT(SwapEngToPol()), QKeySequence( tr("Ctrl+.", "Options|ENG --> POL") ) );
		but_PlEn = new QToolButton(QIconSet(QPixmap(pl_en_xpm)), "POL --> ENG", QString::null, this, SLOT(SwapPolToEng()), toolBar, "butPlEn" );
		but_EnPl = new QToolButton(QIconSet(QPixmap(en_pl_xpm)), "ENG --> POL", QString::null, this, SLOT(SwapEngToPol()), toolBar, "butEnPl" );
	}
	if(myDict->CheckDictionary(config->topPath, "dict200.idx", "dict200.dat") && myDict->CheckDictionary(config->topPath, "dict201.idx", "dict201.dat")) {
		settings->insertSeparator();
		polToGer = settings->insertItem("POL --> GER", this, SLOT(SwapPolToGer()), QKeySequence( tr("Ctrl+:", "Options|POL --> GER") ) );
		gerToPol = settings->insertItem("GER --> POL", this, SLOT(SwapGerToPol()), QKeySequence( tr("Ctrl+>", "Options|GER --> POL") ) );
		but_PlDe = new QToolButton(QIconSet(QPixmap(pl_de_xpm)), "POL --> GER", QString::null, this, SLOT(SwapPolToGer()), toolBar, "butPlDe" );
		but_DePl = new QToolButton(QIconSet(QPixmap(de_pl_xpm)), "GER --> POL", QString::null, this, SLOT(SwapGerToPol()), toolBar, "butDePl" );
	}
	settings->insertSeparator();
	settings->insertItem( tr("Swap direction"), this, SLOT(SwapLanguages()), QKeySequence( tr("Ctrl+`", "Options|Swap direction") ) );
	settings->insertSeparator();
	settings->insertItem( tr("&Play sample"), this, SLOT(PlayCurrent()), QKeySequence( tr("Ctrl+P", "Options|Play sample") ) );
	settings->setCheckable(TRUE);

	QPopupMenu *help = new QPopupMenu( this );
	Q_CHECK_PTR( help );
	help->insertItem(QPixmap(babelfish_small_xpm), tr("About"), this, SLOT(ShowAbout()));
	help->insertItem(QPixmap(help_xpm), tr("About Qt"), this, SLOT(ShowAboutQt()));

	trayMenu = new QPopupMenu( this );
	Q_CHECK_PTR( trayMenu );
	trayMenu->insertItem(QPixmap(conf_xpm), tr("&Settings"), this, SLOT(ConfigureKydpdict()) );
	trayMenu->insertSeparator();
	trayMenu->insertItem( tr("Swap direction"), this, SLOT(SwapLanguages()) );
	trayMenuItemClipTrack = trayMenu->insertItem( tr("Clipboard tracking"), this, SLOT(ToggleClipTracking()) );
	trayMenu->insertSeparator();
	trayMenu->insertItem(QPixmap(babelfish_small_xpm), tr("About"), this, SLOT(ShowAbout()));
	trayMenu->insertItem(QPixmap(exit_xpm), tr("&Quit"), qApp, SLOT(quit()) );

	Q_CHECK_PTR( menu );
	menu->insertItem(  tr("&File"), file );
	menu->insertItem(  tr("&Options"), settings );
	menu->insertItem(  tr("&Help"), help );

 	t = new DynamicTip( RTFOutput );

	toolBar->addSeparator();
	but_SwapLang = new QToolButton(QIconSet(QPixmap(loop_xpm)), tr("Swap direction"), QString::null, this, SLOT(SwapLanguages()), toolBar, "but_swaplang" );
	toolBar->addSeparator();
	but_Play = new QToolButton(QIconSet(QPixmap(player_play_xpm)), tr("Play"), QString::null, this, SLOT(PlayCurrent()), toolBar, "but_play" );
	toolBar->addSeparator();
	but_Clipboard = new QToolButton(QIconSet(QPixmap(clipboard_xpm)), tr("Toggle clipboard tracking"), QString::null, this, SLOT(ToggleClipTracking()), toolBar, "but_clipboard");
	but_Clipboard->setToggleButton(TRUE);
	but_Clipboard->setOn(config->clipTracking);
	toolBar->addSeparator();
	but_Settings = new QToolButton(QIconSet(QPixmap(configure_xpm)), tr("Settings"), QString::null, this, SLOT(ConfigureKydpdict()), toolBar, "but_settings" );

	connect (dictList, SIGNAL(highlighted(int)), this, SLOT(NewDefinition(int)));
	connect (dictList, SIGNAL(selected(int)), this, SLOT(PlayCurrent()));
	connect (wordInput, SIGNAL(textChanged(const QString&)), this, SLOT(NewFromLine(const QString&)));
	connect (wordInput, SIGNAL(activated(int)), this, SLOT(PlayCurrent()));
	connect (listclear, SIGNAL(clicked()), wordInput, SLOT(clearEdit()));
	connect (listclear, SIGNAL(clicked()), wordInput, SLOT(setFocus()));
 	connect (RTFOutput, SIGNAL(highlighted( const QString & )), this, SLOT(updateText( const QString & )));
 	connect (m_checkTimer, SIGNAL(timeout()), this, SLOT(newClipData()));
	connect (toolBar, SIGNAL(visibilityChanged(bool)), this, SLOT(ToolbarShowHide(bool)));
 	connect (cb, SIGNAL(selectionChanged() ), SLOT(newClipData()));
 	connect (cb, SIGNAL(dataChanged() ), SLOT(newClipData()));

	connect (scrollBar, SIGNAL(nextLine()), myDict, SLOT(ListScroll1Up()));
	connect (scrollBar, SIGNAL(nextPage()), myDict, SLOT(ListScrollPageUp()));
	connect (scrollBar, SIGNAL(prevLine()), myDict, SLOT(ListScroll1Down()));
	connect (scrollBar, SIGNAL(prevPage()), myDict, SLOT(ListScrollPageDown()));
	connect (scrollBar, SIGNAL(sliderMoved(int)), myDict, SLOT(ListRefresh(int)));

	QGridLayout *grid = new QGridLayout(centralFrame, 1, 1);
	grid->addWidget(splitterH, 0, 0);

	QAccel *accel = new QAccel(this);
	accel->connectItem(accel->insertItem(Key_Escape), this, SLOT(onEscaped()));

	UpdateLook();

	if (!config->toolBarVisible)
	    toolBar->hide();

	this->show();
	myDict->ListRefresh(0);
	dictList->setCurrentItem(0);

	RTFOutput->mimeSourceFactory()->setFilePath( config->tipsPath );
	RTFOutput->mimeSourceFactory()->setExtensionType("html", "text/html;charset=iso8859-2");

	if (config->dock) {
	    trayicon = new TrayIcon(this,"trayicon");
	    trayicon->setDictWidget(this);
	    trayicon->setPopupMenu(trayMenu);
	    trayicon->show();
	}
	setFocusProxy(wordInput);
	RTFOutput->setFocusProxy(wordInput);
}

Kydpdict::~Kydpdict()
{
    	myDict->CloseDictionary();
 	delete t;
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

	myDict->ListRefresh(dictList->currentItem());
	scrollBar->setPageStep(dictList->numItemsVisible());
}

void Kydpdict::moveEvent(QMoveEvent *)
{
	QPoint aPosition;

	aPosition = this->pos();
	config->kGeometryX = aPosition.x();
	config->kGeometryY = aPosition.y();
}

void Kydpdict::onEscaped()
{
    if (!wordInput->hasFocus())
	wordInput->setFocus();
    else
	if (wordInput->currentText().isEmpty())
	    this->hide();
	else
	    wordInput->clearEdit();
}

void Kydpdict::windowActivationChange(bool oldActive)
{
    if (isActiveWindow())
	wordInput->setFocus();
}

void Kydpdict::newClipData()
{
    int result;
    static int lastresult=-1;

    // do nothing if minimized
    if (this->isMinimized())
        return;
    // do nothing if it comes from us
    if (((RTFOutput->hasSelectedText())||(wordInput->lineEdit()->hasSelectedText())) && (config->ignoreOwnSelection))
        return;

    // get selection - if selection and clipboard are changed simultaneously selection
    // is chosen an clipboard is ignored
    QString lnewEntry = cb->text(QClipboard::Selection).stripWhiteSpace(); 
    if (lnewEntry.length() && lnewEntry != slastSelection)
	slastSelection = lnewEntry;
    else
	lnewEntry = "";

    if (lnewEntry.isEmpty()) {
	// get clipboard if selection empty or unchanged
	lnewEntry = cb->text(QClipboard::Clipboard).stripWhiteSpace();    
	if(lnewEntry.length() && lnewEntry != slastClipboard) {
	    slastClipboard = lnewEntry;
	    if(slastSelection == slastClipboard)
		lnewEntry = "";
	} else
	    lnewEntry = "";
    } else
	slastClipboard = cb->text(QClipboard::Clipboard).stripWhiteSpace();
	// prevent from spurious apperance (in next second after selection was shown)

    if(lnewEntry.isEmpty())
	return;

    lnewEntry.simplifyWhiteSpace();
    lnewEntry.truncate(20);

    result = myDict->FindWord(lnewEntry);
    myDict->ListRefresh(result);
    dictList->blockSignals(TRUE);
    dictList->setCurrentItem(0);
    dictList->blockSignals(FALSE);

    this->raise();
    this->show();
    this->setActiveWindow();	// XXX this sets focus

    UpdateHistory(result);
    wordInput->setEditText(lnewEntry);

    if ((config->autoPlay) && (lastresult!=result))
	PlaySelected (dictList->currentItem());

    lastresult = result;
}

void Kydpdict::showMinimized()
{
    QMainWindow::showMinimized();
    m_checkTimer->stop();
}

void Kydpdict::showEvent(QShowEvent *ashowEvent)
{
    static bool sfirstStart = true;

    if (sfirstStart)
	sfirstStart = false;
    else
	newClipData();
    QMainWindow::showEvent(ashowEvent);
    m_checkTimer->start(TIMER_PERIOD, FALSE);
}

void Kydpdict::NewDefinition (int index)
{
    UpdateHistory(index+myDict->topitem);
    RTFOutput->setText(myDict->GetDefinition(index+myDict->topitem));
    RTFOutput->setCursorPosition(0,0);
    scrollBar->setValue(index+myDict->topitem);
}

void Kydpdict::NewFromLine (const QString &newText)
{
    int result;

    // prevent spurious aperance when user starts writing immediately after Kydpdict gets focus
    slastSelection = cb->text(QClipboard::Selection).stripWhiteSpace();
    slastClipboard = cb->text(QClipboard::Clipboard).stripWhiteSpace();

    if (newText.length()) {
	result=myDict->FindWord(newText);
	myDict->ListRefresh(result);
	dictList->setCurrentItem(0);
    }
}

void Kydpdict::PlaySelected (int index)
{
    if ((index>=0) && config->toPolish)
	myDict->Play(index+myDict->topitem,config);
}

void Kydpdict::PlayCurrent ()
{
    UpdateHistory(dictList->currentItem());
    PlaySelected(dictList->currentItem());
}

void Kydpdict::UpdateHistory(int index)
{
    int i;
    QString content = (dictList->currentText()).simplifyWhiteSpace();

    if (content.length()<1)
	return;
    if ((wordInput->listBox())->findItem(content, Qt::ExactMatch) != 0)
	return;

    wordInput->blockSignals( TRUE );

    if ((wordInput->listBox())->count() < COMBO_HISTORY_SIZE) {
        wordInput->insertItem(content);
    } else {
	for (i=0;i<COMBO_HISTORY_SIZE;i++)
	    (wordInput->listBox())->changeItem( (wordInput->listBox())->text(i+1), i );
	(wordInput->listBox())->changeItem( content, COMBO_HISTORY_SIZE-1 );
    }
    wordInput->blockSignals( FALSE );
}

void Kydpdict::SwapLanguages ()
{
	SwapLang(!(config->toPolish), config->language);
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
	QString word;

	if(! ((config->toPolish == direction) && (config->language == language)) ) {
		word=wordInput->currentText();
		myDict->CloseDictionary();
		config->toPolish=direction;
		config->language = language;
		config->save();
		do {
			a=myDict->OpenDictionary(config);
			if (a) Configure(TRUE);
		} while (a);
		wordInput->setEditText(word);
		NewFromLine(word);
		UpdateLook();
	}
}

void Kydpdict::ShowAbout()
{
	cb->blockSignals( TRUE );
	QMessageBox aboutBox;

	aboutBox.setCaption("Kydpdict");
	aboutBox.setText(
	    tr("This is frontend to YDP Collins dictionary.\n"
	    "Authors: Andrzej Para , Maciej Witkowiak.\n"
	    "Program based on ydpdict by Wojtek Kaniewski"));
	aboutBox.setIconPixmap(QPixmap(babelfish_xpm));
	aboutBox.exec();

	cb->blockSignals( FALSE );
}

void Kydpdict::ShowAboutQt()
{
	cb->blockSignals( TRUE );
	QMessageBox::aboutQt(this, "Kydpdict");
	cb->blockSignals( FALSE);
}

void Kydpdict::ToggleClipTracking()
{
	config->clipTracking=!config->clipTracking;
	config->save();
	but_Clipboard->setOn(config->clipTracking);
	trayMenu->setItemChecked(trayMenuItemClipTrack, config->clipTracking);
}

void Kydpdict::ConfigureKydpdict()
{
	Configure(FALSE);
	UpdateLook();
}

void Kydpdict::ToolbarShowHide(bool visible)
{
    menu->setItemEnabled(toolBarMenuItem, !visible);

    if (visible != config->toolBarVisible) {
	config->toolBarVisible = visible;
	config->save();
    }
}

void Kydpdict::ShowToolbar()
{
    toolBar->show();
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
		   "In order to use this program you have to have data files from Windows\n"
		    "dictionary installation. For more information please read README.\n"
		    "Good bye!"));
		if(myConf)
  			delete myConf;
		exit(1);
	}

	if(myConf)
        	delete myConf;

	if (result == QDialog::Accepted) {	// just a performance gain
		config->save();	
	}

	if (cb->supportsSelection())
	    cb->clear(QClipboard::Selection);
	else
	    cb->clear(QClipboard::Clipboard);
	cb->blockSignals( FALSE );
	m_checkTimer->start(TIMER_PERIOD, FALSE);
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
	} else {
		RTFOutput->setPaper(QColor(config->kBckgrndKol));
		dictList->setPaletteBackgroundColor(QColor(config->kBckgrndKol));
	}
	dictList->setPaletteForegroundColor ( QColor(config->kFontKolor4) );

	if (but_EnPl!=NULL) but_EnPl->setEnabled(TRUE);
	if (but_PlEn!=NULL) but_PlEn->setEnabled(TRUE);
	if (but_DePl!=NULL) but_DePl->setEnabled(TRUE);
	if (but_PlDe!=NULL) but_PlDe->setEnabled(TRUE);

	but_Clipboard->setOn(config->clipTracking);
	trayMenu->setItemChecked(trayMenuItemClipTrack, config->clipTracking);

	switch (config->language) { //mozna dopisac kolejne case'y dla kolejnych jezykow
		case LANG_DEUTSCH:
			menu->setItemChecked(gerToPol, config->toPolish);
			menu->setItemChecked(polToGer, !config->toPolish);
			menu->setItemChecked(polToEng, FALSE);
			menu->setItemChecked(engToPol, FALSE);
			if(config->toPolish) {
				this->setCaption("De --> Pl - Kydpdict");
				but_DePl->setEnabled(FALSE);
			
			} else {
				this->setCaption("Pl --> De - Kydpdict");
				but_PlDe->setEnabled(FALSE);
			}
			break;
		case LANG_ENGLISH:
			menu->setItemChecked(engToPol, config->toPolish);
			menu->setItemChecked(polToEng, !config->toPolish);
			menu->setItemChecked(gerToPol, FALSE);
			menu->setItemChecked(polToGer, FALSE);
			if(config->toPolish) {
				this->setCaption("En --> Pl - Kydpdict");
				but_EnPl->setEnabled(FALSE);
			} else {
				this->setCaption("Pl --> En - Kydpdict");
				but_PlEn->setEnabled(FALSE);
			}
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

	RTFOutput->setFont(config->fontTransFont);
	dictList->setFont(config->fontTransFont);
	wordInput->setFont(config->fontTransFont);

	NewDefinition(dictList->currentItem() < 0 ? 0 : dictList->currentItem() );

	scrollBar->setMaxValue(myDict->wordCount);
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
