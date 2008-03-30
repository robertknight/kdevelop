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
void XmlWriterVisitor::visitUnqualifiedName(UnqualifiedNameAST* node)
{
	m_streamWriter->writeCharacters(tokenString(node->tilde) + tokenString(node->id));

	DefaultVisitor::visitUnqualifiedName(node);
}
void XmlWriterVisitor::visitUnaryExpression(UnaryExpressionAST* node)
{
	m_streamWriter->writeAttribute("operator",tokenString(node->op));
	DefaultVisitor::visitUnaryExpression(node);
}
void XmlWriterVisitor::visitIncrDecrExpression(IncrDecrExpressionAST* node)
{
	m_streamWriter->writeAttribute("operator",tokenString(node->op));
	DefaultVisitor::visitIncrDecrExpression(node);
}
void XmlWriterVisitor::visitPrimaryExpression(PrimaryExpressionAST* node)
{
	m_streamWriter->writeCharacters(tokenString(node->token));
	DefaultVisitor::visitPrimaryExpression(node);
}
void XmlWriterVisitor::visitAccessSpecifier(AccessSpecifierAST* node)
{
	m_streamWriter->writeCharacters(tokenListString(node->specs));
}
void XmlWriterVisitor::visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node)
{
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
