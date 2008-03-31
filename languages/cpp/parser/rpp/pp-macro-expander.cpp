/*
  Copyright 2005 Roberto Raggi <roberto@kdevelop.org>
  Copyright 2006 Hamish Rodda <rodda@kde.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "pp-macro-expander.h"

#include <QDate>
#include <QTime>

#include <kdebug.h>
#include <klocale.h>

#include <iproblem.h>

#include "pp-internal.h"
#include "pp-engine.h"
#include "pp-environment.h"
#include "pp-location.h"
#include "preprocessor.h"
#include "chartools.h"

QString joinByteArray(const QList<QByteArray>& arrays, QString between) {
  QString ret;
  foreach(const QByteArray& array, arrays) {
    if(!ret.isEmpty())
      ret += between;
    ret += QString::fromUtf8(array);
  }
  return ret;
}

using namespace rpp;

pp_frame::pp_frame(pp_macro* __expandingMacro, const QList<pp_actual>& __actuals)
  : expandingMacro(__expandingMacro)
  , actuals(__actuals)
{
}

pp_actual pp_macro_expander::resolve_formal(const QByteArray& name, Stream& input)
{
  if (!m_frame)
    return pp_actual();

  Q_ASSERT(m_frame->expandingMacro != 0);

  const QList<QByteArray>& formals = m_frame->expandingMacro->formals;

  if(name.isEmpty()) {
    KDevelop::Problem problem;
    problem.setFinalLocation(KDevelop::DocumentRange(m_engine->currentFileName(), KTextEditor::Range(input.originalInputPosition().textCursor(), 0)));
    problem.setDescription(i18n("Macro error"));
    m_engine->problemEncountered(problem);
    return pp_actual();
  }
  
  for (int index = 0; index < formals.size(); ++index) {
    if (name == formals[index]) {
      if (index < m_frame->actuals.size())
        return m_frame->actuals[index];
      else {
        KDevelop::Problem problem;
        problem.setFinalLocation(KDevelop::DocumentRange(m_engine->currentFileName(), KTextEditor::Range(input.originalInputPosition().textCursor(), 0)));
        problem.setDescription(i18n("Call to macro %1 missing argument number %2", QString::fromUtf8(name), index));
        problem.setExplanation(i18n("Formals: %1", joinByteArray(formals, ", ")));
        m_engine->problemEncountered(problem);
      }
    }
  }

  return pp_actual();
}

pp_macro_expander::pp_macro_expander(pp* engine, pp_frame* frame, bool inHeaderSection)
  : m_engine(engine)
  , m_frame(frame)
  , m_in_header_section(inHeaderSection)
{
}

//A header-section ends when the first non-directive and non-comment occurs
#define check_header_section \
  if( m_in_header_section ) \
  { \
     \
    m_in_header_section = false; \
    m_engine->preprocessor()->headerSectionEnded(input); \
    if( input.atEnd() ) \
      continue; \
  } \


void pp_macro_expander::operator()(Stream& input, Stream& output)
{
  skip_blanks(input, output);

  while (!input.atEnd())
  {
    if (isComment(input))
    {
      skip_comment_or_divop(input, output, true);
    }else{
      if (input == '\n')
      {
        output << input;

        skip_blanks(++input, output);

        if (!input.atEnd() && input == '#')
          break;
      }
      else if (input == '#')
      {
        skip_blanks(++input, output);

        QByteArray identifier = skip_identifier(input);

        KDevelop::SimpleCursor inputPosition = input.inputPosition();
        KDevelop::SimpleCursor originalInputPosition = input.originalInputPosition();
        QByteArray formal = resolve_formal(identifier, input).mergeText();

        if (!formal.isEmpty()) {
          Stream is(&formal, inputPosition);
          is.setOriginalInputPosition(originalInputPosition);
          skip_whitespaces(is, devnull());

          output << '\"';

          while (!is.atEnd()) {
            if (input == '"') {
              output << '\\' << is;

            } else if (input == '\n') {
              output << '"' << is << '"';

            } else {
              output << is;
            }

            skip_whitespaces(++is, output);
          }

          output << '\"';

        } else {
          output << '#'; // TODO ### warning message?
        }

      }
      else if (input == '\"')
      {
        check_header_section
        
        skip_string_literal(input, output);
      }
      else if (input == '\'')
      {
        check_header_section
        
        skip_char_literal(input, output);
      }
      else if (isSpace(input.current()))
      {
        do {
          if (input == '\n' || !isSpace(input.current()))
            break;

          output << input;

        } while (!(++input).atEnd());
      }
      else if (isNumber(input.current()))
      {
        check_header_section
        
        skip_number (input, output);
      }
      else if (isLetter(input.current()) || input == '_')
      {
        check_header_section
        
        KDevelop::SimpleCursor inputPosition = input.inputPosition();
        QByteArray name = skip_identifier (input);

        // search for the paste token
        int blankStart = input.offset();
        skip_blanks (input, devnull());
        if (!input.atEnd() && input == '#') {
          ++input;

          if (!input.atEnd() && input == '#')
            skip_blanks(++input, devnull());
          else
            input.seek(blankStart);

        } else {
          input.seek(blankStart);
        }

        //Q_ASSERT(name.length() >= 0 && name.length() < 512);

        KDevelop::SimpleCursor inputPosition2 = input.inputPosition();
        pp_actual actual = resolve_formal(name, input);
        if (actual.isValid()) {
          Q_ASSERT(actual.text.size() == actual.inputPosition.size());
          
          QList<QByteArray>::const_iterator textIt = actual.text.begin();
          QList<KDevelop::SimpleCursor>::const_iterator cursorIt = actual.inputPosition.begin();

          for( ; textIt != actual.text.end(); ++textIt, ++cursorIt )
          {
            output.appendString(*cursorIt, *textIt);
          }
          output.mark(input.inputPosition());
          continue;
        }

        // TODO handle inbuilt "defined" etc functions

        pp_macro* macro = m_engine->environment()->retrieveMacro(name);
        if (!macro || !macro->defined || macro->hidden || m_engine->hideNextMacro())
        {
          m_engine->setHideNextMacro(name == "defined");

          if (name == "__LINE__")
            output.appendString(inputPosition, QString::number(input.inputPosition().line).toUtf8());
          else if (name == "__FILE__")
            output.appendString(inputPosition, QString("\"%1\"").arg(m_engine->currentFile()).toUtf8());
          else if (name == "__DATE__")
            output.appendString(inputPosition, QDate::currentDate().toString("MMM dd yyyy").toUtf8());
          else if (name == "__TIME__")
            output.appendString(inputPosition, QTime::currentTime().toString("hh:mm:ss").toUtf8());
          else
            output.appendString(inputPosition, name);
          continue;
        }

        if (!macro->function_like)
        {
          pp_macro* m = 0;

          if (!macro->definition.isEmpty()) {
            macro->hidden = true;

            pp_macro_expander expand_macro(m_engine);
            Stream ms(&macro->definition, input.inputPosition());
            ms.setOriginalInputPosition(input.originalInputPosition());
            QByteArray expanded;
            {
              Stream es(&expanded);
              expand_macro(ms, es);
            }

            if (!expanded.isEmpty())
            {
              Stream es(&expanded, input.inputPosition());
              es.setOriginalInputPosition(input.originalInputPosition());
              skip_whitespaces(es, devnull());
              QByteArray identifier = skip_identifier(es);

              pp_macro* m2 = 0;
              if (es.atEnd() && (m2 = m_engine->environment()->retrieveMacro(identifier)) && m2->defined) {
                m = m2;
              } else {
                output.appendString(inputPosition, expanded);
              }
            }

            macro->hidden = false;
          }

          if (!m)
            continue;

          macro = m;
        }

        skip_whitespaces(input, devnull());

        //In case expansion fails, we can skip back to this position
        int openingPosition = input.offset();
        KDevelop::SimpleCursor openingPositionCursor = input.inputPosition();
        
        // function like macro
        if (input.atEnd() || input != '(')
        {
          output.appendString(inputPosition, name);
          continue;
        }

        QList<pp_actual> actuals;
        ++input; // skip '('

        pp_macro_expander expand_actual(m_engine, m_frame);

        int before = input.offset();
        {
          actual.clear();

          QByteArray actualText;
          skip_whitespaces(input, devnull());
          KDevelop::SimpleCursor actualStart = input.inputPosition();
          {
            Stream as(&actualText);
            skip_argument_variadics(actuals, macro, input, as);
          }
          actualText = actualText.trimmed();

          if (input.offset() != before)
          {
            pp_actual newActual;
            {
              QByteArray newActualText;
              Stream as(&actualText, actualStart);
              as.setOriginalInputPosition(input.originalInputPosition());///@todo What does originalInputPosition mean?

              rpp::LocationTable table;
              table.anchor(0, actualStart);
              Stream nas(&newActualText, actualStart, &table);
              expand_actual(as, nas);
              
              table.splitByAnchors(newActualText, actualStart, newActual.text, newActual.inputPosition);
            }
            actuals.append(newActual);
          }
        }

        // TODO: why separate from the above?
        while (!input.atEnd() && input == ',')
        {
          actual.clear();
          ++input; // skip ','

          {
            QByteArray actualText;
            skip_whitespaces(input, devnull());
            KDevelop::SimpleCursor actualStart = input.inputPosition();
            {
              Stream as(&actualText);
              skip_argument_variadics(actuals, macro, input, as);
            }
            actualText = actualText.trimmed();

            pp_actual newActual;
            {
              QByteArray newActualText;
              Stream as(&actualText, actualStart);
              as.setOriginalInputPosition(input.originalInputPosition());

              QByteArray actualText;
              rpp::LocationTable table;
              table.anchor(0, actualStart);
              Stream nas(&newActualText, actualStart, &table);
              expand_actual(as, nas);
              
              table.splitByAnchors(newActualText, actualStart, newActual.text, newActual.inputPosition);
            }
            actuals.append(newActual);
          }
        }

        if( input != ')' ) {
          //Failed to expand the macro. Output the macro name and continue normal
          //processing behind it.(Code completion depends on this behavior when expanding
          //incomplete input-lines)
          output.appendString(inputPosition, name);
          input.seek(openingPosition);
          input.setInputPosition(openingPositionCursor);
          continue;
        }
        //Q_ASSERT(!input.atEnd() && input == ')');

        ++input; // skip ')'

#if 0 // ### enable me
        assert ((macro->variadics && macro->formals.size () >= actuals.size ())
                    || macro->formals.size() == actuals.size());
#endif

        pp_frame frame(macro, actuals);
        pp_macro_expander expand_macro(m_engine, &frame);
        macro->hidden = true;
        Stream ms(&macro->definition, input.inputPosition());
        ms.setOriginalInputPosition(input.originalInputPosition());
        expand_macro(ms, output);
        macro->hidden = false;

      } else {
        output << input;
        ++input;
      }
    }

  }
}

void pp_macro_expander::skip_argument_variadics (const QList<pp_actual>& __actuals, pp_macro *__macro, Stream& input, Stream& output)
{
  int first;

  do {
    first = input.offset();
    skip_argument(input, output);

  } while ( __macro->variadics
            && first != input.offset()
            && !input.atEnd()
            && input == '.'
            && (__actuals.size() + 1) == __macro->formals.size());
}

