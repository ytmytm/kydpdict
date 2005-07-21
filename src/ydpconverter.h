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
//			virtual char toLower(const char c);
			virtual QString toUnicode(const char *input);
			virtual QCString fromUnicode(QString input);
			virtual	QString convertChunk(const char *inp, int size = 0, bool unicodeFont = false);

	};

#endif
