#include "tablewidgeto.h"

#include "component/constantwebsocket.h"
#include "component/signalblocker.h"
#include "global/nodepool.h"
#include "global/printhub.h"
#include "ui_tablewidgeto.h"
#include "utils/mainwindowutils.h"
#include "websocket/websocket.h"
#include "workspace/sectionpermissions.h"

TableWidgetO::TableWidgetO(COrderWidgetArg& arg, NodeO* node, int permission, QWidget* parent)
    : TableWidget(parent)
    , ui(new Ui::TableWidgetO)
    , node_ { node }
    , original_node_ { *node }
    , table_model_ { qobject_cast<TableModelO*>(arg.table_model) }
    , section_config_ { arg.section_config }
    , node_id_ { node->id }
    , section_ { arg.section }
    , permission_ { permission }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    InitWidget(arg.pmodel, arg.emodel);
    InitRuleGroup();
    InitUnitGroup();
    InitData(node_);
    InitConnect();

    if (node->sync_state == SyncState::kCreating)
        QTimer::singleShot(0, this, [this]() { ui->comboPartner->setFocus(); });
}

TableWidgetO::~TableWidgetO()
{
    NodePool::Instance().Recycle(node_, section_);
    delete ui;
}

QTableView* TableWidgetO::View() const { return ui->tableViewO; }

void TableWidgetO::SyncSucceeded(int version)
{
    qDebug() << Q_FUNC_INFO;

    table_model_->SyncSucceeded();
    pending_update_ = QJsonObject();

    node_->version = version;
    node_->sync_state = SyncState::kSynced;

    original_node_ = *node_;

    ui->pBtnSave->setEnabled(false);
    ui->pBtnRelease->setEnabled(permission_ & order_permission::kRelease);
}

void TableWidgetO::PermissionDenied()
{
    qDebug() << Q_FUNC_INFO;

    node_->sync_state = SyncState::kSynced;
}

void TableWidgetO::StatusSucceeded(int status, int version)
{
    qDebug() << Q_FUNC_INFO;

    Q_ASSERT(status >= std::to_underlying(OrderStatus::kDraft) && status <= std::to_underlying(OrderStatus::kRecalled));

    node_->order_status = static_cast<OrderStatus>(status);
    node_->version = version;
    node_->sync_state = SyncState::kSynced;

    UpdateButtons(node_->order_status);
}

bool TableWidgetO::HasPendingUpdate() const
{
    const bool node_pending { original_node_ != *node_ };
    const bool entry_pending { table_model_->HasPendingUpdate() };

    qDebug() << "[HasPendingUpdate]"
             << "node_pending =" << node_pending << ", entry_pending =" << entry_pending << ", result =" << (node_pending || entry_pending);

    return node_pending || entry_pending;
}

void TableWidgetO::RSyncDelta(QUuid node_id, const entry::Delta& delta)
{
    Q_ASSERT(node_id_ == node_id && "RSyncDelta called with mismatched node_id");

    const double factor { node_->direction_rule == direction_rule::kRO ? -1.0 : 1.0 };

    node_->initial_total += delta.initial * factor;
    node_->count_total += delta.count * factor;
    node_->measure_total += delta.measure * factor;

    InitUiValue();
}

void TableWidgetO::RDataChanged() { UpdateSaveReleaseButtons(); }

void TableWidgetO::InitWidget(QSortFilterProxyModel* pmodel, QSortFilterProxyModel* emodel)
{
    {
        ui->comboPartner->setModel(pmodel);
        ui->comboPartner->setCurrentIndex(-1);

        ui->comboEmployee->setModel(emodel);
        ui->comboEmployee->setCurrentIndex(-1);
    }

    {
        ui->tableViewO->setModel(table_model_);
        table_model_->setParent(ui->tableViewO);
    }

    {
        ui->dateTimeEdit->setDisplayFormat(datetime_format::kDateTime);
    }

    {
        ui->dSpinInitialTotal->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
        ui->dSpinMeasureTotal->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
        ui->dSpinCountTotal->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());

        ui->dSpinInitialTotal->setDecimals(section_config_.amount_decimal);
        ui->dSpinMeasureTotal->setDecimals(section_config_.quantity_decimal);
        ui->dSpinCountTotal->setDecimals(section_config_.quantity_decimal);

        ui->dSpinCountTotal->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui->dSpinMeasureTotal->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }

    {
        utils::SetPushButton(ui->pBtnSave, QKeySequence::Save);
        utils::SetPushButton(ui->pBtnRelease, QKeySequence(Qt::CTRL | Qt::Key_Return));
        utils::SetPushButton(ui->pBtnRecall, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
        utils::SetPushButton(ui->pBtnPreview, QKeySequence(Qt::CTRL | Qt::Key_P));
        utils::SetPushButton(ui->pBtnPrint, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));

        utils::SetRadioButton(ui->rBtnIS, QKeySequence(Qt::CTRL | Qt::Key_1));
        utils::SetRadioButton(ui->rBtnMS, QKeySequence(Qt::CTRL | Qt::Key_2));
        utils::SetRadioButton(ui->rBtnPEND, QKeySequence(Qt::CTRL | Qt::Key_3));
        utils::SetRadioButton(ui->rBtnRO, QKeySequence(Qt::CTRL | Qt::Key_5));
        utils::SetRadioButton(ui->rBtnFO, QKeySequence(Qt::CTRL | Qt::Key_4));
    }

    {
        auto& templates { PrintHub::Instance().TemplateMap() };
        for (auto it = templates.constBegin(); it != templates.constEnd(); ++it) {
            const QString display_name { it.key().section('_', 1) };
            ui->comboTemplate->addItem(display_name, it.value());
        }

        if (ui->comboTemplate->count() > 0) {
            PrintHub::Instance().LoadTemplate(ui->comboTemplate->currentData().toString());
        }
    }
}

void TableWidgetO::InitConnect()
{
    connect(rule_group_, &QButtonGroup::idClicked, this, &TableWidgetO::RRuleGroupClicked);
    connect(unit_group_, &QButtonGroup::idClicked, this, &TableWidgetO::RUnitGroupClicked);
}

void TableWidgetO::InitData(const NodeO* node)
{
    {
        (node->direction_rule ? ui->rBtnRO : ui->rBtnFO)->setChecked(true);
        ui->dateTimeEdit->setDateTime(node->issued_time);
    }

    {
        const NodeUnit kUnit { node->unit };

        switch (kUnit) {
        case NodeUnit::OImmediate:
            ui->rBtnIS->setChecked(true);
            break;
        case NodeUnit::OMonthly:
            ui->rBtnMS->setChecked(true);
            break;
        case NodeUnit::OPending:
            ui->rBtnPEND->setChecked(true);
            break;
        default:
            break;
        }
    }

    {
        UpdateButtons(OrderStatus(node->order_status));
    }

    if (node_->sync_state == SyncState::kCreating)
        return;

    {
        InitUiValue();

        ui->lineDescription->setText(node->description);

        int partner_index { ui->comboPartner->findData(node->partner_id) };
        ui->comboPartner->setCurrentIndex(partner_index);

        int employee_index { ui->comboEmployee->findData(node->employee_id) };
        ui->comboEmployee->setCurrentIndex(employee_index);
    }
}

void TableWidgetO::UpdateButtons(OrderStatus value)
{
    const bool draft { value == OrderStatus::kDraft };
    const bool released { value == OrderStatus::kReleased };
    const bool pending { table_model_->HasPendingUpdate() || original_node_ != *node_ };
    const bool no_pending { !pending };

    ui->comboPartner->setReadOnly(!draft);
    ui->comboEmployee->setReadOnly(released);
    ui->pBtnPrint->setEnabled(released && no_pending);

    ui->rBtnRO->setEnabled(draft);
    ui->rBtnFO->setEnabled(draft);

    ui->rBtnIS->setEnabled(!released);
    ui->rBtnMS->setEnabled(!released);
    ui->rBtnPEND->setEnabled(!released);

    ui->lineDescription->setReadOnly(released);
    ui->dateTimeEdit->setReadOnly(released);

    ui->pBtnSave->setEnabled(!released && pending);

    ui->pBtnRelease->setEnabled(
        !released && no_pending && node_->sync_state == SyncState::kSynced && node_->unit != NodeUnit::OPending && (permission_ & order_permission::kRelease));
    ui->pBtnRecall->setEnabled(released && (permission_ & order_permission::kUnrelease));
}

void TableWidgetO::UpdateSaveReleaseButtons()
{
    const bool pending { table_model_->HasPendingUpdate() || original_node_ != *node_ };
    const bool released { node_->order_status == OrderStatus::kReleased };

    ui->pBtnSave->setEnabled(!released && pending);

    ui->pBtnRelease->setEnabled(
        !released && !pending && node_->sync_state == SyncState::kSynced && node_->unit != NodeUnit::OPending && (permission_ & order_permission::kRelease));
}

void TableWidgetO::InitUiValue()
{
    ui->dSpinCountTotal->setValue(node_->count_total);
    ui->dSpinMeasureTotal->setValue(node_->measure_total);
    ui->dSpinInitialTotal->setValue(node_->initial_total);
}

void TableWidgetO::InitRuleGroup()
{
    rule_group_ = new QButtonGroup(this);
    rule_group_->addButton(ui->rBtnRO, static_cast<int>(direction_rule::kRO));
    rule_group_->addButton(ui->rBtnFO, static_cast<int>(direction_rule::kFO));
}

void TableWidgetO::InitUnitGroup()
{
    unit_group_ = new QButtonGroup(this);
    unit_group_->addButton(ui->rBtnIS, std::to_underlying(NodeUnit::OImmediate));
    unit_group_->addButton(ui->rBtnMS, std::to_underlying(NodeUnit::OMonthly));
    unit_group_->addButton(ui->rBtnPEND, std::to_underlying(NodeUnit::OPending));
}

void TableWidgetO::on_comboPartner_currentIndexChanged(int /*index*/)
{
    const QUuid partner_id { ui->comboPartner->currentData().toUuid() };
    if (node_->partner_id == partner_id)
        return;

    node_->partner_id = partner_id;
    emit SSyncPartner(node_id_, partner_id);

    UpdateSaveReleaseButtons();
}

void TableWidgetO::on_comboEmployee_currentIndexChanged(int /*index*/)
{
    const QUuid employee_id { ui->comboEmployee->currentData().toUuid() };
    if (node_->employee_id == employee_id)
        return;

    node_->employee_id = employee_id;

    UpdateSaveReleaseButtons();
}

void TableWidgetO::on_dateTimeEdit_dateTimeChanged(const QDateTime& date_time)
{
    if (node_->issued_time == date_time)
        return;

    node_->issued_time = date_time;

    UpdateSaveReleaseButtons();
}

void TableWidgetO::on_lineDescription_textChanged(const QString& arg1)
{
    if (node_->description == arg1)
        return;

    node_->description = arg1;

    UpdateSaveReleaseButtons();
}

void TableWidgetO::RRuleGroupClicked(int id)
{
    if (node_->order_status != OrderStatus::kDraft)
        return;

    node_->direction_rule = static_cast<bool>(id);

    node_->count_total *= -1;
    node_->measure_total *= -1;
    node_->initial_total *= -1;

    InitUiValue();
    UpdateSaveReleaseButtons();
}

void TableWidgetO::RUnitGroupClicked(int id)
{
    const NodeUnit unit { id };

    node_->unit = unit;
    UpdateSaveReleaseButtons();
}

void TableWidgetO::on_pBtnSave_clicked()
{
    if (!ValidatePartner())
        return;

    if (!ValidateSyncState())
        return;

    table_model_->Purify();

    if (!HasPendingUpdate())
        return;

    if (!ValidateUnitPrice())
        return;

    Q_ASSERT(node_->order_status != OrderStatus::kReleased);

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section_));

    table_model_->Finalize(message);

    if (node_->sync_state == SyncState::kSynced) {
        pending_update_ = QJsonObject();
        BuildNodeUpdate(pending_update_);

        message.insert(kNodeId, node_->id.toString(QUuid::WithoutBraces));
        message.insert(kVersion, node_->version);
        message.insert(kNodeUpdate, pending_update_);

        WebSocket::Instance()->SendMessage(WsKey::kOrderUpdate, message);
    }

    if (node_->sync_state == SyncState::kCreating) {
        BuildNodeInsert(message);
        WebSocket::Instance()->SendMessage(WsKey::kOrderInsert, message);
    }

    qInfo() << Q_FUNC_INFO << node_->version;

    node_->sync_state = SyncState::kUpdating;
}

void TableWidgetO::on_pBtnPrint_clicked()
{
    QEvent leave_event(QEvent::Leave);
    QApplication::sendEvent(ui->pBtnPrint, &leave_event);

    if (!PreparePrint())
        return;

    PrintHub::Instance().Print();
}

void TableWidgetO::on_pBtnPreview_clicked()
{
    QEvent leave_event(QEvent::Leave);
    QApplication::sendEvent(ui->pBtnPreview, &leave_event);

    if (!PreparePrint())
        return;

    PrintHub::Instance().Preview();
}

bool TableWidgetO::PreparePrint()
{
    if (ui->comboTemplate->currentIndex() == -1) {
        utils::ShowMessage(
            QMessageBox::Warning, tr("Required Information Missing"), tr("Please select a printable template before continuing."), time_const::kAutoCloseMs);
        return false;
    }

    PrintHub::Instance().SetSectionConfig(&section_config_);
    PrintHub::Instance().SetValue(node_, table_model_->GetEntryList());

    return true;
}

void TableWidgetO::BuildNodeInsert(QJsonObject& message)
{
    const QJsonObject node_json { node_->WriteJson() };

    QJsonObject path_json {};
    path_json.insert(kAncestor, node_->parent->id.toString(QUuid::WithoutBraces));
    path_json.insert(kDescendant, node_id_.toString(QUuid::WithoutBraces));

    message.insert(kNode, node_json);
    message.insert(kPath, path_json);
}

void TableWidgetO::BuildNodeUpdate(QJsonObject& message)
{
    if (node_->partner_id != original_node_.partner_id)
        message.insert(kPartnerId, node_->partner_id.toString(QUuid::WithoutBraces));

    if (node_->employee_id != original_node_.employee_id)
        message.insert(kEmployeeId, node_->employee_id.toString(QUuid::WithoutBraces));

    if (node_->issued_time != original_node_.issued_time)
        message.insert(kIssuedTime, node_->issued_time.toUTC().toString(Qt::ISODate));

    if (node_->description != original_node_.description)
        message.insert(kDescription, node_->description);

    if (node_->direction_rule != original_node_.direction_rule)
        message.insert(kDirectionRule, node_->direction_rule);

    if (node_->unit != original_node_.unit)
        message.insert(kUnit, std::to_underlying(node_->unit));

    message.insert(kInitialTotal, QString::number(node_->initial_total, 'f', numeric_const::kDecimalPlaces4));
    message.insert(kCountTotal, QString::number(node_->count_total, 'f', numeric_const::kDecimalPlaces8));
    message.insert(kMeasureTotal, QString::number(node_->measure_total, 'f', numeric_const::kDecimalPlaces8));
}

QJsonObject TableWidgetO::BuildStatusMessage() const
{
    QJsonObject message {};

    message.insert(kSection, std::to_underlying(section_));
    message.insert(kNodeId, node_->id.toString(QUuid::WithoutBraces));
    message.insert(kVersion, node_->version);
    message.insert(kStatus, std::to_underlying(node_->order_status));

    return message;
}

void TableWidgetO::on_pBtnRecall_clicked()
{
    Q_ASSERT(node_->order_status == OrderStatus::kReleased && (permission_ & order_permission::kUnrelease));
    Q_ASSERT(!table_model_->HasPendingUpdate() && original_node_ == *node_);
    Q_ASSERT(node_->sync_state == SyncState::kSynced);

    if (!ValidateSyncState())
        return;

    if (!node_->settlement_id.isNull()) {
        utils::ShowMessage(
            QMessageBox::Information, tr("Operation Rejected"), tr("This order has already been settled and cannot be recalled."), time_const::kAutoCloseMs);

        return;
    }

    qDebug() << Q_FUNC_INFO << node_->version;

    WebSocket::Instance()->SendMessage(WsKey::kOrderRecall, BuildStatusMessage());

    node_->sync_state = SyncState::kUpdating;
}

bool TableWidgetO::ValidatePartner() const
{
    if (node_->partner_id.isNull()) {
        utils::ShowMessage(QMessageBox::Information, tr("Operation Rejected"), tr("A partner must be selected before continuing."), time_const::kAutoCloseMs);
        return false;
    }

    return true;
}

bool TableWidgetO::ValidateSyncState() const
{
    if (node_->sync_state == SyncState::kUpdating) {
        utils::ShowMessage(
            QMessageBox::Information, tr("Operation Rejected"), tr("The data is being updated. Please refresh and try again."), time_const::kAutoCloseMs);
        return false;
    }

    qDebug() << Q_FUNC_INFO << static_cast<int>(node_->sync_state);

    return true;
}

bool TableWidgetO::ValidateUnitPrice() const
{
    if (!table_model_->HasZeroUnitPrice())
        return true;

    const auto result { QMessageBox::question(const_cast<TableWidgetO*>(this), tr("Operation Warning"),
        tr("Some entries have a unit price of zero. Do you want to continue saving?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) };

    return result == QMessageBox::Yes;
}

void TableWidgetO::on_pBtnRelease_clicked()
{
    Q_ASSERT(node_->order_status != OrderStatus::kReleased && (permission_ & order_permission::kRelease));
    Q_ASSERT(!table_model_->HasPendingUpdate() && original_node_ == *node_);

    if (!ValidateSyncState())
        return;

    WebSocket::Instance()->SendMessage(WsKey::kOrderRelease, BuildStatusMessage());

    qInfo() << Q_FUNC_INFO << node_->version;

    node_->sync_state = SyncState::kUpdating;
}

void TableWidgetO::on_comboTemplate_currentIndexChanged(int /*index*/)
{
    if (!PrintHub::Instance().LoadTemplate(ui->comboTemplate->currentData().toString())) {
        utils::ShowMessage(QMessageBox::Warning, tr("Operation Failed"), tr("Failed to load the print template."), time_const::kAutoCloseMs);
    }
}
