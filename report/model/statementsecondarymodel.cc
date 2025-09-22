#include "statementsecondarymodel.h"

#include <QDir>
#include <QFileDialog>
#include <QtConcurrent>

#include "component/constant.h"
#include "component/enumclass.h"
#include "document.h"
#include "global/resourcepool.h"
#include "utils/mainwindowutils.h"

StatementSecondaryModel::StatementSecondaryModel(
    EntryHub* dbhub, CSectionInfo& info, const QUuid& party_id, CUuidString& item_leaf, TreeModel* stakeholder, CString& company_name, QObject* parent)
    : QAbstractItemModel { parent }
    , dbhub_ { static_cast<EntryHubO*>(dbhub) }
    , info_ { info }
    , party_id_ { party_id }
    , item_leaf_ { item_leaf }
    , stakeholder_leaf_ { stakeholder->LeafPath() }
    , stakeholder_ { stakeholder }
    , company_name_ { company_name }
{
}

StatementSecondaryModel::~StatementSecondaryModel() { ResourcePool<StatementSecondary>::Instance().Recycle(statement_secondary_list_); }

QModelIndex StatementSecondaryModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex StatementSecondaryModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int StatementSecondaryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return statement_secondary_list_.size();
}

int StatementSecondaryModel::columnCount(const QModelIndex& /*parent*/) const { return info_.statement_secondary_header.size(); }

QVariant StatementSecondaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* entry { statement_secondary_list_.at(index.row()) };
    const StatementSecondaryEnum kColumn { index.column() };

    switch (kColumn) {
    case StatementSecondaryEnum::kIssuedTime:
        return entry->issued_time;
    case StatementSecondaryEnum::kRhsNode:
        return entry->rhs_node;
    case StatementSecondaryEnum::kSupportNode:
        return entry->support_id.isNull() ? QVariant() : entry->support_id;
    case StatementSecondaryEnum::kFirst:
        return entry->first == 0 ? QVariant() : entry->first;
    case StatementSecondaryEnum::kSecond:
        return entry->second == 0 ? QVariant() : entry->second;
    case StatementSecondaryEnum::kUnitPrice:
        return entry->unit_price == 0 ? QVariant() : entry->unit_price;
    case StatementSecondaryEnum::kDescription:
        return entry->description;
    case StatementSecondaryEnum::kInitialTotal:
        return entry->initial == 0 ? QVariant() : entry->initial;
    case StatementSecondaryEnum::kIsChecked:
        return entry->is_checked ? entry->is_checked : QVariant();
    default:
        return QVariant();
    }
}

bool StatementSecondaryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const StatementSecondaryEnum kColumn { index.column() };
    const int kRow { index.row() };

    auto* entry { statement_secondary_list_.at(kRow) };

    switch (kColumn) {
    case StatementSecondaryEnum::kIsChecked:
        entry->is_checked = value.toBool();
        break;
    default:
        return false;
    }

    return true;
}

QVariant StatementSecondaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_.statement_secondary_header.at(section);

    return QVariant();
}

void StatementSecondaryModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_.statement_secondary_header.size() - 1)
        return;

    auto Compare = [column, order](const StatementSecondary* lhs, const StatementSecondary* rhs) -> bool {
        const StatementSecondaryEnum kColumn { column };

        switch (kColumn) {
        case StatementSecondaryEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case StatementSecondaryEnum::kRhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_node < rhs->rhs_node) : (lhs->rhs_node > rhs->rhs_node);
        case StatementSecondaryEnum::kSupportNode:
            return (order == Qt::AscendingOrder) ? (lhs->support_id < rhs->support_id) : (lhs->support_id > rhs->support_id);
        case StatementSecondaryEnum::kFirst:
            return (order == Qt::AscendingOrder) ? (lhs->first < rhs->first) : (lhs->first > rhs->first);
        case StatementSecondaryEnum::kSecond:
            return (order == Qt::AscendingOrder) ? (lhs->second < rhs->second) : (lhs->second > rhs->second);
        case StatementSecondaryEnum::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (lhs->unit_price < rhs->unit_price) : (lhs->unit_price > rhs->unit_price);
        case StatementSecondaryEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case StatementSecondaryEnum::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial < rhs->initial) : (lhs->initial > rhs->initial);
        case StatementSecondaryEnum::kIsChecked:
            return (order == Qt::AscendingOrder) ? (lhs->is_checked < rhs->is_checked) : (lhs->is_checked > rhs->is_checked);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(statement_secondary_list_.begin(), statement_secondary_list_.end(), Compare);
    emit layoutChanged();
}

void StatementSecondaryModel::RResetModel(int unit, const QDateTime& start, const QDateTime& end)
{
    if (party_id_.isNull() || !start.isValid() || !end.isValid())
        return;

    beginResetModel();
    if (!statement_secondary_list_.isEmpty())
        ResourcePool<StatementSecondary>::Instance().Recycle(statement_secondary_list_);

    dbhub_->ReadStatementSecondary(statement_secondary_list_, party_id_, unit, start.toUTC(), end.toUTC());
    endResetModel();
}

void StatementSecondaryModel::RExport(int unit, const QDateTime& start, const QDateTime& end)
{
    double pbalance { 0.0 };
    double cdelta { 0.0 };

    if (unit != std::to_underlying(UnitO::kImmediate)) {
        dbhub_->ReadBalance(pbalance, cdelta, party_id_, unit, start, end);
    }

    CString name { QDir::homePath() + QDir::separator() + stakeholder_->Name(party_id_) + QStringLiteral("-") + company_name_ + QStringLiteral("-")
        + end.toString(kMonthFST) };

    QString destination { QFileDialog::getSaveFileName(nullptr, tr("Export Excel"), name, QStringLiteral("*.xlsx")) };
    if (!MainWindowUtils::PrepareNewFile(destination, kDotSuffixXLSX))
        return;

    auto future = QtConcurrent::run([=, this]() {
        try {
            YXlsx::Document d(destination);

            auto book { d.GetWorkbook() };

            book->AppendSheet(tr("Statement"));
            auto sheet { book->GetCurrentWorksheet() };

            sheet->Write(1, 1, stakeholder_->Name(party_id_));
            sheet->Write(2, 1, tr("Date"));
            sheet->Write(2, 2, start.toString(kDateFST));
            sheet->Write(2, 3, end.toString(kDateFST));
            sheet->Write(3, 1, tr("Previous Balance"));
            sheet->Write(3, 2, pbalance);
            sheet->Write(4, 1, tr("Current Delta"));
            sheet->Write(4, 3, cdelta);
            sheet->Write(5, 1, tr("Current Balance"));
            sheet->Write(5, 3, pbalance + cdelta);

            const QStringList header { tr("Date"), tr("Internal"), tr("External"), tr("First"), tr("Second"), tr("UnitPrice"), tr("Description"),
                tr("GrossAmount") };
            sheet->WriteRow(7, 4, header);

            const qsizetype rows { statement_secondary_list_.size() };

            QList<QVariantList> list(rows);

            qsizetype row_index { 0 };
            for (const auto* entry : std::as_const(statement_secondary_list_)) {
                list[row_index] << entry->issued_time << item_leaf_.value(entry->rhs_node) << stakeholder_leaf_.value(entry->support_id) << entry->first
                                << entry->second << entry->unit_price << entry->description << entry->initial;
                ++row_index;
            }

            for (qsizetype start_row = 0; start_row != rows; ++start_row) {
                sheet->WriteRow(start_row + 8, 4, list.at(start_row));
            }

            d.Save();
            return true;
        } catch (...) {
            qWarning() << "Export failed due to an unknown exception.";
            return false;
        }
    });

    auto* watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [watcher, destination]() {
        watcher->deleteLater();

        bool success { watcher->future().result() };
        if (success) {
            MainWindowUtils::Message(QMessageBox::Information, tr("Export Completed"), tr("Export completed successfully."), kThreeThousand);
        } else {
            QFile::remove(destination);
            MainWindowUtils::Message(QMessageBox::Critical, tr("Export Failed"), tr("Export failed. The file has been deleted."), kThreeThousand);
        }
    });

    watcher->setFuture(future);
}
