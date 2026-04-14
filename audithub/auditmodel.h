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

#ifndef AUDITMODEL_H
#define AUDITMODEL_H

#include <QAbstractItemModel>
#include <QJsonArray>

#include "auditentry.h"
#include "auditinfo.h"

namespace audit_hub {

class AuditModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit AuditModel(const AuditInfo& info, QObject* parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override
    {
        Q_UNUSED(index)
        return QModelIndex();
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return list_.size();
    }
    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return info_.header.size();
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order) override;

    void ResetModel(const QJsonArray& array);

private:
    const QString& NodePath(const QHash<QUuid, QString>* leaf, const QHash<QUuid, QString>* branch, const QUuid& node_id) const;
    QVariant ResolveNode(const AuditEntry* entry, const QUuid& node_id) const;
    static QString JsonValueToString(const QJsonValue& value);
    static QString FormatArray(const QByteArray& json_data)
    {
        QString result { QString::fromUtf8(json_data) };

        // Replace "},{" with "},\n{" for better readability
        result.replace("},{", "}\n{");

        // Remove outer brackets
        if (result.startsWith('['))
            result.remove(0, 1);
        if (result.endsWith(']'))
            result.chop(1);

        return result.trimmed();
    }

private:
    QList<AuditEntry*> list_ {};
    const AuditInfo& info_; // owned by MainWindow, outlives model
};
}

#endif // AUDITMODEL_H
