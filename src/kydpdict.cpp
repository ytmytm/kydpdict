/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qframe.h>
#include <qmessagebox.h>
#include <qdialog.h>
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
#define LAST_ESC_PERIOD		750

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
#include "../icons/clipboard.xpm"
#include "../icons/babelfish.xpm"

#include "kydpdict.h"
#include "kydpconfig.h"
#include "dock_widget.h"

#ifndef WITHOUT_X11
// this is for updateUserTimestamp
#if QT_VERSION >= 0x030300
#include <X11/Xlib.h>
#include <X11/Xatom.h>
extern Time qt_x_user_time;	// this is from Qt
#endif
#endif

Kydpdict::Kydpdict(QWidget *parent, const char *name) : QMainWindow(parent, name)
{
    bool wasMaximized;

	updateUserTimestamp();
	tipsVisible = false;

	toolBar = new QToolBar(this, "toolbar");
	toolBar->setLabel(tr("Kydpdict toolbar"));
	toolBar->setCloseMode(QToolBar::Undocked);

	QFrame *centralFrame = new QFrame(this);
	splitterH = new QSplitter(centralFrame, "splitter");
	splitterV = new QSplitter(Qt::Vertical, splitterH);
	QHBox *hbox1 = new QHBox(splitterV);
	QHBox *hbox2 = new QHBox(splitterV);
	wordInput = new QComboBox( hbox1, "wordInput");
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
	wordInput->setAutoCompletion(false);
	wordInput->setInsertionPolicy(QComboBox::AtTop);

	cb = QApplication::clipboard();

	menu = menuBar();

	config = new kydpConfig;
	config->load();

 	myDict = new ydpDictionary(config,dictList);

	mySearch = new ydpFuzzySearch(parent);
	mySearch->setModal(FALSE);
	// this has to be before OpenDictionary so the notification will be passed
	connect(myDict, SIGNAL(dictionaryChanged(const int, char **)), mySearch, SLOT(updateDictionary(const int, char **)));
	connect(mySearch, SIGNAL(textChanged(const QString&)), this, SLOT(newFromLine(const QString&)));

	wasMaximized = config->kMaximized;
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
	escTimer = new QTimer(this, "esctimer");
	lastEdit = "";
	lastSelection = "";
	lastClipboard = "";
	eventLock = false;

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
	file->insertItem(tr("&Fuzzy search"), this, SLOT(FuzzySearch()), QKeySequence( tr("Ctrl+F", "File|Fuzzy search") ) );

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
	trayMenuItemClipTrack = trayMenu->insertItem( tr("C&lipboard tracking"), this, SLOT(ToggleClipTracking()), QKeySequence(tr("Ctrl+L", "Tray|Clipboard tracking")));
	trayMenu->insertSeparator();
	trayMenu->insertItem(QPixmap(babelfish_small_xpm), tr("About"), this, SLOT(ShowAbout()));
	trayMenu->insertItem(QPixmap(exit_xpm), tr("&Quit"), qApp, SLOT(quit()) );

	Q_CHECK_PTR( menu );
	menu->insertItem(  tr("&File"), file );
	menu->insertItem(  tr("&Options"), settings );
	menu->insertItem(  tr("&Help"), help );

 	myDynamicTip = new DynamicTip( RTFOutput );

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

	connect (dictList, SIGNAL(highlighted(int)), this, SLOT(newFromList(int)));
	connect (dictList, SIGNAL(selected(int)), this, SLOT(PlayCurrent()));
	connect (dictList, SIGNAL(clicked(QListBoxItem*)), this, SLOT(newFromClick(QListBoxItem*)));
	connect (wordInput, SIGNAL(textChanged(const QString&)), this, SLOT(newFromLine(const QString&)));
	connect (wordInput, SIGNAL(activated(int)), this, SLOT(PlayCurrent()));
	connect (RTFOutput, SIGNAL(highlighted(const QString &)), this, SLOT(handleTip(const QString &)));
	connect (RTFOutput, SIGNAL(linkClicked(const QString &)), this, SLOT(handleLink(const QString &)));
	connect (m_checkTimer, SIGNAL(timeout()), this, SLOT(newFromSelection()));
	connect (toolBar, SIGNAL(visibilityChanged(bool)), this, SLOT(ToolbarShowHide(bool)));
	connect (cb, SIGNAL(selectionChanged() ), SLOT(newFromSelection()));
	connect (cb, SIGNAL(dataChanged() ), SLOT(newFromSelection()));

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

	cb->clear(QClipboard::Selection);
	cb->clear(QClipboard::Clipboard);
	this->show();
	myDict->ListRefresh(0);
	dictList->setCurrentItem(0);
	wordInput->clearEdit();

#ifndef WITHOUT_X11
	if (config->dock) {
	    trayicon = new TrayIcon(this,"trayicon");
	    trayicon->setDictWidget(this);
	    trayicon->setPopupMenu(trayMenu);
	    trayicon->show();
	}
#endif
	setFocusProxy(wordInput);
	RTFOutput->setFocusProxy(wordInput);

	if (wasMaximized)
	    this->showMaximized();
}

Kydpdict::~Kydpdict()
{
    	myDict->CloseDictionary();
 	delete myDynamicTip;
 	delete m_checkTimer;
	delete escTimer;
	delete splitterH;
}

void Kydpdict::flushConfig(void)
{
	QSize aSize;
	QPoint aPosition;

	config->kMaximized = this->isMaximized();

	aSize = this->size();
	config->kGeometryW = aSize.width();
	config->kGeometryH = aSize.height();

	aPosition = this->pos();
	config->kGeometryX = aPosition.x();
	config->kGeometryY = aPosition.y();

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
	int item;

	item=dictList->currentItem();
	myDict->ListRefresh(myDict->topitem);
	dictList->setCurrentItem(item);
	scrollBar->setPageStep(dictList->numItemsVisible());
}

void Kydpdict::onEscaped()
{
    if (!wordInput->hasFocus())
	wordInput->setFocus();
    else
	if (wordInput->currentText().isEmpty() && (!(escTimer->isActive()))) {
	    this->hide();
	} else {
	    wordInput->clearEdit();
	    escTimer->start(LAST_ESC_PERIOD,TRUE);
	}
}

void Kydpdict::windowActivationChange(bool oldActive)
{
    if (isActiveWindow()) {
	m_checkTimer->stop();
	wordInput->setFocus();
	wordInput->lineEdit()->selectAll();
    } else {
	wordInput->lineEdit()->deselect();
	lastClipboard = cb->text(QClipboard::Clipboard).stripWhiteSpace();   // protect from catching own copy in clipboard
	m_checkTimer->start(TIMER_PERIOD,FALSE);
    }
}

void Kydpdict::hideEvent( QHideEvent *ahideEvent)
{
    QMainWindow::hideEvent(ahideEvent);
    m_checkTimer->stop();
}

void Kydpdict::showEntry(QString *aEntry, int aindex)
{
// when aindex is set aEntity is ignored (only for performance reason)
    if (aindex < 0) {
	int lindex = myDict->FindWord(*aEntry);
	myDict->ListRefresh(lindex);
	dictList->setCurrentItem(lindex - myDict->topitem);
	scrollBar->setValue(lindex);
	if (aindex == -2)   // from selection or clipboard
	    wordInput->setEditText(*aEntry);
	aindex = lindex - myDict->topitem;
    } else {
	wordInput->setEditText(dictList->text(dictList->currentItem()));
	wordInput->setFocus();
	wordInput->lineEdit()->selectAll();
    }

    RTFOutput->setText(myDict->GetDefinition(myDict->topitem + dictList->currentItem()));
    RTFOutput->setCursorPosition(0, 0);

    UpdateHistory();
    tipsVisible = false;

    if (config->autoPlay)
	PlaySelected (dictList->currentItem());

    this->show();
    this->raise();

    if (this->isMinimized())
	this->showNormal();

    if (config->setFocusOnSelf)
	this->setActiveWindow();
}

void Kydpdict::newFromLine(const QString& aEntry)
{
    if (eventLock)
	return;

    eventLock = true;
    if (!aEntry.isEmpty()) {
	QString lentry = aEntry.stripWhiteSpace();
	if ((lentry.length())&&(lentry != lastEdit)) {
		lastEdit = lentry;
		showEntry(&lentry);
	}
    }
    eventLock = false;
}

void Kydpdict::newFromList(int aindex)
{
    if (eventLock)
      return;

    eventLock = true;
    if (aindex >= 0) {
	showEntry(NULL, aindex);
	lastEdit = "";
    }
    eventLock = false;
}

void Kydpdict::newFromSelection()
{
    // do nothing if clipboard tracking is disabled
    if (!(config->clipTracking))
	return;
    // do nothing if minimized
    if (this->isMinimized())
	return;
    // do nothing if it comes from us
    if (((RTFOutput->hasSelectedText())||(wordInput->lineEdit()->hasSelectedText())) && (config->ignoreOwnSelection))
        return;
    if (eventLock)
	return;

    eventLock = true;
    updateUserTimestamp();

    // get selection - if selection and clipboard are changed simultaneously selection
    // is chosen an clipboard is ignored
    QString lnewEntry = cb->text(QClipboard::Selection).stripWhiteSpace(); 
    if (lnewEntry.length() && lnewEntry != lastSelection)
	lastSelection = lnewEntry;
    else
	lnewEntry = "";

    if (lnewEntry.isEmpty()) {
	// get clipboard if selection empty or unchanged
	lnewEntry = cb->text(QClipboard::Clipboard).stripWhiteSpace();    
	if (lnewEntry.length() && lnewEntry != lastClipboard) {
	    lastClipboard = lnewEntry;
	    if (lastSelection == lastClipboard)
		lnewEntry = "";
	} else
	    lnewEntry = "";
    } else
	lastClipboard = cb->text(QClipboard::Clipboard).stripWhiteSpace();
	// prevent from spurious apperance (in next second after selection was shown)

    if (lnewEntry.length())
	showEntry(&lnewEntry, -2);

    eventLock = false;
}

void Kydpdict::newFromClick(QListBoxItem *lbi)
{
    if (tipsVisible)
	newFromList(dictList->currentItem());
}

void Kydpdict::showEvent(QShowEvent *ashowEvent)
{
    static bool sfirstStart = true;

    if (sfirstStart)
	sfirstStart = false;
    else
	newFromSelection();
    QMainWindow::showEvent(ashowEvent);
    m_checkTimer->start(TIMER_PERIOD, FALSE);
}

void Kydpdict::FuzzySearch()
{
    mySearch->show();
}

void Kydpdict::PlaySelected (int index)
{
    if ((index>=0) && config->toPolish)
	myDict->Play(index+myDict->topitem,config);
}

void Kydpdict::PlayCurrent ()
{
    UpdateHistory();
    PlaySelected(dictList->currentItem());
}

void Kydpdict::UpdateHistory(void)
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
		UpdateLook();
		newFromLine(word);
	}
}

void Kydpdict::ShowAbout()
{
	cb->blockSignals( TRUE );
	QMessageBox aboutBox;

	aboutBox.setCaption("Kydpdict " VERSION);
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
	cb->blockSignals( FALSE );
}

void Kydpdict::ToggleClipTracking()
{
	config->clipTracking=!config->clipTracking;
	config->save();
	but_Clipboard->setOn(config->clipTracking);
	trayMenu->setItemChecked(trayMenuItemClipTrack, config->clipTracking);
	if (config->clipTracking) {
	    cb->clear(QClipboard::Selection);
	    cb->clear(QClipboard::Clipboard);
	}
}

void Kydpdict::ConfigureKydpdict()
{
	Configure(FALSE);
	UpdateLook();
}

void Kydpdict::ToolbarShowHide(bool visible)
{
    menu->setItemChecked(toolBarMenuItem, visible);
    if (visible != config->toolBarVisible) {
	config->toolBarVisible = visible;
	config->save();
	if (config->toolBarVisible)
	    toolBar->show();
	else
	    toolBar->hide();
    }
}

void Kydpdict::ShowToolbar()
{
    ToolbarShowHide(!config->toolBarVisible);
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

	if (result == QDialog::Accepted)	// just a performance gain
	    config->save();

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
		wordInput->setBackgroundMode(PaletteBase);
		wordInput->setPaletteBackgroundColor(config->kBckgrndKol);
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

	char *tst = const_cast<char*>((const char*)tmp);
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

	int item;
	item = dictList->currentItem();
	myDict->ListRefresh(myDict->topitem);
	dictList->setCurrentItem(item);

	scrollBar->setMaxValue(myDict->wordCount);
}

void Kydpdict::handleLink( const QString & href )
{
	QString link = href;
	QString tmp;
	int i;

	tipsVisible = true;

	if (link.startsWith("2")) {
		QString output = "<h2>Skróty wystêpuj±ce w t³umaczeniach</h2>";
		output += "<h3>Czê¶æ I - GRAMATYKA</h3>";

		for(i=0; i<myDict->GetTipNumber(1); i++) {
			tmp.sprintf("%d", i);
			output += "<a name=\""+ tmp + "\"></a><h4><font color=\"red\">"+ myDict->GetInputTip(i) + "</font></h4>" + myDict->GetOutputTip(i) + "<hr>";
		}

		output += "<h3>Czê¶æ II - DZIEDZINY</h3>";

		for(i=myDict->GetTipNumber(1); i<myDict->GetTipNumber(0); i++) {
			tmp.sprintf("%d", i);
			output += "<a name=\""+ tmp + "\"></a><h4><font color=\"red\">"+ myDict->GetInputTip(i) + "</font></h4>" + myDict->GetOutputTip(i) + "<hr>";
		}

		RTFOutput->setText(output);
		RTFOutput->scrollToAnchor(link.remove(0,1));
	} else {
		if (link.startsWith("1"))
			SwapLanguages();

		link.remove(0,1);
		if (link.compare(dictList->currentText()) != 0) {
		    newFromLine(link);
		} else {
		    // it is required to do something with RTFOutput even when it's not needed
		    RTFOutput->setText(RTFOutput->text());
		}
	}
}

void Kydpdict::handleTip( const QString & href )
{
    QString tmp = href;

    if (tmp.startsWith("2")) {
	tmp = tmp.remove(0,1);
	if(tmp.toInt() >= 0 && tmp.toInt() < myDict->GetTipNumber(0))
	    myDynamicTip->tekst = myDict->GetOutputTip(tmp.toInt());
    } else
	myDynamicTip->tekst = "";
}

void Kydpdict::updateUserTimestamp(void) {
#ifndef WITHOUT_X11
#if QT_VERSION >= 0x030300
#if defined Q_WS_X11
    // this is from kapplication.cpp, as suggested by klipper.cpp, quote:
    // QClipboard uses qt_x_user_time as the timestamp for selection operations.
    // It is updated mainly from user actions, but Klipper polls the clipboard
    // without any user action triggering it, so qt_x_user_time may be old,
    // which could possibly lead to QClipboard reporting empty clipboard.
    // Therefore, qt_x_user_time needs to be updated to current X server timestamp.
    unsigned long time;
    // get current X timestamp
    Window w = XCreateSimpleWindow( qt_xdisplay(), qt_xrootwin(), 0, 0, 1, 1, 0, 0, 0 );
    XSelectInput( qt_xdisplay(), w, PropertyChangeMask );
    unsigned char data[ 1 ];
    XChangeProperty( qt_xdisplay(), w, XA_ATOM, XA_ATOM, 8, PropModeAppend, data, 1 );
    XEvent ev;
    XWindowEvent( qt_xdisplay(), w, PropertyChangeMask, &ev );
    time = ev.xproperty.time;
    XDestroyWindow( qt_xdisplay(), w );
    qt_x_user_time = time;
#endif
#endif
#endif
}
