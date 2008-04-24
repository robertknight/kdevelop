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

#include "mergedvisitor.h"
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
class KDEVCPPPARSER_EXPORT XmlWriterVisitor : public MergedVisitor
{
public:
	XmlWriterVisitor();

	/**
	 * Writes out the contents of an AST as a XML document.
	 *
	 * @param device The device to write out the XML document to
	 * @param node The root node of the AST to dump 
	 */
	void write(QIODevice* device, AST* node);

    enum Feature
    {
        PositionFeature = 0x1
    };
    Q_DECLARE_FLAGS(Features,Feature);

    void setFeatures(Features features)
    { m_features = features; }
    Features features() const
    { return m_features; }

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
	virtual void visitEnumSpecifier(EnumSpecifierAST*);
	virtual void visitExceptionDeclaration(ExceptionDeclarationAST*);
	virtual void visitExceptionSpecification(ExceptionSpecificationAST*);
	virtual void visitFunctionDefinition(FunctionDefinitionAST*);
	virtual void visitIncrDecrExpression(IncrDecrExpressionAST*);
  	virtual void visitJumpStatement(JumpStatementAST*);
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
	// TypeSpecifierAST* is a base class for various type specifiers which 
	// are not instantiated directly
	//
	// TODO: Merge into DefaultVisitor?
	void visitTypeSpecifier(TypeSpecifierAST*);

	// returns the text of a token in m_tokenStream 
	QString tokenString(AST* node,std::size_t token) const;
	// returns the text of a list of tokens in m_tokenStream
	// 'list' may be the back of a list, ListNode::toFront() is called
	// on list before converting each node to a string with tokenString()
	QString tokenListString(AST* node,const ListNode<std::size_t>* list) const;

	// current XML stream, only valid during a call to write()
	QXmlStreamWriter* m_streamWriter;

    Features m_features;

	static const char* TRUE_STR;
	static const char* FALSE_STR;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(XmlWriterVisitor::Features);

#endif // XMLWRITERVISITOR 

