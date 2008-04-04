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

// Own
#include "xmlwritervisitor.h"

// Qt
#include <QXmlStreamWriter>
#include <QtDebug>

// CPP Parser
#include "lexer.h"

const char* XmlWriterVisitor::TRUE_STR = "1";
const char* XmlWriterVisitor::FALSE_STR = "0";

XmlWriterVisitor::XmlWriterVisitor()
	: m_streamWriter(0)
	, m_tokenStream(0)
{
}

void XmlWriterVisitor::write(QIODevice* device, AST* node, TokenStream* tokenStream)
{
	m_streamWriter = new QXmlStreamWriter(device);
	m_tokenStream = tokenStream;
	m_streamWriter->setAutoFormatting(true);
	m_streamWriter->writeStartDocument();
	visit(node);
	m_streamWriter->writeEndDocument();
	
	delete m_streamWriter;
	m_streamWriter = 0;
}

void XmlWriterVisitor::visit(AST* node)
{
	Q_ASSERT(m_streamWriter);

	if (node)
	{
		m_streamWriter->writeStartElement(AST::kindNames[node->kind]);
			DefaultVisitor::visit(node);
		m_streamWriter->writeEndElement();
	}
}
void XmlWriterVisitor::visitBinaryExpression(BinaryExpressionAST* node)
{
	m_streamWriter->writeAttribute("operator",tokenString(node->op));
	DefaultVisitor::visitBinaryExpression(node);
}
void XmlWriterVisitor::visitBaseSpecifier(BaseSpecifierAST* node)
{
	m_streamWriter->writeAttribute("visbility",tokenString(node->access_specifier));
	m_streamWriter->writeAttribute("virtual",tokenString(node->virt));
	DefaultVisitor::visitBaseSpecifier(node);
}
void XmlWriterVisitor::visitClassMemberAccess(ClassMemberAccessAST* node)
{
	m_streamWriter->writeAttribute("operator",tokenString(node->op));
	DefaultVisitor::visitClassMemberAccess(node);
}
void XmlWriterVisitor::visitClassSpecifier(ClassSpecifierAST* node)
{
	visitTypeSpecifier(node);
	m_streamWriter->writeAttribute("type",tokenString(node->class_key));
	DefaultVisitor::visitClassSpecifier(node);
}
void XmlWriterVisitor::visitCppCastExpression(CppCastExpressionAST* node)
{
	m_streamWriter->writeAttribute("operator",tokenString(node->op));
	DefaultVisitor::visitCppCastExpression(node);
}
void XmlWriterVisitor::visitDeclarator(DeclaratorAST* node)
{
	m_streamWriter->writeAttribute("cvqualifiers",tokenListString(node->fun_cv));
	DefaultVisitor::visitDeclarator(node);
}
void XmlWriterVisitor::visitDeleteExpression(DeleteExpressionAST* node)
{
	m_streamWriter->writeAttribute("isarray",node->lbracket_token > 0 ? TRUE_STR : FALSE_STR);
	DefaultVisitor::visitDeleteExpression(node);
}
void XmlWriterVisitor::visitUnqualifiedName(UnqualifiedNameAST* node)
{
	m_streamWriter->writeCharacters(tokenString(node->tilde) + tokenString(node->id));
	DefaultVisitor::visitUnqualifiedName(node);
}
void XmlWriterVisitor::visitEnumSpecifier(EnumSpecifierAST* node)
{
	visitTypeSpecifier(node);
	DefaultVisitor::visitEnumSpecifier(node);
}
void XmlWriterVisitor::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node)
{
	visitTypeSpecifier(node);
	m_streamWriter->writeAttribute("type",tokenString(node->type));
	DefaultVisitor::visitElaboratedTypeSpecifier(node);
}
void XmlWriterVisitor::visitEnumerator(EnumeratorAST* node)
{
	m_streamWriter->writeAttribute("identifier",tokenString(node->id));
	DefaultVisitor::visitEnumerator(node);
}
void XmlWriterVisitor::visitExceptionSpecification(ExceptionSpecificationAST* node)
{
	m_streamWriter->writeAttribute("anyType",node->ellipsis > 0 ? TRUE_STR : FALSE_STR);
	DefaultVisitor::visitExceptionSpecification(node);
}
void XmlWriterVisitor::visitFunctionDefinition(FunctionDefinitionAST* node)
{
	m_streamWriter->writeAttribute("storage",tokenListString(node->storage_specifiers));
	m_streamWriter->writeAttribute("specifiers",tokenListString(node->function_specifiers));
	DefaultVisitor::visitFunctionDefinition(node);
}
void XmlWriterVisitor::visitUnaryExpression(UnaryExpressionAST* node)
{
	m_streamWriter->writeAttribute("operator",tokenString(node->op));
	DefaultVisitor::visitUnaryExpression(node);
}
void XmlWriterVisitor::visitLinkageSpecification(LinkageSpecificationAST* node)
{
	m_streamWriter->writeAttribute("extern",tokenString(node->extern_type));
	DefaultVisitor::visitLinkageSpecification(node);
}
void XmlWriterVisitor::visitName(NameAST* node)
{
	m_streamWriter->writeAttribute("global",node->global ? TRUE_STR : FALSE_STR);
	DefaultVisitor::visitName(node);
}
void XmlWriterVisitor::visitNamespace(NamespaceAST* node)
{
	m_streamWriter->writeAttribute("name",tokenString(node->namespace_name));
	DefaultVisitor::visitNamespace(node);
}
void XmlWriterVisitor::visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST* node)
{
	m_streamWriter->writeAttribute("name",tokenString(node->namespace_name));
	DefaultVisitor::visitNamespaceAliasDefinition(node);
}
void XmlWriterVisitor::visitOperator(OperatorAST* node)
{
	m_streamWriter->writeAttribute("operator",tokenString(node->op));
	m_streamWriter->writeAttribute("open",tokenString(node->open));
	m_streamWriter->writeAttribute("close",tokenString(node->close));
	DefaultVisitor::visitOperator(node);
}
void XmlWriterVisitor::visitIncrDecrExpression(IncrDecrExpressionAST* node)
{
	m_streamWriter->writeAttribute("operator",tokenString(node->op));
	DefaultVisitor::visitIncrDecrExpression(node);
}
void XmlWriterVisitor::visitParameterDeclarationClause(ParameterDeclarationClauseAST* node)
{
	m_streamWriter->writeAttribute("vararg",node->ellipsis > 0 ? TRUE_STR : FALSE_STR);
	DefaultVisitor::visitParameterDeclarationClause(node);
}
void XmlWriterVisitor::visitPrimaryExpression(PrimaryExpressionAST* node)
{
	m_streamWriter->writeCharacters(tokenString(node->token));
	DefaultVisitor::visitPrimaryExpression(node);
}
void XmlWriterVisitor::visitPtrOperator(PtrOperatorAST* node)
{
	m_streamWriter->writeAttribute("cvqualifiers",tokenListString(node->cv));
	DefaultVisitor::visitPtrOperator(node);
}
void XmlWriterVisitor::visitSimpleDeclaration(SimpleDeclarationAST* node)
{
	m_streamWriter->writeAttribute("storage",tokenListString(node->storage_specifiers));
	m_streamWriter->writeAttribute("specifiers",tokenListString(node->function_specifiers));
	DefaultVisitor::visitSimpleDeclaration(node);
}
void XmlWriterVisitor::visitStringLiteral(StringLiteralAST* node)
{
	m_streamWriter->writeCharacters(tokenListString(node->literals));
	DefaultVisitor::visitStringLiteral(node);
}
void XmlWriterVisitor::visitTemplateDeclaration(TemplateDeclarationAST* node)
{
	m_streamWriter->writeAttribute("export",node->exported > 0 ? TRUE_STR : FALSE_STR);
	DefaultVisitor::visitTemplateDeclaration(node);
}
void XmlWriterVisitor::visitAccessSpecifier(AccessSpecifierAST* node)
{
	m_streamWriter->writeCharacters(tokenListString(node->specs));
	DefaultVisitor::visitAccessSpecifier(node);
}
void XmlWriterVisitor::visitTypeParameter(TypeParameterAST* node)
{
	m_streamWriter->writeAttribute("type",tokenString(node->type));
	DefaultVisitor::visitTypeParameter(node);
}
void XmlWriterVisitor::visitUsing(UsingAST* node)
{
	m_streamWriter->writeAttribute("name",tokenString(node->type_name));
	DefaultVisitor::visitUsing(node);
}
void XmlWriterVisitor::visitTypeSpecifier(TypeSpecifierAST* node)
{
	m_streamWriter->writeAttribute("cvqualifiers",tokenListString(node->cv));
}
void XmlWriterVisitor::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)
{
	visitTypeSpecifier(node);
	
	// primitive types
	QString typeName = tokenListString(node->integrals);
	if (node->name)
	{
		if (!typeName.isEmpty())
			typeName.append(' ');
		typeName.append(tokenString(node->name->unqualified_name->id));
	}

	m_streamWriter->writeCharacters(typeName);

	DefaultVisitor::visitSimpleTypeSpecifier(node);
}
void XmlWriterVisitor::visitWinDeclSpec(WinDeclSpecAST* node)
{
	m_streamWriter->writeAttribute("specifier",tokenString(node->specifier));
	m_streamWriter->writeAttribute("modifier",tokenString(node->modifier));
	DefaultVisitor::visitWinDeclSpec(node);
}
QString XmlWriterVisitor::tokenString(std::size_t token) const
{
	Q_ASSERT(m_tokenStream);
	return m_tokenStream->token(token).symbol();
}
QString XmlWriterVisitor::tokenListString(const ListNode<std::size_t>* list) const
{
	if (list != 0)
		list = list->toFront();

	QString result;
	while (list != 0)
	{
		result.append(tokenString(list->element));
		if (list->hasNext())
		{
			result.append(' ');
			list = list->next;
		}
		else
			break;
	}
	return result;
}

