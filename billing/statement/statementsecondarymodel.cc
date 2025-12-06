#include "statementsecondarymodel.h"

#include <QDir>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QtConcurrent>

#include "component/constant.h"
#include "document.h"
#include "enum/statementenum.h"
#include "global/resourcepool.h"
#include "utils/mainwindowutils.h"

StatementSecondaryModel::StatementSecondaryModel(
    CSectionInfo& info, const QUuid& partner_id, CUuidString& item_leaf, TreeModel* partner, CString& company_name, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
    , partner_id_ { partner_id }
    , item_leaf_ { item_leaf }
    , partner_leaf_ { partner->LeafPath() }
    , partner_ { partner }
    , company_name_ { company_name }
{
}

StatementSecondaryModel::~StatementSecondaryModel() { ResourcePool<StatementSecondary>::Instance().Recycle(list_); }

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
    return list_.size();
}

int StatementSecondaryModel::columnCount(const QModelIndex& /*parent*/) const { return info_.statement_secondary_header.size(); }

QVariant StatementSecondaryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto* entry { list_.at(index.row()) };
    const StatementSecondaryEnum column { index.column() };

    switch (column) {
    case StatementSecondaryEnum::kIssuedTime:
        return entry->issued_time;
    case StatementSecondaryEnum::kInternalSku:
        return entry->internal_sku;
    case StatementSecondaryEnum::kExternalSku:
        return entry->external_sku;
    case StatementSecondaryEnum::kCount:
        return entry->count;
    case StatementSecondaryEnum::kMeasure:
        return entry->measure;
    case StatementSecondaryEnum::kUnitPrice:
        return entry->unit_price;
    case StatementSecondaryEnum::kDescription:
        return entry->description;
    case StatementSecondaryEnum::kAmount:
        return entry->amount;
    case StatementSecondaryEnum::kStatus:
        return entry->status;
    default:
        return QVariant();
    }
}

bool StatementSecondaryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const StatementSecondaryEnum column { index.column() };
    const int kRow { index.row() };

    auto* entry { list_.at(kRow) };

    switch (column) {
    case StatementSecondaryEnum::kStatus:
        entry->status = value.toBool();
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
        const StatementSecondaryEnum e_column { column };

        switch (e_column) {
        case StatementSecondaryEnum::kIssuedTime:
            return (order == Qt::AscendingOrder) ? (lhs->issued_time < rhs->issued_time) : (lhs->issued_time > rhs->issued_time);
        case StatementSecondaryEnum::kInternalSku:
            return (order == Qt::AscendingOrder) ? (lhs->internal_sku < rhs->internal_sku) : (lhs->internal_sku > rhs->internal_sku);
        case StatementSecondaryEnum::kExternalSku:
            return (order == Qt::AscendingOrder) ? (lhs->external_sku < rhs->external_sku) : (lhs->external_sku > rhs->external_sku);
        case StatementSecondaryEnum::kCount:
            return (order == Qt::AscendingOrder) ? (lhs->count < rhs->count) : (lhs->count > rhs->count);
        case StatementSecondaryEnum::kMeasure:
            return (order == Qt::AscendingOrder) ? (lhs->measure < rhs->measure) : (lhs->measure > rhs->measure);
        case StatementSecondaryEnum::kUnitPrice:
            return (order == Qt::AscendingOrder) ? (lhs->unit_price < rhs->unit_price) : (lhs->unit_price > rhs->unit_price);
        case StatementSecondaryEnum::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case StatementSecondaryEnum::kAmount:
            return (order == Qt::AscendingOrder) ? (lhs->amount < rhs->amount) : (lhs->amount > rhs->amount);
        case StatementSecondaryEnum::kStatus:
            return (order == Qt::AscendingOrder) ? (lhs->status < rhs->status) : (lhs->status > rhs->status);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(list_.begin(), list_.end(), Compare);
    emit layoutChanged();
}

void StatementSecondaryModel::ResetModel(const QJsonArray& entry_array)
{
    beginResetModel();

    ResourcePool<StatementSecondary>::Instance().Recycle(list_);

    for (const auto& value : entry_array) {
        if (!value.isObject())
            continue;

        const QJsonObject obj { value.toObject() };

        auto* statement_secondary { ResourcePool<StatementSecondary>::Instance().Allocate() };
        statement_secondary->ReadJson(obj);

        list_.emplaceBack(statement_secondary);
    }

    endResetModel();
}

void StatementSecondaryModel::Export(const QDateTime& start, const QDateTime& end)
{
    const QDateTime adjust_end { end.addDays(-1) };

    CString name { QDir::homePath() + QDir::separator() + partner_->Name(partner_id_) + QStringLiteral("-") + company_name_ + QStringLiteral("-")
        + adjust_end.toString(kMonthFST) };

    QString destination { QFileDialog::getSaveFileName(nullptr, tr("Export Excel"), name, QStringLiteral("*.xlsx")) };
    if (!MainWindowUtils::PrepareNewFile(destination, kDotSuffixXLSX))
        return;

    auto future = QtConcurrent::run([=, this]() {
        try {
            YXlsx::Document d(destination);

            auto book { d.GetWorkbook() };

            book->AppendSheet(tr("Statement"));
            auto sheet { book->GetCurrentWorksheet() };

            sheet->Write(1, 1, partner_->Name(partner_id_));
            sheet->Write(2, 1, tr("Date"));
            sheet->Write(2, 2, start.toString(kDateFST));
            sheet->Write(2, 3, adjust_end.toString(kDateFST));
            sheet->Write(3, 1, tr("Previous Balance"));
            sheet->Write(3, 3, pbalance_);
            sheet->Write(4, 1, tr("Current Amount"));
            sheet->Write(4, 3, camount_);
            sheet->Write(5, 1, tr("Current Settlement"));
            sheet->Write(5, 3, csettlement_);
            sheet->Write(6, 1, tr("Current Balance"));
            sheet->Write(6, 3, cbalance_);

            const QStringList header { tr("Date"), tr("Internal"), tr("External"), tr("Count"), tr("Measure"), tr("UnitPrice"), tr("Description"),
                tr("Amount") };
            sheet->WriteRow(8, 1, header);

            const qsizetype rows { list_.size() };

            QList<QVariantList> list(rows);

            qsizetype row_index { 0 };
            for (const auto* entry : std::as_const(list_)) {
                list[row_index] << entry->issued_time.toString(kDateFST) << item_leaf_.value(entry->internal_sku) << partner_leaf_.value(entry->external_sku)
                                << entry->count << entry->measure << entry->unit_price << entry->description << entry->amount;
                ++row_index;
            }

            for (qsizetype start_row = 0; start_row != rows; ++start_row) {
                sheet->WriteRow(start_row + 9, 1, list.at(start_row));
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

void StatementSecondaryModel::ResetTotal(const QJsonObject& obj)
{
    if (obj.contains(kPBalance))
        pbalance_ = obj[kPBalance].toString().toDouble();

    if (obj.contains(kCAmount))
        camount_ = obj[kCAmount].toString().toDouble();

    if (obj.contains(kCSettlement))
        csettlement_ = obj[kCSettlement].toString().toDouble();

    if (obj.contains(kCBalance))
        cbalance_ = obj[kCBalance].toString().toDouble();
}
