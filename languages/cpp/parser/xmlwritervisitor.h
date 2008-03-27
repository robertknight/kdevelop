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

class KDEVCPPPARSER_EXPORT XmlWriterVisitor : protected DefaultVisitor
{
public:
	XmlWriterVisitor();

	void write(QIODevice* device, AST* node, TokenStream* tokenStream);

protected:
	virtual void visit(AST* node);
	virtual void visitUnqualifiedName(UnqualifiedNameAST *);
	virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST*);
	virtual void visitBinaryExpression(BinaryExpressionAST*);
	virtual void visitIncrDecrExpression(IncrDecrExpressionAST*);
	virtual void visitUnaryExpression(UnaryExpressionAST*);
	virtual void visitPrimaryExpression(PrimaryExpressionAST*);

private:
	QString tokenString(std::size_t token) const;

	QXmlStreamWriter* m_streamWriter;
	TokenStream* m_tokenStream;
};

#endif // XMLWRITERVISITOR 

