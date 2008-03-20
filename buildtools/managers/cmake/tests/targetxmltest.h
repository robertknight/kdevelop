/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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
#ifndef TARGETXMLTEST_H
#define TARGETXMLTEST_H

#include <QtTest/QtTest>
#include "cmakexmlparser.h"

/**
 * @author Matt Rogers <mattr@kde.org>
 */
class TargetXmlTest: public QObject
{
Q_OBJECT
public:
    TargetXmlTest(QObject *parent = 0);
    ~TargetXmlTest();

private slots:
    void emptyTargetTest();
    void emptyTargetTest_data();
    
    void noSourcesTargetTest();
    void noSourcesTargetTest_data();
    
    void fullTargetTest();
    void fullTargetTest_data();

private:
    CMakeXmlParser m_parser;
};

#endif

