/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <qapplication.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include "kydpdict.h"

// for those qmake morons...
#ifndef LOCALEDIR
# define LOCALEDIR "/usr/share/kydpdict"
#endif

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

	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("ISO8859-2"));

	Kydpdict *kydpdict = new Kydpdict ();
	a->setMainWidget (kydpdict);

	a->exec ();
	kydpdict->flushConfig();
	return 0;
}
