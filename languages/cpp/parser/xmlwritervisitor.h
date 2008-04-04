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

#ifndef XMLWRITERVISITOR
#define XMLWRITERVISITOR 

#include "default_visitor.h"
#include <cppparserexport.h>

class QXmlStreamWriter;
class TokenStream;

/** 
 * Writes out an AST as an XML document.  Each AST node maps to one XML
 * element.  The root AST node passed to write() maps to the root element
 * int the document.
 *
 * Each type of AST node maps to one type of XML element.  The XML element
 * name for a type is taken from the AST::kindNames array.
 */
class KDEVCPPPARSER_EXPORT XmlWriterVisitor : protected DefaultVisitor
{
public:
	XmlWriterVisitor();

	/**
	 * Writes out the contents of an AST as a XML document.
	 *
	 * @param device The device to write out the XML document to
	 * @param node The node of the AST which  
	 * @param tokenStream The token stream associated with @p node.  This 
	 * is used to obtain the values for attributes (eg. the type of operator 
	 * in a binary expression or name of a type specifier)
	 */
	void write(QIODevice* device, AST* node, TokenStream* tokenStream);

protected:
	virtual void visit(AST* node);
	virtual void visitAccessSpecifier(AccessSpecifierAST*);
	virtual void visitBaseSpecifier(BaseSpecifierAST*);
	virtual void visitBinaryExpression(BinaryExpressionAST*);
	virtual void visitClassMemberAccess(ClassMemberAccessAST*);
	virtual void visitClassSpecifier(ClassSpecifierAST*);
	virtual void visitCppCastExpression(CppCastExpressionAST*);
	virtual void visitDeclarator(DeclaratorAST*);
	virtual void visitDeleteExpression(DeleteExpressionAST*);
	virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST*);
	virtual void visitEnumerator(EnumeratorAST*);
	virtual void visitExceptionSpecification(ExceptionSpecificationAST*);
	virtual void visitFunctionDefinition(FunctionDefinitionAST*);
	virtual void visitIncrDecrExpression(IncrDecrExpressionAST*);
	virtual void visitLinkageSpecification(LinkageSpecificationAST*);
	virtual void visitName(NameAST*);
	virtual void visitNamespace(NamespaceAST*);
	virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST*);
	virtual void visitOperator(OperatorAST*);
	virtual void visitParameterDeclarationClause(ParameterDeclarationClauseAST*);
	virtual void visitPrimaryExpression(PrimaryExpressionAST*);
	virtual void visitPtrOperator(PtrOperatorAST*);
	virtual void visitSimpleDeclaration(SimpleDeclarationAST*);
	virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST*);
	virtual void visitStringLiteral(StringLiteralAST*);
	virtual void visitTemplateDeclaration(TemplateDeclarationAST*);
	virtual void visitTypeParameter(TypeParameterAST*);
	virtual void visitUnaryExpression(UnaryExpressionAST*);
	virtual void visitUnqualifiedName(UnqualifiedNameAST *);
	virtual void visitUsing(UsingAST*);
	virtual void visitWinDeclSpec(WinDeclSpecAST*);

private:
	// returns the text of a token in m_tokenStream 
	QString tokenString(std::size_t token) const;
	// returns the text of a list of tokens in m_tokenStream
	// 'list' may be the back of a list, ListNode::toFront() is called
	// on list before converting each node to a string with tokenString()
	QString tokenListString(const ListNode<std::size_t>* list) const;

	// current XML stream, only valid during a call to write()
	QXmlStreamWriter* m_streamWriter;
	// current token stream, only valid during a call to write()
	TokenStream* m_tokenStream;

	static const char* TRUE_STR;
	static const char* FALSE_STR;
};

#endif // XMLWRITERVISITOR 

