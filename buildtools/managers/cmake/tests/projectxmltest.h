/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#ifndef PROJECTXMLTEST_H
#define PROJECTXMLTEST_H

#include <QtTest/QtTest>
#include "cmakexmlparser.h"

/**
 * @author Matt Rogers <mattr@kde.org>
 */
class ProjectXmlTest : public QObject
{
Q_OBJECT
public:
    ProjectXmlTest(QObject* parent = 0);
    ~ProjectXmlTest();

private slots:
    void testEmptyProject();
    void testEmptyProject_data();
    
    void testFullProject();
    void testFullProject_data();

private:
    CMakeXmlParser m_parser;
};

#endif
