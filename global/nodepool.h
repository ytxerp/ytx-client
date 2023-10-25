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
#include <QVector>

#include "component/enumclass.h"
#include "concepts.h"
#include "tree/node.h"

class NodePool {
public:
    static NodePool& Instance();

    Node* Allocate(Section section);
    void Recycle(Node* node, Section section);
    template <Iterable Container> void Recycle(Container& container, Section section);

    NodePool(const NodePool&) = delete;
    NodePool& operator=(const NodePool&) = delete;
    NodePool(NodePool&&) = delete;
    NodePool& operator=(NodePool&&) = delete;

private:
    NodePool();
    ~NodePool();

    void Expand(QVector<Node*>& pool, Section section, int count);
    QVector<Node*>& GetPool(Section section);

    QVector<Node*> f_pool_;
    QVector<Node*> i_pool_;
    QVector<Node*> t_pool_;
    QVector<Node*> s_pool_;
    QVector<Node*> o_pool_; // 用于 kSale 和 kPurchase

    QMutex mutex_;

    static constexpr qsizetype kDefaultExpandSize { 100 };
    static constexpr qsizetype kShrinkThreshold { 1000 };
};

template <Iterable Container> inline void NodePool::Recycle(Container& container, Section section)
{
    if (container.isEmpty())
        return;

    auto& pool = GetPool(section);

    if (pool.size() + container.size() >= kShrinkThreshold) {
        qDeleteAll(container);
    } else {
        QMutexLocker locker(&mutex_);

        for (Node* resource : container) {
            if (resource) {
                resource->ResetState();
                pool.push_back(resource);
            }
        }
    }

    container.clear();
}

inline NodePool& NodePool::Instance()
{
    static NodePool instance;
    return instance;
}

inline NodePool::NodePool()
{
    Expand(f_pool_, Section::kFinance, kDefaultExpandSize);
    Expand(i_pool_, Section::kItem, kDefaultExpandSize);
    Expand(t_pool_, Section::kTask, kDefaultExpandSize);
    Expand(s_pool_, Section::kStakeholder, kDefaultExpandSize);
    Expand(o_pool_, Section::kSale, kDefaultExpandSize);
}

inline NodePool::~NodePool()
{
    qDeleteAll(f_pool_);
    qDeleteAll(i_pool_);
    qDeleteAll(t_pool_);
    qDeleteAll(s_pool_);
    qDeleteAll(o_pool_);
}

inline void NodePool::Expand(QVector<Node*>& pool, Section section, int count)
{
    for (int i = 0; i != count; ++i) {
        switch (section) {
        case Section::kFinance:
            pool.append(new NodeF());
            break;
        case Section::kItem:
            pool.append(new NodeI());
            break;
        case Section::kTask:
            pool.append(new NodeT());
            break;
        case Section::kStakeholder:
            pool.append(new NodeS());
            break;
        case Section::kSale:
        case Section::kPurchase:
            pool.append(new NodeO());
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }
}

inline QVector<Node*>& NodePool::GetPool(Section section)
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

inline Node* NodePool::Allocate(Section section)
{
    QMutexLocker locker(&mutex_);

    auto& pool = GetPool(section);
    if (pool.isEmpty()) {
        Expand(pool, section, kDefaultExpandSize);
    }
    return pool.takeLast();
}

inline void NodePool::Recycle(Node* node, Section section)
{
    if (!node)
        return;

    auto& pool = GetPool(section);

    if (pool.size() >= kShrinkThreshold) {
        delete node;
        return;
    }

    QMutexLocker locker(&mutex_);
    node->ResetState();
    pool.append(node);
}

#endif // NODEPOOL_H
