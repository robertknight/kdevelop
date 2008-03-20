/***************************************************************************
 *   Copyright 2005 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef VIEWER_H
#define VIEWER_H

#include <QStack>

#include "viewerbase.h"

namespace AutoMake { class ProjectAST; }

class Q3ListViewItem;

class Viewer: public QWidget, private Ui::ViewerBase
{
Q_OBJECT
public:
    Viewer(QWidget *parent = 0);
    void processAST(AutoMake::ProjectAST *projectAST, Q3ListViewItem *globAfter = 0);
public slots:
    virtual void tabWidget2_selected(const QString&);
    virtual void on_files_currentChanged(Q3ListBoxItem*);
    virtual void on_choose_clicked();
    virtual void on_addAll_clicked();
private:
    AutoMake::ProjectAST *projectAST;
    QStack<Q3ListViewItem *> parentProject;
};

#endif
