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

Kydpdict::Kydpdict(QWidget *parent, const char *name) : QWidget(parent, name)
{
	dictList = new QListBox( this, "dictList" );
	wordInput = new QLineEdit (this, "wordInput");
	RTFOutput = new QTextEdit (this, "RTFOutput");
	listclear = new QPushButton(tr("Clear"),this, "Clear");
	listclear->setAccel ( QKeySequence( tr("Ctrl+X", "Clear") ) );

	RTFOutput->setTextFormat( RichText );
	RTFOutput->setReadOnly(TRUE);
	RTFOutput->setLineWidth( 1 );
	RTFOutput->setFrameStyle( QFrame::Sunken | QFrame::Panel);
	dictList->setBottomScrollBar(FALSE);
	dictList->setFrameStyle( QFrame::Sunken | QFrame::Panel);
	dictList->setLineWidth( 1 );
	wordInput ->setMaxLength(20);
	wordInput->setLineWidth( 1 );
	wordInput->setFrameStyle( QFrame::Sunken | QFrame::Panel);

	cb =  QApplication::clipboard();

 	menu = new QMenuBar( this, "menu" );

	config = new kydpConfig;
	config->load();

 	myDict = new ydpDictionary(config,dictList);

	setGeometry (config->kGeometryX, config->kGeometryY, config->kGeometryW, config->kGeometryH);
	InitVisual();

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

	connect (dictList, SIGNAL(highlighted(int)), this, SLOT(NewDefinition(int)));
	connect (dictList, SIGNAL(selected(int)), this, SLOT(PlayCurrent()));
	connect (wordInput, SIGNAL(textChanged(const QString&)), this, SLOT(NewFromLine(const QString&)));
	connect (wordInput, SIGNAL(returnPressed()), this, SLOT(PlayCurrent()));
	connect (listclear, SIGNAL(clicked()),wordInput, SLOT(clear()));
	connect (cb, SIGNAL(selectionChanged()), this, SLOT(PasteClippboard()));

	setIcon(QPixmap(tux_xpm));

	this->show();

	UpdateLook();
}

Kydpdict::~Kydpdict()
{
	myDict->CloseDictionary();
}

void Kydpdict::resizeEvent(QResizeEvent *)
{
	QSize aRozmiar;

	InitVisual();
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

void Kydpdict::InitVisual(void)
{
	RTFOutput->setGeometry( (this->width()-15)/3+10, 30,
				(this->width()-(this->width()-15)/3-15),
				(this->height()-35));

	wordInput->setGeometry( 5, 30, (this->width()-10)/3-40-5, 20);
	listclear->setGeometry(5 + (this->width()-10)/3-40-1,30,40,20);
	dictList->setGeometry( 5, 55, (this->width()-10)/3, (this->height()-60) );

	dictList->repaint();
}

void Kydpdict::ShowDefinition(QString def)
{

	RTFOutput->setText(def.fromLocal8Bit(def));
}

void Kydpdict::PasteClippboard()
{
	int index;
	QListBoxItem *result, *tmp_result;

	// do nothing if minimized
	if (this->isMinimized())
		return;

	cb->setSelectionMode(true);

	QString clipContents = cb->text().stripWhiteSpace();

	clipContents.truncate(20);

	if(!(result = dictList->findItem(clipContents, Qt::ExactMatch))) {
		int down = 0;
		int up = clipContents.length() + 1;

		while(up - down > 1) {
			if((tmp_result = dictList->findItem(clipContents.left((up+down)/2)))) {
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
	wordInput->setText(clipContents);
	wordInput->blockSignals( FALSE );
}

void Kydpdict::NewDefinition (int index) {
	ShowDefinition(myDict->GetDefinition(index));
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
