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

#ifndef FOLDERXMLTEST_H
#define FOLDERXMLTEST_H

#include <QtTest/QtTest>
#include "cmakexmlparser.h"



/**
 * Test for all the cmake xml possibilities under
 * @code <folder> @endcode in the cmake information xml
 * Target definitions are not parsed in this test, but their
 * presence is acknowledged
 */
class SimpleFolderXmlTest : public QObject
{
   Q_OBJECT
public:
    SimpleFolderXmlTest() {}

private slots:
    void testNonValidFolder();
    void testNonValidFolder_data();

    void testEmptyFolder();
    void testEmptyFolder_data();

    void testFolderWithSubFolders();
    void testFolderWithSubFolders_data();

    void testFolderWithMultiSubfolders();
    void testFolderWithMultiSubfolders_data();

    void testFolderWithIncludes();
    void testFolderWithIncludes_data();

    void testFolderWithDefines();
    void testFolderWithDefines_data();

    void testFolderWithTargets();
    void testFolderWithTargets_data();

    void fullFolderTest();
    void fullFolderTest_data();


private:
    CMakeXmlParser m_parser;
};


#endif
