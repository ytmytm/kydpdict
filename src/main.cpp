/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <qfont.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include "kydpdict.h"

QApplication *a;

int main (int argc, char *argv[])
{
	QString locname;

	a = new QApplication(argc, argv, "Kydpdict");

	QTranslator myapp( 0 );

        if (!myapp.load( QString( "kydpdict_" ) + QTextCodec::locale(), LOCALEDIR ) ) {
	    locname = getenv("HOME");
    	    if (!myapp.load( QString( "kydpdict_" ) + QTextCodec::locale(), locname ) ) {
		locname += "/";
		locname += getenv("CONFIG_DIR");
    		if (!myapp.load( QString( "kydpdict_" ) + QTextCodec::locale(), locname ) ) {
		    myapp.load( QString( "kydpdict_" ) + QTextCodec::locale(), "." );
		}
	    }
	}
        a->installTranslator( &myapp );

	Kydpdict *kydpdict = new Kydpdict ();
	a->setMainWidget (kydpdict);

	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("ISO8859-2"));

	a->exec ();
	kydpdict->flushConfig();
	return 0;
}
