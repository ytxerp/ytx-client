/*
 * Copyright (C) 2023 YTX
 *
 * This file is part of YTX.
 *
 * YTX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YTX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YTX. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef USING_H
#define USING_H

#include <QDialog>
#include <QHash>
#include <QMap>
#include <QString>

using CIntString = const QMap<int, QString>;
using CBoolString = const QMap<bool, QString>;

using CUuidString = const QHash<QUuid, QString>;
using UuidString = QHash<QUuid, QString>;
using CUuidSet = const QSet<QUuid>;

using CUuid = const QUuid;
using CString = const QString;
using CVariant = const QVariant;
using CStringList = const QStringList;
using CDateTime = const QDateTime;
using CJsonArray = const QJsonArray;
using CJsonObject = const QJsonObject;

#endif // USING_H
