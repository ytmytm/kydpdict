/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qapplication.h>
#include <qcursor.h>

#include "kdynamictip.h"

DynamicTip::DynamicTip( QWidget * parent )
    : QToolTip( parent )
{
    // no explicit initialization needed
}


void DynamicTip::maybeTip( const QPoint &pos )
{
    if( tekst.isEmpty())
        return;

    QRect r = QRect (pos, pos);

    tip( r, tekst.fromLocal8Bit(tekst));
}

DynamicTip::~DynamicTip()
{
    delete tekst;
}
