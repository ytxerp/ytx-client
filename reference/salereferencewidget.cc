#include "salereferencewidget.h"

#include "component/constant.h"
#include "component/signalblocker.h"
#include "ui_salereferencewidget.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

SaleReferenceWidget::SaleReferenceWidget(SaleReferenceModel* model, Section section, CUuid& widget_id, CUuid& node_id, int node_unit, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SaleReferenceWidget)
    , start_ { QDateTime(QDate(QDate::currentDate().year() - 1, 1, 1), kStartTime) }
    , end_ { QDateTime(QDate(QDate::currentDate().year() + 1, 1, 1), kStartTime) }
    , model_ { model }
    , node_id_ { node_id }
    , widget_id_ { widget_id }
    , node_unit_ { node_unit }
    , section_ { section }
{
    ui->setupUi(this);
    SignalBlocker blocker(this);

    ui->tableView->setModel(model);
    model->setParent(this);

    IniWidget();
    InitTimer();

    QTimer::singleShot(0, this, &SaleReferenceWidget::on_pBtnFetch_clicked);
}

SaleReferenceWidget::~SaleReferenceWidget() { delete ui; }

QTableView* SaleReferenceWidget::View() const { return ui->tableView; }

void SaleReferenceWidget::on_start_dateChanged(const QDate& date)
{
    const bool valid { date <= end_.date() };
    start_.setDate(date);

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
}

void SaleReferenceWidget::on_end_dateChanged(const QDate& date)
{
    const bool valid { date >= start_.date() };

    cooldown_timer_->stop();
    ui->pBtnFetch->setEnabled(valid);
    end_ = QDateTime(date.addDays(1), kStartTime);
}

void SaleReferenceWidget::on_pBtnFetch_clicked()
{
    if (!ui->pBtnFetch->isEnabled()) {
        return;
    }

    ui->pBtnFetch->setEnabled(false);

    const auto message { JsonGen::SaleReference(section_, widget_id_, node_id_, node_unit_, start_.toUTC(), end_.toUTC()) };
    WebSocket::Instance()->SendMessage(kSaleReference, message);

    cooldown_timer_->start(kTwoThousand);
}

void SaleReferenceWidget::IniWidget()
{
    ui->start->setDisplayFormat(kDateFST);
    ui->end->setDisplayFormat(kDateFST);
    ui->start->setDateTime(start_);
    ui->end->setDateTime(end_.addSecs(-1));

    ui->pBtnFetch->setFocus();
}

void SaleReferenceWidget::InitTimer()
{
    cooldown_timer_ = new QTimer(this);
    cooldown_timer_->setSingleShot(true);
    connect(cooldown_timer_, &QTimer::timeout, this, [this]() { ui->pBtnFetch->setEnabled(true); });
}
