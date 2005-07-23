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

char ydpConverter::toLower(const char c) {
	return c;
}

char *ydpConverter::toLocal(const char *input) {
	return (char*)input;
}

QString ydpConverter::toUnicode(const char *input) {
	return QString(input);
}

QCString ydpConverter::fromUnicode(QString input) {
	return input.ascii();
}

QString ydpConverter::convertChunk(const char *input, int size, bool unicodeFont) {
	return QString(input);
}

int ydpConverter::charIndex(const char c) {
	return c;
}

int ydpConverter::localeCompare(const char *w1, const char *w2) {
	int i = 0;
	int len1 = strlen(w1);
	int len2 = strlen(w2);
	int c1, c2;
	for (; ((i<len1) && (i<len2)); i++) {
		c1 = charIndex(w1[i]); c2 = charIndex(w2[i]);
		if (c1!=c2) {
		    return c1-c2;
		}
	}
	// identical substrings so longer is bigger
	return len1-len2;
}

int ydpConverter::scoreWord(const char *w1, const char *w2) {
	int i = 0;
	int len1 = strlen(w1);
	int len2 = strlen(w2);
	for (; ((i<len1) && (i<len2)); i++)
		if (w1[i] != w2[i])
			break;
	return i;
}
