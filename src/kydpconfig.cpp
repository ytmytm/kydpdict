/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <qfile.h>

#include "kydpconfig.h"

kydpConfig::kydpConfig()
{
	cfgname = getenv("HOME");
	cfgname += "/";
	cfgname += getenv("CONFIG_DIR");
	cfgname += "/";
	cfgname += ".kydpdict";
}

kydpConfig::~kydpConfig()
{

}

void kydpConfig::setDefaultConfiguration(void)
{
	topPath =  "/usr/share/ydpdict";
	cdPath = "/mnt/cdrom";
	cd2Path = "/mnt/cdrom";
	tipsPath = KYDPDATADIR ;
	toPolish = true;
	language = LANG_ENGLISH;
	kGeometryX = 1;
	kGeometryY = 1;
	kGeometryW = 500;
	kGeometryH = 300;
	kFontKolor1 = "#C00000";
	kFontKolor2 = "#0000ff";
	kFontKolor3 = "#736324";
	kFontKolor4 = "#000000";
	kBckgrndPix = "NoBackground";
	player = "/usr/bin/play";
	spH1 = 200;
	spH2 = 300;
	spV1 = 30;
	spV2 = 236;
	clipTracking = true;
	italicFont = false;
	toolTips = true;
	autoPlay= false;
}

void kydpConfig::load(void)
{
	QFile config;
	QString line, value;
	int pos;

	setDefaultConfiguration();

	config.setName(cfgname);
	if (config.exists()) {
		// read configuration strings
		config.open(IO_ReadOnly);
		// parse the file, ignore lines starting with '#'
		while (!config.atEnd()) {
			config.readLine(line,128);
			line = line.simplifyWhiteSpace();
			line.truncate(line.find('#', 0));

			if (!line.isEmpty()) {
				pos = line.find("dictpath");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					topPath = value.stripWhiteSpace();
				}
				pos = line.find("cdpath");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					cdPath = value.stripWhiteSpace();
				}
				pos = line.find("cd2path");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					cd2Path = value.stripWhiteSpace();
				}
				pos = line.find("tipspath");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					tipsPath = value.stripWhiteSpace();
				}
				pos = line.find("topolish");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					toPolish = ( value.toInt() != 0);
				}
				pos = line.find("language");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					language = value.toInt();
				}
				pos=line.find("geometry_x");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					kGeometryX = value.toInt();
				}
				pos=line.find("geometry_y");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					kGeometryY = value.toInt();
				}
				pos=line.find("geometry_w");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					kGeometryW = value.toInt();
				}
				pos=line.find("geometry_h");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					kGeometryH = value.toInt();
				}
				pos=line.find("translation_text");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					kFontKolor1 = "#" + value.stripWhiteSpace();
				}
				pos=line.find("describe_text");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					kFontKolor2 = "#" + value.stripWhiteSpace();
				}
				pos=line.find("example_text");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					kFontKolor3 = "#" + value.stripWhiteSpace();
				}
				pos=line.find("plain_text");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					kFontKolor4 = "#" + value.stripWhiteSpace();
				}
				pos=line.find("bckgrnd_pix");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					kBckgrndPix = value.stripWhiteSpace();
				}
				pos=line.find("player");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					player = value.stripWhiteSpace();
				}
				pos=line.find("splitterH_1");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					spH1 = value.toInt();
				}
				pos=line.find("splitterH_2");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					spH2 = value.toInt();
				}
				pos=line.find("splitterV_1");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					spV1 = value.toInt();
				}
				pos=line.find("splitterV_2");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					spV2 = value.toInt();
				}
				pos = line.find("clipTracking");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					clipTracking = ( value.toInt() != 0);
				}
				pos = line.find("italicFont");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					italicFont = ( value.toInt() != 0);
				}
				pos = line.find("toolTips");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					toolTips = ( value.toInt() != 0);
				}
				pos = line.find("autoPlay");
				if (pos==0) {
					pos = line.find('=');
					value = line.mid(pos+1);
					autoPlay = ( value.toInt() != 0);
				}
			}
		}
	config.close();
	} else {
		// try to override defaults by /etc/ydpconfig.conf
		readYDPConfig();
		// create file and write defaults
		save();
	}
	
	updateFName();
}

void kydpConfig::save(void)
{
	QFile config;
	QString line,tmp;

	updateFName();

	config.setName(cfgname);
	config.open(IO_WriteOnly);
	line = 	"# This is Kydpdict configuration file, edit carefully.\n\n";
	line += "dictpath\t=\t";
	line += topPath;
	line += "\ncdpath\t=\t";
	line += cdPath;
	line += "\ncd2path\t=\t";
	line += cd2Path;
	line += "\ntipspath\t=\t";
	line += tipsPath;
	line += "\ntopolish\t=\t";
	if (toPolish) { line+="1"; } else { line+="0"; };

	line += "\nlanguage\t=\t";
	tmp.sprintf("%d",language);
	line += tmp;

	line += "\ngeometry_x\t=\t";
	tmp.sprintf("%d",kGeometryX);
	line += tmp;

	line += "\ngeometry_y\t=\t";
	tmp.sprintf("%d",kGeometryY);
	line += tmp;

	line += "\ngeometry_w\t=\t";
	tmp.sprintf("%d",kGeometryW);
	line += tmp;

	line += "\ngeometry_h\t=\t";
	tmp.sprintf("%d",kGeometryH);
	line += tmp;

	line += "\ntranslation_text\t=\t";
	line += kFontKolor1.right(6);
	line += "\ndescribe_text\t=\t";
	line += kFontKolor2.right(6);
	line += "\nexample_text\t=\t";
	line += kFontKolor3.right(6);
	line += "\nplain_text\t=\t";
	line += kFontKolor4.right(6);
	line += "\nbckgrnd_pix\t=\t";
	line += kBckgrndPix;

	line += "\nplayer\t=\t";
	line += player;
	
	line += "\nsplitterH_1\t=\t";
	tmp.sprintf("%d",spH1);
	line += tmp;
	line += "\nsplitterH_2\t=\t";
	tmp.sprintf("%d",spH2);
	line += tmp;

	line += "\nsplitterV_1\t=\t";
	tmp.sprintf("%d",spV1);
	line += tmp;
	line += "\nsplitterV_2\t=\t";
	tmp.sprintf("%d",spV2);
	line += tmp;

	line += "\nclipTracking\t=\t";
	if (clipTracking) { line+="1"; } else { line+="0"; };
	line += "\nitalicFont\t=\t";
	if (italicFont) { line+="1"; } else { line+="0"; };
	line += "\ntoolTips\t=\t";
	if (toolTips) { line+="1"; } else { line+="0"; };
	line += "\nautoPlay\t=\t";
	if (autoPlay) { line+="1"; } else { line+="0"; };

	line += "\n";

	config.writeBlock(line.latin1(), line.length());
	config.close();
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
