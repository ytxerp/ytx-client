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

#ifndef ENTRYSHADOWPOOL_H
#define ENTRYSHADOWPOOL_H

#include <QMutex>
#include <QVector>

#include "component/enumclass.h"
#include "concepts.h"
#include "table/entryshadow.h"

class EntryShadowPool {
public:
    static EntryShadowPool& Instance();

    EntryShadow* Allocate(Section section);
    void Recycle(EntryShadow* entry, Section section);
    template <Iterable Container> void Recycle(Container& container, Section section);

    EntryShadowPool(const EntryShadowPool&) = delete;
    EntryShadowPool& operator=(const EntryShadowPool&) = delete;
    EntryShadowPool(EntryShadowPool&&) = delete;
    EntryShadowPool& operator=(EntryShadowPool&&) = delete;

private:
    EntryShadowPool();
    ~EntryShadowPool();

    void Expand(QVector<EntryShadow*>& pool, Section section, int count);
    QVector<EntryShadow*>& GetPool(Section section);

    QVector<EntryShadow*> f_pool_;
    QVector<EntryShadow*> i_pool_;
    QVector<EntryShadow*> t_pool_;
    QVector<EntryShadow*> s_pool_;
    QVector<EntryShadow*> o_pool_; // 用于 kSale 和 kPurchase

    QMutex mutex_;

    static constexpr qsizetype kDefaultExpandSize { 100 };
    static constexpr qsizetype kShrinkThreshold { 1000 };
};

template <Iterable Container> inline void EntryShadowPool::Recycle(Container& container, Section section)
{
    if (container.isEmpty())
        return;

    auto& pool = GetPool(section);

    if (pool.size() + container.size() >= kShrinkThreshold) {
        qDeleteAll(container);
    } else {
        QMutexLocker locker(&mutex_);

        for (EntryShadow* resource : container) {
            if (resource) {
                resource->ResetState();
                pool.push_back(resource);
            }
        }
    }

    container.clear();
}

inline EntryShadowPool& EntryShadowPool::Instance()
{
    static EntryShadowPool instance;
    return instance;
}

inline EntryShadowPool::EntryShadowPool()
{
    Expand(f_pool_, Section::kFinance, kDefaultExpandSize);
    Expand(i_pool_, Section::kItem, kDefaultExpandSize);
    Expand(t_pool_, Section::kTask, kDefaultExpandSize);
    Expand(s_pool_, Section::kStakeholder, kDefaultExpandSize);
    Expand(o_pool_, Section::kSale, kDefaultExpandSize);
}

inline EntryShadowPool::~EntryShadowPool()
{
    qDeleteAll(f_pool_);
    qDeleteAll(i_pool_);
    qDeleteAll(t_pool_);
    qDeleteAll(s_pool_);
    qDeleteAll(o_pool_);
}

inline void EntryShadowPool::Expand(QVector<EntryShadow*>& pool, Section section, int count)
{
    for (int i = 0; i != count; ++i) {
        switch (section) {
        case Section::kFinance:
            pool.append(new EntryShadowF());
            break;
        case Section::kItem:
            pool.append(new EntryShadowI());
            break;
        case Section::kTask:
            pool.append(new EntryShadowT());
            break;
        case Section::kStakeholder:
            pool.append(new EntryShadowS());
            break;
        case Section::kSale:
        case Section::kPurchase:
            pool.append(new EntryShadowO());
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }
}

inline QVector<EntryShadow*>& EntryShadowPool::GetPool(Section section)
{
    switch (section) {
    case Section::kFinance:
        return f_pool_;
    case Section::kItem:
        return i_pool_;
    case Section::kTask:
        return t_pool_;
    case Section::kStakeholder:
        return s_pool_;
    case Section::kSale:
    case Section::kPurchase:
        return o_pool_;
    default:
        Q_UNREACHABLE();
    }
}

inline EntryShadow* EntryShadowPool::Allocate(Section section)
{
    QMutexLocker locker(&mutex_);

    auto& pool = GetPool(section);
    if (pool.isEmpty()) {
        Expand(pool, section, kDefaultExpandSize);
    }
    return pool.takeLast();
}

inline void EntryShadowPool::Recycle(EntryShadow* entry, Section section)
{
    if (!entry)
        return;

    auto& pool = GetPool(section);

    if (pool.size() >= kShrinkThreshold) {
        delete entry;
        return;
    }

    QMutexLocker locker(&mutex_);
    entry->ResetState();
    pool.append(entry);
}

#endif // ENTRYSHADOWPOOL_H
