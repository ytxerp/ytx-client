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

#ifndef NODEPOOL_H
#define NODEPOOL_H

#include <QMutex>
#include <array>
#include <deque>

#include "component/constant.h"
#include "concepts.h"
#include "enum/section.h"
#include "tree/node.h"

/*
 * NodePool is a singleton object pool for Node-derived objects.
 *
 * Features:
 * 1. Thread-safe allocation and recycling using QMutex.
 * 2. Supports pre-allocation and dynamic expansion of pools.
 * 3. Avoids pool overgrowth by deleting objects when a threshold is exceeded.
 * 4. Batch recycling is optimized: only one lock acquisition for all entries.
 * 5. Modern C++ style using std::array for fixed-size section pools.
 */
class NodePool {
public:
    // Get the singleton instance
    static NodePool& Instance();

    // Allocate a single Node object from the pool corresponding to the given section.
    Node* Allocate(Section section);

    // Recycle a single Node object back to its pool.
    void Recycle(Node* node, Section section);

    // Recycle multiple Node objects in a container back to their pool.
    // Optimized: acquires the lock only once for all elements.
    template <Iterable Container> void Recycle(Container& container, Section section);

    // Delete copy and move constructors/operators
    NodePool(const NodePool&) = delete;
    NodePool& operator=(const NodePool&) = delete;
    NodePool(NodePool&&) = delete;
    NodePool& operator=(NodePool&&) = delete;

private:
    NodePool();
    ~NodePool();

    // Factory function to create a new Node instance for a given section
    static Node* NewResource(Section section);

    // Expand the given pool by creating 'count' new Node objects
    static void Expand(std::deque<Node*>& pool, Section section, qsizetype count);

private:
    std::array<std::deque<Node*>, 6> pools_ {}; // Pools for each Section
    QMutex mutex_ {}; // Mutex protecting all pools
};

inline NodePool& NodePool::Instance()
{
    static NodePool instance;
    return instance;
}

inline NodePool::NodePool()
{
    for (size_t i = 0; i != kSectionArray.size(); ++i) {
        Expand(pools_[i], kSectionArray[i], Pool::kExpandSize);
    }
}

inline NodePool::~NodePool() { }

inline void NodePool::Expand(std::deque<Node*>& pool, Section section, qsizetype count)
{
    for (qsizetype i = 0; i != count; ++i) {
        pool.push_back(NewResource(section));
    }
}

inline Node* NodePool::NewResource(Section section)
{
    switch (section) {
    case Section::kFinance:
        return new NodeF();
    case Section::kInventory:
        return new NodeI();
    case Section::kTask:
        return new NodeT();
    case Section::kPartner:
        return new NodeP();
    case Section::kSale:
        return new NodeO();
    case Section::kPurchase:
        return new NodeO();
    default:
        Q_UNREACHABLE();
    }
}

inline Node* NodePool::Allocate(Section section)
{
    QMutexLocker locker(&mutex_);
    auto& pool = pools_[static_cast<size_t>(section)];

    if (pool.empty()) {
        Expand(pool, section, Pool::kExpandSize);
    }

    Node* n = pool.front();
    pool.pop_front();

    return n;
}

inline void NodePool::Recycle(Node* node, Section section)
{
    if (!node)
        return;

    QMutexLocker locker(&mutex_);
    auto& pool = pools_[static_cast<size_t>(section)];

    // If pool exceeds threshold, delete node instead of recycling
    if (static_cast<qsizetype>(pool.size()) + 1 >= Pool::kMaxSize) {
        locker.unlock();
        delete node;
        return;
    }

    node->ResetState();
    pool.push_back(node);
}

template <Iterable Container> inline void NodePool::Recycle(Container& container, Section section)
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

    for (Node* n : container) {
        if (n) {
            n->ResetState();
            pool.push_back(n);
        }
    }

    container.clear();
}

#endif // NODEPOOL_H
