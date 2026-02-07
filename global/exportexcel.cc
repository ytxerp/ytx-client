#include "exportexcel.h"

#include <QtConcurrent/qtconcurrentrun.h>
#include <QtCore/qfuturewatcher.h>

#include <QDir>
#include <QFileDialog>

#include "document.h"
#include "utils/mainwindowutils.h"

void ExportExcel::StatementAsync(EntryHubP* entry_hub_p, TreeModelI* tree_model_i, CString& path, CString& partner_name, CUuid& partner_id,
    CString& unit_string, CDateTime& start, CDateTime& end, CJsonObject& total, CStatementEntryList& list)
{
    auto future = QtConcurrent::run(
        [=]() -> bool { return Statement(entry_hub_p, tree_model_i, path, partner_name, partner_id, unit_string, start, end, total, list); });

    auto* watcher = new QFutureWatcher<bool>();
    QObject::connect(watcher, &QFutureWatcher<bool>::finished, [watcher, path]() {
        bool ok = watcher->result();
        watcher->deleteLater();

        if (ok) {
            Utils::ShowNotification(QMessageBox::Information, QObject::tr("Export Completed"), QObject::tr("Export completed successfully."), kThreeThousand);
        } else {
            QFile::remove(path);
            Utils::ShowNotification(
                QMessageBox::Critical, QObject::tr("Export Failed"), QObject::tr("Export failed. The file has been deleted."), kThreeThousand);
        }
    });

    watcher->setFuture(future);
}

bool ExportExcel::Statement(EntryHubP* entry_hub_p, TreeModelI* tree_model_i, CString& path, CString& partner_name, CUuid& partner_id, CString& unit_string,
    CDateTime& start, CDateTime& end, CJsonObject& total, CStatementEntryList& list)
{
    // Extract totals
    const double pbalance { total.value("pbalance").toString().toDouble() };
    const double camount { total.value("camount").toString().toDouble() };
    const double csettlement { total.value("csettlement").toString().toDouble() };
    const double cbalance { total.value("cbalance").toString().toDouble() };

    // Create excel document
    YXlsx::Document d(path);
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

    sheet->Write(start_row + 2, 1, QObject::tr("Period"));
    sheet->Write(start_row + 2, 2, start.toString(kDateFST));
    sheet->Write(start_row + 2, 3, end.toString(kDateFST));

    sheet->Write(start_row + 3, 1, QObject::tr("Previous Balance"));
    sheet->Write(start_row + 3, 3, pbalance);

    sheet->Write(start_row + 4, 1, QObject::tr("Current Amount"));
    sheet->Write(start_row + 4, 3, camount);

    sheet->Write(start_row + 5, 1, QObject::tr("Current Settlement"));
    sheet->Write(start_row + 5, 3, csettlement);

    sheet->Write(start_row + 6, 1, QObject::tr("Current Balance"));
    sheet->Write(start_row + 6, 3, cbalance);

    // ===========================
    // Table Header
    // ===========================
    const QStringList header { QObject::tr("Date"), QObject::tr("InternalSku"), QObject::tr("ExternalSku"), QObject::tr("Count"), QObject::tr("Measure"),
        QObject::tr("UnitPrice"), QObject::tr("Description"), QObject::tr("Amount") };

    sheet->WriteRow(start_row + 8, 1, header);

    // ===========================
    // Table Data
    // ===========================
    int row { start_row + 9 };

    for (const auto* entry : list) {
        const QUuid external_sku { entry_hub_p->ExternalSku(partner_id, entry->internal_sku) };

        QVariantList line { entry->issued_time.toString(kDateFST), tree_model_i->Path(entry->internal_sku), tree_model_i->Name(external_sku), entry->count,
            entry->measure, entry->unit_price, entry->description, entry->amount };

        sheet->WriteRow(row++, 1, line);
    }

    return d.Save();
}
