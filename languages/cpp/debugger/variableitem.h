/*
 * GDB Debugger Support
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
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

#ifndef _VARIABLEITEM_H_
#define _VARIABLEITEM_H_

#include "mi/gdbmi.h"

#include "abstractvariableitem.h"

namespace GDBDebugger
{

class GDBController;
class VariableCollection;

class VariableItem : public AbstractVariableItem
{
    Q_OBJECT

public:
    enum FormatTypes { natural, hexadecimal, decimal, character, binary };

    /** Creates top-level variable item from the specified expression.
        Optionally, alternative display name can be provided.
    */
    VariableItem(AbstractVariableItem* parent);

    void setExpression(const QString& expression);
    void setFrozen(bool frozen = true);

    virtual void setVariableObject(const GDBMI::Value& varobj, FormatTypes format, bool baseClassMember);

    virtual void registerWithGdb();

    /** Recursively clears the varobjName_ field, making
       *this completely disconnected from gdb.
       Automatically makes *this and children disables,
       since there's no possible interaction with unhooked
       object.
    */
    virtual void deregisterWithGdb();

    virtual Qt::ItemFlags flags(int column) const;
    virtual QVariant data(int column, int role = Qt::DisplayRole ) const;
    virtual bool hasChildren() const;

    /// Returns the gdb expression for *this.
    QString gdbExpression() const;

    /** Returns true is this VarItem should be unconditionally
        updated on each step, not matter what's the result of
        -var-update command.
    */
    bool updateUnconditionally() const;

    /**
     * Retrieve the value for this object from GDB
     */
    virtual void updateValue();

    /**
     * Search for and create children if not already performed.
     */
    virtual void refreshChildren();

    /** Mark the variable as alive, or not alive.
        Variables that are not alive a shown as "gray",
        and nothing can be done about them except for
        removing. */
    void setAliveRecursively(bool enable);

    FormatTypes format() const;
    void setFormat(FormatTypes f);
    FormatTypes formatFromGdbModifier(char c) const;

    /** Clears highliting for this variable and
        all its children. */
    void clearHighlight();
    void setHighlight(bool highlight = true);
    bool highlight();

    /** Sets new top-level textual value of this variable.
    */
    void setValue(const QString& new_value);

    const QString& type() const;

    bool isAlive() const;

    VariableItem* parentItem() const;

    void setVariableName(const QString& name);
    const QString& variableName() const;

    static QString nextVariableObjectName();

Q_SIGNALS:
    /**
     * Emitted whenever the name of varobj associated with *this changes:
     *
     * Either 'from' or 'to' can be empty string.
     */
    void varobjNameChange(const QString& from, const QString& to);

private:

    /** Creates new gdb "variable object". The controller_,
        expression_ and format_ member variables should already
        be set.
     */
    void createVarobj();

    /** Precondition: 'name' is a name of existing
        gdb variable object.
        Effects:
           - sets varobjName_ to 'name'
           - sets format, if it's not default one
           - gets initial value
           - if item is open, gets children.
    */
    void setVarobjName(const QString& name);

    void varobjCreated(const GDBMI::ResultRecord& r);
    void valueDone(const GDBMI::ResultRecord& r);
    void childrenDone(const GDBMI::ResultRecord& r);
    void childrenOfFakesDone(const GDBMI::ResultRecord& r);

    void createChildren(const GDBMI::ResultRecord& r, bool children_of_fake);

    // Assuming 'expression_' is already set, returns the
    // displayName to use when showing this to the user.
    // This function exists because if we have item with
    // gdb expression '$1' and displayName 'P4', we want the child
    // to show up as *P4, not as '*$1', so we can't uncondionally
    // use expression gdb reports to us.
    QString displayName() const;

    QString varobjFormatName() const;

private:
    // The gdb expression for this varItem relatively to
    // parent VarItem.
    QString expression_;

    bool      highlight_;

    QString varobjName_;

    // the non-cast type of the variable
    QString originalValueType_;

    FormatTypes format_;

    static int varobjIndex;

    int numChildren_;

    QString currentAddress_;
    QString lastObtainedAddress_;

    bool frozen_;

    /* Set to true whan calling createVarobj for the
       first time, and to false other time. */
    bool initialCreation_;

    /* Set if this VarItem corresponds to base class suboject.  */
    bool baseClassMember_;

    bool alive_;

    QString m_value, m_type;
};

}

#endif
