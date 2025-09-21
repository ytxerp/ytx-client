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

#ifndef WIDGETUTILS_H
#define WIDGETUTILS_H

#include <QtWidgets/qtableview.h>

#include <QAbstractItemView>
#include <QSettings>

#include "component/enumclass.h"
#include "component/using.h"
#include "table/model/leafmodel.h"

template <typename T>
concept InheritQAbstractItemView = std::is_base_of_v<QAbstractItemView, T>;

template <typename T>
concept InheritQWidget = std::is_base_of_v<QWidget, T>;

template <typename T>
concept MemberFunction = std::is_member_function_pointer_v<T>;

template <typename T>
concept EntryWidgetLike = std::is_base_of_v<QWidget, T> && requires(T t) {
    { t.Model() } -> std::convertible_to<LeafModel*>;
    { t.View() } -> std::convertible_to<QTableView*>;
};

namespace WidgetUtils {

template <typename T> void SafeDelete(QPointer<T>& ptr)
{
    if (ptr) {
        delete ptr;
        ptr = nullptr;
    }
}

template <InheritQWidget T> void FreeWidget(const QUuid& node_id, QHash<QUuid, QPointer<T>>& hash)
{
    auto it = hash.constFind(node_id);
    if (it != hash.constEnd()) {
        auto widget = it.value();

        if (widget) {
            hash.erase(it);
            delete widget;
        }
    }
}

// QList<QPointer<T>>
template <InheritQWidget T> void ClearWidgets(QList<QPointer<T>>& list)
{
    for (auto& widget : list) {
        if (widget) {
            widget->close();
            delete widget;
        }
    }
    list.clear();
}

// QHash<QUuid, QPointer<T>>
template <InheritQWidget T> void ClearWidgets(QHash<QUuid, QPointer<T>>& hash)
{
    for (auto& widget : hash) {
        if (widget) {
            widget->close();
            delete widget;
        }
    }
    hash.clear();
}

template <InheritQAbstractItemView T> bool HasSelection(QPointer<T> view) { return view && view->selectionModel() && view->selectionModel()->hasSelection(); }

template <InheritQAbstractItemView T> bool HasSelection(T* view) { return view && view->selectionModel() && view->selectionModel()->hasSelection(); }

// For Finance / Item / Stakeholder / Task  (FIST) sections
// Behavior:
// - If no empty row exists: insert a new row and focus the IssuedTime column
//   (so the user can first input the entry time).
// - If an empty row exists: focus the RhsNode column
//   (so the user can quickly continue linking the related node).
template <EntryWidgetLike T> void AppendEntryFIST(T* widget, Section start)
{
    assert(widget);

    auto* model { widget->Model() };
    assert(model);

    const int empty_row = model->GetRhsRow({});

    QModelIndex target_index {};

    if (empty_row == -1) {
        const int new_row = model->rowCount();
        if (!model->insertRows(new_row, 1))
            return;

        target_index = model->index(new_row, std::to_underlying(EntryEnum::kIssuedTime));
    } else {
        const int rhs_col = EntryUtils::RhsNodeColumn(start);
        if (rhs_col != -1)
            target_index = model->index(empty_row, rhs_col);
    }

    if (target_index.isValid()) {
        widget->View()->setCurrentIndex(target_index);
    }
}

// For Order section
// Behavior:
// - If no empty row exists: insert a new row
// - Regardless of whether a new row is created or not,
//   always focus the RhsNode column (orders only care about related nodes).
template <EntryWidgetLike T> void AppendEntryO(T* widget, Section start)
{
    assert(widget);

    auto* model { widget->Model() };
    assert(model);

    const int empty_row = model->GetRhsRow({});

    if (empty_row == -1) {
        const int new_row = model->rowCount();
        if (!model->insertRows(new_row, 1))
            return;
    }

    const int rhs_col = EntryUtils::RhsNodeColumn(start);
    QModelIndex target_index {};

    if (rhs_col != -1)
        target_index = model->index(empty_row, rhs_col);

    if (target_index.isValid()) {
        widget->View()->setCurrentIndex(target_index);
    }
}

template <EntryWidgetLike T> void RemoveEntry(T* widget)
{
    assert(widget);

    auto* view { widget->View() };
    assert(view);

    if (!WidgetUtils::HasSelection(view))
        return;

    const QModelIndex current_index { view->currentIndex() };
    if (!current_index.isValid())
        return;

    auto* model { widget->Model() };
    assert(model);

    const int current_row { current_index.row() };
    if (!model->removeRows(current_row, 1)) {
        qDebug() << "Failed to remove row:" << current_row;
        return;
    }

    const int new_row_count { model->rowCount() };
    if (new_row_count == 0)
        return;

    QModelIndex new_index {};
    if (current_row < new_row_count) {
        new_index = model->index(current_row, 0);
    } else {
        new_index = model->index(new_row_count - 1, 0);
    }

    if (new_index.isValid()) {
        view->setCurrentIndex(new_index);
        view->closePersistentEditor(new_index);
    }
}

inline void WriteConfig(QSharedPointer<QSettings> settings, const QVariant& value, CString& section, CString& property)
{
    assert(settings);
    settings->setValue(QString("%1/%2").arg(section, property), value);
}

template <InheritQWidget Widget, MemberFunction Function, typename... Args>
void WriteConfig(Widget* widget, Function function, QSharedPointer<QSettings> settings, CString& section, CString& property, Args&&... args)
{
    static_assert(std::is_same_v<decltype((std::declval<Widget>().*function)(std::forward<Args>(args)...)), QByteArray>, "Function must return QByteArray");

    assert(widget);
    assert(settings);

    auto value { std::invoke(function, widget, std::forward<Args>(args)...) };
    settings->setValue(QString("%1/%2").arg(section, property), value);
}

template <InheritQWidget Widget, MemberFunction Function, typename... Args>
void ReadConfig(Widget* widget, Function function, QSharedPointer<QSettings> settings, CString& section, CString& property, Args&&... args)
{
    static_assert(std::is_same_v<decltype((std::declval<Widget>().*function)(std::declval<QByteArray>(), std::declval<Args>()...)), bool>,
        "Function must accept QByteArray and additional arguments, and return bool");

    assert(widget);
    assert(settings);

    auto value { settings->value(QString("%1/%2").arg(section, property)).toByteArray() };
    if (!value.isEmpty()) {
        std::invoke(function, widget, value, std::forward<Args>(args)...);
    }
}

} // namespace WidgetUtils

#endif // WIDGETUTILS_H
