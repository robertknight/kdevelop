/***************************************************************************
    begin                : Tue Oct 5 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MEMVIEW_H_
#define _MEMVIEW_H_

#include "mi/gdbmi.h"

#include <kdialog.h>

#include <QContextMenuEvent>

#include "gdbglobal.h"

class KLineEdit;
class QToolBox;

namespace GDBDebugger
{
    class MemoryView;
    class GDBController;
    class CppDebuggerPlugin;

    class ViewerWidget : public QWidget
    {
        Q_OBJECT
    public:
        ViewerWidget(CppDebuggerPlugin* plugin, GDBController* controller,
                     QWidget* parent);

    public Q_SLOTS:
        /** Adds a new memory view to *this, initially showing
            no data. */
        void slotAddMemoryView();
        /** Informs *this about change in debugger state. Should always
            be connected to, so that *this can disable itself when
            debugger is not running. */
        void slotStateChanged(DBGStateFlags oldState, DBGStateFlags newState);

    Q_SIGNALS:
        void requestRaise();
        void setViewShown(bool shown);


    private Q_SLOTS:
        void slotChildCaptionChanged(const QString& caption);
        void slotChildDestroyed(QObject* child);

    private: // Data
        CppDebuggerPlugin* m_plugin;
        GDBController* controller_;
        QToolBox* toolBox_;
        QList<MemoryView*> memoryViews_;
    };

    class MemoryView : public QWidget
    {
        Q_OBJECT
    public:
        MemoryView(CppDebuggerPlugin* plugin, GDBController* controller,
                   QWidget* parent);

        void debuggerStateChanged(DBGStateFlags state);

    Q_SIGNALS:
        void captionChanged(const QString& caption);

    private: // Callbacks
        void sizeComputed(const QString& value);

        void memoryRead(const GDBMI::ResultRecord& r);

    private Q_SLOTS:
        void memoryEdited(int start, int end);

    private:
        // Returns true is we successfully created the hexeditor, and so
        // can work.
        bool isOk() const;



    private Q_SLOTS:
        /** Invoked when user has changed memory range.
            Gets memory for the new range. */
        void slotChangeMemoryRange();
        void slotHideRangeDialog();
        void slotEnableOrDisable();

    private: // QWidget overrides
        void contextMenuEvent(QContextMenuEvent* e);

    private:
        void initWidget();

    private:
        GDBController* controller_;
        class MemoryRangeSelector* rangeSelector_;
        QWidget* khexedit2_widget;
        QWidget* khexedit2_real_widget;

        uint start_, amount_;
        QString startAsString_, amountAsString_;
        char* data_;

        int debuggerState_;
    };
}

#endif
