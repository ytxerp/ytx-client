#include "exportexcel.h"

#include <QtConcurrent/qtconcurrentrun.h>
#include <QtCore/qfuturewatcher.h>

#include <QDir>
#include <QFileDialog>

#include "component/constantint.h"
#include "component/constantstring.h"
#include "document.h"
#include "global/masterdataregistry.h"
#include "global/partner_inventory_registry.h"
#include "utils/mainwindowutils.h"

void ExportExcel::StatementAsync(
    CString& path, CString& partner_name, CUuid& partner_id, CString& unit_string, const utils::DateRange& range, statement::CTertiaryList& list)
{
    auto future = QtConcurrent::run([=]() -> bool { return Statement(path, partner_name, partner_id, unit_string, range, list); });

    auto* watcher = new QFutureWatcher<bool>();
    QObject::connect(watcher, &QFutureWatcher<bool>::finished, [watcher, path]() {
        bool ok = watcher->result();
        watcher->deleteLater();

        if (ok) {
            utils::ShowMessage(
                QMessageBox::Information, QObject::tr("Export Completed"), QObject::tr("The export completed successfully."), time_const::kAutoCloseMs);
        } else {
            QFile::remove(path);
            utils::ShowMessage(QMessageBox::Critical, QObject::tr("Operation Failed"), QObject::tr("The export failed. The incomplete file has been removed."),
                time_const::kAutoCloseMs);
        }
    });

    watcher->setFuture(future);
}

bool ExportExcel::Statement(
    CString& path, CString& partner_name, CUuid& partner_id, CString& unit_string, const utils::DateRange& range, statement::CTertiaryList& list)
{
    // Create excel document
    yxlsx::Document d(path);
    auto book { d.GetWorkbook() };
    if (!book)
        return false;

    if (!book->AppendSheet(QObject::tr("Statement")))
        return false;

    auto sheet { book->GetCurrentWorksheet() };
    if (!sheet)
        return false;

    const int start_row { 1 };

    // ===========================
    // Write Header
    // ===========================
    sheet->Write(start_row, 1, partner_name);
    sheet->Write(start_row, 3, unit_string);

    sheet->Write(start_row + 1, 1, QObject::tr("Period"));
    sheet->Write(start_row + 1, 2, range.start.toString(datetime_format::kDashedDate));
    sheet->Write(start_row + 1, 3, range.end.toString(datetime_format::kDashedDate));

    // ===========================
    // Table Header
    // ===========================
    const QStringList header { QObject::tr("Date"), QObject::tr("Code"), QObject::tr("InternalSku"), QObject::tr("ExternalSku"), QObject::tr("Count"),
        QObject::tr("Measure"), QObject::tr("UnitPrice"), QObject::tr("Description"), QObject::tr("Amount") };

    sheet->WriteRow(start_row + 3, 1, header);

    // ===========================
    // Table Data
    // ===========================
    int row { start_row + 4 };
    const auto& master { MasterDataRegistry::Instance() };
    const auto& partner { PartnerInventoryRegistry::Instance() };

    double total_count {};
    double total_measure {};
    double total_amount {};

    for (const auto* entry : list) {
        const QUuid external_sku { partner.ExternalSku(partner_id, entry->internal_sku) };

        QVariantList line { entry->issued_time.toString(datetime_format::kDashedDate), entry->code, master.InventoryPath(entry->internal_sku),
            master.InventoryName(external_sku), entry->count, entry->measure, entry->unit_price, entry->description, entry->amount };

        sheet->WriteRow(row++, 1, line);

        total_count += entry->count;
        total_measure += entry->measure;
        total_amount += entry->amount;
    }

    // ===========================
    // Write Total
    // ===========================
    sheet->Write(row + 1, 1, QObject::tr("Total"));
    sheet->Write(row + 1, 5, total_count);
    sheet->Write(row + 1, 6, total_measure);
    sheet->Write(row + 1, 9, total_amount);

    return d.Save();
}
