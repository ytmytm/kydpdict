/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ydpconverter.h"

#include <qstring.h>
#include <qcstring.h>

/////////////////////
// utf16 <-> anything convertion stuff below
//

ydpConverter::ydpConverter(void) {
}

ydpConverter::~ydpConverter() {
}

//char ydpConverter::toLower(const char c) {
//	return c;
//}

QString ydpConverter::toUnicode(const char *input) {
	return QString(input);
}

QCString ydpConverter::fromUnicode(QString input) {
	return input.ascii();
}

QString ydpConverter::convertChunk(const char *input, int size, bool unicodeFont) {
	return QString(input);
}
