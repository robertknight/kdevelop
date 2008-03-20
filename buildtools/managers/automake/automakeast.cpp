/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   Copyright (c) 2005 by Matt Rogers <mattr@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "automakeast.h"

namespace AutoMake {

//AST

AST::~AST()
{
	qDeleteAll( m_children );
}

void AST::addChildAST(AST *node)
{
	m_children.append(node);
}

void AST::writeBack(QString &buffer)
{
	foreach( AST* a, m_children )
	{
		a->writeBack(buffer);
	}
}

QString AST::indentation()
{
	QString result;
	for (int i = 0; i < depth(); i++)
		result += '\t';
	return result;
}

bool AST::hasChildren() const
{
	return !m_children.isEmpty();
}

QList<AST*> AST::children() const
{
	return m_children;
}

//ProjectAST

void ProjectAST::writeBack(QString &buffer)
{
	if ( isRule() )
		buffer += scopedID;
	else
		buffer += indentation();

	AST::writeBack(buffer);

}

void ProjectAST::addChildAST(AST *node)
{
	statements.append(node);
	AST::addChildAST(node);
}

void AssignmentAST::writeBack(QString &buffer)
{
	buffer += indentation() + scopedID + ' ' + op + ' ' + values.join(' ');
}

void AutomakeTargetAST::writeBack( QString& buffer )
{
	buffer += target + ':' + deps.join('');
}

void ConditionAST::writeBack( QString& buffer )
{
	buffer += indentation() + type + ' ' + conditionName;
}

void NewLineAST::writeBack(QString &buffer)
{
	buffer += '\n';
}

void CommentAST::writeBack(QString &buffer)
{
	buffer += indentation() + comment;
}

}

// kate: indent-mode csands; tab-width 4; space-indent off;
