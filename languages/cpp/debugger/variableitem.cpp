/*
 * GDB Debugger Support
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "variableitem.h"

#include <QRegExp>
#include <QApplication>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <KGlobalSettings>

#include "gdbparser.h"
#include "gdbcommand.h"
#include "gdbcontroller.h"
#include "gdbglobal.h"
#include "variablecollection.h"

using namespace GDBMI;
using namespace GDBDebugger;

int VariableItem::varobjIndex = 0;

VariableItem::VariableItem(AbstractVariableItem* parent)
    : AbstractVariableItem(parent),
      highlight_(false),
      format_(natural),
      numChildren_(0),
      frozen_(false),
      initialCreation_(true),
      baseClassMember_(false),
      alive_(true)
{
}

void VariableItem::setExpression(const QString & expression)
{
    expression_ = expression;
}

void VariableItem::setFrozen(bool frozen)
{
    if (frozen_ != frozen) {
        frozen_ = frozen;

        if (isRegisteredWithGdb())
            addCommand(new GDBCommand(VarSetFrozen, QString("%1 %2").arg(frozen_ ? "1" : "0").arg(expression_)));
    }
}

void VariableItem::setVariableObject(const GDBMI::Value & varobj, FormatTypes format, bool baseClassMember)
{
    varobjName_ = varobj["name"].literal();

    if (varobj.hasField("exp"))
        expression_ = varobj["exp"].literal();

    format_ = format;
    baseClassMember_ = baseClassMember;

    // Set type and children.
    m_type = originalValueType_ = varobj["type"].literal();
    numChildren_ = varobj["numchild"].literal().toInt();

    updateValue();
}

void VariableItem::registerWithGdb()
{
    AbstractVariableItem::registerWithGdb();

    if (varobjName_.isEmpty()) {
        // User might have entered format together with expression: like
        //   /x i1+i2
        // If we do nothing, it will be impossible to watch the variable in
        // different format, as we'll just add extra format specifier.
        // So:
        //   - detect initial value of format_
        //   - remove the format specifier from the string.
        static QRegExp explicit_format("^\\s*/(.)\\s*(.*)");
        if (explicit_format.indexIn(expression_) == 0)
        {
            format_ = formatFromGdbModifier(explicit_format.cap(1)[0].toLatin1());
            expression_ = explicit_format.cap(2);
        }

        createVarobj();

    } else {
        collection()->addVariableObject(varobjName_, this);
    }
}

QString VariableItem::nextVariableObjectName()
{
    return QString("KDEV%1").arg(varobjIndex++);
}

void VariableItem::createVarobj()
{
    if (!varobjName_.isEmpty())
        collection()->removeVariableObject(varobjName_);

    varobjName_ = nextVariableObjectName();

    collection()->addVariableObject(varobjName_, this);

    if (frozen_)
    {
        addCommand(new GDBCommand(VarSetFrozen, QString("1 %1").arg(expression_)));
    }
    else
    {
        addCommand(
            // Need to quote expression, otherwise gdb won't like
            // spaces inside it.
            new GDBCommand(VarCreate, QString("%1 * \"%2\"")
                           .arg(varobjName_)
                           .arg(expression_),
                           this,
                           &VariableItem::varobjCreated,
                           initialCreation_ ? false : true));
    }
}

void VariableItem::varobjCreated(const GDBMI::ResultRecord& r)
{
    // If we've tried to recreate varobj (for example for watched expression)
    // after step, and it's no longer valid, it's fine.
    if (r.reason == "error")
    {
        varobjName_ = "";
        return;
    }
    setAliveRecursively(true);

    QString oldType = originalValueType_;
    originalValueType_ = r["type"].literal();
    if (!oldType.isEmpty() && oldType != originalValueType_)
    {
        deleteAllChildren();
    }

    if (r.hasField("exp"))
        expression_ = r["exp"].literal();

    numChildren_ = r["numchild"].literal().toInt();
    currentAddress_ = lastObtainedAddress_;

    setVarobjName(varobjName_);
}

void VariableItem::setVarobjName(const QString& name)
{
    if (!varobjName_.isEmpty())
        collection()->removeVariableObject(varobjName_);

    varobjName_ = name;

    collection()->addVariableObject(varobjName_, this);

    if (format_ != natural)
    {
        addCommand(
            new GDBCommand(VarSetFormat, QString("\"%1\" %2")
                           .arg(varobjName_).arg(varobjFormatName())));
    }

    // Get the initial value.
    updateValue();
}

void VariableItem::valueDone(const GDBMI::ResultRecord& r)
{
    QString s;

    if (r.reason == "done")
    {
        s = GDBParser::getGDBParser()->undecorateValue(
            r["value"].literal());

        if (format_ == character)
        {
            QString encoded = s;
            bool ok;
            int value = s.toInt(&ok);
            if (ok)
            {
                char c = (char)value;
                encoded += " '";
                if (std::isprint(c))
                    encoded += c;
                else {
                    // Try common escape characters.
                    static char *backslashed[] = {"a", "b", "f", "n",
                                                  "r", "t", "v", "0"};
                    static char represented[] = "\a\b\f\n\r\t\v";

                    const char* ix = strchr (represented, c);
                    if (ix) {
                        encoded += "\\";
                        encoded += backslashed[ix - represented];
                    }
                    else
                        encoded += "\\" + s;
                }
                encoded += "'";
                s = encoded;
            }
        }

        if (format_ == binary)
        {
            // For binary format, split the value at 4-bit boundaries
            static QRegExp r("^[01]+$");
            int i = r.indexIn(s);
            if (i == 0)
            {
                QString split;
                for(int i = 0; i < s.length(); ++i)
                {
                    // For string 11111, we should split it as
                    // 1 1111, not as 1111 1.

                    // 0 is past the end character
                    int distance = i - s.length();

                    if (distance % 4 == 0 && !split.isEmpty())
                        split.append(' ');
                    split.append(s[i]);
                }
                s = split;
            }
        }
    }
    else
    {
        s = r["msg"].literal();
        // Error response.
        if (s.startsWith("Cannot access memory"))
        {
            s = "(inaccessible)";
        }
    }

    if (m_value != s) {
        m_value = s;
        dataChanged(ColumnValue);
    }
}

void VariableItem::createChildren(const GDBMI::ResultRecord& r,
                             bool children_of_fake)
{
    if (!r.hasField("children"))
        return;

    const GDBMI::Value& children = r["children"];

    /* In order to figure out which variable objects correspond
       to base class subobject, we first must detect if *this
       is a structure type. We use present of 'public'/'private'/'protected'
       fake child as an indicator. */
    bool structureType = false;
    if (!children_of_fake && children.size() > 0)
    {
        QString exp = children[0]["exp"].literal();
        bool ok = false;
        exp.toInt(&ok);
        if (!ok || exp[0] != '*')
        {
            structureType = true;
        }
    }

    for (int i = 0; i < children.size(); ++i)
    {
        QString exp = children[i]["exp"].literal();
        // For artificial accessibility nodes,
        // fetch their children.
        if (exp == "public" || exp == "protected" || exp == "private")
        {
            QString name = children[i]["name"].literal();
            addCommand(new GDBCommand(VarListChildren,
                                        "\"" +
                                        name + "\"",
                                        this,
                                        &VariableItem::childrenOfFakesDone));
        }
        else
        {
            /* All children of structures that are not artifical
               are base subobjects. */
            bool baseObject = structureType;

            VariableItem* existing = 0;
            foreach (AbstractVariableItem* child, AbstractVariableItem::children())
            {
                VariableItem* v = qobject_cast<VariableItem*>(child);
                kDebug(9012) << "Child exp:" << v->expression_ <<
                    "new exp" << exp;

                if (v->expression_ == exp)
                {
                    existing = v;
                }
            }

            if (existing)
            {
                existing->setVarobjName(children[i]["name"].literal());
            }
            else
            {
                kDebug(9012) << "Creating new varobj" << exp << baseObject;
                // Propagate format from parent.

                VariableItem* v = 0;
                QString type;
                if (children[i].hasField("type"))
                    type = children[i]["type"].literal();

                v = collection()->createVariableItem(type, this);

                v->setVariableObject(children[i], format_, baseObject);
                addChild(v);
            }
        }
    }
}


void VariableItem::childrenDone(const GDBMI::ResultRecord& r)
{
    createChildren(r, false);
}

void VariableItem::childrenOfFakesDone(const GDBMI::ResultRecord& r)
{
    createChildren(r, true);
}

QString VariableItem::displayName() const
{
    if (!expression_.isEmpty() && expression_[0] != '*')
        return expression_;

    if (VariableItem* p = parentItem())
    {
        return "*" + p->displayName();
    }
    else
    {
        return expression_;
    }
}

void VariableItem::setVariableName(const QString & name)
{
    expression_ = name;
}

void VariableItem::setAliveRecursively(bool enable)
{
    alive_ = true;

    foreach (AbstractVariableItem* child, children())
    {
        if (VariableItem* v = qobject_cast<VariableItem*>(child))
            v->setAliveRecursively(enable);
    }
}

QString VariableItem::gdbExpression() const
{
    // The expression for this item can be either:
    //  - number, for array element
    //  - identifier, for member,
    //  - ***intentifier, for derefenreced pointer.
    const VariableItem* parent = parentItem();

    bool ok = false;
    expression_.toInt(&ok);
    if (ok)
    {
        // Array, parent always exists.
        return parent->gdbExpression() + "[" + expression_ + "]";
    }
    else if (expression_[0] == '*')
    {
        if (parent)
        {
            // For MI, expression_ can be "*0" (meaing
            // references 0-th element of some array).
            // So, we really need to get to the parent to computed the right
            // gdb expression.
            return "*" + parent->gdbExpression();
        }
        else
        {
            // Parent can be null for watched expressions. In that case,
            // expression_ should be a valid C++ expression.
            return expression_;
        }
    }
    else
    {
        if (parent)
            /* This is varitem corresponds to a base suboject,
               the expression should cast parent to the base's
               type. */
            if (baseClassMember_)
                return "((" + expression_ + ")" + parent->gdbExpression() + ")";
            else
                return parent->gdbExpression() + "." + expression_;
        else
            return expression_;
    }
}

// **************************************************************************


void VariableItem::clearHighlight()
{
    highlight_ = false;

    foreach (AbstractVariableItem* child, children())
    {
        if (VariableItem* v = qobject_cast<VariableItem*>(child))
            v->clearHighlight();
    }
}

// **************************************************************************

void VariableItem::updateValue()
{
    addCommand(
        new GDBCommand(VarEvaluateExpression,
            "\"" + varobjName_ + "\"",
            this,
            &VariableItem::valueDone,
            true /* handle error */));

    setValueDirty(false);
}

void VariableItem::setValue(const QString& new_value)
{
    addCommand(
        new GDBCommand(VarAssign, QString("\"%1\" %2").arg(varobjName_)
                       .arg(new_value)));

    // And immediately reload it from gdb,
    // so that it's display format is the one gdb uses,
    // not the one user has typed. Otherwise, on the next
    // step, the visible value might change and be highlighted
    // as changed, which is bogus.
    updateValue();
}

// **************************************************************************

void VariableItem::refreshChildren()
{
    if (isChildrenDirty())
    {
        setChildrenDirty(false);

        addCommand(new GDBCommand(VarListChildren,
                                    "\"" + varobjName_ + "\"",
                                    this,
                                    &VariableItem::childrenDone));
    }
}

// **************************************************************************

VariableItem::FormatTypes VariableItem::format() const
{
    return format_;
}

void VariableItem::setFormat(FormatTypes f)
{
    if (f == format_)
        return;

    format_ = f;

    if (!children().isEmpty())
    {
        // If variable has children, change format for children.
        // - for structures, that's clearly right
        // - for arrays, that's clearly right
        // - for pointers, this can be confusing, but nobody ever wants to
        //   see the pointer in decimal!
        foreach (AbstractVariableItem* child, children())
        {
            if (VariableItem* v = qobject_cast<VariableItem*>(child))
                v->setFormat(f);
        }
    }
    else
    {
         addCommand(
            new GDBCommand(VarSetFormat, QString("\"%1\" %2")
                           .arg(varobjName_).arg(varobjFormatName())));

        refresh();
    }
}

VariableItem::FormatTypes VariableItem::formatFromGdbModifier(char c) const
{
    FormatTypes nf;
    switch(c)
    {
    case 'n': // Not quite gdb modifier, but used in our UI.
        nf = natural; break;
    case 'x':
        nf = hexadecimal; break;
    case 'd':
        nf = decimal; break;
    case 'c':
        nf = character; break;
    case 't':
        nf = binary; break;
    default:
        nf = natural; break;
    }
    return nf;
}

QString VariableItem::varobjFormatName() const
{
    switch(format_)
    {
    case natural:
        return "natural";
        break;

    case hexadecimal:
        return "hexadecimal";
        break;

    case decimal:
        return "decimal";
        break;

        // Note: gdb does not support 'character' natively,
        // so we'll generate appropriate representation
        // ourselfs.
    case character:
        return "decimal";
        break;

    case binary:
        return "binary";
        break;
    }
    return "<undefined>";
}


void VariableItem::deregisterWithGdb()
{
    deleteAllChildren();

    AbstractVariableItem::deregisterWithGdb();

    alive_ = false;
    setDirty(true);

    collection()->removeVariableObject(varobjName_);

    if (!controller()->stateIsOn(s_dbgNotStarted) && !varobjName_.isEmpty())
    {
        addCommand(
            new GDBCommand(VarDelete,
                QString("\"%1\"").arg(varobjName_)));
    }

    varobjName_ = "";
}

bool VariableItem::isAlive() const
{
    return alive_;
}

Qt::ItemFlags VariableItem::flags(int column) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable;

    if (alive_)
        flags |= Qt::ItemIsEnabled;

    if (column == ColumnName && !expression_.isEmpty() && expression_[0] != '*')    // Allow to change variable name by editing.
        flags |= Qt::ItemIsEditable;

    return flags;
}

QVariant VariableItem::data(int column, int role) const
{
    if (isValueDirty())
        const_cast<VariableItem*>(this)->updateValue();

    switch (role) {
        case Qt::DisplayRole:
            switch (column) {
                case ColumnName:
                    return displayName();

                case ColumnValue:
                    return m_value;

                case ColumnType:
                    return m_type;
            }
            break;

        case Qt::FontRole:
            switch (column) {
                case ColumnValue:
                    return KGlobalSettings::fixedFont();
            }
            break;

        case Qt::ToolTipRole: {
            const int maxTooltipSize = 70;
            QString tip = displayName();

            if (tip.length() > maxTooltipSize)
                tip = tip.mid(0, maxTooltipSize - 1 ) + " [...]";

            if (!tip.isEmpty())
                tip += "\n" + originalValueType_;

            return tip;
        }
    }

    return QVariant();
}

VariableItem * VariableItem::parentItem() const
{
    return qobject_cast<VariableItem*>(const_cast<QObject*>(QObject::parent()));
}

const QString & VariableItem::variableName() const
{
    return varobjName_;
}

bool VariableItem::hasChildren() const
{
    return numChildren_ || !children().isEmpty();
}

void VariableItem::setHighlight(bool highlight)
{
    highlight_ = highlight;
}

bool VariableItem::highlight()
{
    return highlight_;
}

const QString & VariableItem::type() const
{
    return m_type;
}

#include "variableitem.moc"
