/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _YDPCONVERTER_H
#define _YDPCONVERTER_H

class QString;
class QCString;

	class ydpConverter {
		public:
			ydpConverter(void);
			virtual ~ydpConverter();
			// dictionary-dependent convertion routines

			// convert a letter to lowercase
			virtual char toLower(const char c);
			// strip input (from dict. index, native) from any non-letter characters
			// return lowercase
			virtual	char *toLocal(const char *input);
			// convert input (dict. index, native) to unicode, replace accents, remove tags etc.
			virtual QString toUnicode(const char *input);
			// convert input (unicode) to dict. index native encoding
			virtual QCString fromUnicode(QString input);
			// same as toUnicode but for dictionary data, not index
			virtual	QString convertChunk(const char *inp, int size = 0, bool unicodeFont = false);
			// return character index in dict. index native encoding alphabet
			virtual int charIndex(const char c);
			// number of letters common to both words, starting from left
			int scoreWord(const char *w1, const char *w2);
			// same as strcoll but dict. index native, not related to current locale
			virtual int localeCompare(const char *w1, const char *w2);
	};

#endif
