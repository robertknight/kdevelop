/* This file is part of KDevelop
    Copyright 2006 Roberto Raggi <roberto@kdevelop.org>
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

#ifndef CONTEXTBUILDER_H
#define CONTEXTBUILDER_H

#include "default_visitor.h"

#include <QtCore/QSet>

#include "cppducontext.h"

#include <duchainpointer.h>
#include <identifier.h>
#include <ducontext.h>
#include <ksharedptr.h>
#include "cppduchainexport.h"

//Uncomment this to debug what happens to context ranges when new ones are inserted
//#define DEBUG_CONTEXT_RANGES

namespace KDevelop
{
class DUChain;
class KDevelop::DUChainBase;
class DUContext;
class TopDUContext;
}
class CppEditorIntegrator;
class ParseSession;
class NameCompiler;

namespace KTextEditor { class Range; }

namespace Cpp {
  class EnvironmentFile;
  typedef KSharedPtr<EnvironmentFile> EnvironmentFilePointer;
}

struct KDEVCPPDUCHAIN_EXPORT LineContextPair {
  LineContextPair( KDevelop::TopDUContext* _context, int _sourceLine ) : context(_context), sourceLine(_sourceLine) {
  }
  KDevelop::TopDUContext* context;
  int sourceLine;
};

typedef QList<LineContextPair> IncludeFileList;

///@return Whether @param context is contained as a context in @param lineContexts
bool KDEVCPPDUCHAIN_EXPORT containsContext( const QList<LineContextPair>& lineContexts, TopDUContext* context );

///Removes @param context from the list
void KDEVCPPDUCHAIN_EXPORT removeContext( QList<LineContextPair>& lineContexts, TopDUContext* context );

/**
 * A class which iterates the AST to identify contexts.
 */
class KDEVCPPDUCHAIN_EXPORT  ContextBuilder: protected DefaultVisitor
{
  friend class IdentifierVerifier;

public:
  ContextBuilder(ParseSession* session);
  ContextBuilder(CppEditorIntegrator* editor);
  virtual ~ContextBuilder ();


  /**
   * Builds or updates a proxy-context that represents a content-context under a different environment.
   * The built proxy-context will have exactly 1 imported content-context, or zero imports in case of failure.
   * */
  
  KDevelop::TopDUContext* buildProxyContextFromContent(const Cpp::EnvironmentFilePointer& file, const TopDUContextPointer& content, const TopDUContextPointer& updateContext);
  
  /**
   * Compile either a context-definition chain, or add uses to an existing
   * chain.
   *
   * \param includes contexts to reference from the top context.  The list may be changed by this function.
   * \param removeOldImports Should old imports that are not in the includes-list be removed?
   */
  KDevelop::TopDUContext* buildContexts(const Cpp::EnvironmentFilePointer& file, AST *node, IncludeFileList* includes = 0, const TopDUContextPointer& updateContext = TopDUContextPointer(), bool removeOldImports = true);

  /**
   * Build.an independent du-context based on a given parent-context. Such a context may be used for expression-parsing,
   * but should be deleted as fast as possible because it keeps a reference to an independent context.
   *
   * Warning: the resulting context should be deleted some time. Before deleting it, the du-chain must be locked.
   * Warning: The new context is added as a child to the parent-context.
   * \param url A temporary url that can be used to identify this context @todo remove this
   *
   * \param parent Context that will be used as parent for this context
   */
  KDevelop::DUContext* buildSubContexts(const HashedString& url, AST *node, KDevelop::DUContext* parent = 0);

  /**
   * Support another builder by tracking the current context.
   * @param context the context to use. Must be set when the given node has no context. When it has one attached, this parameter is not needed.
   */
  void supportBuild(AST *node, KDevelop::DUContext* context = 0);

protected:
  inline KDevelop::DUContext* currentContext() { return m_contextStack.top(); }

  /**Signalize that a specific item has been encoutered while parsing.
   * All contained items that are not signalized will be deleted at some stage
   * */
  void setEncountered( KDevelop::DUChainBase* item ) {
    m_encountered.insert(item);
  }

  /**
   * @return whether the given item is in the set of encountered items
   * */
  bool wasEncountered( KDevelop::DUChainBase* item ) {
    return m_encountered.contains(item);
  }

  /**
   * Compile an identifier for the specified NameAST \a id.
   *
   * \note this reference will only be valid until the next time the function
   * is called, so you need to create a copy (store as non-reference).
   * @param typeSpecifier a pointer that will eventually be filled with a type-specifier that can be found in the name(for example the return-type of a cast-operator)
   */
  KDevelop::QualifiedIdentifier identifierForName(NameAST* id, TypeSpecifierAST** typeSpecifier = 0) const;

  CppEditorIntegrator* m_editor;

  // Notifications for subclasses
  /// Returns true if we are recompiling a definition-use chain
  inline bool recompiling() const { return m_recompiling; }
  inline void setRecompiling(bool recomp) { m_recompiling = recomp; }

  virtual void addBaseType( CppClassType::BaseClassInstance base );
  
  // Write lock is already held here...
  virtual void openContext(KDevelop::DUContext* newContext);
  // Write lock is already held here...
  virtual void closeContext();

  ///Open/close prefix contexts around the class specifier that make the qualified identifier
  ///of the class Declaration match, because Declarations have only unqualified names.
  ///@param id should be the whole identifier. A prefix-context will only be created if it
  ///has more than 1 element.
  void openPrefixContext(ClassSpecifierAST* ast, const QualifiedIdentifier& id);
  void closePrefixContext(const QualifiedIdentifier& id);
  
  // Split up visitors created for subclasses to use
  /// Visits the type specifier and init declarator for a function.
  virtual void visitFunctionDeclaration (FunctionDefinitionAST *);
  virtual void visitPostSimpleDeclaration(SimpleDeclarationAST*);

  virtual void visitTemplateDeclaration(TemplateDeclarationAST *);

  // Normal overridden visitors
  virtual void visitNamespace(NamespaceAST *);
  virtual void visitClassSpecifier(ClassSpecifierAST *);
  virtual void visitTypedef(TypedefAST *);
  virtual void visitFunctionDefinition(FunctionDefinitionAST *);
  virtual void visitCompoundStatement(CompoundStatementAST *);
  virtual void visitSimpleDeclaration(SimpleDeclarationAST *);
  virtual void visitName(NameAST *);
  virtual void visitUsing(UsingAST*);
  virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST*);
  virtual void visitForStatement(ForStatementAST*);
  virtual void visitIfStatement(IfStatementAST*);

  KDevelop::DUContext* openContext(AST* range, KDevelop::DUContext::ContextType type, const KDevelop::QualifiedIdentifier& identifier);
  KDevelop::DUContext* openContext(AST* range, KDevelop::DUContext::ContextType type, NameAST* identifier = 0);
  KDevelop::DUContext* openContext(AST* fromRange, AST* toRange, KDevelop::DUContext::ContextType type, NameAST* identifier = 0);
  //Opens a context of size 0, starting at the given node
  KDevelop::DUContext* openContextEmpty(AST* range, KDevelop::DUContext::ContextType type);
  
  KDevelop::DUContext* openContextInternal(const KDevelop::SimpleRange& range, KDevelop::DUContext::ContextType type, const KDevelop::QualifiedIdentifier& identifier);

  bool createContextIfNeeded(AST* node, const QList<KDevelop::DUContext*>& importedParentContexts);
  bool createContextIfNeeded(AST* node, KDevelop::DUContext* importedParentContext);
  void addImportedContexts();

  void smartenContext(TopDUContext* topLevelContext);
  
  int templateDeclarationDepth() const {
    return m_templateDeclarationDepth;
  }

protected:
  // Variables
  NameCompiler* m_nameCompiler;

  bool m_ownsEditorIntegrator: 1;
  bool m_compilingContexts: 1;
  bool m_recompiling : 1;
  bool m_inFunctionDefinition;
  bool smart() const;

  int m_templateDeclarationDepth;

  //Here all valid declarations/uses/... will be collected
  QSet<KDevelop::DUChainBase*> m_encountered;

  QStack<KDevelop::DUContext*> m_contextStack;
  QStack<int> m_nextContextStack;
  
  KDevelop::DUContext* m_lastContext; //Last context that was opened

  inline int& nextContextIndex() { return m_nextContextStack.top(); }

#ifdef DEBUG_CONTEXT_RANGES
  void checkRanges();
  QHash<KDevelop::DUContext*, KDevelop::SimpleRange> m_contextRanges;
#endif

  QList<KDevelop::DUContext*> m_importedParentContexts;
};

#endif // CONTEXTBUILDER_H

