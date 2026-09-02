#include "settlement_view_model.h"

#include "utils/templateutils.h"

namespace settlement_view {

Model::Model(const QHash<QUuid, QString>* partner_leaf_path, QObject* parent)
    : QAbstractItemModel(parent)
    , partner_leaf_path_ { partner_leaf_path }
{
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    if (section < 0 || section >= columns_.size())
        return {};

    return columns_.at(section).title;
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex Model::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int Model::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return rows_.size();
}

int Model::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return columns_.size();
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return {};

    const Row& row { rows_.at(index.row()) };
    const Column& column { columns_.at(index.column()) };

    if (column.value_index == -1)
        return partner_leaf_path_->value(row.partner_id);

    return row.values.at(column.value_index);
}

void Model::sort(int column, Qt::SortOrder order)
{
    if (column < 0 || column >= columns_.size())
        return;

    const Column& e_column { columns_.at(column) };

    auto Compare = [this, e_column, order](const Row& lhs, const Row& rhs) -> bool {
        if (e_column.value_index == -1) {
            return utils::CompareString(partner_leaf_path_->value(lhs.partner_id), partner_leaf_path_->value(rhs.partner_id), order);
        }

        return utils::CompareValue(lhs.values.at(e_column.value_index), rhs.values.at(e_column.value_index), order);
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(rows_, Compare);
    emit layoutChanged();
}

void Model::Rebuild(const QJsonArray& array)
{
    Q_ASSERT(date_range_.IsValid());

    if (array.isEmpty())
        qDebug() << Q_FUNC_INFO << "Received empty array";

    QList<Row> new_rows {};
    new_rows.reserve(array.size());

    for (const auto& value : array) {
        if (!value.isObject()) {
            qWarning() << Q_FUNC_INFO << "Invalid data, expected object:" << value;
            continue;
        }

        Row row {};
        row.ReadJson(value.toObject());

        new_rows.emplaceBack(std::move(row));
    }

    std::ranges::sort(new_rows, [](const Row& lhs, const Row& rhs) { return utils::CompareValue(lhs.values.back(), rhs.values.back(), Qt::DescendingOrder); });

    beginResetModel();

    RebuildHeader(date_range_);
    rows_ = std::move(new_rows);

    endResetModel();
}

void Model::RebuildHeader(const utils::DateRange& date_range)
{
    Q_ASSERT(date_range.IsValid());

    beginResetModel();

    columns_.clear();
    rows_.clear();

    columns_.append({ tr("Partner"), -1 });
    columns_.append({ tr("Previous Balance"), 0 });

    int value_index { 1 };

    QDate date { date_range.start.year(), date_range.start.month(), 1 };
    const QDate end { date_range.end.year(), date_range.end.month(), 1 };

    while (date <= end) {
        columns_.append({ date.toString(QStringLiteral("yyyy-MM")), value_index++ });

        date = date.addMonths(1);
    }

    columns_.append({ tr("Current Amount"), value_index++ });
    columns_.append({ tr("Current Settled"), value_index++ });
    columns_.append({ tr("Current Unsettled"), value_index++ });
    columns_.append({ tr("Current Balance"), value_index++ });

    endResetModel();
}
}
