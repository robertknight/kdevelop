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

#include "dumptree.h"
#include "lexer.h"
#include "parsesession.h"

#include <QtCore/QString>

#include <kdebug.h>

DumpTree::DumpTree()
  : m_tokenStream(0), indent(0)
{
}

void DumpTree::dump( AST * node, class TokenStream * tokenStream )
{
  m_tokenStream = tokenStream;
  visit(node);
  m_tokenStream = 0;
}

void DumpTree::visit(AST *node)
{
  QString nodeText;
  if( m_tokenStream && node ) {
    for( std::size_t a = node->start_token; a != node->end_token; a++ ) {
      const Token& tok( m_tokenStream->token((int) a) );
      nodeText += QByteArray( tok.session->contents()+tok.position, tok.size );
    }
  }
  if (node)
    kDebug(9007) << QString(indent * 2, ' ').toLatin1().constData() << AST::kindNames[node->kind]
             <<  "[" << node->start_token << "," << node->end_token << "]" << nodeText << endl;

  ++indent;
  DefaultVisitor::visit(node);
  --indent;

  if (node)
    kDebug(9007) << QString(indent * 2, ' ').toLatin1().constData() << AST::kindNames[node->kind];
}

DumpTree::~ DumpTree( )
{
}

