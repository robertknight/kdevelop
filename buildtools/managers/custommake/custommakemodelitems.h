/* KDevelop Custom Makefile Support
 *
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef CUSTOMMAKEMODELITEMS_H
#define CUSTOMMAKEMODELITEMS_H

#include <projectmodel.h>
namespace KDevelop
{
class IProject;
}

class KUrl::List;
template<typename T1,typename T2> class QPair;
// template<typename T1,typename T2> class QHash;
#include <QHash>
template<typename T1> class QList;
class QString;
class CustomMakeTreeSynchronizer;
class CustomMakeManager;

class CustomMakeTargetItem : public KDevelop::ProjectTargetItem
{
public:
    CustomMakeTargetItem( KDevelop::IProject *project, const QString &name, QStandardItem *parent=NULL );

    virtual KUrl::List includeDirectories() const;
    virtual QHash<QString, QString> environment() const;
    virtual QList<QPair<QString, QString> > defines() const;

private:
    KUrl::List m_includeDirs;
    QHash<QString, QString> m_envs;
    QList<QPair <QString, QString> > m_defines;

};

class CustomMakeFolderItem : public KDevelop::ProjectFolderItem
{
public:
    CustomMakeFolderItem( CustomMakeManager*, KDevelop::IProject* project, const KUrl& , QStandardItem *parent = 0 );
    virtual ~CustomMakeFolderItem();

    // watcher is reentrant. Only project parsing thread can call the methods of Watcher.
    // No locking is required because parsing thread is the only one to access this.
    CustomMakeTreeSynchronizer* fsWatcher();

private:
    CustomMakeTreeSynchronizer *m_watcher;
};

#endif
