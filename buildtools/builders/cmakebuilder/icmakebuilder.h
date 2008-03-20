/* This file is part of KDevelop
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
#ifndef ICMAKEBUILDER_H
#define ICMAKEBUILDER_H

#include <iprojectbuilder.h>
#include <iextension.h>

class IProject;
class ProjectItem;

/**
@author Andreas Pakulat
*/

class ICMakeBuilder : public KDevelop::IProjectBuilder
{
public:
//     virtual QString cmakeBinary(  KDevelop::IProject* project  ) = 0;
    virtual ~ICMakeBuilder() {}

};

KDEV_DECLARE_EXTENSION_INTERFACE( ICMakeBuilder, "org.kdevelop.ICMakeBuilder" )
Q_DECLARE_INTERFACE( ICMakeBuilder, "org.kdevelop.ICMakeBuilder" )

#endif
