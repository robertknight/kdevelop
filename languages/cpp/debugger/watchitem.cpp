/*
 * GDB Debugger Support
 *
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

#include "watchitem.h"

#include <QVariant>

#include <KLocale>

using namespace GDBDebugger;

WatchItem::WatchItem(VariableCollection* parent)
    : AbstractVariableItem(parent)
{
}

QVariant WatchItem::data(int column, int role) const
{
    switch (role) {
        case Qt::DisplayRole:
            switch (column) {
                case ColumnName:
                    return i18n("Watches");
            }
    }

    return QVariant();
}

RecentItem::RecentItem(VariableCollection* parent)
    : AbstractVariableItem(parent)
{
}

QVariant RecentItem::data(int column, int role) const
{
    switch (role) {
        case Qt::DisplayRole:
            switch (column) {
                case ColumnName:
                    return i18n("Recent");
            }
    }

    return QVariant();
}

#include "watchitem.moc"
