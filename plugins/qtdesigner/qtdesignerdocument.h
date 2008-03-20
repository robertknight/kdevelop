/***************************************************************************
 *   This file is part of KDevelop                                         *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>                           *
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

#ifndef QTDESIGNERDOCUMENT_H
#define QTDESIGNERDOCUMENT_H

#include <sublime/urldocument.h>
#include <idocument.h>

#include <QtCore/QList>

namespace KDevelop
{
    class ICore;
}

class QtDesignerPlugin;
class QDesignerFormWindowInterface;
class QMdiArea;

class QtDesignerDocument : public Sublime::UrlDocument, public KDevelop::IDocument
{
    Q_OBJECT
public:
    QtDesignerDocument( const KUrl&, KDevelop::ICore* );

    KUrl url() const;

    virtual KSharedPtr<KMimeType> mimeType() const;
    virtual KParts::Part* partForView(QWidget*) const;
    virtual KTextEditor::Document* textDocument() const;
    virtual bool save(KDevelop::IDocument::DocumentSaveMode);
    virtual void reload();
    virtual void close();
    virtual bool isActive() const;
    virtual DocumentState state() const;
    virtual void setCursorPosition(const KTextEditor::Cursor&);
    virtual void activate(Sublime::View*);
    virtual KTextEditor::Cursor cursorPosition() const;
    void setDesignerPlugin(QtDesignerPlugin*);

private slots:
    void formChanged();
protected:
        virtual QWidget *createViewWidget(QWidget *parent = 0);
private:
    QtDesignerPlugin* m_designerPlugin;
    KUrl m_url;
    QList<QDesignerFormWindowInterface*> m_forms;
    QList<QMdiArea*> m_areas;
    KDevelop::IDocument::DocumentState m_state;
};

#endif

