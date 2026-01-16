#include "tablewidgeto.h"

#include <QMessageBox>

#include "component/signalblocker.h"
#include "global/printhub.h"
#include "ui_tablewidgeto.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableWidgetO::TableWidgetO(COrderWidgetArg& arg, const NodeO& node, SyncState sync_state, QWidget* parent)
    : TableWidget(parent)
    , ui(new Ui::TableWidgetO)
    , tmp_node_ { node }
    , table_model_order_ { qobject_cast<TableModelO*>(arg.table_model) }
    , tree_model_partner_ { arg.tree_model_partner }
    , config_ { arg.section_config }
    , sync_state_ { sync_state }
    , node_id_ { node.id }
    , section_ { arg.section }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    IniWidget();
    IniRuleGroup();
    IniUnitGroup();
    IniData(node);
    IniConnect();

    if (sync_state == SyncState::kLocalOnly)
        QTimer::singleShot(0, this, [this]() { ui->comboPartner->setFocus(); });
}

TableWidgetO::~TableWidgetO() { delete ui; }

QTableView* TableWidgetO::View() const { return ui->tableViewO; }

void TableWidgetO::ReleaseSucceeded(int version)
{
    ui->pBtnPrint->setFocus();
    ui->pBtnPrint->setDefault(true);

    MarkSynced(version);

    tmp_node_.status = NodeStatus::kFinished;
    LockWidgets(NodeStatus::kFinished);
}

void TableWidgetO::RecallSucceeded(int version)
{
    ui->pBtnRelease->setFocus();
    ui->pBtnRelease->setDefault(true);

    MarkSynced(version);

    tmp_node_.status = NodeStatus::kUnfinished;
    LockWidgets(NodeStatus::kUnfinished);
}

void TableWidgetO::SaveSucceeded(int version) { MarkSynced(version); }

void TableWidgetO::MarkSynced(int version)
{
    tmp_node_.version = version;
    sync_state_ = SyncState::kSynced;
}

bool TableWidgetO::HasPendingUpdate() const
{
    const bool node_pending { has_pending_update_ };
    const bool entry_pending { table_model_order_->HasPendingUpdate() };

    qDebug() << "[HasPendingUpdate]"
             << "node_pending =" << node_pending << ", entry_pending =" << entry_pending << ", result =" << (node_pending || entry_pending);

    return node_pending || entry_pending;
}

void TableWidgetO::RSyncDeltaO(const QUuid& node_id, double initial_delta, double final_delta, double count_delta, double measure_delta, double discount_delta)
{
    assert(node_id_ == node_id && "RSyncDelta called with mismatched node_id");

    {
        if (tmp_node_.direction_rule == Rule::kRO) {
            initial_delta *= -1;
            final_delta *= -1;
            count_delta *= -1;
            measure_delta *= -1;
            discount_delta *= -1;
        }
    }

    const double adjusted_final_delta { tmp_node_.unit == NodeUnit::OImmediate ? final_delta : 0.0 };

    {
        tmp_node_.count_total += count_delta;
        tmp_node_.measure_total += measure_delta;
        tmp_node_.initial_total += initial_delta;
        tmp_node_.discount_total += discount_delta;
        tmp_node_.final_total += adjusted_final_delta;

        IniUiValue();
    }
}

void TableWidgetO::IniWidget()
{
    {
        auto* pmodel { tree_model_partner_->IncludeUnitModel(section_ == Section::kSale ? NodeUnit::PCustomer : NodeUnit::PVendor, this) };
        ui->comboPartner->setModel(pmodel);
        ui->comboPartner->setCurrentIndex(-1);

        auto* emodel { tree_model_partner_->IncludeUnitModel(NodeUnit::PEmployee, this) };
        ui->comboEmployee->setModel(emodel);
        ui->comboEmployee->setCurrentIndex(-1);
    }

    {
        ui->tableViewO->setModel(table_model_order_);
        table_model_order_->setParent(ui->tableViewO);
    }

    {
        ui->dateTimeEdit->setDisplayFormat(kDateTimeFST);
    }

    {
        ui->dSpinDiscountTotal->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
        ui->dSpinInitialTotal->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
        ui->dSpinFinalTotal->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
        ui->dSpinMeasureTotal->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
        ui->dSpinCountTotal->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());

        ui->dSpinDiscountTotal->setDecimals(config_.amount_decimal);
        ui->dSpinInitialTotal->setDecimals(config_.amount_decimal);
        ui->dSpinFinalTotal->setDecimals(config_.amount_decimal);
        ui->dSpinMeasureTotal->setDecimals(config_.quantity_decimal);
        ui->dSpinCountTotal->setDecimals(config_.quantity_decimal);

        ui->dSpinDiscountTotal->setAlignment(Qt::AlignRight | Qt::AlignBottom);
        ui->dSpinFinalTotal->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
        ui->dSpinCountTotal->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui->dSpinMeasureTotal->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }

    {
        Utils::SetButton(ui->pBtnSave, tr("Save"), QKeySequence::Save);
        Utils::SetButton(ui->pBtnRelease, tr("Release"), QKeySequence(Qt::CTRL | Qt::Key_Return));
        Utils::SetButton(ui->pBtnRecall, tr("Recall"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
        Utils::SetButton(ui->pBtnPreview, tr("Preview"), QKeySequence(Qt::CTRL | Qt::Key_P));
        Utils::SetButton(ui->pBtnPrint, tr("Print"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
    }

    {
        auto& templates { PrintHub::Instance().TemplateMap() };
        for (auto it = templates.constBegin(); it != templates.constEnd(); ++it) {
            ui->comboTemplate->addItem(it.key(), it.value());
        }
    }
}

void TableWidgetO::IniConnect()
{
    connect(rule_group_, &QButtonGroup::idClicked, this, &TableWidgetO::RRuleGroupClicked);
    connect(unit_group_, &QButtonGroup::idClicked, this, &TableWidgetO::RUnitGroupClicked);
}

void TableWidgetO::IniData(const NodeO& node)
{
    {
        (node.direction_rule ? ui->rBtnRO : ui->rBtnTO)->setChecked(true);
        ui->dateTimeEdit->setDateTime(node.issued_time.toLocalTime());
        table_model_order_->SetNode(&tmp_node_);
    }

    {
        const NodeUnit kUnit { node.unit };

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
        LockWidgets(NodeStatus(node.status));
    }

    if (sync_state_ == SyncState::kLocalOnly)
        return;

    {
        IniUiValue();

        ui->lineDescription->setText(node.description);

        int partner_index { ui->comboPartner->findData(node.partner_id) };
        ui->comboPartner->setCurrentIndex(partner_index);

        int employee_index { ui->comboEmployee->findData(node.employee_id) };
        ui->comboEmployee->setCurrentIndex(employee_index);
    }
}

void TableWidgetO::LockWidgets(NodeStatus value)
{
    const bool unfinished { value == NodeStatus::kUnfinished };

    ui->comboPartner->setEnabled(unfinished);
    ui->comboEmployee->setEnabled(unfinished);

    ui->rBtnRO->setEnabled(sync_state_ == SyncState::kLocalOnly);
    ui->rBtnTO->setEnabled(sync_state_ == SyncState::kLocalOnly);

    ui->rBtnIS->setEnabled(unfinished);
    ui->rBtnMS->setEnabled(unfinished);
    ui->rBtnPEND->setEnabled(unfinished);

    ui->lineDescription->setEnabled(unfinished);
    ui->dateTimeEdit->setEnabled(unfinished);

    const bool can_print { !unfinished || tmp_node_.unit == NodeUnit::OPending };
    ui->pBtnPrint->setEnabled(can_print);

    ui->pBtnSave->setVisible(unfinished);
    ui->pBtnRelease->setVisible(unfinished && tmp_node_.unit != NodeUnit::OPending);
    ui->pBtnRecall->setVisible(!unfinished);
}

void TableWidgetO::IniUiValue()
{
    ui->dSpinFinalTotal->setValue(tmp_node_.final_total);
    ui->dSpinDiscountTotal->setValue(tmp_node_.discount_total);
    ui->dSpinCountTotal->setValue(tmp_node_.count_total);
    ui->dSpinMeasureTotal->setValue(tmp_node_.measure_total);
    ui->dSpinInitialTotal->setValue(tmp_node_.initial_total);
}

void TableWidgetO::IniRuleGroup()
{
    rule_group_ = new QButtonGroup(this);
    rule_group_->addButton(ui->rBtnRO, static_cast<int>(Rule::kRO));
    rule_group_->addButton(ui->rBtnTO, static_cast<int>(Rule::kTO));
}

void TableWidgetO::IniUnitGroup()
{
    unit_group_ = new QButtonGroup(this);
    unit_group_->addButton(ui->rBtnIS, std::to_underlying(NodeUnit::OImmediate));
    unit_group_->addButton(ui->rBtnMS, std::to_underlying(NodeUnit::OMonthly));
    unit_group_->addButton(ui->rBtnPEND, std::to_underlying(NodeUnit::OPending));
}

void TableWidgetO::on_comboPartner_currentIndexChanged(int /*index*/)
{
    const QUuid partner_id { ui->comboPartner->currentData().toUuid() };
    if (tmp_node_.partner_id == partner_id)
        return;

    tmp_node_.partner_id = partner_id;
    emit SSyncPartner(node_id_, partner_id);

    if (sync_state_ == SyncState::kSynced) {
        pending_update_.insert(kPartner, partner_id.toString(QUuid::WithoutBraces));
    }

    has_pending_update_ = true;
}

void TableWidgetO::on_comboEmployee_currentIndexChanged(int /*index*/)
{
    const QUuid employee_id { ui->comboEmployee->currentData().toUuid() };
    if (tmp_node_.employee_id == employee_id)
        return;

    tmp_node_.employee_id = employee_id;

    if (sync_state_ == SyncState::kSynced) {
        pending_update_.insert(kEmployee, employee_id.toString(QUuid::WithoutBraces));
    }

    has_pending_update_ = true;
}

void TableWidgetO::on_dateTimeEdit_dateTimeChanged(const QDateTime& date_time)
{
    const QDateTime utc_time { date_time.toUTC() };
    if (tmp_node_.issued_time == utc_time)
        return;

    tmp_node_.issued_time = utc_time;

    if (sync_state_ == SyncState::kSynced) {
        pending_update_.insert(kIssuedTime, utc_time.toString(Qt::ISODate));
    }

    has_pending_update_ = true;
}

void TableWidgetO::on_lineDescription_textChanged(const QString& arg1)
{
    if (tmp_node_.description == arg1)
        return;

    tmp_node_.description = arg1;

    if (sync_state_ == SyncState::kSynced) {
        pending_update_.insert(kDescription, arg1);
    }

    has_pending_update_ = true;
}

void TableWidgetO::RRuleGroupClicked(int id)
{
    if (sync_state_ == SyncState::kSynced)
        return;

    tmp_node_.direction_rule = static_cast<bool>(id);
    has_pending_update_ = true;

    tmp_node_.count_total *= -1;
    tmp_node_.measure_total *= -1;
    tmp_node_.initial_total *= -1;
    tmp_node_.discount_total *= -1;
    tmp_node_.final_total *= -1;

    IniUiValue();
}

void TableWidgetO::RUnitGroupClicked(int id)
{
    const NodeUnit unit { id };

    switch (unit) {
    case NodeUnit::OImmediate:
        tmp_node_.final_total = tmp_node_.initial_total - tmp_node_.discount_total;
        break;
    case NodeUnit::OMonthly:
        tmp_node_.final_total = 0.0;
        break;
    case NodeUnit::OPending:
        tmp_node_.final_total = 0.0;
        break;
    default:
        break;
    }

    tmp_node_.unit = NodeUnit(id);
    ui->dSpinFinalTotal->setValue(tmp_node_.final_total);

    const bool is_pending { unit == NodeUnit::OPending };
    ui->pBtnPrint->setEnabled(is_pending);
    ui->pBtnRelease->setHidden(is_pending);

    if (sync_state_ == SyncState::kSynced) {
        pending_update_.insert(kUnit, id);
    }

    has_pending_update_ = true;
}

void TableWidgetO::on_pBtnSave_clicked() { SaveOrder(); }

void TableWidgetO::on_pBtnPrint_clicked()
{
    if (!PreparePrint())
        return;

    PrintHub::Instance().Print();
}

void TableWidgetO::on_pBtnPreview_clicked()
{
    if (!PreparePrint())
        return;

    PrintHub::Instance().Preview();
}

bool TableWidgetO::PreparePrint()
{
    if (ui->comboTemplate->currentIndex() == -1) {
        QMessageBox::warning(this, tr("No Template"), tr("No printable template was found."));
        return false;
    }

    if (!PrintHub::Instance().LoadTemplate(ui->comboTemplate->currentData().toString())) {
        return false;
    }

    PrintHub::Instance().SetSectionConfig(&config_);
    PrintHub::Instance().SetValue(&tmp_node_, table_model_order_->GetEntryList());

    return true;
}

void TableWidgetO::BuildNodeInsert(QJsonObject& order_message)
{
    const QJsonObject node_json { tmp_node_.WriteJson() };

    QJsonObject path_json {};
    path_json.insert(kAncestor, tmp_node_.parent->id.toString(QUuid::WithoutBraces));
    path_json.insert(kDescendant, node_id_.toString(QUuid::WithoutBraces));

    order_message.insert(kNode, node_json);
    order_message.insert(kPath, path_json);
}

void TableWidgetO::BuildNodeUpdate(QJsonObject& order_message)
{
    if (pending_update_.contains(kUnit))
        pending_update_.insert(kFinalTotal, QString::number(tmp_node_.final_total, 'f', kMaxNumericScale_4));

    pending_update_.insert(kVersion, tmp_node_.version);

    order_message.insert(kNodeId, node_id_.toString(QUuid::WithoutBraces));
    order_message.insert(kPartnerId, tmp_node_.partner_id.toString(QUuid::WithoutBraces));
    order_message.insert(kNodeUpdate, pending_update_);
}

void TableWidgetO::on_pBtnRecall_clicked()
{
    Q_ASSERT(tmp_node_.status == NodeStatus::kFinished);

    if (!ValidatePartner())
        return;

    if (!ValidateSyncState())
        return;

    if (tmp_node_.is_settled) {
        QMessageBox::information(this, tr("Order Settled"), tr("This order has already been settled and cannot be operated."));
        return;
    }

    if (!tmp_node_.settlement_id.isNull()) {
        QMessageBox::information(this, tr("Order Selected"), tr("This order has already been selected in a settlement and cannot be operated."));
        return;
    }

    pending_update_.insert(kStatus, std::to_underlying(NodeStatus::kUnfinished));
    pending_update_.insert(kVersion, tmp_node_.version);

    WebSocket::Instance()->SendMessage(kOrderRecalled, JsonGen::OrderRecalled(section_, node_id_, pending_update_));

    sync_state_ = SyncState::kOutOfSync;
    pending_update_ = QJsonObject();
    has_pending_update_ = false;
}

bool TableWidgetO::ValidatePartner()
{
    if (tmp_node_.partner_id.isNull()) {
        QMessageBox::warning(this, tr("Partner Required"), tr("Please select a partner before performing this action."));
        return false;
    }

    qDebug() << "[ValidatePartner] Partner is set" << tree_model_partner_->Name(tmp_node_.partner_id);
    return true;
}

bool TableWidgetO::ValidateSyncState()
{
    if (sync_state_ == SyncState::kOutOfSync) {
        QMessageBox::information(
            this, tr("Invalid Operation"), tr("The operation you attempted is invalid because your local data is outdated. Please refresh and try again."));
        return false;
    }

    qDebug() << "[ValidateSyncState] Passed: sync_state =" << static_cast<int>(sync_state_);

    return true;
}

void TableWidgetO::SaveOrder()
{
    if (!ValidatePartner())
        return;

    if (!ValidateSyncState())
        return;

    if (!HasPendingUpdate())
        return;

    QJsonObject order_message { JsonGen::MetaMessage(section_) };
    table_model_order_->Finalize(order_message);

    if (sync_state_ == SyncState::kSynced) {
        BuildNodeUpdate(order_message);
        WebSocket::Instance()->SendMessage(kOrderUpdateSaved, order_message);

        pending_update_ = QJsonObject();
    }

    if (sync_state_ == SyncState::kLocalOnly) {
        BuildNodeInsert(order_message);
        WebSocket::Instance()->SendMessage(kOrderInsertSaved, order_message);

        ui->rBtnRO->setEnabled(false);
        ui->rBtnTO->setEnabled(false);
    }

    sync_state_ = SyncState::kOutOfSync;
    has_pending_update_ = false;
    ui->tableViewO->clearSelection();
}

void TableWidgetO::on_pBtnRelease_clicked()
{
    Q_ASSERT(tmp_node_.status == NodeStatus::kUnfinished);

    if (!ValidatePartner())
        return;

    if (!ValidateSyncState())
        return;

    QJsonObject order_message { JsonGen::MetaMessage(section_) };
    table_model_order_->Finalize(order_message);

    if (sync_state_ == SyncState::kSynced) {
        pending_update_.insert(kStatus, std::to_underlying(NodeStatus::kFinished));

        BuildNodeUpdate(order_message);

        WebSocket::Instance()->SendMessage(kOrderUpdateReleased, order_message);

        pending_update_ = QJsonObject();
    }

    if (sync_state_ == SyncState::kLocalOnly) {
        BuildNodeInsert(order_message);
        WebSocket::Instance()->SendMessage(kOrderInsertReleased, order_message);

        ui->rBtnRO->setEnabled(false);
        ui->rBtnTO->setEnabled(false);
    }

    sync_state_ = SyncState::kOutOfSync;
    has_pending_update_ = false;
    ui->tableViewO->clearSelection();
}
