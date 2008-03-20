/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "grepoutputdelegate.h"
#include "grepoutputmodel.h"
#include <QtGui/QPainter>
#include <QtCore/QModelIndex>
#include <kdebug.h>

GrepOutputDelegate::GrepOutputDelegate( QObject* parent )
    : QItemDelegate(parent), textBrush( KColorScheme::View, KColorScheme::LinkText ),
      fileBrush( KColorScheme::View, KColorScheme::InactiveText )
{
}

void GrepOutputDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QStyleOptionViewItem opt = option;
    QVariant status = index.data(Qt::UserRole+1);
    if( status.isValid() && status.toInt() == GrepOutputModel::Text )
    {
        opt.palette.setBrush( QPalette::Text, textBrush.brush( option.palette ) );
    }else if( status.isValid() && status.toInt() == GrepOutputModel::File )
    {
        opt.palette.setBrush( QPalette::Text, fileBrush.brush( option.palette ) );
    }
    QItemDelegate::paint(painter, opt, index);
}

