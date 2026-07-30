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
    auto* entry { shadow->entry };

    const QUuid id { *shadow->id };

    switch (column) {
    case EntryEnumF::kIssuedTime:
        entry::UpdateShadowIssuedTime(
            pending_updates_[id], shadow, kIssuedTime, value.toDateTime(), &EntryShadow::issued_time, [this, id, entry]() { RestartTimer(id, entry); });
        break;
    case EntryEnumF::kCode:
        entry::UpdateShadowField(pending_updates_[id], shadow, kCode, value.toString(), &EntryShadow::code, [this, id, entry]() { RestartTimer(id, entry); });
        break;
    case EntryEnumF::kStatus:
        entry::UpdateShadowField(pending_updates_[id], shadow, kStatus, value.toInt(), &EntryShadow::status, [this, id, entry]() { RestartTimer(id, entry); });
        break;
    case EntryEnumF::kDescription:
        entry::UpdateShadowField(
            pending_updates_[id], shadow, kDescription, value.toString(), &EntryShadow::description, [this, id, entry]() { RestartTimer(id, entry); });
        break;
    case EntryEnumF::kDocument:
        entry::UpdateShadowStringList(
            pending_updates_[id], shadow, kDocument, value.toStringList(), &EntryShadow::document, [this, id, entry]() { RestartTimer(id, entry); });
        break;
    case EntryEnumF::kTag:
        entry::UpdateShadowStringList(
            pending_updates_[id], shadow, kTag, value.toStringList(), &EntryShadow::tag, [this, id, entry]() { RestartTimer(id, entry); });
        break;
    case EntryEnumF::kLhsRate:
        UpdateRate(shadow, value.toDouble());
        break;
    case EntryEnumF::kRhsNode:
        UpdateLinkedNode(shadow, value.toUuid(), row);
        break;
    case EntryEnumF::kDebit:
        UpdateNumeric(shadow, value.toDouble(), row, NumericSide::kDebit);
        break;
    case EntryEnumF::kCredit:
        UpdateNumeric(shadow, value.toDouble(), row, NumericSide::kCredit);
        break;
    case EntryEnumF::kCashKind: {
        auto* d_shadow { static_cast<EntryShadowF*>(shadow) };

        const int raw { value.toInt() };
        const auto cash_kind { static_cast<finance::CashKind>(raw) };

        *d_shadow->cash_kind = cash_kind;
        pending_updates_[id].insert(kCashKind, raw);
        RestartTimer(id, entry);
        break;
    }
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

bool TableModelF::UpdateNumeric(EntryShadow* shadow, double value, int row, NumericSide side)
{
    const double lhs_old_debit { *shadow->lhs_debit };
    const double lhs_old_credit { *shadow->lhs_credit };
    const double lhs_rate { *shadow->lhs_rate };
    const double rhs_rate { *shadow->rhs_rate };

    Q_ASSERT(lhs_rate != 0.0);
    Q_ASSERT(rhs_rate != 0.0);

    // Old value on the side currently being edited (debit or credit).
    // If it hasn't actually changed, there's nothing to update.
    const double old_value { side == NumericSide::kDebit ? lhs_old_debit : lhs_old_credit };
    if (qFuzzyCompare(old_value, value))
        return false;

    // "base" is the old value on the opposite side (the one not being edited).
    // Since debit and credit are mutually exclusive at any given time (only one
    // side holds a non-zero value), base represents the current balance sitting
    // on the other side, which the new value will be netted against.
    const double base { side == NumericSide::kDebit ? lhs_old_credit : lhs_old_debit };

    // Remaining amount after offsetting the opposite side.
    // The sign determines whether the balance belongs to debit or credit;
    // the magnitude is stored in the corresponding side.
    const double diff { std::abs(value - base) };
    const bool debit_side { (side == NumericSide::kDebit && value > base) || (side == NumericSide::kCredit && value <= base) };

    *shadow->lhs_debit = debit_side ? diff : 0.0;
    *shadow->lhs_credit = debit_side ? 0.0 : diff;

    // Cauculate RHS
    *shadow->rhs_debit = (*shadow->lhs_credit) * lhs_rate / rhs_rate;
    *shadow->rhs_credit = (*shadow->lhs_debit) * lhs_rate / rhs_rate;

    if (shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *shadow->id };
    const QUuid rhs_id { *shadow->rhs_node };

    const auto input_side { ToValueInputSide(shadow->binding_mode) };
    QJsonObject update {};

    update.insert(kVersion, *shadow->version);
    update.insert(input_side == InputSide::kLhs ? kLhsDebit : kRhsDebit, QString::number(*shadow->lhs_debit, 'f', numeric_const::kDecimalPlaces8));
    update.insert(input_side == InputSide::kLhs ? kLhsCredit : kRhsCredit, QString::number(*shadow->lhs_credit, 'f', numeric_const::kDecimalPlaces8));

    QJsonObject message { JsonGen::EntryValue(section_, entry_id, update, input_side) };
    WebSocket::Instance()->SendMessage(WsKey::kEntryNumericUpdate, message);

    // Delta calculation follows the DICD rule (Debit - Credit).
    // After the delta is computed, both the node and the server
    // will adjust the delta value according to the node's direction rule
    // (DICD → unchanged, DDCI → inverted).
    const double lhs_initial_delta { *shadow->lhs_debit - *shadow->lhs_credit - (lhs_old_debit - lhs_old_credit) };

    if (!qFuzzyIsNull(lhs_initial_delta)) {
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
    if (value <= 0)
        return false;

    const double old_value { *shadow->lhs_rate };
    if (qFuzzyCompare(old_value, value))
        return false;

    const double proportion { value / old_value };

    *shadow->lhs_rate = value;

    const double rhs_old_debit { *shadow->rhs_debit };
    const double rhs_old_credit { *shadow->rhs_credit };

    *shadow->rhs_debit *= proportion;
    *shadow->rhs_credit *= proportion;

    if (shadow->rhs_node->isNull())
        return false;

    const QUuid entry_id { *shadow->id };

    const auto input_side { ToValueInputSide(shadow->binding_mode) };

    QJsonObject update {};
    update.insert(kVersion, *shadow->version);
    update.insert(input_side == InputSide::kLhs ? kLhsRate : kRhsRate, QString::number(*shadow->lhs_rate, 'f', numeric_const::kDecimalPlaces8));

    QJsonObject message { JsonGen::EntryValue(section_, entry_id, update, input_side) };
    WebSocket::Instance()->SendMessage(WsKey::kEntryRateUpdate, message);

    const double rhs_initial_delta { *shadow->rhs_debit - *shadow->rhs_credit - (rhs_old_debit - rhs_old_credit) };

    if (!qFuzzyIsNull(rhs_initial_delta)) {
        const QUuid rhs_id { *shadow->rhs_node };
        emit SUpdateBalance(rhs_id, *shadow->id);
    }

    return true;
}
