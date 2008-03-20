/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
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

#ifndef ASSIGNMENTTEST_H
#define ASSIGNMENTTEST_H

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <QtCore/QList>

namespace QMake
{
    class StatementAST;
    class ProjectAST;
}

class AssignmentTest : public QObject
{
        Q_OBJECT
    public:
        AssignmentTest( QObject* parent = 0 );
        ~AssignmentTest();
    private slots:
        void init();
        void cleanup();
        void simpleParsed();
        void simpleParsed_data();
        void assignInValue();
        void assignInValue_data();
        void commentCont();
        void commentCont_data();
        void quotedBrace();
        void quotedBrace_data();
        void quotedVal();
        void quotedVal_data();
        void quotedValEscapeQuote();
        void quotedValEscapeQuote_data();
        void dotVariable();
        void dotVariable_data();
        void underscoreVariable();
        void underscoreVariable_data();
        void dashVariable();
        void dashVariable_data();
        void opNoSpace();
        void opNoSpace_data();
        void opPlusEq();
        void opPlusEq_data();
        void opTildeEq();
        void opTildeEq_data();
        void opMinusEq();
        void opMinusEq_data();
        void opStarEq();
        void opStarEq_data();
        void opEqual();
        void opEqual_data();
        void otherVarSimple();
        void otherVarSimple_data();
        void otherVarBrace();
        void otherVarBrace_data();
        void otherVarBracket();
        void otherVarBracket_data();
        void shellVar();
        void shellVar_data();
        void commandExec();
        void commandExec_data();
        void bracketVarQuote();
        void bracketVarQuote_data();
        void pathValue();
        void pathValue_data();
        void varNameDollar();
        void varNameDollar_data();
        void commentInMultiline();
        void commentInMultiline_data();
        void commandExecQMakeVar();
        void commandExecQMakeVar_data();
        void varComment();
        void varComment_data();

        void noDashEndVar();
        void noDashEndVar_data();
        void contAfterComment();
        void contAfterComment_data();
    private:
        QMake::ProjectAST* ast;
};

#endif

