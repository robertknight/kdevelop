/* KDevelop QMake Support
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

#ifndef QMAKELEXER_H
#define QMAKELEXER_H

#include <QtCore/QStack>
#include <QtCore/QString>
#include "parserexport.h"

class QString;
class kdev_pg_location_table;

namespace QMake
{

class Parser;

class KDEVQMAKEPARSER_EXPORT Lexer {
public:
    Lexer(Parser* _parser, const QString& contents);

    int nextTokenKind();
    qint64 tokenBegin() const;
    qint64 tokenEnd() const;

private:
    QString m_content;
    Parser* m_parser;
    int m_curpos;
    int m_contentSize;
    qint64 m_tokenBegin;
    qint64 m_tokenEnd;

    int state() const;
    void pushState(int state);
    void popState();

    QChar* ignoreWhitespaceAndComment(QChar* it);
    void createNewline( int pos );

    QStack<int> mState;
    enum State
    {
        ErrorState = -1,
        DefaultState = 0,
        ContState = 2,
        VariableValueState = 4,
        FunctionArgState = 5
    };

};

}

#endif

