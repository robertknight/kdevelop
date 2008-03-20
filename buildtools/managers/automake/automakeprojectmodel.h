/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>

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
#ifndef AUTOMAKEPROJECTMODEL_H
#define AUTOMAKEPROJECTMODEL_H

#include <QList>
#include <QIcon>
#include "kdevprojectmodel.h"
#include "makefileinterface.h"

namespace AutoTools { class ProjectAST; }

class AutoMakeDirItem : public KDevelop::ProjectFolderItem
{
public:
    explicit AutoMakeDirItem( const KUrl& url, QStandardItem* parent = 0 );
    virtual ~AutoMakeDirItem();

    virtual KDevelop::ProjectFolderItem* folder() const
    {
        return const_cast<AutoMakeDirItem*>( this );
    }

    void parseInfoFromAst( AutoTools::ProjectAST* ) {}

};

class AutoMakeFileItem : public KDevelop::ProjectFileItem
{
public:
    explicit AutoMakeFileItem( const KUrl& url, QStandardItem* parent = 0 );
    virtual ~AutoMakeFileItem();

    virtual KDevelop::ProjectFileItem* file() const
    {
        return const_cast<AutoMakeFileItem*>( this );
    }
};

class AutoMakeTargetItem : public KDevelop::ProjectTargetItem
{
public:
    explicit AutoMakeTargetItem( const TargetInfo& target, QStandardItem* parent = 0 );

    virtual ~AutoMakeTargetItem();

    virtual KDevelop::ProjectTargetItem* target() const
    {
        return const_cast<AutoMakeTargetItem*>( this );
    }

    virtual const KUrl::List& includeDirectories() const { return m_includes; }
    virtual const QHash<QString, QString>& environment() const { return m_environment; }
    virtual const QList<QPair<QString, QString> >& defines() const { return m_defines; }
    void parseInfoFromAst( AutoTools::ProjectAST* ) {}


private:
    TargetInfo m_target;
    KUrl::List m_includes;
    QHash<QString, QString> m_environment;
    QList<QPair<QString, QString> > m_defines;
};

class AutoMakeGenericItem : public KDevelop::ProjectItem
{
public:
    explicit AutoMakeGenericItem( const QString& target, QStandardItem* parent = 0 );
    virtual ~AutoMakeGenericItem();

};

#endif // AUTOMAKEPROJECTMODEL_H

