/***************************************************************************
    begin                : Mon Sep 20 1999
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

#ifndef PROCESSSELECTION_H
#define PROCESSSELECTION_H

#include <kdialog.h>

namespace Ui
{
class ProcessSelection;
}

namespace KDevelop
{
class CommandExecutor;
}

class QFocusEvent;

namespace GDBDebugger
{

/***************************************************************************/

class ProcessSelectionDialog : public KDialog
{
    Q_OBJECT

public:
    ProcessSelectionDialog( QWidget *parent=0 );
    ~ProcessSelectionDialog();

    int pidSelected();

private Q_SLOTS:
    void slotReceivedOutput(const QStringList&);
    void slotProcessExited();

protected:
    void focusIn(QFocusEvent*);

private:
    KDevelop::CommandExecutor* psProc;
    QString   pidCmd;
    Ui::ProcessSelection* m_ui;
};

}

#endif
