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

#ifndef AUDITDIALOG_H
#define AUDITDIALOG_H

#include <QDialog>
#include <QTableView>
#include <QTimer>

#include "audithub/auditmodel.h"
#include "auditinfo.h"

namespace Ui {
class AuditDialog;
}

class AuditDialog final : public QDialog {
    Q_OBJECT

signals:
    void SRefresh();

public:
    explicit AuditDialog(const audit_hub::AuditInfo& info, QWidget* parent = nullptr);
    ~AuditDialog() override;

    QTableView* View();
    audit_hub::AuditModel* Model() { return model_; }

private slots:
    void on_pBtnFetch_clicked();
    void InitTimer();

private:
    Ui::AuditDialog* ui;
    const audit_hub::AuditInfo& info_;
    audit_hub::AuditModel* model_ {};

    QTimer* cooldown_timer_ { nullptr };
};

#endif // AUDITDIALOG_H
