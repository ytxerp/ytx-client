#include "tablewidgeto.h"

#include <QMessageBox>

#include "component/signalblocker.h"
#include "global/nodepool.h"
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
    , is_new_ { arg.is_new }
    , node_id_ { tmp_node_.id }
    , section_ { arg.section }
    , print_template_ { arg.print_template }
    , print_manager_ { arg.app_config, arg.tree_model_inventory, arg.tree_model_partner }
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
    ui->pBtnStatus->setChecked(released);

    IniStatus(released);
    LockWidgets(released);
}

TableWidgetO::~TableWidgetO()
{
    if (is_new_) {
        NodePool::Instance().Recycle(node_, section_);
    }

    delete ui;
}

QTableView* TableWidgetO::View() const { return ui->tableViewO; }

bool TableWidgetO::HasUnsavedData() const
{
    const bool has_delta { initial_delta_ != 0.0 || final_delta_ != 0.0 || count_delta_ != 0.0 || measure_delta_ != 0.0 || discount_delta_ != 0.0 };

    return has_delta || !node_cache_.isEmpty() || table_model_order_->HasUnsavedData();
}

void TableWidgetO::RSyncDelta(const QUuid& node_id, double initial_delta, double final_delta, double count_delta, double measure_delta, double discount_delta)
{
    if (node_id_ != node_id)
        return;

    if (tmp_node_.direction_rule == Rule::kRO) {
        initial_delta *= -1;
        final_delta *= -1;
        count_delta *= -1;
        measure_delta *= -1;
        discount_delta *= -1;
    }

    const double adjusted_final_delta { tmp_node_.unit == std::to_underlying(UnitO::kImmediate) ? final_delta : 0.0 };

    tmp_node_.count_total += count_delta;
    tmp_node_.measure_total += measure_delta;
    tmp_node_.initial_total += initial_delta;
    tmp_node_.discount_total += discount_delta;
    tmp_node_.final_total += adjusted_final_delta;

    if (!is_new_) {
        count_delta_ += count_delta;
        measure_delta_ += measure_delta;
        initial_delta_ += initial_delta;
        discount_delta_ += discount_delta;
        final_delta_ += adjusted_final_delta;
    }

    IniUiValue();
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

    for (auto it = print_template_.constBegin(); it != print_template_.constEnd(); ++it) {
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
    if (is_new_) {
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

void TableWidgetO::LockWidgets(bool released)
{
    const bool enable { !released };

    ui->comboPartner->setEnabled(enable);
    ui->comboEmployee->setEnabled(enable);

    ui->rBtnRO->setEnabled(enable);
    ui->rBtnTO->setEnabled(enable);
    ui->rBtnIS->setEnabled(enable);
    ui->rBtnMS->setEnabled(enable);
    ui->rBtnPEND->setEnabled(enable);

    ui->lineDescription->setReadOnly(released);
    ui->dateTimeEdit->setReadOnly(released);

    const bool can_print { released || tmp_node_.unit == std::to_underlying(UnitO::kPending) };
    ui->pBtnPrint->setEnabled(can_print);
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

void TableWidgetO::IniRule(bool rule) { (rule ? ui->rBtnRO : ui->rBtnTO)->setChecked(true); }

void TableWidgetO::IniStatus(bool released)
{
    ui->pBtnStatus->setText(released ? tr("Recall") : tr("Release"));
    ui->pBtnStatus->setEnabled(tmp_node_.unit != std::to_underlying(UnitO::kPending));

    if (released) {
        ui->pBtnPrint->setFocus();
        ui->pBtnPrint->setDefault(true);
        ui->tableViewO->clearSelection();
    }
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
    unit_group_->addButton(ui->rBtnIS, std::to_underlying(UnitO::kImmediate));
    unit_group_->addButton(ui->rBtnMS, std::to_underlying(UnitO::kMonthly));
    unit_group_->addButton(ui->rBtnPEND, std::to_underlying(UnitO::kPending));
}

void TableWidgetO::on_comboPartner_currentIndexChanged(int /*index*/)
{
    const QUuid partner_id { ui->comboPartner->currentData().toUuid() };
    if (partner_id.isNull())
        return;

    tmp_node_.partner = partner_id;
    emit SSyncPartner(node_id_, partner_id);

    if (!is_new_) {
        node_cache_.insert(kPartner, partner_id.toString(QUuid::WithoutBraces));
    }
}

void TableWidgetO::on_comboEmployee_currentIndexChanged(int /*index*/)
{
    const QUuid employee_id { ui->comboEmployee->currentData().toUuid() };
    tmp_node_.employee = employee_id;

    if (!is_new_) {
        node_cache_.insert(kEmployee, employee_id.toString(QUuid::WithoutBraces));
    }
}

void TableWidgetO::on_dateTimeEdit_dateTimeChanged(const QDateTime& date_time)
{
    tmp_node_.issued_time = date_time.toUTC();

    if (!is_new_) {
        node_cache_.insert(kIssuedTime, tmp_node_.issued_time.toString(Qt::ISODate));
    }
}

void TableWidgetO::on_lineDescription_textChanged(const QString& arg1)
{
    if (tmp_node_.description == arg1)
        return;

    tmp_node_.description = arg1;

    if (!is_new_) {
        node_cache_.insert(kDescription, arg1);
    }
}

void TableWidgetO::RRuleGroupClicked(int id)
{
    tmp_node_.direction_rule = static_cast<bool>(id);

    tmp_node_.count_total *= -1;
    tmp_node_.measure_total *= -1;
    tmp_node_.initial_total *= -1;
    tmp_node_.discount_total *= -1;
    tmp_node_.final_total *= -1;

    IniUiValue();

    if (!is_new_) {
        node_cache_.insert(kDirectionRule, tmp_node_.direction_rule);
        initial_delta_ = -initial_delta_;
        final_delta_ = -final_delta_;
        count_delta_ = -count_delta_;
        measure_delta_ = -measure_delta_;
        discount_delta_ = -discount_delta_;
    }
}

void TableWidgetO::RUnitGroupClicked(int id)
{
    const UnitO unit { id };

    switch (unit) {
    case UnitO::kImmediate:
        tmp_node_.final_total = tmp_node_.initial_total - tmp_node_.discount_total;
        final_delta_ += tmp_node_.final_total;
        ui->pBtnStatus->setEnabled(true);
        break;
    case UnitO::kMonthly:
        final_delta_ += -tmp_node_.final_total;
        tmp_node_.final_total = 0.0;
        ui->pBtnStatus->setEnabled(true);
        break;
    case UnitO::kPending:
        final_delta_ += -tmp_node_.final_total;
        tmp_node_.final_total = 0.0;
        ui->pBtnStatus->setEnabled(false);
        break;
    default:
        break;
    }

    tmp_node_.unit = id;
    ui->dSpinFinalTotal->setValue(tmp_node_.final_total);
    ui->pBtnPrint->setEnabled(unit == UnitO::kPending);

    if (!is_new_) {
        node_cache_.insert(kUnit, id);
    }
}

void TableWidgetO::on_pBtnStatus_toggled(bool checked)
{
    if (!tmp_node_.settlement.isNull()) {
        QMessageBox::information(this, tr("Order Settled"), tr("This order has already been settled and cannot be modified."));
        return;
    }

    tmp_node_.status = static_cast<int>(checked);
    node_->status = static_cast<int>(checked);

    if (!is_new_ && checked) {
        node_cache_.insert(kStatus, checked);
    }

    emit SSyncStatus(node_id_, checked);

    IniStatus(checked);
    LockWidgets(checked);

    if (checked) {
        ReleaseOrder();
    } else {
        WebSocket::Instance()->SendMessage(kOrderRecalled, JsonGen::OrderRecalled(section_, node_));
    }
}

void TableWidgetO::on_pBtnSave_clicked() { SaveOrder(); }

void TableWidgetO::on_pBtnPrint_clicked()
{
    PreparePrint();
    print_manager_.Print();
}

void TableWidgetO::on_pBtnPreview_clicked()
{
    PreparePrint();
    print_manager_.Preview();
}

void TableWidgetO::PreparePrint()
{
    print_manager_.LoadIni(ui->comboTemplate->currentData().toString());

    QString unit {};
    switch (UnitO(tmp_node_.unit)) {
    case UnitO::kMonthly:
        unit = tr("MS");
        break;
    case UnitO::kImmediate:
        unit = tr("IS");
        break;
    case UnitO::kPending:
        unit = tr("PEND");
        break;
    default:
        break;
    }

    PrintData data { tree_model_partner_->Name(tmp_node_.partner), tmp_node_.issued_time.toLocalTime().toString(kDateTimeFST),
        tree_model_partner_->Name(tmp_node_.employee), unit, tmp_node_.initial_total };
    print_manager_.SetData(data, table_model_order_->GetEntryShadowList());
}

void TableWidgetO::SaveOrder()
{
    if (!HasUnsavedData())
        return;

    *node_ = tmp_node_;

    QJsonObject order_cache { BuildOrderCache() };

    if (is_new_) {
        BuildNodeInsert(order_cache);
        WebSocket::Instance()->SendMessage(kOrderInsertSaved, order_cache);

        emit SInsertOrder();
        is_new_ = false;
    } else {
        BuildNodeUpdate(order_cache);
        WebSocket::Instance()->SendMessage(kOrderUpdateSaved, order_cache);

        ResetCache();
    }
}

void TableWidgetO::ReleaseOrder()
{
    if (!HasUnsavedData())
        return;

    *node_ = tmp_node_;

    QJsonObject order_cache { BuildOrderCache() };

    order_cache.insert(kPartnerDelta, BuildPartnerDelta());
    order_cache.insert(kPartnerId, node_->partner.toString(QUuid::WithoutBraces));

    if (is_new_) {
        BuildNodeInsert(order_cache);
        WebSocket::Instance()->SendMessage(kOrderInsertReleased, order_cache);

        emit SInsertOrder();
        is_new_ = false;
    } else {
        BuildNodeUpdate(order_cache);
        WebSocket::Instance()->SendMessage(kOrderUpdateReleased, order_cache);

        ResetCache();
    }
}

QJsonObject TableWidgetO::BuildOrderCache()
{
    QJsonObject order_cache {};
    order_cache.insert(kSection, std::to_underlying(section_));
    order_cache.insert(kSessionId, QString());

    table_model_order_->SaveOrder(order_cache);

    return order_cache;
}

QJsonObject TableWidgetO::BuildNodeDelta()
{
    QJsonObject node_delta {};
    node_delta.insert(kInitialDelta, QString::number(initial_delta_, 'f', kMaxNumericScale_4));
    node_delta.insert(kFinalDelta, QString::number(final_delta_, 'f', kMaxNumericScale_4));
    node_delta.insert(kCountDelta, QString::number(count_delta_, 'f', kMaxNumericScale_4));
    node_delta.insert(kMeasureDelta, QString::number(measure_delta_, 'f', kMaxNumericScale_4));
    node_delta.insert(kDiscountDelta, QString::number(discount_delta_, 'f', kMaxNumericScale_4));

    return node_delta;
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
    order_cache.insert(kNodeId, node_id_.toString(QUuid::WithoutBraces));
    order_cache.insert(kNodeCache, node_cache_);
    order_cache.insert(kNodeDelta, BuildNodeDelta());
}

QJsonObject TableWidgetO::BuildPartnerDelta()
{
    QJsonObject partner_delta {};
    partner_delta.insert(kInitialDelta, QString::number(node_->initial_total, 'f', kMaxNumericScale_4));
    partner_delta.insert(kFinalDelta, QString::number(node_->final_total, 'f', kMaxNumericScale_4));
    return partner_delta;
}

void TableWidgetO::ResetCache()
{
    node_cache_ = QJsonObject();
    initial_delta_ = 0.0;
    final_delta_ = 0.0;
    count_delta_ = 0.0;
    measure_delta_ = 0.0;
    discount_delta_ = 0.0;
}
