/* KDevelop Custom Makefile Support
 *
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "custommakemanager.h"
#include "custommakemodelitems.h"
#include "custommaketreesynchronizer.h"
#include <icore.h>
#include <iproject.h>
#include <iprojectcontroller.h>
#include <iplugincontroller.h>
#include "imakebuilder.h"
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <projectmodel.h>
#include <context.h>

#include <QQueue>
#include <QDir>
#include <QFileInfoList>
#include <QFile>
#include <QActionGroup>
#include <QVariant>
#include <QStack>
#include <QAction>
#include <QtDesigner/QExtensionFactory>

#include <kurl.h>
#include <klocale.h>
#include <kmenu.h>
#include <kactionmenu.h>
#include <kactioncollection.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(CustomMakeSupportFactory, registerPlugin<CustomMakeManager>(); )
K_EXPORT_PLUGIN(CustomMakeSupportFactory("kdevcustommakemanager"))

class CustomMakeManager::Private
{
public:
    IMakeBuilder *m_builder;

    QActionGroup *m_targetGroup;
    KMenu *m_targetMenu;
    KDevelop::ProjectBaseItem *m_ctxItem;

//     QList< KDevelop::ProjectBaseItem* > m_testItems; // for debug
};

CustomMakeManager::CustomMakeManager( QObject *parent, const QVariantList& args )
    : KDevelop::IPlugin( CustomMakeSupportFactory::componentData(), parent )
    , d( new Private )
{
    Q_UNUSED(args)
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )

    setXMLFile( "kdevcustommakemanager.rc" );

    // TODO use CustomMakeBuilder
    IPlugin* i = core()->pluginController()->pluginForExtension( "org.kdevelop.IMakeBuilder" );
    Q_ASSERT(i);
    if( i )
    {
        d->m_builder = i->extension<IMakeBuilder>();
    }

    KActionMenu *actionMenu = new KActionMenu( i18n( "Build &Target" ), this );
    actionCollection()->addAction("build_target", actionMenu);
    d->m_targetMenu = actionMenu->menu();

    d->m_targetGroup = new QActionGroup( this );
//     d->m_targetObjectFilesGroup = new QActionGroup( this );
//     d->m_targetOtherFilesGroup = new QActionGroup( this );

    actionMenu->setToolTip( i18n( "Build target" ) );
    actionMenu->setWhatsThis( i18n( "<b>Build target</b><p>Runs <b>make targetname</b> from the project directory (targetname is the name of the target selected).</p>"
            "<p>Environment variables and make arguments can be specified "
            "in the project settings dialog, <b>Build Options</b> tab.</p>" ) );

//     d->m_targetObjectFilesMenu = new KMenu( i18n( "Object Files" ), d->m_targetMenu );
//     d->m_targetOtherFilesMenu = new KMenu( i18n( "Other Files" ), d->m_targetMenu );

    connect( d->m_targetMenu, SIGNAL( aboutToShow() ), this, SLOT( updateTargetMenu() ) );

    connect( d->m_targetGroup, SIGNAL( triggered( QAction* ) ),
             this, SLOT( targetMenuActivated( QAction* ) ) );

    connect( i, SIGNAL(built( KDevelop::ProjectBaseItem* )),
             this, SLOT(slotBuilt( KDevelop::ProjectBaseItem* )) );

//     connect( d->m_targetObjectFilesGroup, SIGNAL( triggered(QAction*) ),
//              this, SLOT( targetObjectMenuActivated( QAction* ) ) );
//     connect( d->m_targetOtherFilesGroup, SIGNAL( triggered(QAction*) ),
//              this, SLOT( targetOtherMenuActivated( QAction* ) ) );
}

CustomMakeManager::~CustomMakeManager()
{
    delete d;
}

IProjectBuilder* CustomMakeManager::builder(KDevelop::ProjectFolderItem*) const
{
    return d->m_builder;
}

KUrl::List CustomMakeManager::includeDirectories(KDevelop::ProjectBaseItem*) const
{
    return KUrl::List();
}

QHash<QString,QString> CustomMakeManager::defines(KDevelop::ProjectBaseItem*) const
{
    return QHash<QString,QString>();
}

ProjectTargetItem* CustomMakeManager::createTarget(const QString& target, KDevelop::ProjectFolderItem *parent)
{
    Q_UNUSED(target)
    Q_UNUSED(parent)
    return NULL;
}

bool CustomMakeManager::addFileToTarget(KDevelop::ProjectFileItem *file, KDevelop::ProjectTargetItem *parent)
{
    Q_UNUSED( file )
    Q_UNUSED( parent )
    return false;
}

bool CustomMakeManager::removeTarget(KDevelop::ProjectTargetItem *target)
{
    Q_UNUSED( target )
    return false;
}

bool CustomMakeManager::removeFileFromTarget(KDevelop::ProjectFileItem *file, KDevelop::ProjectTargetItem *parent)
{
    Q_UNUSED( file )
    Q_UNUSED( parent )
    return false;
}

KUrl CustomMakeManager::buildDirectory(KDevelop::ProjectBaseItem* item) const
{
    return item->project()->folder();
}

QList<ProjectTargetItem*> CustomMakeManager::targets(KDevelop::ProjectFolderItem*) const
{
    QList<ProjectTargetItem*> ret;
    return ret;
}

QList<ProjectFolderItem*> CustomMakeManager::parse(KDevelop::ProjectFolderItem *item)
{
    QList<KDevelop::ProjectFolderItem*> folder_list;
    QDir dir( item->url().toLocalFile() );

    QFileInfoList entries = dir.entryInfoList();

    KDevelop::ProjectFolderItem *prjitem = item->project()->projectItem();
    CustomMakeFolderItem *topItem = dynamic_cast<CustomMakeFolderItem*>( prjitem );

    // fill subfolders
    for ( int i = 0; i < entries.count(); ++i )
    {
        QFileInfo fileInfo = entries.at( i );
        QString fileName = fileInfo.fileName();
        QString absFilePath = fileInfo.absoluteFilePath();

        if ( fileInfo.isDir() && fileName != QLatin1String( "." )
             && fileName != QLatin1String( ".." ) )
        {
//             KDevelop::ProjectFolderItem *cmfi= new KDevelop::ProjectFolderItem(
//                     item->project(), KUrl( fileInfo.absoluteFilePath() ), item );
            // TODO more faster algorithm. should determine whether this directory
            // contains makefile or not.
            KDevelop::ProjectBuildFolderItem *cmfi = new KDevelop::ProjectBuildFolderItem(
                    item->project(), KUrl( absFilePath ), item );
            folder_list.append( cmfi );
//             d->m_testItems.append( cmfi ); // debug
            if( topItem )
                topItem->fsWatcher()->addDirectory( absFilePath, cmfi );
        }
        else if ( fileInfo.isFile() )
        {
            KUrl fileUrl( absFilePath );
            KDevelop::ProjectFileItem *fileItem =
                new KDevelop::ProjectFileItem( item->project(), fileUrl, item );

            if( topItem && fileName == QString("Makefile") )
            {
                topItem->fsWatcher()->addFile( absFilePath, fileItem );
                QStringList targetlist = this->parseCustomMakeFile( KUrl(absFilePath) );
                foreach( QString target, targetlist )
                {
                    new CustomMakeTargetItem( item->project(), target, item );
    //             d->m_testItems.append( targetItem ); // debug
                }
            }
        }
    }
    // find makefile, parse and get the target list
//     KUrl makefileUrl = this->findMakefile( item );
//     if( makefileUrl.isValid() )
//     {
//         QStringList targetlist = this->parseCustomMakeFile( makefileUrl );
//         foreach( QString target, targetlist )
//         {
//             new CustomMakeTargetItem( item->project(), target, item );
// //             d->m_testItems.append( targetItem ); // debug
//         }
//         if( topItem )
//             topItem->fsWatcher()->addFile( makefileUrl.toLocalFile() );
//     }

    return folder_list;
}

KDevelop::ProjectFolderItem* CustomMakeManager::import(KDevelop::IProject *project)
{
    if( !project ) return NULL;
//     return new KDevelop::ProjectFolderItem( project, project->folder().pathOrUrl(), NULL );
    CustomMakeFolderItem *item = new CustomMakeFolderItem( this, project, project->folder(), NULL );
    item->setProjectRoot( true );
    item->fsWatcher()->addDirectory( project->folder().toLocalFile(), item );

    return item;
}

ProjectFolderItem* CustomMakeManager::addFolder(const KUrl& folder, KDevelop::ProjectFolderItem *parent)
{
    Q_UNUSED( folder )
    Q_UNUSED( parent)
    return NULL;
}

ProjectFileItem* CustomMakeManager::addFile(const KUrl& folder, KDevelop::ProjectFolderItem *parent)
{
    Q_UNUSED( folder )
    Q_UNUSED( parent )
    return NULL;
}

bool CustomMakeManager::removeFolder(KDevelop::ProjectFolderItem *folder)
{
    Q_UNUSED( folder )
    return false;
}

bool CustomMakeManager::removeFile(KDevelop::ProjectFileItem *file)
{
    Q_UNUSED( file )
    return false;
}

bool CustomMakeManager::renameFile(KDevelop::ProjectFileItem* oldFile, const KUrl& newFile)
{
    Q_UNUSED( oldFile )
    Q_UNUSED( newFile )
    return false;
}

bool CustomMakeManager::renameFolder(KDevelop::ProjectFolderItem* oldFolder, const KUrl& newFolder )
{
    Q_UNUSED( oldFolder )
    Q_UNUSED( newFolder )
    return false;
}

QPair<QString, QList<QAction*> > CustomMakeManager::requestContextMenuActions( KDevelop::Context* context )
{
    if( context->type() != KDevelop::Context::ProjectItemContext )
    {
        return IPlugin::requestContextMenuActions( context );
    }
    KDevelop::ProjectItemContext* ctx = dynamic_cast<KDevelop::ProjectItemContext*>( context );
    QList<KDevelop::ProjectBaseItem*> baseitemList = ctx->items();
    if( baseitemList.count() != 1 )
        return IPlugin::requestContextMenuActions( context );

    KDevelop::ProjectBaseItem *baseitem = baseitemList.first();
    IPlugin *manager = baseitem->project()->managerPlugin();
    if( manager != this )
    {
        // This project is not managed by me. No context menu.
        return IPlugin::requestContextMenuActions( context );
    }

    QList<QAction*> actions;
    if( KDevelop::ProjectFolderItem *prjItem = dynamic_cast<KDevelop::ProjectFolderItem*>( baseitem ) )
    {
        KAction* prjBldAction = new KAction( i18n( "Build this project" ), this );
        d->m_ctxItem = prjItem;
        connect( prjBldAction, SIGNAL(triggered()), this, SLOT(slotCtxTriggered()) );
        actions << prjBldAction;
    }
    else if( KDevelop::ProjectTargetItem *targetItem = baseitem->target() )
    {
        KAction* targetBldAction = new KAction( i18n( "Build this target" ), this );
//         targetBldAction->setObjectName( d->build_objectname );
//         d->contextMenuMapper->setMapping( targetBldAction, targetBldAction->objectName() );
//         d->contexts[ targetBldAction->objectName() ] = context;
//         connect( targetBldAction, SIGNAL(triggered()), d->contextMenuMapper, SLOT( map() ) );
        d->m_ctxItem = targetItem;
        connect( targetBldAction, SIGNAL(triggered()), this, SLOT(slotCtxTriggered()) );
        actions << targetBldAction;
    }
    else if( baseitem->type() == KDevelop::ProjectBaseItem::BuildFolder )
    {
        KAction *bldFolderAction = new KAction( i18n( "Build this directory" ), this );
        d->m_ctxItem = baseitem;
        connect( bldFolderAction, SIGNAL(triggered()), this, SLOT(slotCtxTriggered()) );
        actions << bldFolderAction;
    }

    return qMakePair(QString("Custom Make Manager"), actions);
}

/////////////////////////////////////////////////////////////////////////////
// private slots
void CustomMakeManager::slotCtxTriggered()
{
    if( d->m_ctxItem )
    {
        d->m_builder->build( d->m_ctxItem );
    }
    d->m_ctxItem = NULL;
}

void CustomMakeManager::updateTargetMenu()
{
    d->m_targetMenu->clear();
//     d->m_targetObjectFilesMenu->clear();
//     d->m_targetOtherFilesMenu->clear();
//     d->m_targetMenu->addMenu( d->m_targetObjectFilesMenu );
//     d->m_targetMenu->addMenu( d->m_targetOtherFilesMenu );

//     d->m_makefilesToParse.clear();
//     d->m_makefilesToParse.push( "Makefile" );
//     d->m_makefilesToParse.push( "makefile" );
//     putEnvVarsInVarMap();
//     while ( !(d->m_makefilesToParse.isEmpty()) )
//         parseMakefile( d->m_makefilesToParse.pop() );

    // disabled because we shouldn't rely on rootItem.
//     QStringList targetlist = parseCustomMakeFile( findMakefile( d->m_rootItem ) );
//
//     QAction *action = NULL;
//     foreach( QString target, targetlist )
//     {
//         action = d->m_targetMenu->addAction( target );
//         action->setData( target );
//         action->setActionGroup( d->m_targetGroup );
//     }

//     for ( it = d->m_targetsObjectFiles.begin(); it != d->m_targetsObjectFiles.end(); ++it ){
//         action = d->m_targetObjectFilesMenu->addAction( *it );
//         action->setData( *it );
//         action->setActionGroup( d->m_targetObjectFilesGroup );
//     }
//
//     for ( it = d->m_targetsOtherFiles.begin(); it != d->m_targetsOtherFiles.end(); ++it ){
//         action = d->m_targetOtherFilesMenu->addAction( *it );
//         action->setData( *it );
//         action->setActionGroup( d->m_targetOtherFilesGroup );
//     }
}

void CustomMakeManager::targetMenuActivated( QAction* action )
{
    kDebug(9025) << "targetActivated:" << action->data().toString();
    // test
//     foreach( KDevelop::ProjectBaseItem *item , d->m_testItems )
//     {
//         builder( d->m_rootItem )->build( item );
//     }

}

void CustomMakeManager::slotBuilt( KDevelop::ProjectBaseItem* item )
{
    if( item->type() != KDevelop::ProjectBaseItem::Target )
        return;
    kDebug(9025) << "CustomMakeManager:: slotBuilt()";
//     KDevelop::ProjectTargetItem *target = static_cast< KDevelop::ProjectTargetItem* >( item );
//
//     if( d->m_targetsByMenu.contains( target ) )
//     {
//         d->m_targetsByMenu.removeAll( target );
//         delete item;
//     }
}

QStringList CustomMakeManager::parseCustomMakeFile( const KUrl &makefile )
{
    if( !makefile.isValid() )
        return QStringList();

    QStringList ret; // the list of targets
//     KUrl absFileUrl = dir;
    // TODO support Makefile, Makefile.xxx, makefile
//     absFileUrl.addPath( "Makefile" );
    QFile f( makefile.toLocalFile() );
    if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        kDebug(9025) << "could not open" << makefile;
        return ret;
    }

    QRegExp targetRe( "^ *([^\\t$.#]\\S+) *:.*$" );
    targetRe.setMinimal( true );

    QString str = "";
    QTextStream stream( &f );
    while ( !stream.atEnd() )
    {
        str = stream.readLine();

        if ( targetRe.indexIn( str ) != -1 )
        {
            QString tmpTarget = targetRe.cap( 1 ).simplified();
//             if ( tmpTarget.endsWith( ".o" ) )
//             {
//                 if ( ! ret.contains(tmpTarget) )
//                     ret.append( tmpTarget );
//             }
//             else if ( tmpTarget.contains( '.' ) )
//             {
//                 if ( ! ret.contains(tmpTarget) )
//                     ret.append( tmpTarget );
//             }
//             else
//             {
            if ( ! ret.contains( tmpTarget ) )
                ret.append( tmpTarget );
//             }
        }
    }
    f.close();
    return ret;
}

#include "custommakemanager.moc"

