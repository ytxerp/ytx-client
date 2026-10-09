#include "exportexcel.h"

#include <QtConcurrent/qtconcurrentrun.h>
#include <QtCore/qfuturewatcher.h>

#include <QDir>
#include <QFileDialog>

#include "component/constantint.h"
#include "component/constantstring.h"
#include "document.h"
#include "utils/mainwindowutils.h"

void ExportExcel::StatementTertiaryAsync(
    CString& path, CString& partner_name, CString& unit_string, const utils::DateRange& range, const QList<QVariantList>& lines)
{
    auto future = QtConcurrent::run([=]() -> bool { return StatementTertiary(path, partner_name, unit_string, range, lines); });

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

void ExportExcel::StatementSecondaryAsync(
    CString& path, CString& partner_name, CString& unit_string, const utils::DateRange& range, const QList<QVariantList>& lines)
{
    auto future = QtConcurrent::run([=]() -> bool { return StatementSecondary(path, partner_name, unit_string, range, lines); });

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

bool ExportExcel::StatementTertiary(CString& path, CString& partner_name, CString& unit_string, const utils::DateRange& range, const QList<QVariantList>& lines)
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
    // Table
    // ===========================
    int row { start_row + 3 };
    for (const auto& line : lines)
        sheet->WriteRow(row++, 1, line);

    return d.Save();
}

bool ExportExcel::StatementSecondary(
    CString& path, CString& partner_name, CString& unit_string, const utils::DateRange& range, const QList<QVariantList>& lines)
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
    // Table
    // ===========================
    int row { start_row + 3 };
    for (const auto& line : lines)
        sheet->WriteRow(row++, 1, line);

    return d.Save();
}