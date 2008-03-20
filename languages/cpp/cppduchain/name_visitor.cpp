/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

//krazy:excludeall=cpp

#include "name_visitor.h"
#include "name_compiler.h"
#include "type_visitor.h"
#include "lexer.h"
#include "symbol.h"
#include "parsesession.h"
#include "tokens.h"
#include "expressionparser.h"
#include "expressionvisitor.h"
#include <duchain/duchainlock.h>

#include <QtCore/qdebug.h>

#define LOCKDUCHAIN     DUChainReadLocker lock(DUChain::lock())

using namespace KDevelop;
using namespace Cpp;

NameASTVisitor::NameASTVisitor(ParseSession* session, Cpp::ExpressionVisitor* visitor, const KDevelop::DUContext* context, const KDevelop::ImportTrace& trace, const SimpleCursor& position, KDevelop::DUContext::SearchFlags localSearchFlags, bool debug )
: m_session(session), m_visitor(visitor), m_context(context), m_trace(trace), m_find(m_context, m_trace, localSearchFlags, SimpleCursor(position) ), m_debug(debug), m_finalName(0)
{
}

QString decode(ParseSession* session, AST* ast, bool without_spaces)
{
  QString ret;
  if( without_spaces ) {
    //Decode operator-names without spaces for now, since we rely on it in other places.
    ///@todo change this, here and in all the places that rely on it. Operators should then by written like "operator [ ]"(space between each token)
    for( size_t a = ast->start_token; a < ast->end_token; a++ ) {
      const Token &tk = session->token_stream->token(a);
      ret += tk.symbol();
    }
  } else {
    for( size_t a = ast->start_token; a < ast->end_token; a++ ) {
      const Token &tk = session->token_stream->token(a);
      ret += tk.symbol() + " ";
    }
  }
  return ret;
}

void NameASTVisitor::visitUnqualifiedName(UnqualifiedNameAST *node)
{
  QString tmp_name;

  if (node->tilde)
    tmp_name += QLatin1String("~");

  if (node->id)
    tmp_name += m_session->token_stream->token(node->id).symbol();

  if (OperatorFunctionIdAST *op_id = node->operator_id)
    {
#if defined(__GNUC__)
#warning "NameASTVisitor::visitUnqualifiedName() -- implement me"
#endif

      tmp_name += QLatin1String("operator");

      if (op_id->op && op_id->op->op)
        tmp_name +=  decode(m_session, op_id->op, true);
      else
        tmp_name += QLatin1String("{...cast...}");

      m_typeSpecifier = op_id->type_specifier;
    }

  m_currentIdentifier = Identifier(tmp_name);
  m_find.openIdentifier(m_currentIdentifier);

  if (node->template_arguments)
    {
      visitNodes(this, node->template_arguments);
    }

  {
    LOCKDUCHAIN;
    m_find.closeIdentifier(node == m_finalName);
  }

  if( node->id && !m_find.lastDeclarations().isEmpty() ) {
    bool had = false;
    foreach(DeclarationPointer decl, m_find.lastDeclarations()) {
      if(decl && !decl->isForwardDeclaration()) {
        //Prefer registering non forward-declaration uses
        m_visitor->newUse( node, node->id, node->id+1, decl );
        had = true;
        break;
      }
    }
    
    if(!had) //only forward-declarations, register to any.
      m_visitor->newUse( node, node->id, node->id+1, m_find.lastDeclarations()[0] );
  } else if( m_debug )
    kDebug( 9007 ) << "failed to find " << m_currentIdentifier << " as part of " << decode( m_session, node ) << ", searched in " << m_find.describeLastContext();

    
  
  _M_name.push(m_currentIdentifier);
}

TypeSpecifierAST* NameASTVisitor::lastTypeSpecifier() const {
  return m_typeSpecifier;
}

void NameASTVisitor::visitTemplateArgument(TemplateArgumentAST *node)
{
  bool opened = false;
  if( node->expression ) {
    m_visitor->visit( node->expression );
    
    ExpressionEvaluationResult res;
    if( !m_visitor->lastDeclarations().isEmpty() ) {
      LOCKDUCHAIN;
      res.type = m_visitor->lastType();
      res.allDeclarations = m_visitor->lastDeclarations();
      res.instance = m_visitor->lastInstance();
      m_find.openQualifiedIdentifier(res);
      opened = true;
    }else if( m_debug ) {
      kDebug(9007) << "Failed to resolve template-argument " << decode(m_session, node->expression);
    }
  } else if( node->type_id )
  {
    TypeASTVisitor v( m_session, m_visitor, m_context, m_trace, m_debug );
    v.run( node->type_id->type_specifier );

    ExpressionEvaluationResult res;
    if( !v.declarations().isEmpty() ) {
      LOCKDUCHAIN;
      if( v.declarations()[0] )
        res.type = v.declarations()[0]->abstractType();
      res.allDeclarations = v.declarations();

      if( node->type_id->declarator && node->type_id->declarator->ptr_ops ) {
        //Apply pointer operators
        const ListNode<PtrOperatorAST*> *it = node->type_id->declarator->ptr_ops->toFront(), *end = it;

        do
          {
            PtrOperatorAST* ptrOp = it->element;
            if (ptrOp && ptrOp->op) { ///@todo check ordering, eventually walk the chain in reversed order
              QString op = m_session->token_stream->token(ptrOp->op).symbol();
              if (!op.isEmpty()) {
                if (op[0] == '&') {
                  CppReferenceType::Ptr pointer(new CppReferenceType(parseConstVolatile(m_session, ptrOp->cv)));
                  pointer->setBaseType(res.type);
                  res.type = AbstractType::Ptr( pointer.data() );
                } else if (op[0] == '*') {
                  CppPointerType::Ptr pointer(new CppPointerType(parseConstVolatile(m_session, ptrOp->cv)));
                  pointer->setBaseType(res.type);
                  res.type = AbstractType::Ptr( pointer.data() );
                }
              }
            }
            it = it->next;
          }
        while (it != end);
      }
      
      
      m_find.openQualifiedIdentifier(res);
      opened = true;
    }
  }else{
    LOCKDUCHAIN;
    m_find.openQualifiedIdentifier(false);
    m_find.openIdentifier(Identifier(decode(m_session, node)));
    m_find.closeIdentifier(false);
    opened = true;
  }
  LOCKDUCHAIN;
  if(opened)
    m_find.closeQualifiedIdentifier();

  m_currentIdentifier.appendTemplateIdentifier( typeIdentifierFromTemplateArgument(m_session, node) );
}

const QualifiedIdentifier& NameASTVisitor::identifier() const
{
  return _M_name;
}

void NameASTVisitor::run(UnqualifiedNameAST *node)
{
  m_finalName = node;
  
  m_find.openQualifiedIdentifier(false);
  m_typeSpecifier = 0;
  _M_name.clear();
  visit(node);
  LOCKDUCHAIN;
  m_find.closeQualifiedIdentifier();
}

QList<KDevelop::DeclarationPointer> NameASTVisitor::declarations() const
{
  return m_find.lastDeclarations();
}


void NameASTVisitor::run(NameAST *node, bool skipLastNamePart)
{
  m_find.openQualifiedIdentifier(node->global);
  m_typeSpecifier = 0;

  _M_name.clear();

  m_finalName = node->unqualified_name;

  if(skipLastNamePart)
    visitNodes(this, node->qualified_names); //Skip the unqualified name
  else
    visit(node);

  _M_name.setExplicitlyGlobal( node->global );
  LOCKDUCHAIN;
  m_find.closeQualifiedIdentifier();
}
