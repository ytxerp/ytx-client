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

#ifndef MAINWINDOWUTILS_H
#define MAINWINDOWUTILS_H

#include <QMessageBox>
#include <QSettings>
#include <QWidget>

#include "component/sectioncontex.h"
#include "worksheet.h"

template <typename T>
concept MapType = requires(T a) {
    typename T::const_iterator;
    typename T::key_type;
    typename T::mapped_type;
    { a.constBegin() } -> std::same_as<typename T::const_iterator>;
    { a.constEnd() } -> std::same_as<typename T::const_iterator>;
    requires std::is_same_v<typename T::mapped_type, QString>;
    requires std::is_same_v<typename T::key_type, int> || std::is_same_v<typename T::key_type, bool>;
};

namespace MainWindowUtils {

QString ResourceFile();

void ReadPrintTmplate(QMap<QString, QString>& print_template);

void ExportExcel(CString& table, QSharedPointer<YXlsx::Worksheet> worksheet, bool where = true);
void Message(QMessageBox::Icon icon, CString& title, CString& text, int timeout);

bool PrepareNewFile(QString& file_path, CString& suffix);
bool CheckFileValid(CString& file_path, CString& suffix);
void SwitchDialog(const SectionContext* sc, bool enable);

template <MapType T> ItemModel* CreateModelFromMap(const T& map, QObject* parent)
{
    auto* model { new ItemModel(parent) };

    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        model->AppendItem(it.value(), it.key());
    }

    model->sort(0);
    return model;
}

// ============================================================================
// Reserved Feature: RestoreTab
// ----------------------------------------------------------------------------
// This interface is currently disabled. It is kept here for potential future
// activation (e.g., automatic tab restore functionality).
// - Tabs are reopened manually in the current workflow.
// - Keep both declaration and implementation under #if 0 so that the compiler
//   ignores them, but developers are reminded of the reserved functionality.
// ============================================================================
#if 0
    void RestoreTab(NodeModel* tree_model, EntryHub* dbhub, LeafWgtHash& entry_wgt_hash, CUuidSet& set, CSectionInfo& info, CSectionConfig& config)
    {
        assert(tree_model);

        if (set.isEmpty() || info.section == Section::kSale || info.section == Section::kPurchase)
            return;

        for (const auto& node_id : set) {
            if (tree_model->Contains(node_id) && tree_model->Kind(node_id) == kLeaf)
                CreateLeafFIST(tree_model, dbhub, entry_wgt_hash, info, config, node_id);
        }
    }

inline QVariantList SaveTab(CLeafWgtHash& entry_wgt_hash)
{
    if (entry_wgt_hash.isEmpty())
        return {};

    const auto keys { entry_wgt_hash.keys() };
    QVariantList list {};

    for (const auto& node_id : keys) {
        list.emplaceBack(node_id.toString(QUuid::WithoutBraces));
    }

    return list;
}

inline QSet<QUuid> ReadTab(QSharedPointer<QSettings> settings, CString& section, CString& property)
{
    assert(settings);

    auto variant { settings->value(QString("%1/%2").arg(section, property)) };

    if (!variant.isValid())
        return {};

    QSet<QUuid> set {};
    const auto variant_list { variant.toList() };

    for (const auto& node_id : variant_list)
        set.insert(QUuid(node_id.toString()));

    return set;
}
#endif

};

#endif // MAINWINDOWUTILS_H
