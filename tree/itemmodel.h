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

#ifndef ITEMMODEL_H
#define ITEMMODEL_H

#include <QAbstractItemModel>
#include <QString>

class ItemModel final : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit ItemModel(QObject* parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QModelIndex parent(const QModelIndex&) const override { return {}; }
    int rowCount(const QModelIndex& parent = QModelIndex()) const override { return parent.isValid() ? 0 : items_.size(); }
    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return 1;
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override { return index.isValid() ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable) : Qt::NoItemFlags; }

    QVariant ItemData(int row, int role = Qt::DisplayRole) const;

    void AppendItem(const QString& display, const QVariant& user);
    void RemoveItem(int row);
    void UpdateSeparator(const QString& old_separator, const QString& new_separator);

    void SetDisplay(int row, const QString& display);
    void Clear();

protected:
    struct Item {
        QString display {};
        QVariant user {};
    };

private:
    QList<Item> items_ {};
};

#endif // ITEMMODEL_H
