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

#include <QLabel>
#include <QMessageBox>
#include <QRadioButton>
#include <QSettings>

#include "component/sectioncontex.h"
#include "enum/stateenum.h"

namespace utils {

void ResetSectionContext(SectionContext& sc);
void SetConnectionStatus(QLabel* label, ConnectionState status);
void SetLoginStatus(QLabel* label, LoginState status);
void SetPushButton(QPushButton* btn, const QKeySequence& ks);
void SetRadioButton(QRadioButton* btn, const QKeySequence& ks);

QString AccountIniFileName(const QString& email, const QString& workspace);
QString UuidToShortCode(const QUuid& uuid, int length = 10);

QUuid ManageDialog(QHash<QUuid, WidgetContext>& widget_hash, QDialog* dialog);
void ManageDialog(QHash<QUuid, WidgetContext>& widget_hash, QDialog* dialog, const QUuid& id);

QByteArray ZstdCompress(const QByteArray& data);
QByteArray ZstdDecompress(const QByteArray& data);

inline void CloseWidget(const QUuid& node_id, QHash<QUuid, WidgetContext>& widget_hash)
{
    // Take removes the widget from hash and returns it
    if (auto widget { widget_hash.take(node_id).widget }) {
        // Check if QPointer is still valid (widget not already deleted)
        if (widget) {
            // Schedule asynchronous deletion (safe, won't trigger signals immediately)
            widget->deleteLater();
        }
    }
}

inline void CloseWidgets(QHash<QUuid, WidgetContext>& hash)
{
    // Schedule all widgets for asynchronous deletion
    for (auto& vc : hash) {
        if (vc.widget)
            vc.widget->deleteLater();
    }

    // Clear hash immediately
    hash.clear();
}

QMessageBox* CreateMessage(QMessageBox::Icon icon, CString& title, CString& text, bool modal = false,
    QMessageBox::StandardButtons buttons = QMessageBox::NoButton, QWidget* parent = nullptr);

/**
 * Notification Message Guidelines
 *
 * Use consistent notification titles according to the operation result type.
 *
 * ---------------------------------------------------------------------------
 * Category                    Title                    Usage
 * ---------------------------------------------------------------------------
 *
 * Confirmation Dialog         Confirm Delete           User confirmation is
 *                                                        required before a
 *                                                        destructive action.
 *
 * Unsaved Changes             Unsaved Changes          User is leaving a page
 *                                                        with unsaved changes.
 *
 * Invalid Input               Invalid Input            User input format or
 *                                                        validation error.
 *
 * Required Information Missing Required Information    Required data is missing
 *                                                        before an operation.
 *
 * Not Found                   Not Found                Target object or resource
 *                                                        does not exist.
 *
 * Data Outdated               Data Outdated            Local data is outdated or
 *                                                        conflicts with server data.
 *
 * Operation Rejected          Operation Rejected       Operation is not allowed
 *                                                        due to business rules or
 *                                                        current object state.
 *
 * Operation Unavailable       Operation Unavailable    Required resource or
 *                                                        configuration is not
 *                                                        available.
 *
 * Operation Failed            Operation Failed         Operation execution failed
 *                                                        due to system or runtime
 *                                                        errors.
 *
 * Operation Completed         Operation Completed      Operation completed
 *                                                        successfully.
 *
 * Export Completed            Export Completed         Export task completed
 *                                                        successfully.
 *
 * Language Changed            Language Changed         Configuration change that
 *                                                        requires user action
 *                                                        (e.g. restart/login).
 *
 * Role Updated                Role Updated             Account permission or
 *                                                        role information changed.
 *
 * ---------------------------------------------------------------------------
 *
 * Examples:
 *
 * 1. Invalid user input:
 *      Invalid Input
 *      Username must be 3-32 characters...
 *
 * 2. Missing required data:
 *      Required Information Missing
 *      Please select a partner before continuing.
 *
 * 3. Object does not exist:
 *      Not Found
 *      The document could not be found.
 *
 * 4. Business restriction:
 *      Operation Rejected
 *      The order cannot be deleted because it has been released.
 *
 * 5. Synchronization conflict:
 *      Data Outdated
 *      The data has changed. Please refresh and try again.
 *
 * 6. Execution failure:
 *      Operation Failed
 *      Failed to export the document.
 *
 * 7. Configuration update:
 *      Language Changed
 *      Please restart the application to apply the changes.
 *
 * ---------------------------------------------------------------------------
 *
 * Avoid:
 *
 *  - Invalid Operation
 *      Too generic. Use Operation Rejected or Data Outdated instead.
 *
 *  - Update Failed
 *      Do not use for business restrictions. Use Operation Rejected.
 *
 *  - Load Failed / Save Failed / Export Failed
 *      Prefer Operation Failed with a detailed message.
 *
 * ---------------------------------------------------------------------------
 */
void ShowMessage(QMessageBox::Icon icon, CString& title, CString& text, int duration_ms = 0, QMessageBox::StandardButtons buttons = QMessageBox::NoButton,
    QWidget* parent = nullptr);

bool PrepareNewFile(QString& file_path, CString& suffix);
void SwitchDialog(const SectionContext* sc, bool enable);
int CompareVersion(const QString& v1, const QString& v2);

void SetupHeaderStatus(QHeaderView* header, const QSharedPointer<QSettings>& settings, Section section, const QString& key);

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
        Q_ASSERT(tree_model);

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
    Q_ASSERT(settings);

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
