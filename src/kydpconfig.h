/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef KYDPCONFIG_H
#define KYDPCONFIG_H

#include <qstring.h>

class kydpConfig
{
public:
	kydpConfig();
	~kydpConfig();
	
	QString cdPath;
	QString topPath;
	QString player;
	QString kFontKolor1;
	QString kFontKolor2;
	QString kFontKolor3;
	QString kFontKolor4;
	QString kBckgrndPix;
	QString indexFName;
	QString dataFName;
	bool	toPolish, useEnglish;
	int kGeometryX, kGeometryY, kGeometryW, kGeometryH;
	int spH1, spH2;

	void load(void);
	void save(void);
	void setDefaultConfiguration(void);

private:
	QString cfgname;

	void readYDPConfig(void);
	void updateFName(void);
};

#endif // KYDPCONFIG_H
