/*
 * GDB Debugger Support
 *
 * Copyright 2003 John Birch <jbb@kdevelop.org>
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

#ifndef _BREAKPOINT_H_
#define _BREAKPOINT_H_

#include <klocale.h>

#include <QObject>
#include <QString>
#include <QStringList>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace GDBMI
{
    class ResultRecord;
}

namespace GDBDebugger
{

    class GDBController;
    class BreakpointController;

enum BP_TYPES
{
    BP_TYPE_Invalid,
    BP_TYPE_FilePos,
    BP_TYPE_Watchpoint,
    BP_TYPE_ReadWatchpoint
};

class Breakpoint : public QObject
{
    Q_OBJECT
public:
    Breakpoint(BreakpointController* controller, bool temporary=false, bool enabled=true);
    virtual ~Breakpoint();

    BreakpointController* breakpointController() const;
    GDBController* controller() const;

    void sendToGdb();

    // Called whenever this breakpoint is removed on gdb side.
    virtual void removedInGdb();

    virtual void applicationExited();



    virtual QString dbgSetCommand() const = 0;
    virtual QString dbgRemoveCommand() const;
    /** Returns true if 'breakpoint' is identical to *this.
        Checks for trival cases like pointer equality and
        differing typeid() and then calls virtual 
        match_data.
    */
    bool match(const Breakpoint* breakpoint) const;
    /** Returns true if essential data in 'breakpoint' is equivalent
        to *this. The caller should guarantee that dynamic type
        of *this and *breakpoint is the same.
    */
    virtual bool match_data(const Breakpoint* breakpoint) const = 0;

    virtual bool hasFileAndLine() const { return false; }


    virtual void reset();

    virtual void remove();

    void notifyModified();

    void setActive(int active, int id);
    bool isActive(int active) const                 { return (active_ == active) ||
                                                        (s_pending_ && !s_actionClear_); }

    void setEnabled(bool enabled);
    bool isEnabled() const                          { return s_enabled_; }

    void setTemporary(bool temporary)               { s_temporary_ = temporary; }
    bool isTemporary() const                        { return s_temporary_; }

    void setHardwareBP(bool hardwareBP)             { s_hardwareBP_ = hardwareBP; }
    bool isHardwareBP() const                       { return s_hardwareBP_; }

    void setIgnoreCount(int ignoreCount)            { ignoreCount_ = ignoreCount; }
    int ignoreCount() const                         { return ignoreCount_; }

    void setAddress(const QString &address)         { address_ = address; }
    QString address() const                         { return address_; }

    void setConditional(const QString &condition)   { condition_ = condition; }
    QString conditional() const                     { return condition_; }

    void setPending(bool pending)                   { s_pending_ = pending; }
    bool isPending() const                          { return s_pending_; }

    void setActionAdd(bool actionAdd)               { s_actionDie_ = false;
                                                      s_actionAdd_ = actionAdd; }
    bool isActionAdd() const                        { return s_actionAdd_; }

    void setActionClear(bool actionClear)           { s_actionClear_ = actionClear; }
    bool isActionClear() const                      { return s_actionClear_; }

    void setActionModify(bool actionModify)         { s_actionDie_ = false;
                                                      s_actionModify_ = actionModify; }
    bool isActionModify() const                     { return s_actionModify_; }

    void setDbgProcessing(bool dbgProcessing)       { s_dbgProcessing_ = dbgProcessing; }
    bool isDbgProcessing() const                    { return s_dbgProcessing_; }
    void setActionDie()                             { s_actionDie_ = true;
                                                      s_actionClear_ = false; }
    bool isActionDie() const                        { return s_actionDie_; }

    int key() const                                 { return key_; }
    void setDbgId(int dbgId)                        { dbgId_ = dbgId; }
    int  dbgId() const                              { return dbgId_; }
    void setHits(int hits)                          { hits_ = hits; }
    int  hits() const                               { return hits_; }

    virtual QString statusDisplay(int activeFlag) const;
    virtual BP_TYPES type() const                   { return BP_TYPE_Invalid; }
    virtual QString displayType() const             { return i18n( "Invalid" ); }


    bool tracingEnabled() const                     { return s_tracingEnabled_; }
    void setTracingEnabled(bool enable)             { s_tracingEnabled_ = enable; }

    const QStringList& tracedExpressions() const    { return tracedExpressions_; }
    void setTracedExpressions(const QStringList& l) { tracedExpressions_ = l; }

    bool traceFormatStringEnabled() const           { return s_traceFormatStringEnabled_; }
    void setTraceFormatStringEnabled(bool en)       { s_traceFormatStringEnabled_ = en; }

    const QString& traceFormatString() const        { return traceFormatString_; }
    void setTraceFormatString(const QString& s);

    QString traceRealFormatString() const;  

    virtual QString location(bool compact=true) const = 0;
    virtual void setLocation(const QString& )       = 0;
    virtual bool isValid() const                    = 0;

Q_SIGNALS:
    /** Emitted whenever this breakpoint is modified from gdb side,
        say when it's first created, or when gdb reports that any
        property has changes.
    */
    void modified(Breakpoint*);
    void enabledChanged(Breakpoint*);

private:
    void handleDeleted(const GDBMI::ResultRecord&);
    virtual void setBreakpoint();
    void modifyBreakpoint();

protected:
    virtual void handleSet(const GDBMI::ResultRecord&);
    void clearBreakpoint();

private:
    bool s_pending_             :1;
    bool s_actionAdd_           :1;
    bool s_actionClear_         :1;
    bool s_actionModify_        :1;
    bool s_actionDie_           :1;
    bool s_dbgProcessing_       :1;
    bool s_enabled_             :1;
    bool s_temporary_           :1;
    bool s_hardwareBP_          :1;     // assigned by gdb
    bool s_tracingEnabled_      :1;
    bool s_traceFormatStringEnabled_ :1;

    int dbgId_;                         // assigned by gdb
    int hits_;                          // assigned by gdb

    int key_;                           // internal unique key
    int active_;                        // counter incremented on receipt of all BP's

    int ignoreCount_;
    QString address_;
    QString condition_;
    QStringList tracedExpressions_;
    QString traceFormatString_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class FilePosBreakpoint : public Breakpoint
{
    Q_OBJECT

public:
    FilePosBreakpoint(BreakpointController* controller);

    FilePosBreakpoint(BreakpointController* controller, const QString &fileName, int lineNum,
                      bool temporary=false, bool enabled=true);
    virtual ~FilePosBreakpoint();
    virtual QString dbgSetCommand() const;
    virtual bool match_data(const Breakpoint *brkpt) const;

    BP_TYPES type () const                      { return BP_TYPE_FilePos; }
    QString displayType() const;
    QString location(bool compact=true) const;
    void setLocation(const QString& location);
    bool isValid() const;

    bool hasFileAndLine() const;
    QString fileName() const;
    int lineNum() const;

protected:
    void handleSet(const GDBMI::ResultRecord&);


private:

    enum subtype { filepos = 1, function, address };
    subtype subtype_;

    QString location_;
    QString fileName_;
    int line_;
};

class Watchpoint : public Breakpoint
{
    Q_OBJECT

public:
    Watchpoint(BreakpointController* controller, const QString &varName, bool temporary=false, bool enabled=true);
    virtual ~Watchpoint();
    virtual QString dbgSetCommand() const;

    void applicationExited();
    void removedInGdb();
 
    bool match_data(const Breakpoint *brkpt) const;

    BP_TYPES type () const                      { return BP_TYPE_Watchpoint; }
    QString displayType() const                 { return i18n("Watchpoint"); }
    void setVarName(const QString& varName)     { varName_ = varName; }
    QString varName() const                     { return varName_; }
    quint64 address() const          { return address_; }
    QString location(bool) const                { return varName_; }
    void setLocation(const QString& location)   { varName_ = location; }
    bool isValid() const                        { return !varName_.isEmpty(); }

private:
    void setBreakpoint();
    void handleAddressComputed(const GDBMI::ResultRecord&);

    QString varName_;
    quint64 address_;
};

class ReadWatchpoint : public Watchpoint
{
    Q_OBJECT

public:
    ReadWatchpoint(BreakpointController* controller, const QString &varName, bool temporary=false, bool enabled=true);
    virtual QString dbgSetCommand() const;
    bool match_data(const Breakpoint *brkpt) const;

    BP_TYPES type () const                      { return BP_TYPE_ReadWatchpoint; }
    QString displayType() const                 { return i18n("Read Watchpoint"); }
};

}

#endif
