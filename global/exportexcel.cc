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
#include "order/statement/statementenum.h"
#include "utils/mainwindowutils.h"

void ExportExcel::StatementTertiaryAsync(CString& path, CString& partner_name, CUuid& partner_id, CString& unit_string, const utils::DateRange& range,
    CStringList& header, const QList<statement::TertiaryRow>& list)
{
    auto future = QtConcurrent::run([=]() -> bool { return StatementTertiary(path, partner_name, partner_id, unit_string, range, header, list); });

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

bool ExportExcel::StatementTertiary(CString& path, CString& partner_name, CUuid& partner_id, CString& unit_string, const utils::DateRange& range,
    CStringList& header, const QList<statement::TertiaryRow>& list)
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
    sheet->WriteRow(start_row + 3, 1, header);

    // ===========================
    // Table Data
    // ===========================
    int row { start_row + 4 };
    const auto& master { MasterDataRegistry::Instance() };
    const auto& partner { PartnerInventoryRegistry::Instance() };

    for (const auto& entry : list) {
        const QUuid external_sku { partner.ExternalSku(partner_id, entry.internal_sku) };

        QVariantList line {};
        line.reserve(header.size());

        for (int column { 0 }; column != header.size(); ++column) {
            switch (static_cast<statement::TertiaryField>(column)) {
            case statement::TertiaryField::kIssuedTime:
                line.append(entry.issued_time.toString(datetime_format::kDashedDate));
                break;
            case statement::TertiaryField::kCode:
                line.append(entry.code);
                break;
            case statement::TertiaryField::kInternalSku:
                line.append(master.InventoryPath(entry.internal_sku));
                break;
            case statement::TertiaryField::kExternalSku:
                line.append(master.InventoryName(external_sku));
                break;
            case statement::TertiaryField::kCount:
                line.append(entry.count);
                break;
            case statement::TertiaryField::kMeasure:
                line.append(entry.measure);
                break;
            case statement::TertiaryField::kUnitPrice:
                line.append(entry.unit_price);
                break;
            case statement::TertiaryField::kDescription:
                line.append(entry.description);
                break;
            case statement::TertiaryField::kAmount:
                line.append(entry.amount);
                break;
            case statement::TertiaryField::kStatus:
            default:
                break;
            }
        }

        sheet->WriteRow(row++, 1, line);
    }

    // ===========================
    // Write Total
    // ===========================
    sheet->Write(row + 1, 1, QObject::tr("Total"));

    return d.Save();
}
