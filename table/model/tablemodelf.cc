#include "tablemodelf.h"

#include "component/constant.h"
#include "component/constantwebsocket.h"
#include "utils/entryutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableModelF::TableModelF(CTableModelArg& arg, QObject* parent)
    : TableModel { arg, parent }
{
}

QVariant TableModelF::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    const EntryEnumF column { index.column() };
    auto* d_shadow { static_cast<EntryShadowF*>(index.internalPointer()) };

    switch (column) {
    case EntryEnumF::kId:
        return *d_shadow->id;
    case EntryEnumF::kVersion:
        return *d_shadow->version;
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
    case EntryEnumF::kTag:
        return *d_shadow->tag;
    case EntryEnumF::kDebit:
        return *d_shadow->lhs_debit;
    case EntryEnumF::kCredit:
        return *d_shadow->lhs_credit;
    case EntryEnumF::kBalance:
        return d_shadow->balance;
    case EntryEnumF::kCashKind:
        return std::to_underlying(*d_shadow->cash_kind);
    }
}

bool TableModelF::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const EntryEnumF column { index.column() };
    const int row { index.row() };

    if (data(index, role) == value)
        return false;

    auto* shadow { static_cast<EntryShadow*>(index.internalPointer()) };

    const QUuid id { *shadow->id };
    const int version { *shadow->version };

    switch (column) {
    case EntryEnumF::kIssuedTime:
        entry::UpdateShadowIssuedTime(
            pending_updates_[id], shadow, kIssuedTime, value.toDateTime(), &EntryShadow::issued_time, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumF::kCode:
        entry::UpdateShadowField(
            pending_updates_[id], shadow, kCode, value.toString(), &EntryShadow::code, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumF::kStatus:
        entry::UpdateShadowField(
            pending_updates_[id], shadow, kStatus, value.toInt(), &EntryShadow::status, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumF::kDescription:
        entry::UpdateShadowField(
            pending_updates_[id], shadow, kDescription, value.toString(), &EntryShadow::description, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumF::kDocument:
        entry::UpdateShadowStringList(
            pending_updates_[id], shadow, kDocument, value.toStringList(), &EntryShadow::document, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumF::kTag:
        entry::UpdateShadowStringList(
            pending_updates_[id], shadow, kTag, value.toStringList(), &EntryShadow::tag, [this, id, version]() { RestartTimer(id, version); });
        break;
    case EntryEnumF::kLhsRate:
        UpdateRate(shadow, value.toDouble());
        break;
    case EntryEnumF::kRhsNode:
        UpdateLinkedNode(shadow, value.toUuid(), row);
        break;
    case EntryEnumF::kDebit:
        UpdateNumeric(shadow, value.toDouble(), row, true);
        break;
    case EntryEnumF::kCredit:
        UpdateNumeric(shadow, value.toDouble(), row, false);
        break;
    case EntryEnumF::kCashKind: {
        auto* d_shadow { static_cast<EntryShadowF*>(shadow) };

        const int raw { value.toInt() };
        const auto cash_kind { static_cast<finance::CashKind>(raw) };

        *d_shadow->cash_kind = cash_kind;
        pending_updates_[id].insert(kCashKind, raw);
        RestartTimer(id, version);
        break;
    }
    case EntryEnumF::kId:
    case EntryEnumF::kVersion:
    case EntryEnumF::kLhsNode:
    case EntryEnumF::kBalance:
        return false;
    }

    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    return true;
}

void TableModelF::sort(int column, Qt::SortOrder order)
{
    const EntryEnumF e_column { column };
    if (e_column == EntryEnumF::kBalance)
        return;

    auto Compare = [order, e_column](const EntryShadow* lhs, const EntryShadow* rhs) -> bool {
        auto* d_lhs { DerivedPtr<EntryShadowF>(lhs) };
        auto* d_rhs { DerivedPtr<EntryShadowF>(rhs) };

        switch (e_column) {
        case EntryEnumF::kCode:
            return utils::CompareShadowMember(lhs, rhs, &EntryShadow::code, order);
        case EntryEnumF::kDescription:
            return utils::CompareShadowMember(lhs, rhs, &EntryShadow::description, order);
        case EntryEnumF::kIssuedTime:
            return utils::CompareShadowMember(lhs, rhs, &EntryShadow::issued_time, order);
        case EntryEnumF::kLhsRate:
            return utils::CompareShadowMember(lhs, rhs, &EntryShadow::lhs_rate, order);
        case EntryEnumF::kRhsNode:
            return utils::CompareShadowMember(lhs, rhs, &EntryShadow::rhs_node, order);
        case EntryEnumF::kStatus:
            return utils::CompareShadowMember(lhs, rhs, &EntryShadow::status, order);
        case EntryEnumF::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document->size() < rhs->document->size()) : (lhs->document->size() > rhs->document->size());
        case EntryEnumF::kTag:
            return utils::CompareShadowMember(lhs, rhs, &EntryShadow::tag, order);
        case EntryEnumF::kDebit:
            return utils::CompareShadowMember(lhs, rhs, &EntryShadow::lhs_debit, order);
        case EntryEnumF::kCredit:
            return utils::CompareShadowMember(lhs, rhs, &EntryShadow::lhs_credit, order);
        case EntryEnumF::kCashKind:
            return utils::CompareShadowMember(d_lhs, d_rhs, &EntryShadowF::cash_kind, order);
        case EntryEnumF::kId:
        case EntryEnumF::kVersion:
        case EntryEnumF::kLhsNode:
        case EntryEnumF::kBalance:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::ranges::sort(shadow_list_, Compare);
    emit layoutChanged();

    AccumulateBalance(0);
}

Qt::ItemFlags TableModelF::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };

    auto* shadow { static_cast<EntryShadow*>(index.internalPointer()) };
    if (*shadow->sync_state == SyncState::kDeleting)
        return flags & ~Qt::ItemIsEditable;

    const EntryEnumF column { index.column() };

    switch (column) {
    case EntryEnumF::kId:
    case EntryEnumF::kVersion:
    case EntryEnumF::kBalance:
    case EntryEnumF::kDocument:
    case EntryEnumF::kTag:
    case EntryEnumF::kStatus:
    case EntryEnumF::kLhsNode:
        flags &= ~Qt::ItemIsEditable;
        break;
    case EntryEnumF::kIssuedTime:
    case EntryEnumF::kLhsRate:
    case EntryEnumF::kRhsNode:
    case EntryEnumF::kCode:
    case EntryEnumF::kDescription:
    case EntryEnumF::kCashKind:
    case EntryEnumF::kDebit:
    case EntryEnumF::kCredit:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}

bool TableModelF::UpdateLinkedNode(EntryShadow* shadow, const QUuid& value, int row)
{
    if (value.isNull())
        return false;

    const QUuid old_node { *shadow->rhs_node };
    if (old_node == value)
        return false;

    *shadow->rhs_node = value;

    const QUuid entry_id { *shadow->id };
    auto* entry { shadow->entry };

    QJsonObject message { JsonGen::EntryMessage(section_, entry_id) };

    if (old_node.isNull()) {
        *shadow->sync_state = SyncState::kSynced;

        message.insert(kEntry, shadow->WriteJson());
        message.insert(kLhsTotal, QJsonObject());
        message.insert(kRhsTotal, QJsonObject());
        WebSocket::Instance()->SendMessage(WsKey::kEntryInsert, message);

        const double lhs_debit { *shadow->lhs_debit };
        const double lhs_credit { *shadow->lhs_credit };

        const bool has_leaf_delta { std::abs(lhs_debit - lhs_credit) > kTolerance };
        if (has_leaf_delta) {
            AccumulateBalance(row);
            EmitDataChanged(row, row, std::to_underlying(EntryEnumF::kBalance), std::to_underlying(EntryEnumF::kBalance));
        }

        emit STransferOneEntry(entry);
    }

    if (!old_node.isNull()) {
        // Indicates whether the EntryShadow's lhs_node corresponds to the Entry's lhs_node.
        // If true, the node is not collapsed; if false, it has been collapsed and flipped.
        const bool is_parallel { shadow->is_parallel };
        // If true, the EntryShadow corresponds to the left-side node in the Postgres table,
        // so we need to update the right-side node (kRhsNode).
        // If false, it means the entry is collapsed (lhs and rhs flipped), so update the left-side node (kLhsNode).
        const auto field { is_parallel ? kRhsNode : kLhsNode };

        QJsonObject update {};
        update.insert(field, value.toString(QUuid::WithoutBraces));
        update.insert(kVersion, *shadow->version);

        message.insert(kUpdate, update);
        message.insert(kIsParallel, is_parallel);

        WebSocket::Instance()->SendMessage(WsKey::kEntryLinkedNodeUpdate, message);

        emit SDetachOneEntry(old_node, entry_id);
    }

    emit SAttachOneEntry(value, entry);
    return true;
}

bool TableModelF::UpdateNumeric(EntryShadow* shadow, double value, int row, bool is_debit)
{
    const double lhs_old_debit { *shadow->lhs_debit };
    const double lhs_old_credit { *shadow->lhs_credit };
    const double lhs_rate { *shadow->lhs_rate };
    const double rhs_rate { *shadow->rhs_rate };

    Q_ASSERT(lhs_rate != 0.0);
    Q_ASSERT(rhs_rate != 0.0);

    const double old_value { is_debit ? lhs_old_debit : lhs_old_credit };
    if (FloatEqual(old_value, value))
        return false;

    // Base represents the opposite side (used to compute the new diff)
    const double base { is_debit ? lhs_old_credit : lhs_old_debit };
    const double diff { std::abs(value - base) };

    // Determine which side (debit/credit) should hold the new value
    const bool to_debit { (is_debit && value > base) || (!is_debit && value <= base) };

    *shadow->lhs_debit = to_debit ? diff : 0.0;
    *shadow->lhs_credit = to_debit ? 0.0 : diff;

    // Cauculate RHS
    *shadow->rhs_debit = (*shadow->lhs_credit) * lhs_rate / rhs_rate;
    *shadow->rhs_credit = (*shadow->lhs_debit) * lhs_rate / rhs_rate;

    if (shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *shadow->id };
    const QUuid rhs_id { *shadow->rhs_node };

    QJsonObject update {};
    const bool is_parallel { shadow->is_parallel };

    update.insert(kVersion, *shadow->version);
    update.insert(is_parallel ? kLhsDebit : kRhsDebit, QString::number(*shadow->lhs_debit, 'f', numeric_const::kDecimalPlaces8));
    update.insert(is_parallel ? kLhsCredit : kRhsCredit, QString::number(*shadow->lhs_credit, 'f', numeric_const::kDecimalPlaces8));

    QJsonObject message { JsonGen::EntryValue(section_, entry_id, update, is_parallel) };
    message.insert(kLhsTotal, QJsonObject());
    message.insert(kRhsTotal, QJsonObject());

    // Delta calculation follows the DICD rule (Debit - Credit).
    // After the delta is computed, both the node and the server
    // will adjust the delta value according to the node's direction rule
    // (DICD → unchanged, DDCI → inverted).
    const double lhs_initial_delta { *shadow->lhs_debit - *shadow->lhs_credit - (lhs_old_debit - lhs_old_credit) };
    const bool has_leaf_delta { std::abs(lhs_initial_delta) > kTolerance };

    WebSocket::Instance()->SendMessage(WsKey::kEntryNumericUpdate, message);

    if (has_leaf_delta) {
        AccumulateBalance(row);
        EmitDataChanged(row, row, std::to_underlying(EntryEnumF::kBalance), std::to_underlying(EntryEnumF::kBalance));

        emit SUpdateBalance(rhs_id, *shadow->id);
    }

    return true;
}

#if 0
bool LeafModelF::UpdateDebit(EntryShadow* shadow, double value, int row)
{
    auto* shadow { DerivedPtr<EntryShadowF>(shadow) };

    const double lhs_debit { *shadow->lhs_debit };
    if (FloatEqual(lhs_debit, value))
        return false;

    const double lhs_credit { *shadow->lhs_credit };
    const double lhs_rate { *shadow->lhs_rate };

    const double abs { qAbs(value - lhs_credit) };
    *shadow->lhs_debit = (value > lhs_credit) ? abs : 0;
    *shadow->lhs_credit = (value <= lhs_credit) ? abs : 0;

    const double rhs_debit { *shadow->rhs_debit };
    const double rhs_credit { *shadow->rhs_credit };
    const double rhs_rate { *shadow->rhs_rate };

    *shadow->rhs_debit = (*shadow->lhs_credit) * lhs_rate / rhs_rate;
    *shadow->rhs_credit = (*shadow->lhs_debit) * lhs_rate / rhs_rate;

    if (shadow->rhs_node->isNull())
        return false;

    const double lhs_debit_delta { *shadow->lhs_debit - lhs_debit };
    const double lhs_credit_delta { *shadow->lhs_credit - lhs_credit };
    emit SSyncDelta(lhs_id_, lhs_debit_delta, lhs_credit_delta, lhs_debit_delta * lhs_rate, lhs_credit_delta * lhs_rate);

    const double rhs_debit_delta { *shadow->rhs_debit - rhs_debit };
    const double rhs_credit_delta { *shadow->rhs_credit - rhs_credit };
    emit SSyncDelta(*shadow->rhs_node, rhs_debit_delta, rhs_credit_delta, rhs_debit_delta * rhs_rate, rhs_credit_delta * rhs_rate);

    return true;
}

bool LeafModelF::UpdateCredit(EntryShadow* shadow, double value, int row)
{
    auto* shadow { DerivedPtr<EntryShadowF>(shadow) };

    const double lhs_credit { *shadow->lhs_credit };
    if (FloatEqual(lhs_credit, value))
        return false;

    const double lhs_debit { *shadow->lhs_debit };
    const double lhs_rate { *shadow->lhs_rate };

    const double abs { qAbs(value - lhs_debit) };
    *shadow->lhs_debit = (value > lhs_debit) ? 0 : abs;
    *shadow->lhs_credit = (value <= lhs_debit) ? 0 : abs;

    const double rhs_debit { *shadow->rhs_debit };
    const double rhs_credit { *shadow->rhs_credit };
    const double rhs_rate { *shadow->rhs_rate };

    *shadow->rhs_debit = (*shadow->lhs_credit) * lhs_rate / rhs_rate;
    *shadow->rhs_credit = (*shadow->lhs_debit) * lhs_rate / rhs_rate;

    if (shadow->rhs_node->isNull())
        return false;

    const double lhs_debit_delta { *shadow->lhs_debit - lhs_debit };
    const double lhs_credit_delta { *shadow->lhs_credit - lhs_credit };
    emit SSyncDelta(lhs_id_, lhs_debit_delta, lhs_credit_delta, lhs_debit_delta * lhs_rate, lhs_credit_delta * lhs_rate);

    const double rhs_debit_delta { *shadow->rhs_debit - rhs_debit };
    const double rhs_credit_delta { *shadow->rhs_credit - rhs_credit };
    emit SSyncDelta(*shadow->rhs_node, rhs_debit_delta, rhs_credit_delta, rhs_debit_delta * rhs_rate, rhs_credit_delta * rhs_rate);

    return true;
}
#endif

bool TableModelF::UpdateRate(EntryShadow* shadow, double value)
{
    const double old_rate { *shadow->lhs_rate };
    if (FloatEqual(old_rate, value) || value <= 0)
        return false;

    const double proportion { value / old_rate };

    *shadow->lhs_rate = value;

    const double rhs_old_debit { *shadow->rhs_debit };
    const double rhs_old_credit { *shadow->rhs_credit };

    *shadow->rhs_debit *= proportion;
    *shadow->rhs_credit *= proportion;

    if (shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *shadow->id };

    QJsonObject update {};
    update.insert(kVersion, *shadow->version);

    const bool is_parallel { shadow->is_parallel };
    update.insert(is_parallel ? kLhsRate : kRhsRate, QString::number(*shadow->lhs_rate, 'f', numeric_const::kDecimalPlaces8));

    QJsonObject message { JsonGen::EntryValue(section_, entry_id, update, is_parallel) };
    message.insert(kLhsTotal, QJsonObject());
    message.insert(kRhsTotal, QJsonObject());
    WebSocket::Instance()->SendMessage(WsKey::kEntryRateUpdate, message);

    const double rhs_initial_delta { *shadow->rhs_debit - *shadow->rhs_credit - (rhs_old_debit - rhs_old_credit) };
    const bool has_leaf_delta { std::abs(rhs_initial_delta) > kTolerance };

    if (has_leaf_delta) {
        const QUuid rhs_id { *shadow->rhs_node };
        emit SUpdateBalance(rhs_id, *shadow->id);
    }

    return true;
}
