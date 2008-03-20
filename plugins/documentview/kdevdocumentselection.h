/* This file is part of KDevelop
 Copyright 2005 Adam Treat <treat@kde.org>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*/

#ifndef KDEVDOCUMENTSELECTION_H
#define KDEVDOCUMENTSELECTION_H

#include <QtGui/QItemSelectionModel>

class KDevDocumentModel;

class KDevDocumentSelection: public QItemSelectionModel
{
    Q_OBJECT
public:
    KDevDocumentSelection( KDevDocumentModel * model );
    virtual ~KDevDocumentSelection();

public slots:
    virtual void select( const QModelIndex & index,
                         QItemSelectionModel::SelectionFlags command );
    virtual void select( const QItemSelection & selection,
                         QItemSelectionModel::SelectionFlags command );
};

#endif // KDEVDOCUMENTSELECTION_H

