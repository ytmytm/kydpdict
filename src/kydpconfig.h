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

#include <qfont.h>

enum { LANG_ENGLISH, LANG_DEUTSCH, LANG_LAST };
enum { ENGINE_SAP, ENGINE_PWN, ENGINE_YDP, ENGINE_LAST };

class kydpConfig
{
public:
	kydpConfig();
	~kydpConfig();
	
	QString topPath;
	QString cdPath, cd2Path;
	QString player;
	QString kFontKolor1;
	QString kFontKolor2;
	QString kFontKolor3;
	QString kFontKolor4;
	QString kBckgrndPix;
	QString kBckgrndKol;
	QString indexFName;
	QString dataFName;
	QFont   fontTransFont;
	bool	toPolish;
	bool	toolBarVisible;
	bool	unicodeFont;
	int language;
	int engine;
	int kGeometryX, kGeometryY, kGeometryW, kGeometryH;
	bool kMaximized;
	int spH1, spH2, spV1, spV2;
	bool clipTracking, italicFont, toolTips, autoPlay, ignoreOwnSelection, dock, setFocusOnSelf;
	int fuzzyDistance;

	void load(void);
	void save(void);
	void setDefaultConfiguration(void);

private:
	QString cfgname;

	void readYDPConfig(void);
	QString fixColour(const QString);
};

#endif // KYDPCONFIG_H
