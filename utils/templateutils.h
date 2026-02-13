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

#ifndef TEMPLATEUTILS_H
#define TEMPLATEUTILS_H

#include <QAbstractItemView>
#include <QHeaderView>
#include <QSettings>
#include <QTableView>

#include "component/using.h"
#include "global/collator.h"
#include "tree/itemmodel.h"

template <typename T>
concept InheritQAbstractItemView = std::is_base_of_v<QAbstractItemView, T>;

template <typename T>
concept InheritQWidget = std::is_base_of_v<QWidget, T>;

template <typename T>
concept MapType = std::is_same_v<typename T::mapped_type, QString> && (std::is_same_v<typename T::key_type, int> || std::is_same_v<typename T::key_type, bool>);

namespace Utils {

template <typename Obj, typename T> inline bool CompareMember(const Obj* lhs, const Obj* rhs, const T Obj::* member, Qt::SortOrder order)
{
    Q_ASSERT(lhs != nullptr);
    Q_ASSERT(rhs != nullptr);

    if constexpr (std::is_same_v<T, QString>) {
        // QString comparison using collator
        const auto& collator = Collator::Instance();
        const int r = collator.compare(lhs->*member, rhs->*member);
        return (order == Qt::AscendingOrder) ? (r < 0) : (r > 0);
    } else {
        // Regular comparison for other types
        return (order == Qt::AscendingOrder) ? (lhs->*member < rhs->*member) : (lhs->*member > rhs->*member);
    }
}

template <typename Obj, typename T> inline bool CompareShadowMember(const Obj* lhs, const Obj* rhs, T* Obj::* member, Qt::SortOrder order)
{
    Q_ASSERT(lhs != nullptr);
    Q_ASSERT(rhs != nullptr);

    const T* lp = lhs->*member;
    const T* rp = rhs->*member;

    Q_ASSERT(lp != nullptr);
    Q_ASSERT(rp != nullptr);

    if constexpr (std::is_same_v<T, QString>) {
        const auto& collator = Collator::Instance();
        const int r = collator.compare(*lp, *rp);
        return (order == Qt::AscendingOrder) ? (r < 0) : (r > 0);
    } else {
        return (order == Qt::AscendingOrder) ? (*lp < *rp) : (*lp > *rp);
    }
}

template <typename Derived, typename Base> Derived* DerivedPtr(Base* base)
{
    Q_ASSERT(base != nullptr);
    return static_cast<Derived*>(base);
}

template <typename Derived, typename Base> const Derived* DerivedPtr(const Base* base)
{
    Q_ASSERT(base != nullptr);
    return static_cast<const Derived*>(base);
}

template <MapType T> ItemModel* CreateModelFromMap(const T& map, QObject* parent)
{
    auto* model { new ItemModel(parent) };

    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        model->AppendItem(it.value(), it.key());
    }

    model->sort(0);
    return model;
}

template <InheritQAbstractItemView T> void SetupVerticalHeader(T* view, int row_height)
{
    if (!view)
        return;

    auto* v_header = view->verticalHeader();
    if (!v_header)
        return;

    v_header->setDefaultSectionSize(row_height);
    v_header->setSectionResizeMode(QHeaderView::Fixed);
    v_header->setHidden(true);
}

template <InheritQAbstractItemView T> bool HasSelection(T* view) { return view && view->selectionModel() && view->selectionModel()->hasSelection(); }

inline void WriteConfig(const QSharedPointer<QSettings>& settings, const QVariant& value, CString& section, CString& property)
{
    Q_ASSERT(settings);
    settings->setValue(section % "/" % property, value);
}

template <InheritQWidget Widget, typename Function, typename... Args>
void WriteConfig(Widget* widget, Function getter, const QSharedPointer<QSettings>& settings, CString& section, CString& property, Args&&... args)
{
    static_assert(std::is_invocable_r_v<QByteArray, Function, Widget*, Args...>, "Getter must return QByteArray");
    Q_ASSERT(widget && settings);

    settings->setValue(section % "/" % property, std::invoke(getter, widget, std::forward<Args>(args)...));
}

template <InheritQWidget Widget, typename Function, typename... Args>
void ReadConfig(Widget* widget, Function setter, const QSharedPointer<QSettings>& settings, CString& section, CString& property, Args&&... args)
{
    static_assert(std::is_invocable_r_v<bool, Function, Widget*, QByteArray, Args...>, "Setter must accept QByteArray and return bool");
    Q_ASSERT(widget && settings);

    if (auto value = settings->value(section % "/" % property).toByteArray(); !value.isEmpty()) {
        std::invoke(setter, widget, value, std::forward<Args>(args)...);
    }
}

}

#endif // TEMPLATEUTILS_H
