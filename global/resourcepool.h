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

#ifndef RESOURCEPOOL_H
#define RESOURCEPOOL_H

#include <QMutex>
#include <deque>

#include "component/constantint.h"
#include "concepts.h"

template <typename T>
concept Resettable = requires(T& t) {
    { t.Reset() } -> std::same_as<void>;
};

template <Resettable T> class ResourcePool {
public:
    static ResourcePool& Instance();
    T* Allocate();
    void Recycle(T* resource);
    template <Iterable Container> void Recycle(Container& container);

    ResourcePool(const ResourcePool&) = delete;
    ResourcePool& operator=(const ResourcePool&) = delete;
    ResourcePool(ResourcePool&&) = delete;
    ResourcePool& operator=(ResourcePool&&) = delete;

private:
    ResourcePool();
    ~ResourcePool();

    void Expand(qsizetype size);

private:
    std::deque<T*> pool_;
    QMutex mutex_;
};

template <Resettable T> ResourcePool<T>& ResourcePool<T>::Instance()
{
    static ResourcePool<T> instance {};
    return instance;
}

template <Resettable T> T* ResourcePool<T>::Allocate()
{
    QMutexLocker locker(&mutex_);

    if (pool_.empty())
        Expand(PoolConst::kExpandSize);

    Q_ASSERT(!pool_.empty());

    T* resource = pool_.front();
    pool_.pop_front();

    return resource;
}

template <Resettable T> void ResourcePool<T>::Recycle(T* resource)
{
    if (!resource)
        return;

    QMutexLocker locker(&mutex_);

    if (static_cast<qsizetype>(pool_.size()) + 1 >= PoolConst::kMaxSize) {
        locker.unlock();
        delete resource;
        return;
    }

    resource->Reset();
    pool_.push_back(resource);
}

template <Resettable T> template <Iterable Container> void ResourcePool<T>::Recycle(Container& container)
{
    if (container.isEmpty())
        return;

    QMutexLocker locker(&mutex_);

    if (static_cast<qsizetype>(pool_.size()) + container.size() >= PoolConst::kMaxSize) {
        locker.unlock();
        qDeleteAll(container);
        container.clear();
        return;
    }

    for (T* resource : container) {
        if (resource) {
            resource->Reset();
            pool_.push_back(resource);
        }
    }

    container.clear();
}

template <Resettable T> ResourcePool<T>::ResourcePool() { Expand(PoolConst::kExpandSize); }

template <Resettable T> void ResourcePool<T>::Expand(qsizetype size)
{
    for (qsizetype i = 0; i != size; ++i) {
        pool_.push_back(new T());
    }
}

template <Resettable T> ResourcePool<T>::~ResourcePool() { }

#endif // RESOURCEPOOL_H
