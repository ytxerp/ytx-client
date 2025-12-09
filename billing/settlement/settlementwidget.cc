#include "settlementwidget.h"

#include <QJsonArray>
#include <QTimer>

#include "component/constant.h"
#include "component/signalblocker.h"
#include "enum/settlementenum.h"
#include "global/resourcepool.h"
#include "ui_settlementwidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SettlementWidget::SettlementWidget(SettlementModel* model, Section section, CUuid& widget_id, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SettlementWidget)
    , model_ { model }
    , start_ { QDateTime(QDate(QDate::currentDate().year() - 1, 1, 1), kStartTime) }
    , end_ { QDateTime(QDate(QDate::currentDate().year() + 1, 1, 1), kStartTime) }
    , section_ { section }
    , widget_id_ { widget_id }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(this);

    IniWidget();
    InitTimer();
    InitSharedPtr();

    QTimer::singleShot(0, this, &SettlementWidget::on_pBtnFetch_clicked);
}

SettlementWidget::~SettlementWidget() { delete ui; }

QTableView* SettlementWidget::View() const { return ui->tableView; }

void SettlementWidget::ResetUnsettledOrder(const QJsonArray& array)
{
    ResourcePool<SettlementNode>::Instance().Recycle(*unsettled_order_);

    for (const auto& value : array) {
        if (!value.isObject())
            continue;

        auto* settlement_node { ResourcePool<SettlementNode>::Instance().Allocate() };
        settlement_node->ReadJson(value.toObject());

        unsettled_order_->emplaceBack(settlement_node);
    }
}

void SettlementWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= end_.date() };
    start_.setDate(date);

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void SettlementWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
    end_ = QDateTime(date.addDays(1), kStartTime);
}

void SettlementWidget::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    const auto message { JsonGen::SettlementAcked(section_, widget_id_, start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(kSettlementAcked, message);

    cooldown_timer_->start(kTwoThousand);
}

void SettlementWidget::IniWidget()
{
    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);

    ui->pBtnFetch->setFocus();

    ui->start->setDateTime(start_);
    ui->end->setDateTime(end_.addSecs(-1));
}

void SettlementWidget::on_pBtnAppend_clicked()
{
    // Allocate a Settlement and wrap it in a shared_ptr with the deleter
    const std::shared_ptr<Settlement> settlement(ResourcePool<Settlement>::Instance().Allocate(), kSettlementDeleter);

    settlement->issued_time = QDateTime::currentDateTimeUtc();
    settlement->id = QUuid::createUuidV7();

    // Emit the signal with shared_ptr
    emit SSettlementNode(settlement, false, unsettled_order_);
}

void SettlementWidget::on_pBtnRemove_clicked()
{
    auto* view { ui->tableView };

    const auto index { view->selectionModel()->selectedIndexes().first() };
    if (!index.isValid())
        return;

    model_->removeRows(index.row(), 1);
}

void SettlementWidget::on_tableView_doubleClicked(const QModelIndex& index)
{
    if (index.column() != std::to_underlying(SettlementEnum::kAmount))
        return;

    auto& settlement { model_->SettlementAt(index.row()) };

    emit SSettlementNode(settlement, true, unsettled_order_);
}

void SettlementWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}

void SettlementWidget::InitSharedPtr()
{
    auto deleter = [](SettlementNodeList* list) {
        ResourcePool<SettlementNode>::Instance().Recycle(*list);
        delete list;
    };

    unsettled_order_ = std::shared_ptr<SettlementNodeList>(new SettlementNodeList(), deleter);
}
