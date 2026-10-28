#include "settlement_view_model.h"

#include "global/resourcepool.h"
#include "utils/templateutils.h"

namespace settlement_view {

Model::Model(const QHash<QUuid, QString>& partner_leaf_path, QObject* parent)
    : QAbstractItemModel(parent)
    , partner_leaf_path_ { partner_leaf_path }
{
}

Model::~Model() { ResourcePool<Row>::Instance().Recycle(list_); }

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    if (section < 0 || section >= header_.size())
        return {};

    return header_.at(section).title;
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, list_.at(row));
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return {};

    const auto* row { static_cast<const Row*>(index.internalPointer()) };
    const Column& column { header_.at(index.column()) };

    switch (column.type) {
    case ColumnType::kPartner:
        return row->partner_path;

    case ColumnType::kPreviousBalance:
        return row->previous_balance;

    case ColumnType::kMonth:
        return row->months.at(column.month_index);

    case ColumnType::kCurrentAmount:
        return row->current_amount;

    case ColumnType::kCurrentSettled:
        return row->current_settled;

    case ColumnType::kCurrentUnsettled:
        return row->current_unsettled;

    case ColumnType::kCurrentBalance:
        return row->current_balance;

    case ColumnType::kPlaceholder:
        return QVariant();
    }

    return {};
}

void Model::sort(int column, Qt::SortOrder order)
{
    if (column < 0 || column >= header_.size())
        return;

    const Column& e_column { header_.at(column) };

    if (e_column.type == ColumnType::kPlaceholder)
        return;

    auto Compare = [e_column, order](const Row* lhs, const Row* rhs) -> bool {
        switch (e_column.type) {
        case ColumnType::kPartner:
            return utils::CompareString(lhs->partner_path, rhs->partner_path, order);

        case ColumnType::kPreviousBalance:
            return utils::CompareValue(lhs->previous_balance, rhs->previous_balance, order);

        case ColumnType::kMonth:
            return utils::CompareValue(lhs->months.at(e_column.month_index), rhs->months.at(e_column.month_index), order);

        case ColumnType::kCurrentAmount:
            return utils::CompareValue(lhs->current_amount, rhs->current_amount, order);

        case ColumnType::kCurrentSettled:
            return utils::CompareValue(lhs->current_settled, rhs->current_settled, order);

        case ColumnType::kCurrentUnsettled:
            return utils::CompareValue(lhs->current_unsettled, rhs->current_unsettled, order);

        case ColumnType::kCurrentBalance:
            return utils::CompareValue(lhs->current_balance, rhs->current_balance, order);

        case ColumnType::kPlaceholder:
            return false;
        }

        return false;
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(list_, Compare);
    emit layoutChanged();
}

// Settlement view presentation:
// - Month columns show order amounts grouped by issued time.
// - Current Amount equals the sum of all month columns.
// - Data rows are sorted by current balance in descending order.
// - Summary values are emitted separately for the summary view.
void Model::Rebuild(const QJsonArray& array)
{
    if (array.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Received empty array";
    }

    QList<Row*> new_rows {};
    new_rows.reserve(array.size());

    Row summary {};
    summary.months.resize(columnCount() - 7);

    for (const auto& value : array) {
        Q_ASSERT(value.isObject());

        auto* row { ResourcePool<Row>::Instance().Allocate() };
        row->ReadJson(value.toObject());
        row->partner_path = partner_leaf_path_.value(row->partner_id);

        summary.Accumulate(*row);
        new_rows.emplaceBack(row);
    }

    std::ranges::sort(
        new_rows, [](const Row* lhs, const Row* rhs) { return utils::CompareValue(lhs->current_balance, rhs->current_balance, Qt::DescendingOrder); });

    beginResetModel();

    ResourcePool<Row>::Instance().Recycle(list_);
    list_ = std::move(new_rows);

    endResetModel();

    Q_ASSERT(summary.SummaryValues().size() == columnCount());

    emit SSummaryChanged(summary.SummaryValues());
}

bool Model::RebuildHeader(utils::DateRange date_range)
{
    Q_ASSERT(date_range.IsValid());
    qDebug() << Q_FUNC_INFO;

    if (range_.start == date_range.start && range_.end == date_range.end) {
        return false;
    }

    beginResetModel();

    header_.clear();
    ResourcePool<Row>::Instance().Recycle(list_);

    header_.append({ ColumnType::kPartner, tr("Partner") });
    header_.append({ ColumnType::kPreviousBalance, tr("Previous Balance") });

    int month_index { 0 };

    QDate date { date_range.start.year(), date_range.start.month(), 1 };
    const QDate end { date_range.end.year(), date_range.end.month(), 1 };

    while (date <= end) {
        header_.append({ ColumnType::kMonth, date.toString(QStringLiteral("yyyy-MM")), month_index++ });

        date = date.addMonths(1);
    }

    header_.append({ ColumnType::kCurrentAmount, tr("Current Amount") });
    header_.append({ ColumnType::kPlaceholder, QString() });
    header_.append({ ColumnType::kCurrentSettled, tr("Current Settled") });
    header_.append({ ColumnType::kCurrentUnsettled, tr("Current Unsettled") });
    header_.append({ ColumnType::kCurrentBalance, tr("Current Balance") });

    range_ = date_range;

    endResetModel();

    return true;
}
}
