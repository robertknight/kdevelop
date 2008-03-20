/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "control.h"
#include "lexer.h"

Control::Control()
  : _M_skip_function_body(false)

{
}

Control::~Control()
{
}

const QList<KDevelop::ProblemPointer>& Control::problems() const
{
  return _M_problems;
}

void Control::reportProblem(const KDevelop::Problem &problem)
{
  reportProblem(KDevelop::ProblemPointer(new KDevelop::Problem(problem)));
}

void Control::reportProblem(const KDevelop::ProblemPointer &problem)
{
  _M_problems.append(problem);
}
