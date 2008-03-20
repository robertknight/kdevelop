/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                     *
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

#ifndef MAKEOUTPUTMODEL_H
#define MAKEOUTPUTMODEL_H

#include <QtGui/QStandardItemModel>
#include <ioutputviewmodel.h>

class QObject;
class MakeActionFilter;
class ErrorFilter;
class MakeBuilder;

class MakeOutputModel : public QStandardItemModel, public KDevelop::IOutputViewModel
{
    Q_OBJECT
public:
    enum OutputItemType{
        MakeError,
        MakeWarning,
        MakeBuilt
    };
    explicit MakeOutputModel( MakeBuilder *builder, QObject* parent = 0 );

    // IOutputViewModel interfaces
    void activate( const QModelIndex& index );
    QModelIndex nextHighlightIndex( const QModelIndex &current );
    QModelIndex previousHighlightIndex( const QModelIndex &current );

public slots:
    void addStandardError( const QStringList& );
    void addStandardOutput( const QStringList& );

private:
    MakeActionFilter* actionFilter;
    ErrorFilter* errorFilter;
    MakeBuilder *m_builder;
};

#endif

