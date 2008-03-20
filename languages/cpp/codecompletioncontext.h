/* 
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

#ifndef CODECOMPLETIONCONTEXT_H
#define CODECOMPLETIONCONTEXT_H

#include <ktexteditor/cursor.h>
#include <ksharedptr.h>
#include <duchain/duchainpointer.h>
#include "cppduchain/typeconversion.h"
#include "cppduchain/expressionparser.h"
#include "cppduchain/viablefunctions.h"
#include "cppduchain/overloadresolutionhelper.h"
#include <typesystem.h>
#include "includeitem.h"

namespace KTextEditor {
  class View;
  class Cursor;
}

namespace KDevelop {
  class DUContext;
  class AbstractType;
}

namespace Cpp {
  class OverloadResolutionFunction;

  /**
   * This class is responsible for finding out what kind of completion is needed, what expression should be evaluated for the container-class of the completion, what conversion will be applied to the result of the completion, etc.
   * */
  class CodeCompletionContext : public KShared {
    public:

      typedef KSharedPtr<CodeCompletionContext> Ptr;

      typedef OverloadResolutionFunction Function;

      typedef QList<Function> FunctionList;
      
      enum MemberAccessOperation {
        NoMemberAccess,  ///With NoMemberAccess, a global completion should be done
        MemberAccess,      ///klass.
        ArrowMemberAccess, ///klass->
        StaticMemberChoose, /// Class::
        MemberChoose, /// klass->ParentClass::
        FunctionCallAccess,  ///"function(". Will never appear is initial access-operation, but as parentContext() access-operation.
        SignalAccess,  ///All signals from MemberAccessContainer should be listed
        SlotAccess,     ///All slots from MemberAccessContainer should be listed
        IncludeListAccess ///A list of include-files should be presented. Get the list through includeItems()
      };
      /**
       * The first context created will never be a FunctionCallAccess
       * context. Instead it will at least be a NoMemberAccess. The result completion-list types/return-types should then be matched
       * against any parent FunctionCallAccess'es
       * */

      enum SpecificContextType {
        Normal,
        FunctionCall,
        BinaryOperatorFunctionCall
      };

      /**
       * @param firstContext should be true for a context that has no parent. Such a context will never be a function-call context.
       * @param text the text to analyze. It usually is the text in the range starting at the beginning of the context, and ending at the position where completion should start
       * @warning The du-chain must be unlocked when this is called
       * @param knownArgumentExpressions has no effect when firstContext is set
       * @param line Optional line that will be used to filter the macros
       * */
      CodeCompletionContext(KDevelop::DUContextPointer context, const QString& text, int depth = 0, const QStringList& knownArgumentExpressions = QStringList(), int line = -1 );
      ~CodeCompletionContext();

      ///@return whether this context is valid for code-completion
      bool isValid() const;

      ///@return depth of the context. The basic completion-context has depth 0, its parent 1, and so on..
      int depth() const;
      
      /**In the case of recursive argument-hints, there may be a chain of parent-contexts, each for the higher argument-matching
       * The parentContext() should always have the access-operation FunctionCallAccess.
       * When a completion-list is computed, the members of the list can be highlighted that match the corresponding parentContext()->functions() function-argument, or parentContext()->additionalMatchTypes()
       * */
      CodeCompletionContext* parentContext();

      ///@return the used access-operation
      MemberAccessOperation memberAccessOperation() const;
      
      /**
       * When the access-operation is a MemberAccess or ArrowMemberAccess, this
       * is the container that completion should happen in
       * (the code-completion should list its non-static content).
       *
       * When memberAccessOperation is StaticMemberChoose, the code-completion
       * should list all static members of this container.
       *
       * When memberAccessOperation is MemberChoose, it should be treated equivalently to MemberAccess.
       * 
       * The type does not respect the member-access-operation, so
       * the code-completion may check whether the arrow-access was used correctly
       * and maybe do automatic correction.
       * @return the type of the container that should be completed in.
       * */
      ExpressionEvaluationResult memberAccessContainer() const;

      /**
       * Returns the internal context of memberAccessContainer, if any.
       * 
       * When memberAccessOperation is StaticMemberChoose, this returns all
       * fitting namespace-contexts.
       * */
      QList<DUContext*> memberAccessContainers() const;
      
      /**
       * When memberAccessOperation is FunctionCallAccess,
       * this returns all functions available for matching, together with the argument-number that should be matched.
       *
       * Operators are treated as functions, but there is special-cases that need to be treated, especially operator=(..), because that operator competes with normal type-conversion.
       *
       * To also respect builtin operators, the types returned by additionalMatchTypes() must be respected.
       * */
      const FunctionList& functions() const;

      /**
       * When memberAccessOperation is IncludeListAccess, then this contains all the files to be listed.
      * */
      QList<Cpp::IncludeItem> includeItems() const;
      /**
       *
       * Returns additional potential match-types based on builtin operators(like the = operator)
       *
       * The other match-types are given by functions(), they are the argument-types at the offset Function::matchedArguments.
       *
       * All those types should used to highlight the best matching item in the list created.
       * */
      QList<KDevelop::AbstractType::Ptr> additionalMatchTypes() const;
    private:
      ///@return the extracted expression, without any access-operation
      QString expression() const;

      void processFunctionCallAccess();
      
      ///Returns whether the end of m_text is a valid completion-position
      bool isValidPosition();
      ///Should preprocess the given text(replace macros with their body etc.)
      void preprocessText( int line );
      void processIncludeDirective(QString line);
      void log( const QString& str ) const;
      ///Returns whether the given strings ends with an overloaded operator that can form a parent-context
      bool endsWithOperator( const QString& str ) const;
      /**
       * Returns the function-name operator expressen for the operator the given string ends with
       *
       * Example: For "bla[" it returns "[]"
       * */
      QString getEndOperatorFunction( const QString& str ) const;
      /**
       * Returns the exact end of the string that is an operator.
       * Example: For "bla[" it returns "["
       * */
      QString getEndOperator( const QString& str ) const;
      ///Should map a position in m_text to a position in the underlying document
      MemberAccessOperation m_memberAccessOperation;
      bool m_valid;
      QString m_expression;
      QString m_operator; //If this completion-context ends with a binary operator, this is the operator
      ExpressionEvaluationResult m_expressionResult;

      QList<Cpp::IncludeItem> m_includeItems;
    
      QString m_text;
      int m_depth;

      //Here known argument-expressions and their types, that may have come from sub-contexts, are stored
      QStringList m_knownArgumentExpressions;
      QList<ExpressionEvaluationResult> m_knownArgumentTypes;
      
      KDevelop::DUContextPointer m_duContext;
      SpecificContextType m_contextType;

      QList<Function> m_functions;

      KSharedPtr<CodeCompletionContext> m_parentContext;
  };
}

#endif
