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
#ifndef __PROJECTMANAGER_H__
#define __PROJECTMANAGER_H__



#include <QObject>
#include <QDomDocument>

#include <kxmlguiclient.h>
#include <kservice.h>
#include <shellexport.h>

class KUrl;
class KAction;
class ProjectInfo;
class ProjectSession;
class KRecentFilesAction;
class KDevPlugin;

namespace KSettings { class Dialog; }

class ProjectInfo
{
public:
  KUrl         m_projectURL;
  QDomDocument m_document;
  QString      m_profileName;
  QString      m_projectPlugin, m_language, m_activeLanguage, m_vcsPlugin;
  QStringList  m_ignoreParts, m_keywords;

  QString sessionFile() const;
};

/**
Project manager.
Loads and unloads projects.
*/
class KDEVSHELL_EXPORT ProjectManager : public QObject, public KXMLGUIClient
{
  Q_OBJECT

public:

  ~ProjectManager();

  static QString projectDirectory( const QString& path, bool absolute );

  static ProjectManager *getInstance();

  void loadSettings();
  void saveSettings();
  bool loadDefaultProject();

  bool projectLoaded() const;

  KUrl projectFile() const;

  void createActions( KActionCollection* ac );

  ProjectSession* projectSession() const;

public slots:
  bool loadProject( const KUrl& url);
  bool closeProject( bool exiting = false );

private slots:
  void slotOpenProject();
  void slotProjectOptions();

  void slotLoadProject();

  void loadLocalParts();

private:
  ProjectManager();

  void setupActions();
  void getGeneralInfo();

  bool loadProjectFile();
  bool saveProjectFile();

  bool loadProjectPart();
  void unloadProjectPart();

  bool loadLanguageSupport(const QString& lang);
  void unloadLanguageSupport();

  QString profileByAttributes(const QString &language, const QStringList &keywords);

  ProjectInfo *m_info;

  KAction *m_closeProjectAction, *m_projectOptionsAction;
  KRecentFilesAction *m_openRecentProjectAction;

  static ProjectManager *s_instance;

  ProjectSession* m_pProjectSession;

  KDevPlugin *m_vcsPlugin;
  QString m_vcsName;

  QString m_oldProfileName;

  bool m_toolbarStatus[5];

  KSettings::Dialog* m_settingsDialog;

};


#endif


