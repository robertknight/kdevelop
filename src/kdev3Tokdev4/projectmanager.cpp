/***************************************************************************
 *   Copyright 2005-2006 Jens Dagerbo <jens.dagerbo@swipnet.se>     *
 *   Copyright 2005-2006 Matt Rogers <mattr@kde.org>                *
 *   Copyright 2005-2006 Adam Treat <treat@kde.org>                 *
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
#include "projectmanager.h"

#include <QFile>
#include <qfileinfo.h>
#include <qdom.h>
#include <qstringlist.h>
#include <q3ptrlist.h>
#include <q3vbox.h>
#include <qsize.h>
#include <QTimer>
//Added by qt3to4:
#include <QTextStream>
#include <QTextDocument>

class QDomDocument;

#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <ktoolbar.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kxmlguiwindow.h>
#include <kparts/componentfactory.h>
#include <kaction.h>
#include <kcmdlineargs.h>
#include <k3process.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <ktemporaryfile.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kiconloader.h>
#include <kvbox.h>
#include <krecentfilesaction.h>
#include <ksettings/dialog.h>

#include "kdevproject.h"
#include "kdevlanguagesupport.h"
#include "kdevplugin.h"
#include "kdevcreatefile.h"

#include "toplevel.h"
#include <core.h>
#include <plugincontroller.h>
#include <documentcontroller.h>
#include "partselectwidget.h"
#include "languageselectwidget.h"
#include "generalinfowidget.h"
#include "projectsession.h"
#include "domutil.h"


QString ProjectInfo::sessionFile() const
{
    QString sf = m_projectURL.path(KUrl::RemoveTrailingSlash);
    sf.truncate(sf.length() - 8); // without ".kdevelop"
    sf += "kdevses"; // suffix for a KDevelop session file
    return sf;
}

QString ProjectManager::projectDirectory( const QString& path, bool absolute ) {
    if(absolute)
        return path;
    KUrl url(ProjectManager::getInstance()->projectFile(), path);
    url.cleanPath();
    return url.path(KUrl::RemoveTrailingSlash);
}

ProjectManager *ProjectManager::s_instance = 0;

ProjectManager::ProjectManager()
: m_info(0L),
  m_pProjectSession(new ProjectSession),
  m_settingsDialog( 0 )
{
}

ProjectManager::~ProjectManager()
{
  delete m_pProjectSession;
  delete m_info;
}

ProjectManager *ProjectManager::getInstance()
{
  if (!s_instance)
    s_instance = new ProjectManager;
  return s_instance;
}

void ProjectManager::createActions( KActionCollection* ac )
{
  KAction *action;

  action = new KAction(KIcon("project-open"), i18n("&Open Project..."), ac, "project_open");
  connect(action, SIGNAL(triggered(bool) ), SLOT(slotOpenProject()));
  action->setToolTip( i18n("Open project"));
  action->setWhatsThis(i18n("<b>Open project</b><p>Opens a KDevelop3 or KDevelop2 project.</p>"));

  m_openRecentProjectAction =
    new KRecentFilesAction(i18n("Open &Recent Project"), 0,
                          this, SLOT(loadProject(const KUrl &)),
                          ac, "project_open_recent");
  m_openRecentProjectAction->setToolTip(i18n("Open recent project"));
  m_openRecentProjectAction->setWhatsThis(i18n("<b>Open recent project</b><p>Opens recently opened project.</p>"));
  m_openRecentProjectAction->loadEntries(KGlobal::config(), "RecentProjects");

  m_closeProjectAction = new KAction(KIcon("window-close"), i18n("C&lose Project"), ac, "project_close");
  connect(m_closeProjectAction, SIGNAL(triggered(bool) ), SLOT(closeProject()));
  m_closeProjectAction->setEnabled(false);
  m_closeProjectAction->setToolTip(i18n("Close project"));
  m_closeProjectAction->setWhatsThis(i18n("<b>Close project</b><p>Closes the current project.</p>"));

  m_projectOptionsAction = new KAction(KIcon("configure"), i18n("Project &Options"), ac, "project_options" );
  connect(m_projectOptionsAction, SIGNAL(triggered(bool) ), SLOT(slotProjectOptions()));
  m_projectOptionsAction->setToolTip(i18n("Project options"));
  m_projectOptionsAction->setWhatsThis(i18n("<b>Project options</b><p>Lets you customize project options.</p>"));
  m_projectOptionsAction->setEnabled(false);
}

void ProjectManager::slotOpenProject()
{
    KSharedConfig::Ptr config = KGlobal::config();
    config->setGroup("General Options");
    QString defaultProjectsDir = config->readPathEntry("DefaultProjectsDir", QDir::homePath()+'/');

    KUrl url = KFileDialog::getOpenUrl(defaultProjectsDir,
        i18n("*.kdevelop|KDevelop 3 Project Files\n"),
        TopLevel::getInstance()->main(), i18n("Open Project") );

  if( url.isEmpty() )
      return;

  loadProject( url );
}

void ProjectManager::slotProjectOptions()
{
    if ( !m_settingsDialog )
    {
        using namespace KSettings;
        m_settingsDialog = new Dialog( QStringList( "kdevprojectconfig"),
                                       KDevCore::mainWindow()->main() );
    }

    m_settingsDialog->show();
}

void ProjectManager::loadSettings()
{
}

void ProjectManager::saveSettings()
{
  KSharedConfig::Ptr config = KGlobal::config();

  if (projectLoaded())
  {
    config->setGroup("General Options");
    config->writePathEntry("Last Project", ProjectManager::getInstance()->projectFile().url());
  }

  m_openRecentProjectAction->saveEntries(config, "RecentProjects");
}

bool ProjectManager::loadDefaultProject()
{
  KSharedConfig::Ptr config = KGlobal::config();
  config->setGroup("General Options");
  QString project = config->readPathEntry("Last Project", QString());
  bool readProject = config->readEntry("Read Last Project On Startup", true);
  if (!project.isEmpty() && readProject)
  {
      loadProject(KUrl(project));
      return true;
  }
  return false;
}

bool ProjectManager::loadProject(const KUrl &url)
{
  if (!url.isValid())
    return false;

  // reopen the already opened project?
  if( url.path() == projectFile().toLocalFile() )
  {
    if (KMessageBox::questionYesNo(TopLevel::getInstance()->main(),
        i18n("Are you sure you want to reopen the current project?")) == KMessageBox::No)
      return false;
  }

  TopLevel::getInstance()->main()->menuBar()->setEnabled( false );
  QApplication::setOverrideCursor( Qt::WaitCursor );

  if( projectLoaded() && !closeProject() )
  {
    m_openRecentProjectAction->setCurrentItem( -1 );
    TopLevel::getInstance()->main()->menuBar()->setEnabled( true );
    QApplication::restoreOverrideCursor();
    return false;
  }

  m_info = new ProjectInfo;
  m_info->m_projectURL = url;

  QTimer::singleShot( 0, this, SLOT(slotLoadProject()) );

  // no one cares about this value
  return true;
}

void ProjectManager::slotLoadProject( )
{
  if( !loadProjectFile() )
  {
    m_openRecentProjectAction->removeUrl(m_info->m_projectURL);
    delete m_info; m_info = 0;
    saveSettings();
    TopLevel::getInstance()->main()->menuBar()->setEnabled( true );
    QApplication::restoreOverrideCursor();
    return;
  }

  getGeneralInfo();

  if( !loadLanguageSupport(m_info->m_language) ) {
    delete m_info; m_info = 0;
    TopLevel::getInstance()->main()->menuBar()->setEnabled( true );
    QApplication::restoreOverrideCursor();
    return;
  }

  if( !loadProjectPart() ) {
    unloadLanguageSupport();
    delete m_info; m_info = 0;
    TopLevel::getInstance()->main()->menuBar()->setEnabled( true );
    QApplication::restoreOverrideCursor();
    return;
  }

  TopLevel::getInstance()->statusBar()->message( i18n("Changing plugin profile...") );
  m_oldProfileName = PluginController::getInstance()->changeProfile(m_info->m_profileName);

  TopLevel::getInstance()->statusBar()->message( i18n("Loading project plugins...") );
  loadLocalParts();

  // shall we try to load a session file from network?? Probably not.
    if (m_info->m_projectURL.isLocalFile())
    {
        // first restore the project session stored in a .kdevses file
        if (!m_pProjectSession->restoreFromFile(m_info->sessionFile(), PluginController::getInstance()->loadedPlugins() ))
        {
            kWarning() << i18n("error during restoring of the KDevelop session") ;
            Core::getInstance()->doEmitProjectOpened();
        }
    }

  m_openRecentProjectAction->addUrl(projectFile());

  m_closeProjectAction->setEnabled(true);
  m_projectOptionsAction->setEnabled(true);

  TopLevel::getInstance()->main()->menuBar()->setEnabled( true );
  QApplication::restoreOverrideCursor();

  TopLevel::getInstance()->statusBar()->message( i18n("Project loaded."), 3000 );

  return;
}


bool ProjectManager::closeProject( bool exiting )
{
  if( !projectLoaded() )
    return true;

  // save the session if it is a local file
    if (m_info->m_projectURL.isLocalFile())
    {
        m_pProjectSession->saveToFile(m_info->sessionFile(), PluginController::getInstance()->loadedPlugins() );
    }

    if ( !DocumentController::getInstance()->querySaveDocuments() )
    return false;

  Core::getInstance()->doEmitProjectClosed();

  PluginController::getInstance()->unloadProjectPlugins();
  PluginController::getInstance()->changeProfile(m_oldProfileName);
  unloadLanguageSupport();
  unloadProjectPart();

  /// @todo if this fails, user is screwed
  saveProjectFile();

#ifdef __GNUC__
#warning "port me"
#if 0
  API::getInstance()->codeModel()->wipeout();
#endif
#endif

  delete m_info;
  m_info = 0;

  m_closeProjectAction->setEnabled(false);
  m_projectOptionsAction->setEnabled(false);

  if ( !exiting )
  {
      DocumentController::getInstance()->slotCloseAllWindows();
  }

  return true;
}

bool ProjectManager::loadProjectFile()
{
  QString path;
  if (!KIO::NetAccess::download(m_info->m_projectURL, path, 0)) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("Could not read project file: %1", m_info->m_projectURL.prettyUrl()));
    return false;
  }

  QFile fin(path);
  if (!fin.open(QIODevice::ReadOnly))
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("Could not read project file: %1", m_info->m_projectURL.prettyUrl()));
    return false;
  }

  int errorLine, errorCol;
  QString errorMsg;
  if (!m_info->m_document.setContent(&fin, &errorMsg, &errorLine, &errorCol))
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("This is not a valid project file.\n"
             "XML error in line %1, column %2:\n%3",
              errorLine, errorCol, errorMsg));
    fin.close();
    KIO::NetAccess::removeTempFile(path);
    return false;
  }
  if (m_info->m_document.documentElement().nodeName() != "kdevelop")
  {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("This is not a valid project file."));
    fin.close();
    KIO::NetAccess::removeTempFile(path);
    return false;
  }

  fin.close();
  KIO::NetAccess::removeTempFile(path);

  return true;
}

bool ProjectManager::saveProjectFile()
{
  if (m_info->m_projectURL.isLocalFile()) {
    QFile fout(m_info->m_projectURL.toLocalFile());
    if( !fout.open(QIODevice::WriteOnly) ) {
      KMessageBox::sorry(TopLevel::getInstance()->main(), i18n("Could not write the project file."));
      return false;
    }

    QTextStream stream(&fout);
    fout.close();
  } else {
    KTemporaryFile fout;
    fout.setPrefix("kdevelop3");
    if (!fout.open()) {
      KMessageBox::sorry(TopLevel::getInstance()->main(), i18n("Could not write the project file."));
      return false;
    }
    KIO::NetAccess::upload(fout.fileName(), m_info->m_projectURL, 0);
  }

  return true;
}

static QString getAttribute(const QDomElement& elem, const QString& attr)
{
  QDomElement el = elem.namedItem(attr).toElement();
  return el.firstChild().toText().data();
}

static void getAttributeList(const QDomElement& elem, const QString& attr, const QString& tag, QStringList &list)
{
  list.clear();

  QDomElement el = elem.namedItem(attr).toElement();
  QDomElement item = el.firstChild().toElement();
  while (!item.isNull())
  {
    if (item.tagName() == tag)
      list << item.firstChild().toText().data();
    item = item.nextSibling().toElement();
  }
}

void ProjectManager::getGeneralInfo()
{
  QDomElement docEl = m_info->m_document.documentElement();
  QDomElement generalEl = docEl.namedItem("general").toElement();

  m_info->m_projectPlugin = getAttribute(generalEl, "projectmanagement");
  m_info->m_vcsPlugin = getAttribute(generalEl, "versioncontrol");
  m_info->m_language = getAttribute(generalEl, "primarylanguage");

  getAttributeList(generalEl, "ignoreparts", "part", m_info->m_ignoreParts);
  getAttributeList(generalEl, "keywords", "keyword", m_info->m_keywords);

  //FIXME: adymo: workaround for those project templates without "profile" element
//  m_info->m_profileName = getAttribute(generalEl, "profile");
  QDomElement el = generalEl.namedItem("profile").toElement();
  if (el.isNull())
      m_info->m_profileName = profileByAttributes(m_info->m_language, m_info->m_keywords);
  else
      m_info->m_profileName = el.firstChild().toText().data();
}

bool ProjectManager::loadProjectPart()
{
  KService::Ptr projectService = KService::serviceByDesktopName(m_info->m_projectPlugin);
  if (!projectService) {
    // this is for backwards compatibility with pre-alpha6 projects
    projectService = KService::serviceByDesktopName(m_info->m_projectPlugin.lower());
  }
  if (!projectService) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("No project management plugin %1 found.",
             m_info->m_projectPlugin));
    return false;
  }

  KDevProject *projectPart = KService::createInstance< KDevProject >( projectService, 0,
                                                  PluginController::argumentsFromService( projectService ) );
  if ( !projectPart ) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("Could not create project management plugin %1.",
             m_info->m_projectPlugin));
    return false;
  }

  QString path = DomUtil::readEntry(dom,"/general/projectdirectory", ".");
  bool absolute = DomUtil::readBoolEntry(dom,"/general/absoluteprojectpath",false);
  QString projectDir = projectDirectory( path, absolute );
  kDebug(9000) << "projectDir:" << projectDir << "projectName:" << m_info->m_projectURL.fileName();

  projectPart->openProject(projectDir, m_info->m_projectURL.fileName());

  PluginController::getInstance()->integratePart( projectPart );

  return true;
}

void ProjectManager::unloadProjectPart()
{
  KDevProject *projectPart = KDevCore::activeProject();
  if( !projectPart ) return;
  PluginController::getInstance()->removePart( projectPart );
  projectPart->closeProject();
  delete projectPart;
}

bool ProjectManager::loadLanguageSupport(const QString& lang)
{
  kDebug(9000) << "Looking for language support for" << lang;

  if (lang == m_info->m_activeLanguage)
  {
    kDebug(9000) << "Language support already loaded";
    return true;
  }

  KService::List languageSupportOffers =
    KServiceTypeTrader::self()->query(QLatin1String("KDevelop/LanguageSupport"),
                           QString::fromLatin1("[X-KDevelop-Language] == '%1' and [X-KDevelop-Version] == %2").arg(lang).arg(KDEVELOP_PLUGIN_VERSION));

  if (languageSupportOffers.isEmpty()) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("No language plugin for %1 found.",
             lang));
    return false;
  }

  KService::Ptr languageSupportService = *languageSupportOffers.begin();
  KDevLanguageSupport *langSupport = KService::createInstance<KDevLanguageSupport>( languageSupportService, 0,
                                                        PluginController::argumentsFromService(  languageSupportService ) );

  if ( !langSupport ) {
    KMessageBox::sorry(TopLevel::getInstance()->main(),
        i18n("Could not create language plugin for %1.",
             lang));
    return false;
  }

  KDevCore::setLanguageSupport( langSupport );
  PluginController::getInstance()->integratePart( langSupport );
  m_info->m_activeLanguage = lang;
  kDebug(9000) << "Language support for" << lang << "successfully loaded.";
  return true;
}

void ProjectManager::unloadLanguageSupport()
{
    KDevLanguageSupport *langSupport = KDevCore::languageSupport();
  if( !langSupport ) return;
  kDebug(9000) << "Language support for" << langSupport->name() << "unloading...";
  PluginController::getInstance()->removePart( langSupport );
  delete langSupport;
  KDevCore::setLanguageSupport(0);
}

void ProjectManager::loadLocalParts()
{
    // Make sure to refresh load/ignore lists
    getGeneralInfo();

    PluginController::getInstance()->unloadPlugins( m_info->m_ignoreParts );
    PluginController::getInstance()->loadProjectPlugins( m_info->m_ignoreParts );
    PluginController::getInstance()->loadGlobalPlugins( m_info->m_ignoreParts );
}

KUrl ProjectManager::projectFile() const
{
  if (!m_info)
    return KUrl();
  return m_info->m_projectURL;
}

bool ProjectManager::projectLoaded() const
{
  return m_info != 0;
}

ProjectSession* ProjectManager::projectSession() const
{
  return m_pProjectSession;
}

QString ProjectManager::profileByAttributes(const QString &language, const QStringList &keywords)
{
    KConfig config(KStandardDirs::locate("data", "kdevplatform/profiles/projectprofiles"));
    config.setGroup(language);

    QStringList profileKeywords = QStringList::split("/", "Empty");
    if (config.hasKey("Keywords"))
        profileKeywords = config.readEntry("Keywords", QStringList());

    int idx = 0;
    for (QStringList::const_iterator it = profileKeywords.constBegin();
        it != profileKeywords.constEnd(); ++it)
    {
        if (keywords.contains(*it))
        {
            idx = profileKeywords.findIndex(*it);
            break;
        }
    }

    QStringList profiles;
    if (config.hasKey("Profiles"))
    {
        profiles = config.readEntry("Profiles", QStringList());
        kDebug(9000) << "IDX:" << idx << " PROFILE:" << profiles[idx];
        return profiles[idx];
    }
    return "KDevelop";
}

#include "projectmanager.moc"

