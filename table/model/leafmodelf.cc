#include "leafmodelf.h"

#include "component/constant.h"
#include "global/entryshadowpool.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

LeafModelF::LeafModelF(CLeafModelArg& arg, QObject* parent)
    : LeafModel { arg, parent }
{
}

QVariant LeafModelF::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* d_shadow = DerivedPtr<EntryShadowF>(shadow_list_.at(index.row()));

    const EntryEnumF kColumn { index.column() };

    switch (kColumn) {
    case EntryEnumF::kId:
        return *d_shadow->id;
    case EntryEnumF::kUserId:
        return *d_shadow->user_id;
    case EntryEnumF::kCreateTime:
        return *d_shadow->created_time;
    case EntryEnumF::kCreateBy:
        return *d_shadow->created_by;
    case EntryEnumF::kUpdateTime:
        return *d_shadow->updated_time;
    case EntryEnumF::kUpdateBy:
        return *d_shadow->updated_by;
    case EntryEnumF::kIssuedTime:
        return *d_shadow->issued_time;
    case EntryEnumF::kLhsNode:
        return *d_shadow->lhs_node;
    case EntryEnumF::kCode:
        return *d_shadow->code;
    case EntryEnumF::kLhsRate:
        return *d_shadow->lhs_rate;
    case EntryEnumF::kDescription:
        return *d_shadow->description;
    case EntryEnumF::kRhsNode:
        return *d_shadow->rhs_node;
    case EntryEnumF::kStatus:
        return *d_shadow->status;
    case EntryEnumF::kDocument:
        return *d_shadow->document;
    case EntryEnumF::kDebit:
        return *d_shadow->lhs_debit;
    case EntryEnumF::kCredit:
        return *d_shadow->lhs_credit;
    case EntryEnumF::kBalance:
        return d_shadow->balance;
    default:
        return QVariant();
    }
}

bool LeafModelF::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const EntryEnumF column { index.column() };
    const int row { index.row() };

    auto* shadow { shadow_list_.at(index.row()) };
    auto* d_shadow = DerivedPtr<EntryShadowF>(shadow);

    const QUuid id { *shadow->id };

    switch (column) {
    case EntryEnumF::kIssuedTime:
        EntryUtils::UpdateShadowIssuedTime(caches_[id], shadow, kIssuedTime, value.toDateTime(), &EntryShadow::issued_time, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumF::kCode:
        EntryUtils::UpdateShadowField(caches_[id], shadow, kCode, value.toString(), &EntryShadow::code, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumF::kStatus:
        EntryUtils::UpdateShadowField(caches_[id], shadow, kStatus, value.toInt(), &EntryShadow::status, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumF::kDescription:
        EntryUtils::UpdateShadowField(caches_[id], shadow, kDescription, value.toString(), &EntryShadow::description, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumF::kDocument:
        EntryUtils::UpdateShadowDocument(caches_[id], shadow, kDocument, value.toStringList(), &EntryShadow::document, [id, this]() { RestartTimer(id); });
        break;
    case EntryEnumF::kLhsRate:
        UpdateRate(d_shadow, value.toDouble());
        break;
    case EntryEnumF::kRhsNode:
        UpdateLinkedNode(d_shadow, value.toUuid(), row);
        break;
    case EntryEnumF::kDebit:
        UpdateNumeric(d_shadow, value.toDouble(), row, true);
        break;
    case EntryEnumF::kCredit:
        UpdateNumeric(d_shadow, value.toDouble(), row, false);
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

bool LeafModelF::UpdateLinkedNode(EntryShadow* entry_shadow, const QUuid& value, int row)
{
    if (value.isNull())
        return false;

    auto* d_shadow = DerivedPtr<EntryShadowF>(entry_shadow);
    auto old_node { *d_shadow->rhs_node };
    if (old_node == value)
        return false;

    *d_shadow->rhs_node = value;

    const QUuid entry_id { *d_shadow->id };

    const QString old_node_id { old_node.toString(QUuid::WithoutBraces) };
    const QString new_node_id { value.toString(QUuid::WithoutBraces) };

    QJsonObject cache {};
    cache = d_shadow->WriteJson();

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section_));
    message.insert(kSessionId, QString());
    message.insert(kEntry, cache);
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));

    if (old_node.isNull()) {
        const double lhs_rate { *d_shadow->lhs_rate };
        const double rhs_rate { *d_shadow->rhs_rate };

        const double lhs_debit { *d_shadow->lhs_debit };
        const double lhs_credit { *d_shadow->lhs_credit };

        const double lhs_initial_delta { lhs_debit - lhs_credit };
        const double lhs_final_delta { lhs_rate * lhs_initial_delta };

        const double rhs_initial_delta { -lhs_initial_delta };
        const double rhs_final_delta { rhs_rate * rhs_initial_delta };

        const bool has_leaf_delta { std::abs(lhs_initial_delta) > kTolerance };

        if (has_leaf_delta) {
            QJsonObject lhs_delta { JsonGen::NodeDelta(lhs_id_, lhs_initial_delta, lhs_final_delta) };
            QJsonObject rhs_delta { JsonGen::NodeDelta(value, rhs_initial_delta, rhs_final_delta) };

            message.insert(kLhsDelta, lhs_delta);
            message.insert(kRhsDelta, rhs_delta);
        }

        WebSocket::Instance()->SendMessage(kEntryInsert, message);

        if (has_leaf_delta) {
            AccumulateBalance(row);

            emit SResizeColumnToContents(std::to_underlying(EntryEnumF::kBalance));
            emit SSyncDelta(lhs_id_, lhs_initial_delta, lhs_final_delta);
            emit SSyncDelta(value, rhs_initial_delta, rhs_final_delta);
        }

        emit SAppendOneEntry(value, d_shadow->entry);
    }

    if (!old_node.isNull()) {
        // Indicates whether the EntryShadow's lhs_node corresponds to the Entry's lhs_node.
        // If true, the node is not collapsed; if false, it has been collapsed and flipped.
        const bool is_parallel { d_shadow->is_parallel };
        // If true, the EntryShadow corresponds to the left-side node in the Postgres table,
        // so we need to update the right-side node (kRhsNode).
        // If false, it means the entry is collapsed (lhs and rhs flipped), so update the left-side node (kLhsNode).
        const auto field { is_parallel ? kRhsNode : kLhsNode };

        const double rhs_rate { *d_shadow->rhs_rate };
        const double rhs_debit { *d_shadow->rhs_debit };
        const double rhs_credit { *d_shadow->rhs_credit };

        const double rhs_initial_delta { rhs_debit - rhs_credit };
        const double rhs_final_delta { rhs_rate * rhs_initial_delta };

        const bool has_leaf_delta { std::abs(rhs_initial_delta) > kTolerance };

        if (has_leaf_delta) {
            QJsonObject new_node_delta { JsonGen::NodeDelta(value, rhs_initial_delta, rhs_final_delta) };
            QJsonObject old_node_delta { JsonGen::NodeDelta(old_node, -rhs_initial_delta, -rhs_final_delta) };

            message.insert(kOldNodeDelta, old_node_delta);
            message.insert(kNewNodeDelta, new_node_delta);
        }

        message.insert(kOldNodeId, old_node_id);
        message.insert(kNewNodeId, new_node_id);
        message.insert(kField, field);

        WebSocket::Instance()->SendMessage(kEntryLinkedNode, message);

        if (has_leaf_delta) {
            AccumulateBalance(row);

            emit SResizeColumnToContents(std::to_underlying(EntryEnumF::kBalance));
            emit SSyncDelta(value, rhs_initial_delta, rhs_final_delta);
            emit SSyncDelta(old_node, -rhs_initial_delta, -rhs_final_delta);
        }

        emit SRemoveOneEntry(old_node, entry_id);
        emit SAppendOneEntry(value, d_shadow->entry);
    }

    return true;
}

bool LeafModelF::UpdateNumeric(EntryShadow* entry_shadow, double value, int row, bool is_debit)
{
    auto* d_shadow { DerivedPtr<EntryShadowF>(entry_shadow) };

    const double lhs_old_debit { *d_shadow->lhs_debit };
    const double lhs_old_credit { *d_shadow->lhs_credit };
    const double lhs_rate { *d_shadow->lhs_rate };
    const double rhs_rate { *d_shadow->rhs_rate };

    assert(lhs_rate != 0.0);
    assert(rhs_rate != 0.0);

    double lhs_original { is_debit ? lhs_old_debit : lhs_old_credit };
    if (std::abs(lhs_original - value) < kTolerance)
        return false;

    const double base { is_debit ? lhs_old_credit : lhs_old_debit };
    const double diff { qAbs(value - base) };

    const bool assign_debit { (is_debit && value > base) || (!is_debit && value <= base) };
    const bool assign_credit { !assign_debit };

    *d_shadow->lhs_debit = assign_debit ? diff : 0.0;
    *d_shadow->lhs_credit = assign_credit ? diff : 0.0;

    *d_shadow->rhs_debit = (*d_shadow->lhs_credit) * lhs_rate / rhs_rate;
    *d_shadow->rhs_credit = (*d_shadow->lhs_debit) * lhs_rate / rhs_rate;

    if (d_shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *d_shadow->id };
    const QUuid rhs_id { *d_shadow->rhs_node };

    QJsonObject cache = {};
    const bool is_parallel { entry_shadow->is_parallel };

    cache.insert(is_parallel ? kLhsDebit : kRhsDebit, QString::number(*d_shadow->lhs_debit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kLhsCredit : kRhsCredit, QString::number(*d_shadow->lhs_credit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kRhsDebit : kLhsDebit, QString::number(*d_shadow->rhs_debit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kRhsCredit : kLhsCredit, QString::number(*d_shadow->rhs_credit, 'f', kMaxNumericScale_4));

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section_));
    message.insert(kSessionId, QString());
    message.insert(kCache, cache);
    message.insert(kIsParallel, is_parallel);
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));

    const double lhs_initial_delta { *d_shadow->lhs_debit - *d_shadow->lhs_credit - (lhs_old_debit - lhs_old_credit) };
    const double lhs_final_delta { lhs_rate * lhs_initial_delta };

    const double rhs_initial_delta { -lhs_initial_delta };
    const double rhs_final_delta { rhs_rate * rhs_initial_delta };

    const bool has_leaf_delta { std::abs(lhs_initial_delta) > kTolerance };

    if (has_leaf_delta) {
        QJsonObject lhs_delta { JsonGen::NodeDelta(lhs_id_, lhs_initial_delta, lhs_final_delta) };
        QJsonObject rhs_delta { JsonGen::NodeDelta(*d_shadow->rhs_node, rhs_initial_delta, rhs_final_delta) };

        message.insert(kLhsDelta, lhs_delta);
        message.insert(kRhsDelta, rhs_delta);
    }

    WebSocket::Instance()->SendMessage(kEntryNumeric, message);

    if (has_leaf_delta) {
        AccumulateBalance(row);

        emit SResizeColumnToContents(std::to_underlying(EntryEnumF::kBalance));
        emit SUpdateBalance(rhs_id, *d_shadow->id);

        emit SSyncDelta(lhs_id_, lhs_initial_delta, lhs_final_delta);
        emit SSyncDelta(rhs_id, rhs_initial_delta, rhs_final_delta);
    }

    return true;
}

#if 0
bool LeafModelF::UpdateDebit(EntryShadow* entry_shadow, double value, int row)
{
    auto* d_shadow { DerivedPtr<EntryShadowF>(entry_shadow) };

    const double lhs_debit { *d_shadow->lhs_debit };
    if (std::abs(lhs_debit - value) < kTolerance)
        return false;

    const double lhs_credit { *d_shadow->lhs_credit };
    const double lhs_rate { *d_shadow->lhs_rate };

    const double abs { qAbs(value - lhs_credit) };
    *d_shadow->lhs_debit = (value > lhs_credit) ? abs : 0;
    *d_shadow->lhs_credit = (value <= lhs_credit) ? abs : 0;

    const double rhs_debit { *d_shadow->rhs_debit };
    const double rhs_credit { *d_shadow->rhs_credit };
    const double rhs_rate { *d_shadow->rhs_rate };

    *d_shadow->rhs_debit = (*d_shadow->lhs_credit) * lhs_rate / rhs_rate;
    *d_shadow->rhs_credit = (*d_shadow->lhs_debit) * lhs_rate / rhs_rate;

    if (d_shadow->rhs_node->isNull())
        return false;

    const double lhs_debit_delta { *d_shadow->lhs_debit - lhs_debit };
    const double lhs_credit_delta { *d_shadow->lhs_credit - lhs_credit };
    emit SSyncDelta(lhs_id_, lhs_debit_delta, lhs_credit_delta, lhs_debit_delta * lhs_rate, lhs_credit_delta * lhs_rate);

    const double rhs_debit_delta { *d_shadow->rhs_debit - rhs_debit };
    const double rhs_credit_delta { *d_shadow->rhs_credit - rhs_credit };
    emit SSyncDelta(*d_shadow->rhs_node, rhs_debit_delta, rhs_credit_delta, rhs_debit_delta * rhs_rate, rhs_credit_delta * rhs_rate);

    return true;
}

bool LeafModelF::UpdateCredit(EntryShadow* entry_shadow, double value, int row)
{
    auto* d_shadow { DerivedPtr<EntryShadowF>(entry_shadow) };

    const double lhs_credit { *d_shadow->lhs_credit };
    if (std::abs(lhs_credit - value) < kTolerance)
        return false;

    const double lhs_debit { *d_shadow->lhs_debit };
    const double lhs_rate { *d_shadow->lhs_rate };

    const double abs { qAbs(value - lhs_debit) };
    *d_shadow->lhs_debit = (value > lhs_debit) ? 0 : abs;
    *d_shadow->lhs_credit = (value <= lhs_debit) ? 0 : abs;

    const double rhs_debit { *d_shadow->rhs_debit };
    const double rhs_credit { *d_shadow->rhs_credit };
    const double rhs_rate { *d_shadow->rhs_rate };

    *d_shadow->rhs_debit = (*d_shadow->lhs_credit) * lhs_rate / rhs_rate;
    *d_shadow->rhs_credit = (*d_shadow->lhs_debit) * lhs_rate / rhs_rate;

    if (d_shadow->rhs_node->isNull())
        return false;

    const double lhs_debit_delta { *d_shadow->lhs_debit - lhs_debit };
    const double lhs_credit_delta { *d_shadow->lhs_credit - lhs_credit };
    emit SSyncDelta(lhs_id_, lhs_debit_delta, lhs_credit_delta, lhs_debit_delta * lhs_rate, lhs_credit_delta * lhs_rate);

    const double rhs_debit_delta { *d_shadow->rhs_debit - rhs_debit };
    const double rhs_credit_delta { *d_shadow->rhs_credit - rhs_credit };
    emit SSyncDelta(*d_shadow->rhs_node, rhs_debit_delta, rhs_credit_delta, rhs_debit_delta * rhs_rate, rhs_credit_delta * rhs_rate);

    return true;
}
#endif

bool LeafModelF::UpdateRate(EntryShadow* entry_shadow, double value)
{
    auto* d_shadow { DerivedPtr<EntryShadowF>(entry_shadow) };

    const double old_rate { *d_shadow->lhs_rate };
    if (std::abs(old_rate - value) < kTolerance || value <= 0)
        return false;

    const double delta { value - old_rate };
    const double proportion { value / old_rate };

    *d_shadow->lhs_rate = value;

    const double lhs_debit { *d_shadow->lhs_debit };
    const double lhs_credit { *d_shadow->lhs_credit };

    const double rhs_old_debit { *d_shadow->rhs_debit };
    const double rhs_old_credit { *d_shadow->rhs_credit };

    *d_shadow->rhs_debit *= proportion;
    *d_shadow->rhs_credit *= proportion;

    if (d_shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *d_shadow->id };
    const QUuid rhs_id { *d_shadow->rhs_node };

    QJsonObject cache {};

    const bool is_parallel { entry_shadow->is_parallel };
    cache.insert(is_parallel ? kLhsRate : kRhsRate, QString::number(*d_shadow->lhs_rate, 'f', kMaxNumericScale_8));
    cache.insert(is_parallel ? kRhsDebit : kLhsDebit, QString::number(*d_shadow->rhs_debit, 'f', kMaxNumericScale_4));
    cache.insert(is_parallel ? kRhsCredit : kLhsCredit, QString::number(*d_shadow->rhs_credit, 'f', kMaxNumericScale_4));

    QJsonObject message {};
    message.insert(kSection, std::to_underlying(section_));
    message.insert(kSessionId, QString());
    message.insert(kCache, cache);
    message.insert(kIsParallel, is_parallel);
    message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));

    const double lhs_initial_delta { 0.0 };
    const double lhs_final_delta { delta * (lhs_debit - lhs_credit) };

    const double rhs_initial_delta { *d_shadow->rhs_debit - *d_shadow->rhs_credit - (rhs_old_debit - rhs_old_credit) };
    const double rhs_final_delta { -lhs_final_delta };

    const bool has_leaf_delta { std::abs(lhs_final_delta) > kTolerance };

    if (has_leaf_delta) {
        QJsonObject lhs_delta { JsonGen::NodeDelta(lhs_id_, lhs_initial_delta, lhs_final_delta) };
        QJsonObject rhs_delta { JsonGen::NodeDelta(rhs_id, rhs_initial_delta, rhs_final_delta) };

        message.insert(kLhsDelta, lhs_delta);
        message.insert(kRhsDelta, rhs_delta);
    }

    WebSocket::Instance()->SendMessage(kEntryRate, message);

    if (has_leaf_delta) {
        emit SUpdateBalance(rhs_id, *d_shadow->id);
        emit SSyncDelta(lhs_id_, lhs_initial_delta, lhs_final_delta);
        emit SSyncDelta(rhs_id, rhs_initial_delta, rhs_final_delta);
    }

    return true;
}

void LeafModelF::sort(int column, Qt::SortOrder order)
{
    assert(column >= 0 && column <= info_.entry_header.size() - 1);

    const EntryEnumF kColumn { column };

    switch (kColumn) {
    case EntryEnumF::kId:
    case EntryEnumF::kBalance:
    case EntryEnumF::kUserId:
    case EntryEnumF::kCreateTime:
    case EntryEnumF::kCreateBy:
    case EntryEnumF::kUpdateTime:
    case EntryEnumF::kUpdateBy:
        return;
    default:
        break;
    }

    auto Compare = [order, kColumn](const EntryShadow* lhs, const EntryShadow* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryShadowF>(lhs) };
        auto* d_rhs { DerivedPtr<EntryShadowF>(rhs) };

        switch (kColumn) {
        case EntryEnumF::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (*lhs->issued_time < *rhs->issued_time) : (*lhs->issued_time > *rhs->issued_time);
        case EntryEnumF::kCode:
            return (order == Qt::AscendingOrder) ? (*lhs->code < *rhs->code) : (*lhs->code > *rhs->code);
        case EntryEnumF::kLhsRate:
            return (order == Qt::AscendingOrder) ? (*d_lhs->lhs_rate < *d_rhs->lhs_rate) : (*d_lhs->lhs_rate > *d_rhs->lhs_rate);
        case EntryEnumF::kDescription:
            return (order == Qt::AscendingOrder) ? (*lhs->description < *rhs->description) : (*lhs->description > *rhs->description);
        case EntryEnumF::kRhsNode:
            return (order == Qt::AscendingOrder) ? (*lhs->rhs_node < *rhs->rhs_node) : (*lhs->rhs_node > *rhs->rhs_node);
        case EntryEnumF::kStatus:
            return (order == Qt::AscendingOrder) ? (*lhs->status < *rhs->status) : (*lhs->status > *rhs->status);
        case EntryEnumF::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document->size() < rhs->document->size()) : (lhs->document->size() > rhs->document->size());
        case EntryEnumF::kDebit:
            return (order == Qt::AscendingOrder) ? (*d_lhs->lhs_debit < *d_rhs->lhs_debit) : (*d_lhs->lhs_debit > *d_rhs->lhs_debit);
        case EntryEnumF::kCredit:
            return (order == Qt::AscendingOrder) ? (*d_lhs->lhs_credit < *d_rhs->lhs_credit) : (*d_lhs->lhs_credit > *d_rhs->lhs_credit);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(shadow_list_.begin(), shadow_list_.end(), Compare);
    emit layoutChanged();

    AccumulateBalance(0);
}

Qt::ItemFlags LeafModelF::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const EntryEnumF kColumn { index.column() };

    switch (kColumn) {
    case EntryEnumF::kId:
    case EntryEnumF::kBalance:
    case EntryEnumF::kDocument:
    case EntryEnumF::kStatus:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}

double LeafModelF::CalculateBalance(EntryShadow* entry_shadow)
{
    auto* d_shadow { DerivedPtr<EntryShadowF>(entry_shadow) };
    return (direction_rule_ == Rule::kDICD ? 1 : -1) * (*d_shadow->lhs_debit - *d_shadow->lhs_credit);
}

bool LeafModelF::removeRows(int row, int /*count*/, const QModelIndex& parent)
{
    assert(row >= 0 && row <= rowCount(parent) - 1);

    auto* d_shadow = DerivedPtr<EntryShadowF>(shadow_list_.at(row));
    const auto rhs_node_id { *d_shadow->rhs_node };

    beginRemoveRows(parent, row, row);
    shadow_list_.removeAt(row);
    endRemoveRows();

    if (!rhs_node_id.isNull()) {
        const double lhs_initial_delta { *d_shadow->lhs_credit - *d_shadow->lhs_debit };
        const double lhs_final_delta { *d_shadow->lhs_rate * lhs_initial_delta };

        const double rhs_initial_delta { *d_shadow->rhs_credit - *d_shadow->rhs_debit };
        const double rhs_final_delta { *d_shadow->rhs_rate * rhs_initial_delta };

        const auto entry_id { *d_shadow->id };
        const bool has_leaf_delta { std::abs(lhs_initial_delta) > kTolerance };

        QJsonObject message {};
        message.insert(kSection, std::to_underlying(section_));
        message.insert(kSessionId, QString());
        message.insert(kEntryId, entry_id.toString(QUuid::WithoutBraces));

        if (has_leaf_delta) {
            QJsonObject lhs_delta { JsonGen::NodeDelta(lhs_id_, lhs_initial_delta, lhs_final_delta) };
            QJsonObject rhs_delta { JsonGen::NodeDelta(*d_shadow->rhs_node, rhs_initial_delta, rhs_final_delta) };

            message.insert(kLhsDelta, lhs_delta);
            message.insert(kRhsDelta, rhs_delta);
        }

        WebSocket::Instance()->SendMessage(kEntryRemove, message);

        if (has_leaf_delta) {
            emit SSyncDelta(lhs_id_, lhs_initial_delta, lhs_final_delta);
            emit SSyncDelta(*d_shadow->rhs_node, rhs_initial_delta, rhs_final_delta);
            AccumulateBalance(row);
        }

        emit SRemoveOneEntry(rhs_node_id, entry_id);
    }

    EntryShadowPool::Instance().Recycle(d_shadow, section_);
    return true;
}
