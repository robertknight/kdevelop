/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
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

#ifndef DUMPTREE_H
#define DUMPTREE_H

#include "default_visitor.h"
#include <cppparserexport.h>

class TokenStream;

class KDEVCPPPARSER_EXPORT DumpTree: protected DefaultVisitor
{
public:
  DumpTree();
  virtual ~DumpTree();

  void dump(AST *node, TokenStream* tokenStream = 0);

protected:
  virtual void visit(AST *node);

private:
  TokenStream* m_tokenStream;
  int indent;
};

#endif // DUMPTREE_H

