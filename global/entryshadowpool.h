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
#include <array>
#include <deque>

#include "component/constantint.h"
#include "concepts.h"
#include "enum/section.h"
#include "table/entryshadow.h"

/*
 * EntryShadowPool is a singleton object pool for EntryShadow-derived objects.
 *
 * Features:
 * 1. Thread-safe allocation and recycling using QMutex.
 * 2. Supports pre-allocation and dynamic expansion of pools.
 * 3. Avoids pool overgrowth by deleting objects when a threshold is exceeded.
 * 4. Batch recycling is optimized: only one lock acquisition for all entries.
 * 5. Modern C++ style using std::array for fixed-size section pools.
 */
class EntryShadowPool {
public:
    static EntryShadowPool& Instance();
    EntryShadow* Allocate(Section section);

    void Recycle(EntryShadow* entry_shadow, Section section);
    template <Iterable Container> void Recycle(Container& container, Section section);

    EntryShadowPool(const EntryShadowPool&) = delete;
    EntryShadowPool& operator=(const EntryShadowPool&) = delete;
    EntryShadowPool(EntryShadowPool&&) = delete;
    EntryShadowPool& operator=(EntryShadowPool&&) = delete;

private:
    EntryShadowPool();
    ~EntryShadowPool();

    static EntryShadow* NewResource(Section section);
    static void Expand(std::deque<EntryShadow*>& pool, Section section, qsizetype count);

private:
    std::array<std::deque<EntryShadow*>, 3> pools_ {}; // Pools for each Section
    QMutex mutex_ {}; // Mutex protecting all pools
};

// Singleton
inline EntryShadowPool& EntryShadowPool::Instance()
{
    static EntryShadowPool instance;
    return instance;
}

// Constructor: pre-fill pools
inline EntryShadowPool::EntryShadowPool()
{
    for (size_t i = 0; i != kDoubleSectionArray.size(); ++i) {
        Expand(pools_[i], kDoubleSectionArray[i], pool_const::kExpandSize);
    }
}

// Destructor: delete remaining entries
inline EntryShadowPool::~EntryShadowPool() { }

// Expand pool with 'count' new entries
inline void EntryShadowPool::Expand(std::deque<EntryShadow*>& pool, Section section, qsizetype count)
{
    for (qsizetype i = 0; i != count; ++i) {
        pool.push_back(NewResource(section));
    }
}

// Factory function: create a new entry based on section
inline EntryShadow* EntryShadowPool::NewResource(Section section)
{
    switch (section) {
    case Section::kFinance:
        return new EntryShadowF();
    case Section::kTask:
    case Section::kInventory:
        return new EntryShadow();
    case Section::kPartner:
    case Section::kSale:
    case Section::kPurchase:
        Q_UNREACHABLE();
    }
}

// Allocate one entry
inline EntryShadow* EntryShadowPool::Allocate(Section section)
{
    QMutexLocker locker(&mutex_);
    auto& pool = pools_[static_cast<size_t>(section)];

    if (pool.empty()) {
        Expand(pool, section, pool_const::kExpandSize);
    }

    Q_ASSERT(!pool.empty());

    EntryShadow* e = pool.front();
    pool.pop_front();

    return e;
}

// Recycle one entry
inline void EntryShadowPool::Recycle(EntryShadow* entry_shadow, Section section)
{
    if (!entry_shadow)
        return;

    QMutexLocker locker(&mutex_);
    auto& pool = pools_[static_cast<size_t>(section)];

    if (static_cast<qsizetype>(pool.size()) + 1 >= pool_const::kMaxSize) {
        locker.unlock();
        delete entry_shadow;
        return;
    }

    entry_shadow->Reset();
    pool.push_back(entry_shadow);
}

// Batch recycle
template <Iterable Container> inline void EntryShadowPool::Recycle(Container& container, Section section)
{
    if (container.isEmpty())
        return;

    QMutexLocker locker(&mutex_);
    auto& pool = pools_[static_cast<size_t>(section)];

    if (static_cast<qsizetype>(pool.size()) + container.size() >= pool_const::kMaxSize) {
        locker.unlock();
        qDeleteAll(container);
        container.clear();
        return;
    }

    for (EntryShadow* e : container) {
        if (e) {
            e->Reset();
            pool.push_back(e);
        }
    }

    container.clear();
}

#endif // ENTRYSHADOWPOOL_H
