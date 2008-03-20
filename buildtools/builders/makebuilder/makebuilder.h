/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>
    Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>

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
#ifndef MAKEBUILDER_H
#define MAKEBUILDER_H

#include <iplugin.h>
#include "imakebuilder.h"
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QVariant>


class QStringList;
class MakeOutputModel;
class KDialog;
class QSignalMapper;
class QString;
class QStandardItem;
class KUrl;
class MakeOutputDelegate;

namespace KDevelop {
class ProjectBaseItem;
class CommandExecutor;
}

/**
@author Roberto Raggi
*/
class MakeBuilder: public KDevelop::IPlugin, public IMakeBuilder
{
    Q_OBJECT
    Q_INTERFACES( IMakeBuilder )
    Q_INTERFACES( KDevelop::IProjectBuilder )
private:
    enum CommandType
    {
        BuildCommand,
        CleanCommand,
        CustomTargetCommand,
        InstallCommand
    };
public:
    explicit MakeBuilder(QObject *parent = 0, const QVariantList &args = QVariantList());
    virtual ~MakeBuilder();

    /**
     * If argument is ProjectItem, invoke "make" in IBuildSystemManager::buildDirectory(), with
     * specified target in project setting.
     *
     * If argument is ProjectTargetItem, invoke "make" with targetname QStandardItem::text(). In this case,
     * it tries its best to fetch ProjectItem, which is the argument of IBuildSystemManager::buildDirectory()
     * If it fails to fetch ProjectItem, the top build directory is defaulted to project directory.
     * Based on top build directory, the actual build_dir is computed and handed to outputview
     *
     * If argument is ProjectBuildFolderItem, invoke "make" with specified target in project setting.
     * To determine the build directory, first calculates rel_dir between
     * top_project_dir(ProjectItem::url()) and ProjectBuildFolderItem::url().
     * Then invokes make in top_build_dir/rel_dir.
     * If this fails to fetch top_build_dir, just invoke "make" in ProjectBuildFolderItem::url().
     *
     * @TODO: Work on any project item, for fileitems you may find a target.
     */
    virtual bool build(KDevelop::ProjectBaseItem *dom);
    virtual bool clean(KDevelop::ProjectBaseItem *dom);
    virtual bool install(KDevelop::ProjectBaseItem *dom);

    virtual bool executeMakeTarget(KDevelop::ProjectBaseItem* item, const QString& targetname );
    bool runMake( KDevelop::ProjectBaseItem*, CommandType, const QString& = QString() );

Q_SIGNALS:
    void built( KDevelop::ProjectBaseItem* );
    void failed( KDevelop::ProjectBaseItem* );
    void installed(KDevelop::ProjectBaseItem*);
    void cleaned(KDevelop::ProjectBaseItem*);
    void makeTargetBuilt( KDevelop::ProjectBaseItem* item, const QString& targetname );

private Q_SLOTS:
    void commandFinished(int id);
    void commandFailed(int id);
    void cleanupModel( int id );

private:
    QStringList computeBuildCommand(KDevelop::ProjectBaseItem *item, const QString& = QString() );
    KUrl computeBuildDir( KDevelop::ProjectBaseItem* item );
    QMap<QString, QString> environmentVars( KDevelop::ProjectBaseItem* item );

private:
    QMap< KDevelop::ProjectBaseItem*, int > m_ids;
    QMap< int, CommandType > m_commandTypes;
    QMap< int, KDevelop::CommandExecutor* > m_commands;
    QMap< int, KDevelop::ProjectBaseItem* > m_items;
    QMap< int, MakeOutputModel* > m_models;
    QMap< int, MakeOutputDelegate* > m_delegates;
    QMap< int, QString> m_customTargets;
    QSignalMapper* m_errorMapper;
    QSignalMapper* m_successMapper;
};

#endif // KDEVMAKEBUILDER_H

