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

#include <QAbstractItemView>
#include <QSettings>
#include <QTableView>

#include "component/using.h"

template <typename T>
concept InheritQAbstractItemView = std::is_base_of_v<QAbstractItemView, T>;

template <typename T>
concept InheritQWidget = std::is_base_of_v<QWidget, T>;

template <typename T>
concept MemberFunction = std::is_member_function_pointer_v<T>;

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
        auto widget { it.value() };

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
