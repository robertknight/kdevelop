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

#include "cmake_findpackageast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( FindPackageAstTest )

void FindPackageAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    FindPackageAst* ast = new FindPackageAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void FindPackageAstTest::testGoodParse_data()
{
}

void FindPackageAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void FindPackageAstTest::testBadParse_data()
{
}

#include "cmake_findpackageast_test.moc"
