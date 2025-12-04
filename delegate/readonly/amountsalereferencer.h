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

#ifndef AMOUNTSALEREFERENCER_H
#define AMOUNTSALEREFERENCER_H

#include "component/using.h"
#include "delegate/styleditemdelegate.h"
#include "enum/section.h"

class AmountSaleReferenceR final : public StyledItemDelegate {
    Q_OBJECT

signals:
    void SSaleReferencePrimary(const QUuid& node_id, int unit);

public:
    AmountSaleReferenceR(Section section, const int& decimal, const int& unit, CIntString& unit_symbol_map, QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    QString Format(const QModelIndex& index) const;

private:
    const int& decimal_ {};
    const int& unit_ {};
    const Section section_ {};
    CIntString& unit_symbol_map_;
};

#endif // AMOUNTSALEREFERENCER_H
