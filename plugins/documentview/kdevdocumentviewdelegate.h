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

#ifndef KDEVDOCUMENTVIEWDELEGATE_H
#define KDEVDOCUMENTVIEWDELEGATE_H

#include <QtGui/QItemDelegate>
class QTreeView;

class KDevDocumentViewDelegate: public QItemDelegate
{
    Q_OBJECT
public:
    explicit KDevDocumentViewDelegate( QTreeView *view, QObject *parent = 0 );
    virtual ~KDevDocumentViewDelegate();

    virtual void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    virtual QSize sizeHint( const QStyleOptionViewItem &opt, const QModelIndex &index ) const;

private:
    QTreeView *m_view;
};

#endif // KDEVCLASSVIEWDELEGATE_H

