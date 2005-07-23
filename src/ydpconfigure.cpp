/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qfont.h>
#include <qpixmap.h>
#include <qfiledialog.h>
#include <qfontdialog.h>
#include <qcolordialog.h>
#include <qtextbrowser.h>
#include <qcombobox.h>
#include <qmessagebox.h>

#include "ydpconfigure.h"
#include "ydpconfigure.moc"

ydpConfigure::ydpConfigure( kydpConfig *globalconfig, QWidget* parent,  const char* name, bool modal )
    : QDialog( parent, name, modal )
{
    setFixedSize(400,260);
    setCaption( tr("Configure Kydpdict") );

    defaults = new QPushButton( this, "defaults" );
    defaults->setText(tr("Defaults") );
    defaults->setGeometry(10,220,81,30);

    cancel = new QPushButton( this, "cancel" );
    cancel->setText( tr("Cancel") );
    cancel->setGeometry(310,220,81,30);

    ok = new QPushButton( this, "Ok" );
    ok->setText( tr("Ok") );
    ok->setGeometry(210,220,81,30);

    TabWidget1 = new QTabWidget( this, "TabWidget1" );
    TabWidget1->setGeometry(10,10,381,200);

    tab1 = new QWidget( TabWidget1, "tab1" );

    GroupBox1 = new QGroupBox( tab1, "GroupBox1" );
    GroupBox1->setTitle( tr("Path to dictionary files") );
    GroupBox1->setGeometry(8,5,361,50);

    dictionaryUrl = new QLineEdit( GroupBox1, "dictionaryUrl" );
    dictionaryUrl->setFrameStyle( QFrame::Sunken | QFrame::Panel);
    dictionaryUrl->setLineWidth( 1 );
    dictionaryUrl->setGeometry( 10, 17, 290, 22 );
    changeDictionaryUrl = new QPushButton("...", GroupBox1, "changeDictionaryUrl");
    changeDictionaryUrl->setGeometry(310,17,40,22);

    GroupBox5 = new QGroupBox( tab1, "GroupBox5" );
    GroupBox5->setTitle( tr("Dictionary Engine") );
    GroupBox5->setGeometry(8,59,361,50);
    engineCombo = new QComboBox( GroupBox5, "engineCombo" );
    engineCombo->setGeometry( 10, 17, 290, 22 );
    // this must be done in order of ENGINE_* enums!
    engineCombo->insertItem("PWN Oxford 2003", ENGINE_PWN);
    engineCombo->insertItem("YDP Collins", ENGINE_YDP);
    engineCombo->setEditable(false);

    TabWidget1->insertTab( tab1, tr("Dictionary") );

    tab4 = new QWidget( TabWidget1, "tab4" );

    GroupBox2 = new QGroupBox( tab4, "GroupBox2" );
    GroupBox2->setTitle( tr("Path to English audio files") );
    GroupBox2->setGeometry(8,5,361,50);

    audioUrl = new QLineEdit( GroupBox2, "audioUrl" );
    audioUrl->setFrameStyle( QFrame::Sunken | QFrame::Panel);
    audioUrl->setLineWidth( 1 );
    audioUrl->setGeometry( 10, 17, 290, 22 );
    changeAudioUrl = new QPushButton("...", GroupBox2, "changeAudioUrl");
    changeAudioUrl->setGeometry(310,17,40,22);

    GroupBox4 = new QGroupBox( tab4, "GroupBox4" );
    GroupBox4->setTitle( tr("Path to Deutsch audio files") );
    GroupBox4->setGeometry(8,59,361,50);

    audio2Url = new QLineEdit( GroupBox4, "audio2Url" );
    audio2Url->setFrameStyle( QFrame::Sunken | QFrame::Panel);
    audio2Url->setLineWidth( 1 );
    audio2Url->setGeometry( 10, 17, 290, 22 );
    changeAudio2Url = new QPushButton("...", GroupBox4, "changeAudio2Url");
    changeAudio2Url->setGeometry(310,17,40,22);

    GroupBox3 = new QGroupBox( tab4, "GroupBox3" );
    GroupBox3->setTitle( tr("Path to audio player") );
    GroupBox3->setGeometry(8,111,361,50);

    playerUrl = new QLineEdit( GroupBox3, "mp3Url" );
    playerUrl->setFrameStyle( QFrame::Sunken | QFrame::Panel);
    playerUrl->setLineWidth( 1 );
    playerUrl->setGeometry( 10, 17, 290, 22 );
    changePlayerUrl = new QPushButton("...", GroupBox3, "changePlayerUrl");
    changePlayerUrl->setGeometry(310,17,40,22);

    TabWidget1->insertTab( tab4, tr("Audio") );

    tab2 = new QWidget( TabWidget1, "tab2" );

    textLabel2 = new QLabel(tr("Choose qualifier color"), tab2, "textLabel2");
    textLabel2->setGeometry(6,36,150,22);
    textLabel1 = new QLabel(tr("Choose translation color"), tab2, "textLabel1");
    textLabel1->setGeometry(6,62,150,22);
    textLabel3 = new QLabel(tr("Choose example color"), tab2, "textLabel3");
    textLabel3->setGeometry(6,88,150,22);
    textLabel4 = new QLabel(tr("Choose background image"), tab2, "textLabel4");
    textLabel4->setGeometry(6,138,153,22);
    textLabel5 = new QLabel(tr("Choose plain text color"), tab2, "textLabel5");
    textLabel5->setGeometry(6,10,150,22);
    changeFont2 = new QPushButton("->", tab2, "changeFont2");
    changeFont2->setGeometry(162,36,40,22);
    changeFont1 = new QPushButton("->", tab2, "changeFont1");
    changeFont1->setGeometry(162,62,40,22);
    changeFont3 = new QPushButton("->", tab2, "changeFont3");
    changeFont3->setGeometry(162,88,40,22);
    changeFont4 = new QPushButton("->", tab2, "changeFont4");
    changeFont4->setGeometry(162,10,40,22);
    changeBckgrndUrl = new QPushButton("...", tab2, "changeBckgrnd");
    changeBckgrndUrl->setGeometry(162,138,40,22);

    checkBox1 = new QCheckBox(tr("Enable Background"), tab2, "checkBox1");
    checkBox1->setGeometry(6,117,150,16);

    changeFont = new QPushButton(tr("Font"), tab2, "changeFont");
    changeFont->setGeometry(162,117,40,22);

    exampleLabel1 = new QTextBrowser (tab2, "exampleLabel1");
    exampleLabel1->setAlignment( Qt::AlignCenter );
    exampleLabel1->setFrameStyle( QFrame::Box | QFrame::Plain);
    exampleLabel1->setLineWidth( 1 );
    exampleLabel1->setGeometry(208,10,160,150);
    exampleLabel1-> setReadOnly(TRUE);

    TabWidget1->insertTab( tab2, tr("Colors") );

    tab3 = new QWidget( TabWidget1, "tab3" );

    checkBox3 = new QCheckBox(tr("Enable italic font"), tab3, "checkBox3");
    checkBox3->setGeometry(10,14,350,20);
    checkBox4 = new QCheckBox(tr("Enable tooltips"), tab3, "checkBox4");
    checkBox4->setGeometry(10,40,350,20);
    checkBox7 = new QCheckBox(tr("Enable tray icon"), tab3, "checkBox7");
    checkBox7->setGeometry(10,66,350,20);
    checkBox9 = new QCheckBox(tr("Use Unicode characters"), tab3, "checkBox9");
    checkBox9->setGeometry(10,92,350,20);

    TabWidget1->insertTab( tab3, tr("Miscellaneous") );

    tab5 = new QWidget( TabWidget1, "tab5" );

    checkBox2 = new QCheckBox(tr("Enable clipboard tracking"), tab5, "checkBox2");
    checkBox2->setGeometry(10,14,350,20);
    checkBox6 = new QCheckBox(tr("Ignore own selection"), tab5, "checkBox6");
    checkBox6->setGeometry(10,40,350,20);
    checkBox8 = new QCheckBox(tr("Set focus on self"), tab5, "checkBox8");
    checkBox8->setGeometry(10,66,350,20);
    checkBox5 = new QCheckBox(tr("Enable autoplay"), tab5, "checkBox5");
    checkBox5->setGeometry(10,92,350,20);

    TabWidget1->insertTab( tab5, tr("Clipboard") );

    connect( ok, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( cancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( defaults, SIGNAL( clicked() ), this, SLOT( WriteDefaults() ) );

    connect( changeDictionaryUrl, SIGNAL( clicked() ), this, SLOT( NewDictUrl() ) );
    connect( changeAudioUrl, SIGNAL( clicked() ), this, SLOT( NewAudioUrl() ) );
    connect( changeAudio2Url, SIGNAL( clicked() ), this, SLOT( NewAudio2Url() ) );
    connect( changeFont, SIGNAL( clicked() ), this, SLOT( NewFont() ) );
    connect( changeFont1, SIGNAL( clicked() ), this, SLOT( NewFontColor1() ) );
    connect( changeFont2, SIGNAL( clicked() ), this, SLOT( NewFontColor2() ) );
    connect( changeFont3, SIGNAL( clicked() ), this, SLOT( NewFontColor3() ) );
    connect( changeFont4, SIGNAL( clicked() ), this, SLOT( NewFontColor4() ) );
    connect( changeBckgrndUrl, SIGNAL( clicked() ), this, SLOT( NewBckgrndUrl() ) );
    connect( checkBox1, SIGNAL( stateChanged ( int ) ), this, SLOT(NewCheck(int ) ) );
    connect( checkBox2, SIGNAL( stateChanged ( int ) ), this, SLOT(NewCheck2(int ) ) );
    connect( changePlayerUrl, SIGNAL( clicked() ), this, SLOT( NewPlayerUrl() ) );

    config = globalconfig;
    playerUrl->setText(config->player);
    dictionaryUrl->setText(config->topPath);
    audioUrl->setText(config->cdPath);
    audio2Url->setText(config->cd2Path);
    cFontKolor1 = config->kFontKolor1;
    cFontKolor2	= config->kFontKolor2;
    cFontKolor3 = config->kFontKolor3;
    cFontKolor4 = config->kFontKolor4;
    cBckgrnd = config->kBckgrndPix;
    cBckgrndKol = config->kBckgrndKol;
    cFont = config->fontTransFont;
    engineCombo->setCurrentItem(config->engine);

    exampleLabel1->setFont(cFont);

}

void ydpConfigure::NewDictUrl()
{
    QString tmp;

    tmp = QFileDialog::getExistingDirectory("/", this , "path", tr("Choose path to dictionaries"));
    if (!tmp.isEmpty ()) dictionaryUrl->setText(tmp);
}

void ydpConfigure::NewAudioUrl()
{
    QString tmp;

    tmp = QFileDialog::getExistingDirectory("/", this , "path", tr("Choose path to English audio files"));
    if (!tmp.isEmpty ()) audioUrl->setText(tmp);
}

void ydpConfigure::NewAudio2Url()
{
    QString tmp;

    tmp = QFileDialog::getExistingDirectory("/", this , "path", tr("Choose path to Deutsch audio files"));
    if (!tmp.isEmpty ()) audio2Url->setText(tmp);
}

void ydpConfigure::NewPlayerUrl()
{
    QString tmp;

    tmp = QFileDialog::getOpenFileName("/", NULL, this , "player", tr("Choose audio player"));
    if (!tmp.isEmpty ()) playerUrl->setText(tmp);
}

void ydpConfigure::NewFont()
{
    bool accepted;
    QFont myFont = QFontDialog::getFont( &accepted, cFont, 0 );
    if (accepted) {
	cFont = myFont;
	UpdateLabels();
    }
}

void ydpConfigure::NewFontColor1()
{
    cFontKolor1 = GetNewColor(cFontKolor1);
    UpdateLabels();
}

void ydpConfigure::NewFontColor2()
{
    cFontKolor2 = GetNewColor(cFontKolor2);
    UpdateLabels();
}

void ydpConfigure::NewFontColor3()
{
    cFontKolor3 = GetNewColor(cFontKolor3);
    UpdateLabels();
}

void ydpConfigure::NewFontColor4()
{
    cFontKolor4 = GetNewColor(cFontKolor4);
    UpdateLabels();
}

void ydpConfigure::NewBckgrndUrl()
{
    if (checkBox1->isChecked())
	cBckgrnd = GetNewBackgroundUrl(cBckgrnd);
    else
	cBckgrndKol = GetNewColor(cBckgrndKol);

    UpdateLabels();
}

void ydpConfigure::NewCheck(int state)
{
    switch(state)
    {
    case 0:
	changeBckgrndUrl->setText("-->");
	textLabel4->setText(tr("Choose background color"));
	cBckgrnd = "NoBackground";
    	break;
    case 2:
	changeBckgrndUrl->setText("...");
	textLabel4->setText(tr("Choose background image"));
    	break;
    default:
    	break;
    }
    UpdateLabels();
}

void ydpConfigure::NewCheck2(int state)
{
	checkBox6->setEnabled(checkBox2->isOn());
}

void ydpConfigure::UpdateLabels()
{
    QString label;
    QPixmap pixmap;
    QBrush paper;

    exampleLabel1->setLinkUnderline(FALSE);
    QMimeSourceFactory::defaultFactory()->setText ( "pomoc", "<qt type=\"detail\" title=\"tytul\"><nobr><font color=\"red\">adjective - przymiotnik</font></nobr></qt>" );

    if(checkBox1->isChecked() && pixmap.load(cBckgrnd) ) {
	paper.setPixmap(pixmap);
	exampleLabel1->setPaper( paper );
    } else {
	exampleLabel1->setPaper( QColor(cBckgrndKol) );
    }

    label = "<qt type=\"page\"><font color=" + cFontKolor4 + "><b>ably</b> [eibli]<br><br><i><a href=\"pomoc\"><font color=" + cFontKolor2 + ">adv</font></a></i><br><font color=" +
    cFontKolor1 + ">umiejêtnie</font>, <font color=" + cFontKolor1 + ">zrêcznie</font><br><table cellspacing=0><tr><td width=25></td><td><font color=" +
    cFontKolor3 + ">They were ably supported by the Communist Party.</font></td></tr></table></font></qt>";
    exampleLabel1->setText(label.fromLocal8Bit(label));
    exampleLabel1->setFont(cFont);

}

QString ydpConfigure::GetNewColor(QString kolor)
{
    QColor tmp;
    tmp.setNamedColor(kolor);
    QColor color = QColorDialog::getColor(tmp, this, "color dialog" );
    if ( color.isValid() )
	    return color.name();
    else
	    return kolor;

}

QString ydpConfigure::GetNewBackgroundUrl(QString tlo)
{
    QString tmp;
    tmp = QFileDialog::QFileDialog::getOpenFileName(tlo, "Images (*.png *.xpm *.jpg)", this, "open file dialog" );
    if (!tmp.isEmpty ())
    	return tmp;
    else
    	return tlo;

}

void ydpConfigure::WriteDefaults()
{
    kydpConfig *cnf = new kydpConfig;

    dictionaryUrl->setText(cnf->topPath);
    audioUrl->setText(cnf->cdPath);
    audio2Url->setText(cnf->cd2Path);
    playerUrl->setText(cnf->player);
    cFontKolor1 = cnf->kFontKolor1;
    cFontKolor2 = cnf->kFontKolor2;
    cFontKolor3 = cnf->kFontKolor3;
    cFontKolor4 = cnf->kFontKolor4;
    cBckgrnd = cnf->kBckgrndPix;
    cBckgrndKol = cnf->kBckgrndKol;
    checkBox1->setChecked(FALSE);
    checkBox2->setChecked(cnf->clipTracking);
    checkBox6->setChecked(cnf->ignoreOwnSelection);
    checkBox3->setChecked(cnf->italicFont);
    checkBox4->setChecked(cnf->toolTips);
    checkBox7->setChecked(cnf->dock);
    checkBox5->setChecked(cnf->autoPlay);
    checkBox8->setChecked(cnf->setFocusOnSelf);
    checkBox9->setChecked(cnf->unicodeFont);
    engineCombo->setCurrentItem(cnf->engine);
    UpdateLabels();
    delete cnf;
}

void ydpConfigure::showEvent ( QShowEvent * )
{
    bool a = (QString::compare(cBckgrnd, "NoBackground") != 0);
    if (a)
	NewCheck(2);
    else
	NewCheck(0);
    checkBox1->setChecked(a);

    checkBox2->setChecked(config->clipTracking);
    checkBox6->setChecked(config->ignoreOwnSelection);
    checkBox6->setEnabled(config->clipTracking);
    checkBox3->setChecked(config->italicFont);
    checkBox4->setChecked(config->toolTips);
    checkBox7->setChecked(config->dock);
    checkBox5->setChecked(config->autoPlay);
    checkBox8->setChecked(config->setFocusOnSelf);
    checkBox9->setChecked(config->unicodeFont);
    UpdateLabels();
}

ydpConfigure::~ydpConfigure()
{
    // no need to delete child widgets, Qt does it all for us
	if (result()==Accepted) {
		config->kFontKolor1 = cFontKolor1;
		config->kFontKolor2 = cFontKolor2;
		config->kFontKolor3 = cFontKolor3;
		config->kFontKolor4 = cFontKolor4;
		config->kBckgrndPix = cBckgrnd;
		config->kBckgrndKol = cBckgrndKol;
		config->topPath = dictionaryUrl->text();
		config->cdPath = audioUrl->text();
		config->cd2Path = audio2Url->text();
		config->player = playerUrl->text();
		config->clipTracking = checkBox2->isChecked();
		config->ignoreOwnSelection = checkBox6->isChecked();
		config->italicFont = checkBox3->isChecked();
		config->toolTips = checkBox4->isChecked();
		config->dock = checkBox7->isChecked();
		config->autoPlay = checkBox5->isChecked();
		config->setFocusOnSelf = checkBox8->isChecked();
		config->unicodeFont = checkBox9->isChecked();
		config->fontTransFont = cFont;
		if (config->engine != engineCombo->currentItem()) {
		    QMessageBox::information( this, tr("Information"),
		    tr( "New dictionary engine will be enabled after you restart Kydpdict." ));
		}
		config->engine = engineCombo->currentItem();
	}
}
