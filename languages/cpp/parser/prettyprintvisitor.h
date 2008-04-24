/* This file is part of KDevelop
    Copyright 2008 Robert Knight <robertknight@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation
   or, at your option, any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef PRETTYPRINTVISITOR_H
#define PRETTYPRINTVISITOR_H

#include <QtCore/QTextStream>
#include <QtCore/QStack>

#include "mergedvisitor.h"

class KDEVCPPPARSER_EXPORT PrettyPrintWriter 
{
public:
	virtual ~PrettyPrintWriter() {}

	virtual void reset(QTextStream* output) = 0;
	virtual void enter(AST* node) = 0;
	virtual void leave(AST* node) = 0;
	virtual void print(int tokenType, const QString& text) = 0;

	PrettyPrintWriter& operator<< (int tokenType);
	PrettyPrintWriter& operator<< (const QString& text);
};
class KDEVCPPPARSER_EXPORT SimplePrinter : public PrettyPrintWriter
{
public:
	virtual void reset(QTextStream* output);
	virtual void enter(AST* node);
	virtual void leave(AST* node);
	virtual void print(int tokenType, const QString& text);

private:
	void newLine();

	QString m_currentLine;
	QTextStream m_lineOutput;
	QTextStream* m_output;
	int m_lastTokenType;
	int m_indentation;
	bool m_lastWasAlphaNum:1;
	bool m_inTemplate:1;
	AST* m_topNode;
};

class KDEVCPPPARSER_EXPORT PrettyPrintVisitor : public MergedVisitor
{
public:
	PrettyPrintVisitor();

	void setPrinter(PrettyPrintWriter* rules);	
	PrettyPrintWriter* printer() const;

	void write(QIODevice* device, AST* node);

protected:
	virtual void visit(AST* node);
	virtual void visitAccessSpecifier(AccessSpecifierAST *);
	virtual void visitBaseClause(BaseClauseAST *);
	virtual void visitBaseSpecifier(BaseSpecifierAST *);
	virtual void visitBinaryExpression(BinaryExpressionAST *);
	virtual void visitCastExpression(CastExpressionAST *);
	virtual void visitClassMemberAccess(ClassMemberAccessAST *);
	virtual void visitClassSpecifier(ClassSpecifierAST *);
	virtual void visitCompoundStatement(CompoundStatementAST *);
	virtual void visitCondition(ConditionAST *);
	virtual void visitCppCastExpression(CppCastExpressionAST *);
	virtual void visitCtorInitializer(CtorInitializerAST *);
	virtual void visitDeclarationStatement(DeclarationStatementAST *);
	virtual void visitDeclarator(DeclaratorAST *);
	virtual void visitDeleteExpression(DeleteExpressionAST *);
	virtual void visitDoStatement(DoStatementAST *);
	virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST *);
	virtual void visitEnumSpecifier(EnumSpecifierAST *);
	virtual void visitEnumerator(EnumeratorAST *);
	virtual void visitExceptionDeclaration(ExceptionDeclarationAST *);
	virtual void visitExceptionSpecification(ExceptionSpecificationAST *);
	virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST *);
	virtual void visitExpressionStatement(ExpressionStatementAST *);
	virtual void visitForStatement(ForStatementAST *);
	virtual void visitFunctionCall(FunctionCallAST *);
	virtual void visitHandler(HandlerAST *);
	virtual void visitFunctionDefinition(FunctionDefinitionAST *);
	virtual void visitFunctionTryBlock(FunctionTryBlockAST *);
	virtual void visitIfStatement(IfStatementAST *);
	virtual void visitIncrDecrExpression(IncrDecrExpressionAST *);
	virtual void visitInitDeclarator(InitDeclaratorAST *);
	virtual void visitInitializer(InitializerAST *);
	virtual void visitInitializerClause(InitializerClauseAST *);
	virtual void visitJumpStatement(JumpStatementAST *);
	virtual void visitLabeledStatement(LabeledStatementAST *);
	virtual void visitLinkageBody(LinkageBodyAST *);
	virtual void visitLinkageSpecification(LinkageSpecificationAST *);
	virtual void visitMemInitializer(MemInitializerAST *);
	virtual void visitName(NameAST *);
	virtual void visitNamespace(NamespaceAST *);
	virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST *);
	virtual void visitNewDeclarator(NewDeclaratorAST *);
	virtual void visitNewExpression(NewExpressionAST *);
	virtual void visitNewInitializer(NewInitializerAST *);
	virtual void visitOperator(OperatorAST *);
	virtual void visitOperatorFunctionId(OperatorFunctionIdAST *);
	virtual void visitParameterDeclaration(ParameterDeclarationAST *);
	virtual void visitParameterDeclarationClause(ParameterDeclarationClauseAST *);
	virtual void visitPostfixExpression(PostfixExpressionAST *);
	virtual void visitPrimaryExpression(PrimaryExpressionAST *);
	virtual void visitPtrOperator(PtrOperatorAST *);
	virtual void visitPtrToMember(PtrToMemberAST *);
	virtual void visitReturnStatement(ReturnStatementAST *);
	virtual void visitSimpleDeclaration(SimpleDeclarationAST *);
	virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST *);
	virtual void visitSizeofExpression(SizeofExpressionAST *);
	virtual void visitStringLiteral(StringLiteralAST *);
	virtual void visitSubscriptExpression(SubscriptExpressionAST *);
	virtual void visitSwitchStatement(SwitchStatementAST *);
	virtual void visitTemplateArgument(TemplateArgumentAST *);
	virtual void visitTemplateDeclaration(TemplateDeclarationAST *);
	virtual void visitTemplateParameter(TemplateParameterAST *);
	virtual void visitThrowExpression(ThrowExpressionAST *);
	virtual void visitTryBlockStatement(TryBlockStatementAST *);
	virtual void visitTypeId(TypeIdAST *);
	virtual void visitTypeParameter(TypeParameterAST *);
	virtual void visitTypeSpecifier(TypeSpecifierAST *);
	virtual void visitTypedef(TypedefAST *);
	virtual void visitUnaryExpression(UnaryExpressionAST *);
	virtual void visitUnqualifiedName(UnqualifiedNameAST *);
	virtual void visitUsing(UsingAST *);
	virtual void visitUsingDirective(UsingDirectiveAST *);
	virtual void visitWhileStatement(WhileStatementAST *);
	virtual void visitWinDeclSpec(WinDeclSpecAST *);
	
private:
	void writeToken(AST* node, std::size_t token);
	void writeTokenList(AST* node,const ListNode<std::size_t>* tokens);
	template <class T>
	void visitListWithSeparator(const ListNode<T>* nodes,int separatorToken,
								int startToken = 0, int endToken = 0);
	bool higherPrecedenceInStack(int opKind) const;
	static int opPrecedence(int opKind);

	QStack<AST*> m_nodeStack;
	QStack<int> m_opStack;

	PrettyPrintWriter* m_printer;
	int m_currentIndentation;
	QTextStream m_outputStream;
};
inline void PrettyPrintVisitor::setPrinter(PrettyPrintWriter* rules)
{ m_printer = rules; }
inline PrettyPrintWriter* PrettyPrintVisitor::printer() const
{ return m_printer; }

#endif // PRETTYPRINTVISITOR_H 
