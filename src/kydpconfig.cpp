/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <qdir.h>
#include <qfile.h>

#include "kydpconfig.h"

// for those qmake stubborns...
#ifndef KYDPDATADIR
# define KYDPDATADIR "/usr/share/kydpdict/"
#endif

kydpConfig::kydpConfig()
{
    QDir dir;
    QFile f;

	cfgname = getenv("HOME_ETC");
	if (cfgname == "") {
	    cfgname = getenv("HOME");
	    cfgname += "/";
	    cfgname += getenv("CONFIG_DIR");
	}
	cfgname += "/.kydpdict";

	// get rid of old config file (otherwise stuff would go to ~/.qt
        f.setName(cfgname);
        if (f.exists())
	    f.remove();

	// create the directory if it doesn't exist
	dir.setPath(cfgname);
	if (!dir.exists()) {
	    dir.mkdir(cfgname);
	    topPath="xyz";	// notify loader to try import config
	}
	load();
}

kydpConfig::~kydpConfig()
{

}


void kydpConfig::load(void)
{
	QString transFont;

	if (topPath=="xyz") {
	    topPath = KYDPDATADIR;
	    cdPath = "/mnt/cdrom";
	    player = "/usr/bin/play";
	    readYDPConfig();
	}

	QSettings settings;
	settings.setPath("ytm","kydpdict");
	settings.insertSearchPath(QSettings::Unix, cfgname);

	settings.beginGroup("/kydpdict");
	settings.beginGroup("/paths");
	topPath	= settings.readEntry("/dictpath", topPath);
	cdPath	= settings.readEntry("/cd1path", cdPath);
	cd2Path = settings.readEntry("/cd2path", "/mnt/cdrom");
	player	= settings.readEntry("/player", player);
	settings.endGroup();
	settings.beginGroup("/language");
	toPolish= settings.readBoolEntry("/toPolish", TRUE );
	language= settings.readNumEntry("/language", LANG_ENGLISH );
	settings.endGroup();
	settings.beginGroup("/visual");
	kGeometryX = settings.readNumEntry("/x", 1);
	kGeometryY = settings.readNumEntry("/y", 1);
	kGeometryW = settings.readNumEntry("/w", 500);
	kGeometryH = settings.readNumEntry("/h", 300);
	kMaximized = settings.readBoolEntry("/maximized", FALSE);
	kFontKolor1 = fixColour(settings.readEntry("/translation_text", "#C00000"));
	kFontKolor2 = fixColour(settings.readEntry("/describe_text", "#0000FF"));
	kFontKolor3 = fixColour(settings.readEntry("/example_text", "#736324"));
	kFontKolor4 = fixColour(settings.readEntry("/plain_text", "#000001"));
	kBckgrndPix = settings.readEntry("/background", "NoBackground");
	kBckgrndKol = fixColour(settings.readEntry("/background_colour", "#FFFFFF"));
	spH1 = settings.readNumEntry("/splitterH_1", 200);
	spH2 = settings.readNumEntry("/splitterH_2", 300);
	spV1 = settings.readNumEntry("/splitterV_1", 30);
	spV2 = settings.readNumEntry("/splitterV_2", 236);
	transFont = settings.readEntry("/translation_font", "Helvetica");
	toolBarVisible = settings.readBoolEntry("/toolbar_visible", TRUE);
	settings.endGroup();
	settings.beginGroup("/behavior");
	clipTracking = settings.readBoolEntry("/clip_tracking", TRUE);
	italicFont   = settings.readBoolEntry("/italic_font", FALSE);
	toolTips     = settings.readBoolEntry("/tool_tips", TRUE);
	autoPlay     = settings.readBoolEntry("/auto_play", FALSE);
	ignoreOwnSelection = settings.readBoolEntry("/ignore_own_selection", FALSE);
	dock = settings.readBoolEntry("/dock", TRUE);
	setFocusOnSelf = settings.readBoolEntry("/set_focus_on_self", TRUE);
	settings.endGroup();
	settings.endGroup();

	fontTransFont.fromString(transFont);
	transFont = fontTransFont.toString();

	updateFName();
}

void kydpConfig::save(void)
{
	updateFName();

	QSettings settings;
	settings.setPath("ytm","kydpdict");
	settings.insertSearchPath(QSettings::Unix, cfgname);
	settings.beginGroup("/kydpdict");
	settings.beginGroup("/paths");
	settings.writeEntry("/dictpath", topPath);
	settings.writeEntry("/cd1path", cdPath);
	settings.writeEntry("/cd2path", cd2Path);
	settings.writeEntry("/player", player);
	settings.endGroup();
	settings.beginGroup("/language");
	settings.writeEntry("/toPolish", toPolish);
	settings.writeEntry("/language", language);
	settings.endGroup();
	settings.beginGroup("/visual");
	settings.writeEntry("/x", kGeometryX);
	settings.writeEntry("/y", kGeometryY);
	settings.writeEntry("/w", kGeometryW);
	settings.writeEntry("/h", kGeometryH);
	settings.writeEntry("/maximized", kMaximized);
	settings.writeEntry("/translation_text", fixColour(kFontKolor1));
	settings.writeEntry("/describe_text", fixColour(kFontKolor2));
	settings.writeEntry("/example_text", fixColour(kFontKolor3));
	settings.writeEntry("/plain_text", fixColour(kFontKolor4));
	settings.writeEntry("/background", kBckgrndPix);
	settings.writeEntry("/background_colour", fixColour(kBckgrndKol));
	settings.writeEntry("/splitterH_1", spH1);
	settings.writeEntry("/splitterH_2", spH2);
	settings.writeEntry("/splitterV_1", spV1);
	settings.writeEntry("/splitterV_2", spV2);
	settings.writeEntry("/translation_font", fontTransFont.toString());
	settings.writeEntry("/toolbar_visible", toolBarVisible);
	settings.endGroup();
	settings.beginGroup("/behavior");
	settings.writeEntry("/clip_tracking", clipTracking);
	settings.writeEntry("/italic_font", italicFont);
	settings.writeEntry("/tool_tips", toolTips);
	settings.writeEntry("/auto_play", autoPlay);
	settings.writeEntry("/ignore_own_selection", ignoreOwnSelection);
	settings.writeEntry("/dock", dock);
	settings.writeEntry("/set_focus_on_self", setFocusOnSelf);
	settings.endGroup();
	settings.endGroup();
}

void kydpConfig::readYDPConfig(void)
{
	QFile fd;
	QString line, value;
	int pos;

	fd.setName("/etc/ydpdict.conf");
	if (!fd.exists())
		return;
	fd.open(IO_ReadOnly);
	while (!fd.atEnd()) {
		fd.readLine(line,128);
		line = line.simplifyWhiteSpace();
		line.truncate(line.find('#',0));
		if (!line.isEmpty()) {
			pos = line.find("CDPath");
			if (pos>=0) {
				value = line.mid(pos+6);
				cdPath = value.stripWhiteSpace();
			} else {
				pos = line.find("Path");
				if (pos>=0) {
					value = line.mid(pos+4);
					topPath = value.stripWhiteSpace();
				}
			}
			pos = line.find("Player");
			if (pos>=0) {
				value = line.mid(pos+6);
				player = value.stripWhiteSpace();
			}
		}
	}
	fd.close();
}

void kydpConfig::updateFName(void)
{
	switch (language) {
		case LANG_ENGLISH:
		  indexFName= toPolish ? "dict100.idx" : "dict101.idx";
		  dataFName = toPolish ? "dict100.dat" : "dict101.dat";
		  break;
		case LANG_DEUTSCH:
		  indexFName= toPolish ? "dict200.idx" : "dict201.idx";
		  dataFName = toPolish ? "dict200.dat" : "dict201.dat";
		  break;
	}
}

// this is here due to fact that if link's colour is set to black, Qt ignores that
// and uses default blue instead
QString kydpConfig::fixColour(const QString colour)
{
    if (colour == "#000000") {
	return "#000001";
    }
    else
	return colour;
}
