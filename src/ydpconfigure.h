/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef YDPCONFIGURE_H
#define YDPCONFIGURE_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qtextedit.h>
#include <qtabwidget.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include "kydpconfig.h"

class ydpConfigure : public QDialog
{
	Q_OBJECT

public:
    ydpConfigure( kydpConfig *config, QWidget* parent = 0, const char* name = 0, bool modal = FALSE );
    ~ydpConfigure();

private:
    void UpdateLabels();
    QString GetNewColor(QString kolor);
    QString GetNewBackgroundUrl(QString kolor);

    QTabWidget *TabWidget1;
    QWidget *tab1, *tab2;
    QGroupBox *GroupBox1, *GroupBox2, *GroupBox3;

    QPushButton *changeDictionaryUrl;
    QPushButton *changeAudioUrl;
    QPushButton *changePlayerUrl;
    QPushButton *changeFont1;
    QPushButton *changeFont2;
    QPushButton *changeFont3;
    QPushButton *changeFont4;
    QPushButton *changeBckgrndUrl;
    QLabel *textLabel1;
    QLabel *textLabel2;
    QLabel *textLabel3;
    QLabel *textLabel4;
    QLabel *textLabel5;
    QCheckBox *checkBox1;
//    QTextEdit *exampleLabel1;
    QTextBrowser *exampleLabel1;

    kydpConfig *config;

    QPushButton *cancel;
    QPushButton *ok;
    QPushButton *defaults;

    QLineEdit *dictionaryUrl;
    QLineEdit *audioUrl;
    QLineEdit *playerUrl;

    QString cFontKolor1;
    QString cFontKolor2;
    QString cFontKolor3;
    QString cFontKolor4;
    QString cBckgrnd;

private slots:
    void NewDictUrl();
    void NewAudioUrl();
    void NewFontColor1();
    void NewFontColor2();
    void NewFontColor3();
    void NewFontColor4();
    void NewBckgrndUrl();
    void NewCheck(int state);
    void WriteDefaults();
    void NewPlayerUrl();

protected:
    void showEvent ( QShowEvent * );

};

#endif // YDPCONFIGURE_H
