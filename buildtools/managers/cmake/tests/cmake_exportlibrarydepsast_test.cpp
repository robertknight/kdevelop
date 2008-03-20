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

#include "cmake_exportlibrarydepsast_test.h"
#include "cmakeast.h"
#include "cmakelistsparser.h"

QTEST_MAIN( ExportLibraryDepsAstTest )

void ExportLibraryDepsAstTest::testGoodParse()
{
    QFETCH( CMakeFunctionDesc, function );
    ExportLibraryDepsAst* ast = new ExportLibraryDepsAst();
    QVERIFY( ast->parseFunctionInfo( function ) == true );
    delete ast;
}

void ExportLibraryDepsAstTest::testGoodParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = func2.name = "export_library_dependencies";

    QStringList argList1, argList2;
    argList1 << "dep_file";
    argList2 = argList1;
    argList2 << "APPEND";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "good 1" ) << func1;
    QTest::newRow( "good 2" ) << func2;
 
}

void ExportLibraryDepsAstTest::testBadParse()
{
    QFETCH( CMakeFunctionDesc, function );
    AddExecutableAst* ast = new AddExecutableAst();
    QVERIFY( ast->parseFunctionInfo( function ) == false );
    delete ast;
}

void ExportLibraryDepsAstTest::testBadParse_data()
{
    CMakeFunctionDesc func1, func2;
    func1.name = "export_library_dependencies";
    func2.name = "foo_foo_bar";

    QStringList argList1, argList2;
    argList1 << "dep_file";
    argList2 = argList1;
    argList2 << "APPEND";

    func1.addArguments( argList1 );
    func2.addArguments( argList2 );

    QTest::addColumn<CMakeFunctionDesc>( "function" );
    QTest::newRow( "bad 1" ) << func1;
    QTest::newRow( "bad 2" ) << func2;
}

#include "cmake_exportlibrarydepsast_test.moc"
