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

#ifndef MASTERDATAREGISTRY_H
#define MASTERDATAREGISTRY_H

#include "enum/section.h"
#include "tree/model/treemodel.h"
#include "tree/model/treemodeli.h"

class MasterDataRegistry {
public:
    static MasterDataRegistry& Instance()
    {
        static MasterDataRegistry instance;
        return instance;
    }

    void RegisterTreeModel(Section section, QPointer<TreeModel> node) { tree_model_hash_.insert(section, node); }

    QString InventoryName(const QUuid& id) const
    {
        if (auto model = tree_model_hash_.value(Section::kInventory))
            return model->Name(id);

        return {};
    }

    QString InventoryPath(const QUuid& id) const
    {
        if (auto model = tree_model_hash_.value(Section::kInventory))
            return model->Path(id);

        return {};
    }

    double InventoryUnitPrice(const QUuid& id) const
    {
        if (auto* model = static_cast<TreeModelI*>(tree_model_hash_.value(Section::kInventory).data()))
            return model->UnitPrice(id);

        return {};
    }

    QString PartnerName(const QUuid& id) const
    {
        if (auto model = tree_model_hash_.value(Section::kPartner))
            return model->Name(id);

        return {};
    }

    MasterDataRegistry(const MasterDataRegistry&) = delete;
    MasterDataRegistry& operator=(const MasterDataRegistry&) = delete;
    MasterDataRegistry(MasterDataRegistry&&) = delete;
    MasterDataRegistry& operator=(MasterDataRegistry&&) = delete;

private:
    MasterDataRegistry() = default;
    ~MasterDataRegistry() = default;

private:
    QHash<Section, QPointer<TreeModel>> tree_model_hash_ {};
};

#endif // MASTERDATAREGISTRY_H
