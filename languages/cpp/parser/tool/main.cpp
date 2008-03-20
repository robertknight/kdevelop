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

#include "ast.h"
#include "tokens.h"
#include "lexer.h"
#include "parser.h"
#include "control.h"
#include "default_visitor.h"
#include "dumptree.h"
#include "binder.h"
#include "codemodel.h"

#include <QtCore/QFile>
#include <iostream>

static bool dump = false;
static bool bind = false;

bool parse_file(const QString &fileName)
{
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly))
    return false;

  QByteArray contents = file.readAll();
  file.close();

  Control control;
  Parser p(&control);
  pool __pool;

  TranslationUnitAST *ast = p.parse(contents, contents.size(), &__pool);
  if (ast && dump)
    {
      DumpTree dump;
      dump.dump(ast);
    }

  if (ast && bind)
    {
      CodeModel model;
      Binder binder(&model, &p.token_stream);
      FileModelItem dom = binder.run(ast);
    }

  for (int i=0; i<control.problemCount(); ++i)
    {
      Problem p = control.problem(i);

      std::cerr << "** ERRROR:" << qPrintable(p.fileName())
                << ":" << p.line()
                << ":" << qPrintable(p.message())
                << std::endl;
    }

  return control.problemCount() == 0;
}

int main(int, char *argv[])
{
  const char *filename = 0;

  do
    {
      const char *arg = *++argv;

      if (!strcmp(arg, "-dump"))
	{
	  dump = true;
        }
      else if (!strcmp(arg, "-bind"))
        {
          bind = true;
        }
      else
	{
	  filename = arg;
	  break;
        }
    }
  while (*argv);

  bool parsed = false;

  if (filename)
    {
      parsed = parse_file(filename);
    }
  else
    {
      fprintf(stderr, "r++: no input file\n");
    }

  return parsed ? EXIT_SUCCESS : EXIT_FAILURE;
}

