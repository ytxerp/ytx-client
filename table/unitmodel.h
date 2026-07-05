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

#ifndef UNITMODEL_H
#define UNITMODEL_H

#include <QAbstractItemModel>
#include <QString>
#include <QUuid>

class UnitModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit UnitModel(QObject* parent = nullptr);

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

    void AppendItem(const QString& display, const int& user);

protected:
    struct Item {
        QString display {};
        int user {};
    };

private:
    QList<Item> items_ {};
};

#endif // UNITMODEL_H
