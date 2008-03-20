/***************************************************************************
 *   Copyright 2004 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "kdevideextension.h"

#include <QCheckBox>
#include <qradiobutton.h>
#include <QFontComboBox>

#include <klocale.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kurlrequester.h>
#include <kpagedialog.h>
#include <kpagewidgetmodel.h>

KDevIDEExtension::KDevIDEExtension()
 : ShellExtension()
{
}

void KDevIDEExtension::init()
{
    s_instance = new KDevIDEExtension();
}

QString KDevIDEExtension::xmlFile()
{
    return "kdevelopui.rc";
}

QString KDevIDEExtension::defaultProfile()
{
    return "IDE";
}

KDevelop::AreaParams KDevIDEExtension::defaultArea()
{
    KDevelop::AreaParams params = {"code", i18n("Code")};
    return params;
}

QString KDevIDEExtension::projectFileExtension()
{
    return i18n("*.kdev4");
}

QString KDevIDEExtension::projectFileDescription()
{
    return i18n("KDevelop4 Project Files");
}

