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

#include "component/constant.h"
#include "component/enumclass.h"
#include "concepts.h"
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
    // Get the singleton instance
    static EntryShadowPool& Instance();

    // Allocate a single EntryShadow object from the pool corresponding to the given section.
    EntryShadow* Allocate(Section section);

    // Recycle a single EntryShadow object back to its pool.
    void Recycle(EntryShadow* entry, Section section);

    // Recycle multiple EntryShadow objects in a container back to their pool.
    // Optimized: acquires the lock only once for all elements.
    template <Iterable Container> void Recycle(Container& container, Section section);

    // Delete copy and move constructors/operators
    EntryShadowPool(const EntryShadowPool&) = delete;
    EntryShadowPool& operator=(const EntryShadowPool&) = delete;
    EntryShadowPool(EntryShadowPool&&) = delete;
    EntryShadowPool& operator=(EntryShadowPool&&) = delete;

private:
    EntryShadowPool();
    ~EntryShadowPool();

    // Factory function to create a new EntryShadow instance for a given section
    static EntryShadow* NewResource(Section section);

    // Expand the given pool by creating 'count' new EntryShadow objects
    void Expand(std::deque<EntryShadow*>& pool, Section section, qsizetype count);

private:
    std::array<std::deque<EntryShadow*>, 6> pools_ {}; // Pools for each Section
    QMutex mutex_ {}; // Mutex protecting all pools
};

inline EntryShadowPool& EntryShadowPool::Instance()
{
    static EntryShadowPool instance;
    return instance;
}

inline EntryShadowPool::EntryShadowPool()
{
    for (size_t i = 0; i != kSections.size(); ++i) {
        Expand(pools_[i], kSections[i], Pool::kExpandSize);
    }
}

inline EntryShadowPool::~EntryShadowPool()
{
    for (auto& pool : pools_) {
        qDeleteAll(pool); // Delete all remaining EntryShadow objects
    }
}

inline void EntryShadowPool::Expand(std::deque<EntryShadow*>& pool, Section section, qsizetype count)
{
    for (qsizetype i = 0; i != count; ++i) {
        pool.push_back(NewResource(section));
    }
}

inline EntryShadow* EntryShadowPool::NewResource(Section section)
{
    switch (section) {
    case Section::kFinance:
        return new EntryShadowF();
    case Section::kItem:
        return new EntryShadowI();
    case Section::kTask:
        return new EntryShadowT();
    case Section::kPartner:
        return new EntryShadowP();
    case Section::kSale:
        return new EntryShadowO();
    case Section::kPurchase:
        return new EntryShadowO();
    default:
        Q_UNREACHABLE();
    }
}

inline EntryShadow* EntryShadowPool::Allocate(Section section)
{
    QMutexLocker locker(&mutex_);
    auto& pool = pools_[static_cast<size_t>(section)];

    if (pool.empty()) {
        Expand(pool, section, Pool::kExpandSize);
    }

    if (pool.empty()) {
        return NewResource(section);
    }

    EntryShadow* es = pool.front();
    pool.pop_front();

    return es;
}

inline void EntryShadowPool::Recycle(EntryShadow* entry, Section section)
{
    if (!entry)
        return;

    QMutexLocker locker(&mutex_);
    auto& pool = pools_[static_cast<size_t>(section)];

    // If pool exceeds threshold, delete entry instead of recycling
    if (static_cast<qsizetype>(pool.size()) + 1 >= Pool::kMaxSize) {
        locker.unlock();
        delete entry;
        return;
    }

    entry->ResetState();
    pool.push_back(entry);
}

template <Iterable Container> inline void EntryShadowPool::Recycle(Container& container, Section section)
{
    if (container.isEmpty())
        return;

    QMutexLocker locker(&mutex_);
    auto& pool = pools_[static_cast<size_t>(section)];

    if (static_cast<qsizetype>(pool.size()) + container.size() >= Pool::kMaxSize) {
        locker.unlock();
        qDeleteAll(container);
        return;
    }

    for (EntryShadow* es : container) {
        if (es) {
            es->ResetState();
            pool.push_back(es);
        }
    }

    container.clear();
}

#endif // ENTRYSHADOWPOOL_H
