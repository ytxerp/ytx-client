#include "tablewidgeto.h"

#include <QMessageBox>

#include "component/signalblocker.h"
#include "global/nodepool.h"
#include "global/printhub.h"
#include "ui_tablewidgeto.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableWidgetO::TableWidgetO(COrderWidgetArg& arg, QWidget* parent)
    : TableWidget(parent)
    , ui(new Ui::TableWidgetO)
    , node_ { arg.node }
    , tmp_node_ { *node_ }
    , table_model_order_ { qobject_cast<TableModelO*>(arg.table_model) }
    , tree_model_partner_ { arg.tree_model_partner }
    , config_ { arg.section_config }
    , is_persisted_ { arg.is_persisted }
    , node_id_ { tmp_node_.id }
    , section_ { arg.section }
{
    ui->setupUi(this);
    table_model_order_->setParent(this);
    table_model_order_->SetNode(&tmp_node_);
    SignalBlocker blocker(this);

    IniWidget();
    IniUnit(tmp_node_.unit);
    IniRuleGroup();
    IniUnitGroup();
    IniRule(tmp_node_.direction_rule);
    IniData(tmp_node_.partner, tmp_node_.employee);
    IniConnect();

    const bool released { tmp_node_.status == std::to_underlying(NodeStatus::kReleased) };
    ui->pBtnRecall->setChecked(released);

    LockWidgets(NodeStatus(tmp_node_.status));
}

TableWidgetO::~TableWidgetO()
{
    if (!is_persisted_) {
        NodePool::Instance().Recycle(node_, section_);
    }

    delete ui;
}

QTableView* TableWidgetO::View() const { return ui->tableViewO; }

bool TableWidgetO::HasUnsavedData() const { return node_modified_ || table_model_order_->HasInserts(); }

void TableWidgetO::RSyncDeltaO(
    const QUuid& node_id, double initial_delta, double final_delta, double count_delta, double measure_delta, double discount_delta, bool is_persisted)
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

    const double adjusted_final_delta { tmp_node_.unit == std::to_underlying(UnitO::kImmediate) ? final_delta : 0.0 };

    {
        if (is_persisted) {
            node_->count_total += count_delta;
            node_->measure_total += measure_delta;
            node_->initial_total += initial_delta;
            node_->discount_total += discount_delta;

            NodeUtils::UpdateOrderFinalTotal(node_);
        } else {
            count_delta_ += count_delta;
            measure_delta_ += measure_delta;
            initial_delta_ += initial_delta;
            discount_delta_ += discount_delta;
        }
    }

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
    auto* pmodel { tree_model_partner_->IncludeUnitModel(
        section_ == Section::kSale ? std::to_underlying(UnitP::kCustomer) : std::to_underlying(UnitP::kVendor), this) };
    ui->comboPartner->setModel(pmodel);
    ui->comboPartner->setCurrentIndex(-1);

    auto* emodel { tree_model_partner_->IncludeUnitModel(std::to_underlying(UnitP::kEmployee), this) };
    ui->comboEmployee->setModel(emodel);
    ui->comboEmployee->setCurrentIndex(-1);

    ui->tableViewO->setModel(table_model_order_);
    ui->dateTimeEdit->setDisplayFormat(kDateTimeFST);

    ui->dSpinDiscountTotal->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    ui->dSpinInitialTotal->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    ui->dSpinFinalTotal->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    ui->dSpinSecondTotal->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    ui->dSpinFirstTotal->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

    ui->dSpinDiscountTotal->setDecimals(config_.amount_decimal);
    ui->dSpinInitialTotal->setDecimals(config_.amount_decimal);
    ui->dSpinFinalTotal->setDecimals(config_.amount_decimal);
    ui->dSpinSecondTotal->setDecimals(config_.amount_decimal);
    ui->dSpinFirstTotal->setDecimals(config_.amount_decimal);

    ui->dSpinDiscountTotal->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    ui->dSpinFinalTotal->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    ui->dSpinFirstTotal->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->dSpinSecondTotal->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    ui->tableViewO->setFocus();

    ui->pBtnSave->setShortcut(QKeySequence::Save);
    ui->pBtnRelease->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    ui->pBtnRecall->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Return));

    auto& templates { PrintHub::Instance().TemplateMap() };
    for (auto it = templates.constBegin(); it != templates.constEnd(); ++it) {
        ui->comboTemplate->addItem(it.key(), it.value());
    }
}

void TableWidgetO::IniConnect()
{
    connect(rule_group_, &QButtonGroup::idClicked, this, &TableWidgetO::RRuleGroupClicked);
    connect(unit_group_, &QButtonGroup::idClicked, this, &TableWidgetO::RUnitGroupClicked);
}

void TableWidgetO::IniData(const QUuid& partner, const QUuid& employee)
{
    if (!is_persisted_) {
        const auto date_time { QDateTime::currentDateTimeUtc() };
        ui->dateTimeEdit->setDateTime(date_time.toLocalTime());
        tmp_node_.issued_time = date_time;
        return;
    }

    IniUiValue();

    ui->lineDescription->setText(tmp_node_.description);
    ui->dateTimeEdit->setDateTime(tmp_node_.issued_time.toLocalTime());

    int partner_index { ui->comboPartner->findData(partner) };
    ui->comboPartner->setCurrentIndex(partner_index);

    int employee_index { ui->comboEmployee->findData(employee) };
    ui->comboEmployee->setCurrentIndex(employee_index);
}

void TableWidgetO::LockWidgets(NodeStatus value)
{
    const bool recalled { value == NodeStatus::kRecalled };

    ui->comboPartner->setEnabled(recalled);
    ui->comboEmployee->setEnabled(recalled);

    ui->rBtnRO->setEnabled(!is_persisted_);
    ui->rBtnTO->setEnabled(!is_persisted_);

    ui->rBtnIS->setEnabled(recalled);
    ui->rBtnMS->setEnabled(recalled);
    ui->rBtnPEND->setEnabled(recalled);

    ui->lineDescription->setReadOnly(!recalled);
    ui->dateTimeEdit->setReadOnly(!recalled);

    const bool can_print { !recalled || tmp_node_.unit == std::to_underlying(UnitO::kPending) };
    ui->pBtnPrint->setEnabled(can_print);

    ui->pBtnSave->setHidden(!recalled);
    ui->pBtnRelease->setHidden(!recalled);
    ui->pBtnRecall->setHidden(recalled);
}

void TableWidgetO::IniUnit(int unit)
{
    const UnitO kUnit { unit };

    switch (kUnit) {
    case UnitO::kImmediate:
        ui->rBtnIS->setChecked(true);
        break;
    case UnitO::kMonthly:
        ui->rBtnMS->setChecked(true);
        break;
    case UnitO::kPending:
        ui->rBtnPEND->setChecked(true);
        break;
    default:
        break;
    }
}

void TableWidgetO::IniUiValue()
{
    ui->dSpinFinalTotal->setValue(tmp_node_.final_total);
    ui->dSpinDiscountTotal->setValue(tmp_node_.discount_total);
    ui->dSpinFirstTotal->setValue(tmp_node_.count_total);
    ui->dSpinSecondTotal->setValue(tmp_node_.measure_total);
    ui->dSpinInitialTotal->setValue(tmp_node_.initial_total);
}

void TableWidgetO::SyncNodeDelta(const QJsonObject& delta)
{
    if (delta.isEmpty())
        return;

    const double initial_delta { delta.value(kInitialDelta).toString().toDouble() };
    const double count_delta { delta.value(kCountDelta).toString().toDouble() };
    const double measure_delta { delta.value(kMeasureDelta).toString().toDouble() };
    const double discount_delta { delta.value(kDiscountDelta).toString().toDouble() };

    tmp_node_.count_total += count_delta;
    tmp_node_.measure_total += measure_delta;
    tmp_node_.initial_total += initial_delta;
    tmp_node_.discount_total += discount_delta;

    NodeUtils::UpdateOrderFinalTotal(&tmp_node_);
    IniUiValue();
}

void TableWidgetO::IniRule(bool rule) { (rule ? ui->rBtnRO : ui->rBtnTO)->setChecked(true); }

void TableWidgetO::IniRuleGroup()
{
    rule_group_ = new QButtonGroup(this);
    rule_group_->addButton(ui->rBtnRO, static_cast<int>(Rule::kRO));
    rule_group_->addButton(ui->rBtnTO, static_cast<int>(Rule::kTO));
}

void TableWidgetO::IniUnitGroup()
{
    unit_group_ = new QButtonGroup(this);
    unit_group_->addButton(ui->rBtnIS, std::to_underlying(UnitO::kImmediate));
    unit_group_->addButton(ui->rBtnMS, std::to_underlying(UnitO::kMonthly));
    unit_group_->addButton(ui->rBtnPEND, std::to_underlying(UnitO::kPending));
}

void TableWidgetO::on_comboPartner_currentIndexChanged(int /*index*/)
{
    const QUuid partner_id { ui->comboPartner->currentData().toUuid() };
    if (tmp_node_.partner == partner_id)
        return;

    tmp_node_.partner = partner_id;
    emit SSyncPartner(node_id_, partner_id);

    if (is_persisted_) {
        pending_updates_.insert(kPartner, partner_id.toString(QUuid::WithoutBraces));
    }

    node_modified_ = true;
}

void TableWidgetO::on_comboEmployee_currentIndexChanged(int /*index*/)
{
    const QUuid employee_id { ui->comboEmployee->currentData().toUuid() };
    if (tmp_node_.employee == employee_id)
        return;

    tmp_node_.employee = employee_id;

    if (is_persisted_) {
        pending_updates_.insert(kEmployee, employee_id.toString(QUuid::WithoutBraces));
    }

    node_modified_ = true;
}

void TableWidgetO::on_dateTimeEdit_dateTimeChanged(const QDateTime& date_time)
{
    const QDateTime utc_time { date_time.toUTC() };
    if (tmp_node_.issued_time == utc_time)
        return;

    tmp_node_.issued_time = utc_time;

    if (is_persisted_) {
        pending_updates_.insert(kIssuedTime, utc_time.toString(Qt::ISODate));
    }

    node_modified_ = true;
}

void TableWidgetO::on_lineDescription_textChanged(const QString& arg1)
{
    if (tmp_node_.description == arg1)
        return;

    tmp_node_.description = arg1;

    if (is_persisted_) {
        pending_updates_.insert(kDescription, arg1);
    }

    node_modified_ = true;
}

void TableWidgetO::RRuleGroupClicked(int id)
{
    if (is_persisted_)
        return;

    tmp_node_.direction_rule = static_cast<bool>(id);
    node_modified_ = true;

    tmp_node_.count_total *= -1;
    tmp_node_.measure_total *= -1;
    tmp_node_.initial_total *= -1;
    tmp_node_.discount_total *= -1;
    tmp_node_.final_total *= -1;

    IniUiValue();
}

void TableWidgetO::RUnitGroupClicked(int id)
{
    const UnitO unit { id };

    switch (unit) {
    case UnitO::kImmediate:
        tmp_node_.final_total = tmp_node_.initial_total - tmp_node_.discount_total;
        ui->pBtnRelease->setEnabled(true);
        break;
    case UnitO::kMonthly:
        tmp_node_.final_total = 0.0;
        ui->pBtnRelease->setEnabled(true);
        break;
    case UnitO::kPending:
        tmp_node_.final_total = 0.0;
        ui->pBtnRelease->setEnabled(false);
        break;
    default:
        break;
    }

    tmp_node_.unit = id;
    ui->dSpinFinalTotal->setValue(tmp_node_.final_total);
    ui->pBtnPrint->setEnabled(unit == UnitO::kPending);

    if (is_persisted_) {
        pending_updates_.insert(kUnit, id);
    }

    node_modified_ = true;
}

void TableWidgetO::on_pBtnSave_clicked() { SaveOrder(); }

void TableWidgetO::on_pBtnPrint_clicked()
{
    PreparePrint();
    PrintHub::Instance().Print();
}

void TableWidgetO::on_pBtnPreview_clicked()
{
    PreparePrint();
    PrintHub::Instance().Preview();
}

void TableWidgetO::PreparePrint()
{
    PrintHub::Instance().LoadTemplate(ui->comboTemplate->currentData().toString());
    PrintHub::Instance().SetValue(&tmp_node_, table_model_order_->GetEntryList());
}

QJsonObject TableWidgetO::BuildOrderCache()
{
    QJsonObject order_cache {};
    order_cache.insert(kSection, std::to_underlying(section_));
    order_cache.insert(kSessionId, QString());

    table_model_order_->FinalizeInserts(order_cache);

    return order_cache;
}

void TableWidgetO::BuildPartnerDelta(QJsonObject& order_cache)
{
    if (node_->unit == std::to_underlying(UnitO::kMonthly) && HasPartnerDelta()) {
        QJsonObject partner_delta {};

        partner_delta.insert(kInitialDelta, QString::number(node_->initial_total, 'f', kMaxNumericScale_4));
        partner_delta.insert(kId, node_->partner.toString(QUuid::WithoutBraces));

        order_cache.insert(kPartnerDelta, partner_delta);
    }
}

void TableWidgetO::BuildNodeInsert(QJsonObject& order_cache)
{
    const QJsonObject node_json { tmp_node_.WriteJson() };

    QJsonObject path_json {};
    path_json.insert(kAncestor, tmp_node_.parent->id.toString(QUuid::WithoutBraces));
    path_json.insert(kDescendant, node_id_.toString(QUuid::WithoutBraces));

    order_cache.insert(kNode, node_json);
    order_cache.insert(kPath, path_json);
}

void TableWidgetO::BuildNodeUpdate(QJsonObject& order_cache)
{
    if (HasOrderDelta()) {
        QJsonObject node_delta {};

        node_delta.insert(kInitialDelta, QString::number(initial_delta_, 'f', kMaxNumericScale_4));
        node_delta.insert(kCountDelta, QString::number(count_delta_, 'f', kMaxNumericScale_4));
        node_delta.insert(kMeasureDelta, QString::number(measure_delta_, 'f', kMaxNumericScale_4));
        node_delta.insert(kDiscountDelta, QString::number(discount_delta_, 'f', kMaxNumericScale_4));
        node_delta.insert(kId, node_id_.toString(QUuid::WithoutBraces));

        order_cache.insert(kNodeDelta, node_delta);
    }

    order_cache.insert(kNodeId, node_id_.toString(QUuid::WithoutBraces));
    order_cache.insert(kNodeCache, pending_updates_);
}

bool TableWidgetO::HasOrderDelta() const
{
    return FloatChanged(initial_delta_, 0.0) || FloatChanged(count_delta_, 0.0) || FloatChanged(measure_delta_, 0.0) || FloatChanged(discount_delta_, 0.0);
}

bool TableWidgetO::HasPartnerDelta() const { return FloatChanged(initial_delta_, 0.0); }

void TableWidgetO::SyncNode()
{
    node_->ReadJson(pending_updates_);

    node_->count_total += count_delta_;
    node_->measure_total += measure_delta_;
    node_->initial_total += initial_delta_;
    node_->discount_total += discount_delta_;

    NodeUtils::UpdateOrderFinalTotal(node_);
}

void TableWidgetO::ResetCache()
{
    pending_updates_ = QJsonObject();
    initial_delta_ = 0.0;
    count_delta_ = 0.0;
    measure_delta_ = 0.0;
    discount_delta_ = 0.0;
}

void TableWidgetO::on_pBtnRecall_clicked()
{
    if (node_->id.isNull()) {
        QMessageBox::information(this, tr("Order Deleted"),
            tr("This order has already been deleted by another client.\n"
               "You cannot perform recall operation."));
        return;
    }

    if (!node_->settlement.isNull()) {
        QMessageBox::information(this, tr("Order Settled"), tr("This order has already been settled and cannot be modified."));
        return;
    }

    if (node_->status == std::to_underlying(NodeStatus::kRecalled))
        return;

    tmp_node_.status = std::to_underlying(NodeStatus::kRecalled);
    node_->status = std::to_underlying(NodeStatus::kRecalled);

    WebSocket::Instance()->SendMessage(kOrderRecalled, JsonGen::OrderRecalled(section_, node_));

    LockWidgets(NodeStatus::kRecalled);
    emit SNodeStatus(node_id_, NodeStatus::kRecalled);
}

void TableWidgetO::SaveOrder()
{
    if (node_->id.isNull()) {
        QMessageBox::information(this, tr("Order Deleted"),
            tr("This order has already been deleted by another client.\n"
               "You cannot perform recall operation."));
        return;
    }

    if (node_->status == std::to_underlying(NodeStatus::kReleased)) {
        QMessageBox::information(this, tr("Save Not Allowed"),
            tr("This order has already been released on another client.\n"
               "Your local changes cannot be saved."));
        return;
    }

    if (tmp_node_.partner.isNull()) {
        QMessageBox::warning(this, tr("Partner Required"), tr("Please select a partner before saving or releasing the order."));
        return;
    }

    if (!HasUnsavedData())
        return;

    QJsonObject order_cache { BuildOrderCache() };

    if (is_persisted_) {
        SyncNode();

        BuildNodeUpdate(order_cache);
        WebSocket::Instance()->SendMessage(kOrderUpdateSaved, order_cache);

        ResetCache();
    } else {
        *node_ = tmp_node_;

        BuildNodeInsert(order_cache);
        WebSocket::Instance()->SendMessage(kOrderInsertSaved, order_cache);

        is_persisted_ = true;

        ui->rBtnRO->setEnabled(false);
        ui->rBtnTO->setEnabled(false);

        emit SInsertOrder();
    }

    node_modified_ = false;
}

void TableWidgetO::on_pBtnRelease_clicked()
{
    if (node_->id.isNull()) {
        QMessageBox::information(this, tr("Order Deleted"),
            tr("This order has already been deleted by another client.\n"
               "You cannot perform recall operation."));
        return;
    }

    if (node_->status == std::to_underlying(NodeStatus::kReleased)) {
        QMessageBox::information(this, tr("Release Not Allowed"),
            tr("This order has already been released by another client.\n"
               "You cannot release it again."));
        return;
    }

    if (tmp_node_.partner.isNull()) {
        QMessageBox::warning(this, tr("Partner Required"), tr("Please select a partner before saving or releasing the order."));
        return;
    }

    pending_updates_.insert(kStatus, std::to_underlying(NodeStatus::kReleased));
    tmp_node_.status = std::to_underlying(NodeStatus::kReleased);

    QJsonObject order_cache { BuildOrderCache() };

    if (is_persisted_) {
        SyncNode();

        BuildNodeUpdate(order_cache);
        order_cache.insert(kPartnerId, tmp_node_.partner.toString(QUuid::WithoutBraces));

        WebSocket::Instance()->SendMessage(kOrderUpdateReleased, order_cache);

        ResetCache();
    } else {
        *node_ = tmp_node_;

        BuildNodeInsert(order_cache);
        WebSocket::Instance()->SendMessage(kOrderInsertReleased, order_cache);

        is_persisted_ = true;

        emit SInsertOrder();
    }

    node_modified_ = false;
    LockWidgets(NodeStatus::kReleased);

    // ready for print
    ui->pBtnPrint->setFocus();
    ui->pBtnPrint->setDefault(true);
    ui->tableViewO->clearSelection();

    emit SNodeStatus(node_id_, NodeStatus::kReleased);
}
