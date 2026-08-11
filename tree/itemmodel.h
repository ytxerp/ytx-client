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

#pragma once

#include <QAbstractItemModel>
#include <QString>
#include <QUuid>

class ItemModel final : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit ItemModel(QObject* parent = nullptr);
    ~ItemModel() override;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QModelIndex parent(const QModelIndex&) const override { return {}; }
    int rowCount(const QModelIndex& parent = QModelIndex()) const override { return parent.isValid() ? 0 : list_.size(); }
    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return 1;
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override { return index.isValid() ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable) : Qt::NoItemFlags; }

    void AppendItem(const QString& display, const QUuid& id);
    bool RemoveItem(const QUuid& id);

    void SetSeparator(const QString& old_separator, const QString& new_separator);
    void SetDisplay(const QUuid& id, const QString& display);

    void Rebuild(const QHash<QUuid, QString>& leaf_path);
    void Reset();

protected:
    struct Item {
        QString display {};
        QUuid id {};

        void Reset() { *this = Item {}; };
    };

private:
    QList<Item*> list_ {};
    QHash<QUuid, Item*> hash_ {};
};
