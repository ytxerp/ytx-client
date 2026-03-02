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

#ifndef BOOLCOLORSTRINGR_H
#define BOOLCOLORSTRINGR_H

#include <QEvent>

#include "delegate/styleditemdelegate.h"

class BoolColorStringR final : public StyledItemDelegate {
public:
    explicit BoolColorStringR(CBoolString& map, const QString& true_color, const QString& false_color, QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    CBoolString& map_;
    CString& true_color_;
    CString& false_color_;
};

#endif // BOOLCOLORSTRINGR_H
