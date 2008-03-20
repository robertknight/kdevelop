/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
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

#ifndef CMAKEPREFERENCES_H
#define CMAKEPREFERENCES_H

#include <projectkcmodule.h>
#include "cmakecachemodel.h"

#include <KSharedConfig>

class CMakeSettings;

namespace Ui { class CMakeBuildSettings; }

/**
 * @author Matt Rogers <mattr@kde.org>
 * @author Aleix Pol <aleixpol@gmail.com>
 */
class CMakePreferences : public ProjectKCModule<CMakeSettings>
{
    Q_OBJECT
    public:
        explicit CMakePreferences(QWidget* parent = 0, const QVariantList& args = QVariantList());
        ~CMakePreferences();

    private slots:
        virtual void load();
        virtual void save();
        virtual void defaults();
        void listSelectionChanged ( const QModelIndex & index );
        void showInternal(int state);
        void cacheEdited(QStandardItem * ) { emit changed(true); }
        void buildDirChanged(const QString &);
        void createBuildDir();
        void removeBuildDir();
        void showAdvanced(bool v);
    private:
        KUrl m_srcFolder;
        void updateCache( const KUrl & );
        Ui::CMakeBuildSettings* m_prefsUi;
        CMakeCacheModel* m_currentModel;
};

#endif
