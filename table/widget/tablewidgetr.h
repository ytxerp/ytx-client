/*
 * Copyright (C) 2026 YTX
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

#pragma once

#include "component/config.h"
#include "table/model/tablemodelr.h"
#include "tablewidget.h"

namespace Ui {
class TableWidgetR;
}

class TableWidgetR final : public TableWidget {
    Q_OBJECT

public:
    explicit TableWidgetR(TableModel* model, NodeR* node, CSectionConfig& section_config, int permission, QWidget* parent = nullptr);
    ~TableWidgetR() override;

    void SyncSucceeded(int version);
    void StatusSucceeded(int status, int version);
    void PermissionDenied();

    TableModel* Model() const override { return model_; }
    QTableView* View() const override;

    bool HasPendingUpdate() const;

public slots:
    void RSyncDelta(QUuid node_id, const entry::Delta& delta);
    void RDataChanged();
    void on_pushButtonSave_clicked();

private slots:
    void on_pushButtonApprove_clicked();
    void on_pushButtonPost_clicked();
    void on_dateTimeEdit_dateTimeChanged(const QDateTime& dateTime);
    void on_lineEditDescription_textEdited(const QString& arg1);
    void on_pushButtonUnapprove_clicked();
    void on_pushButtonUnpost_clicked();

private:
    void InitWidget();
    void InitData();
    void UpdateButtons();
    void UpdateSaveApproveButtons();

    void BuildNodeInsert(QJsonObject& message);
    void BuildNodeUpdate(QJsonObject& message);
    QJsonObject BuildStatusMessage() const;

private:
    Ui::TableWidgetR* ui;

    NodeR* node_ {};
    NodeR original_node_ {};
    TableModelR* model_ {};

    QJsonObject pending_update_ {};

    const int permission_ {};
    const QUuid node_id_ {};

    CSectionConfig& config_ {};
};

inline const char* kTableWidgetR = "TableWidgetR";
