#include "tablemodel.h"

#include "component/constantint.h"
#include "component/constantwebsocket.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

TableModel::TableModel(CTableModelArg& arg, QObject* parent)
    : QAbstractItemModel(parent)
    , header_ { arg.header }
    , node_id_ { arg.node->id }
    , section_ { arg.section }
{
}

void TableModel::RRefreshBatchEntries(int column) { EmitDataChanged(0, rowCount() - 1, column, column); }

void TableModel::RRefreshField(QUuid entry_id, int start, int end)
{
    auto idx { GetIndex(entry_id) };
    if (!idx.isValid())
        return;

    int row { idx.row() };

    EmitDataChanged(row, row, start, end);
}

void TableModel::RestartTimer(QUuid id)
{
    auto*& timer { pending_updates_[id].timer };

    if (!timer) {
        timer = new QTimer { this };
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout, this, [this, id]() { FlushTimer(id); });
    }

    timer->start(time_const::kAutoCloseMs);
}

void TableModel::FlushTimer(QUuid id)
{
    auto update { pending_updates_.take(id) };

    if (update.entry && !update.changes.isEmpty()) {
        const QJsonObject message { JsonGen::EntryUpdate(section_, id, update.changes, update.entry->version) };

        const auto key { section_ == Section::kPartner ? WsKey::kPartnerEntryUpdate : WsKey::kEntryUpdate };
        WebSocket::Instance()->SendMessage(key, message);
    }

    if (update.timer) {
        update.timer->stop();
        update.timer->deleteLater();
    }
}

void TableModel::FlushTimers()
{
    const auto ids { pending_updates_.keys() };

    if (ids.isEmpty())
        return;

    qDebug() << std::to_underlying(section_) << Q_FUNC_INFO;

    for (const auto& id : ids) {
        FlushTimer(id);
    }
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return header_.at(section);

    return QVariant();
}

void TableModel::EmitDataChanged(int start_row, int end_row, int start_column, int end_column, const QModelIndex& parent)
{
    // top_left and bottom_right must share the same parent, behavior is undefined otherwise
    Q_ASSERT(!parent.isValid() || parent.model() == this);

    if (start_row < 0 || end_row >= rowCount(parent) || start_row > end_row) {
        qDebug() << "EmitDataChanged: invalid row range" << start_row << end_row << "rowCount" << rowCount(parent);
        return;
    }

    if (start_column < 0 || end_column >= columnCount(parent) || start_column > end_column) {
        qDebug() << "EmitDataChanged: invalid column range" << start_column << end_column << "columnCount" << columnCount(parent);
        return;
    }

    const QModelIndex top_left { index(start_row, start_column, parent) };
    const QModelIndex bottom_right { index(end_row, end_column, parent) };

    Q_ASSERT(top_left.parent() == bottom_right.parent());

    emit dataChanged(top_left, bottom_right, QList<int> { Qt::DisplayRole, Qt::EditRole });
}

void TableModel::CancelPendingUpdate(QUuid entry_id)
{
    // Remove pending update to prevent delayed flush after deletion
    // Stop its timer to avoid accessing recycled member.
    if (auto* timer = pending_updates_.take(entry_id).timer; timer) {
        timer->stop();
        timer->deleteLater();
    }
}
