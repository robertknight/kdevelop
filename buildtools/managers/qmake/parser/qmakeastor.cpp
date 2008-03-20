/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "ast.h"

namespace QMake
{
OrAST::OrAST( AST* parent )
        : ScopeAST( parent )
{}

OrAST::~OrAST()
{
    qDeleteAll(m_scopes);
    m_scopes.clear();
}

void OrAST::addScope( ScopeAST* ast )
{
    m_scopes.append( ast );
}

void OrAST::insertScope( int i, ScopeAST* ast )
{
    m_scopes.insert( i, ast );
}

void OrAST::removeScope( int i )
{
    m_scopes.removeAt( i );
}

QList<ScopeAST*> OrAST::scopes() const
{
    return m_scopes;
}

void OrAST::setIdentifier( ValueAST* id )
{
    m_scopes.front()->setIdentifier(id);
}


AST::Type OrAST::type() const
{
    return AST::Or;
}

int OrAST::column() const
{
    return m_scopes.front()->column();
}

int OrAST::line() const
{
    return m_scopes.front()->line();
}

}

