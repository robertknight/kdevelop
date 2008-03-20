/* KDevelop Automake Support
 *
 * Copyright (C)  2005  Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "automakeimporter.h"

#include <QList>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include "kdevproject.h"
#include "kdevprojectmodel.h"

#include "makefileinterface.h"

K_PLUGIN_FACTORY(AutotoolsSupportFactory, registerPlugin<AutoMakeImporter>(); )
K_EXPORT_PLUGIN(AutotoolsSupportFactory("kdevautomakeimporter"))

AutoMakeImporter::AutoMakeImporter( QObject* parent,
                                    const QVariantList& )
: KDevelop::BuildManager( AutotoolsSupportFactory::componentData(), parent ), m_rootItem(0L)
{
    m_project = qobject_cast<KDevelop::Project*>( parent );
    Q_ASSERT( m_project );
    m_interface = new MakefileInterface( this );
}

AutoMakeImporter::~AutoMakeImporter()
{
	//delete m_rootItem;
}

KDevelop::Project* AutoMakeImporter::project() const
{
    return m_project;
}

KUrl AutoMakeImporter::buildDirectory() const
{
     return project()->folder();
}

QList<KDevelop::ProjectFolderItem*> AutoMakeImporter::parse( KDevelop::ProjectFolderItem* dom )
{
    Q_UNUSED( dom );
    return QList<KDevelop::ProjectFolderItem*>();
}

KDevelop::ProjectItem* AutoMakeImporter::import( KDevelop::ProjectModel* model,
                                           const KUrl& fileName )
{
    Q_UNUSED( model );
    m_rootItem = new AutoMakeDirItem( fileName, 0 );
    m_interface->setProjectRoot( fileName );
    bool parsedCorrectly = m_interface->parse( fileName );

    if ( parsedCorrectly )
    {
        QStringList topLevelSubdirs = m_interface->topSubDirs();
        foreach ( QString dir, topLevelSubdirs )
        {
            QString fullPath = m_interface->projectRoot();
            fullPath = fullPath + QDir::separator() + dir;
            createProjectItems( fullPath, m_rootItem );
        }
    }
    return m_rootItem;

}

KUrl AutoMakeImporter::findMakefile( KDevelop::ProjectFolderItem* dom ) const
{
    Q_UNUSED( dom );
    return KUrl();
}

KUrl::List AutoMakeImporter::findMakefiles( KDevelop::ProjectFolderItem* dom ) const
{
    Q_UNUSED( dom );
    return KUrl::List();
}

void AutoMakeImporter::createProjectItems( const KUrl& folder, KDevelop::ProjectItem* rootItem )
{
    //first look for the subdirs
    //recursively descend into any other subdirs. when finished look for targets.
    //for each target, add the files for the target to the target
    AutoMakeDirItem* folderItem = new AutoMakeDirItem( folder, rootItem );
    rootItem->add( folderItem );

    QStringList subdirs = m_interface->subdirsFor( folder );

    if ( !subdirs.isEmpty() )
    {
        foreach( QString dir, subdirs )
        {
            QString fullPath = folder.path();
            fullPath = fullPath + QDir::separator() + dir;
            createProjectItems( fullPath, folderItem );
        }
    }

    QList<TargetInfo> targets = m_interface->targetsForFolder( folder );
    KDevelop::ProjectItem* dotDesktopTarget = 0;
    KDevelop::ProjectItem* xmlGuiTarget = 0;
    KDevelop::ProjectItem* notInstalledHeaders = 0;
    KDevelop::ProjectItem* installedHeaders = 0;

    foreach( TargetInfo target, targets )
    {
        switch( target.type )
        {
        case AutoMake::Data:
            if ( target.name.contains( ".desktop" ) )
            {
                if ( dotDesktopTarget == 0 )
                    dotDesktopTarget = new KDevelop::ProjectItem( i18n( "freedesktop.org Desktop Entry Files" ),
                                                                folderItem  );
                QFileInfo desktopInfo( target.url.path(), target.name );
                dotDesktopTarget->add( new AutoMakeFileItem( target.name, dotDesktopTarget ) );
            }
            else if ( target.name.contains( ".rc" ) )
            {
                if ( xmlGuiTarget == 0 )
                    xmlGuiTarget = new KDevelop::ProjectItem( i18n( "KDE XMLGUI Definitions" ),
                                                            folderItem );
                QFileInfo rcInfo( target.url.path(), target.name );
                xmlGuiTarget->add( new AutoMakeFileItem( KUrl(rcInfo.absoluteFilePath()),
                                                         xmlGuiTarget ) );
            }
            break;
        case AutoMake::Headers:
            if ( target.location != AutoMake::None )
            {
                if ( installedHeaders == 0 )
                    installedHeaders = new KDevelop::ProjectItem( i18n( "Installed headers" ) );
                QFileInfo headerInfo( target.url.path(), target.name );
                installedHeaders->add( new AutoMakeFileItem( KUrl(headerInfo.absoluteFilePath()),
                                                             installedHeaders ) );
            }
            else
            {
                if ( notInstalledHeaders == 0 )
                    notInstalledHeaders = new KDevelop::ProjectItem( i18n( "Uninstalled headers" ) );
                QFileInfo headerInfo( target.url.path(), target.name );
                notInstalledHeaders->add( new AutoMakeFileItem( KUrl(headerInfo.absoluteFilePath()),
                                                                notInstalledHeaders ) );
            }
            break;
        case AutoMake::Program:
        case AutoMake::Library:
        case AutoMake::LibtoolLibrary:
        default:
            AutoMakeTargetItem* targetItem = new AutoMakeTargetItem( target, folderItem );
            folderItem->add( targetItem );
            QList<QFileInfo> targetFiles = m_interface->filesForTarget( target );
            foreach( QFileInfo fi, targetFiles )
                targetItem->add( new AutoMakeFileItem( KUrl(fi.absoluteFilePath()), targetItem ) );
            break;
        };
    }
    if ( dotDesktopTarget )
        folderItem->add( dotDesktopTarget );
    if ( xmlGuiTarget )
        folderItem->add( xmlGuiTarget );
    if ( installedHeaders )
        folderItem->add( installedHeaders );
    if ( notInstalledHeaders )
        folderItem->add( notInstalledHeaders );
}

QList<KDevelop::ProjectTargetItem*> AutoMakeImporter::targets() const
{
    return QList<KDevelop::ProjectTargetItem*>();
}



