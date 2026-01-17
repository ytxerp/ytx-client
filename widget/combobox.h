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

#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QCompleter>

class ComboBox final : public QComboBox {
public:
    explicit ComboBox(QWidget* parent = nullptr)
        : QComboBox { parent }
    {
        setEditable(true);
        setInsertPolicy(QComboBox::NoInsert);

        auto* completer { new QCompleter(model(), this) };
        completer->setFilterMode(Qt::MatchContains);
        completer->setCaseSensitivity(Qt::CaseInsensitive);

        setCompleter(completer);
        setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    }

protected:
    void showPopup() override
    {
        // Get the popup view
        QAbstractItemView* popup_view = view();
        if (!popup_view)
            return;

        // Force layout update
        popup_view->doItemsLayout();

        // Get ComboBox width as minimum
        const int combobox_width { width() };

        // Calculate content width
        int content_width { popup_view->sizeHintForColumn(0) };
        if (content_width <= 0) {
            content_width = combobox_width;
        }

        // Account for frame border
        const int frame_width { popup_view->frameWidth() * 2 };

        // Add scrollbar width if it will be visible
        const int scrollbar_width { QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) };

        // Calculate final width with all components
        const int suggested_width { content_width + frame_width + scrollbar_width };
        const int final_width { qBound(combobox_width, suggested_width, combobox_width * 4) };

        // Apply width to popup
        popup_view->setFixedWidth(final_width);

        // Call base implementation to show popup
        QComboBox::showPopup();
    }

    QSize sizeHint() const override
    {
        QSize sz { QComboBox::sizeHint() };
        const int scrollbar_width { QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) };
        sz.setWidth(sz.width() + scrollbar_width);
        return sz;
    }
};

#endif // COMBOBOX_H
