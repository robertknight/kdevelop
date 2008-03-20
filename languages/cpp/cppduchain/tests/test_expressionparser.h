/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef TestExpressionParser_H
#define TestExpressionParser_H

#include <QObject>
#include <QByteArray>

#include <kurl.h>

#include <editor/simplecursor.h>

#include "parser.h"
#include "control.h"

#include "typerepository.h"
#include <identifier.h>
#include "dumpchain.h"

namespace KDevelop
{
class Declaration;
class TopDUContext;
}

class TestExpressionParser : public QObject
{
  Q_OBJECT

public:
  TestExpressionParser();

  KDevelop::Declaration* findDeclaration(KDevelop::DUContext* context, const KDevelop::Identifier& id, const KDevelop::SimpleCursor& position = KDevelop::SimpleCursor::invalid());
  KDevelop::Declaration* findDeclaration(KDevelop::DUContext* context, const KDevelop::QualifiedIdentifier& id, const KDevelop::SimpleCursor& position = KDevelop::SimpleCursor::invalid());

private slots:
  void initTestCase();
  void testSimpleExpression();
  void testTypeConversion();
  void testSmartPointer();
  void testCasts();
  void testBaseClasses();
  void testTemplatesSimple();
  void testTemplates();
  void cleanupTestCase();
  void testOperators();
  void testTemplateFunctions();
  void testThis();
  void testArray();
  void testDynamicArray();

public:
  enum DumpArea {
    DumpNone = 0,
    DumpAST = 1,
    DumpDUChain = 2,
    DumpType = 4,
    DumpAll = 7
  };
  Q_DECLARE_FLAGS(DumpAreas, DumpArea)

private:
  KDevelop::DUContext* parse(const QByteArray& unit, DumpAreas dump = static_cast<DumpAreas>(DumpAST | DumpDUChain | DumpType));

  void release(KDevelop::DUContext* top);

  // Parser
  Control control;
  DumpChain dumper;

  KDevelop::AbstractType::Ptr typeVoid;
  KDevelop::AbstractType::Ptr typeInt;

  bool testFileParseOnly;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TestExpressionParser::DumpAreas)

#endif
