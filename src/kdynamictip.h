/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qwidget.h>
#include <qtooltip.h>

class DynamicTip : public QToolTip
{
public:
    DynamicTip( QWidget * parent );
    virtual ~DynamicTip();
    QString tekst;

protected:
    void maybeTip( const QPoint & );
};

