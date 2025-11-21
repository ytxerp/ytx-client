#include "mainwindowutils.h"

#include <QCoreApplication>
#include <QDir>
#include <QHeaderView>

#include "utils/entryutils.h"

bool MainWindowUtils::PrepareNewFile(QString& file_path, CString& suffix)
{
    if (file_path.isEmpty())
        return false;

    if (!file_path.endsWith(suffix, Qt::CaseInsensitive))
        file_path += suffix;

    if (QFile::exists(file_path)) {
        qDebug() << "Destination file already exists. Overwriting:" << file_path;
        QFile::remove(file_path);
    }

    return true;
}

bool MainWindowUtils::CheckFileValid(CString& file_path, CString& suffix)
{
    if (file_path.isEmpty())
        return false;

    const QFileInfo file_info(file_path);

    if (!file_info.exists() || !file_info.isFile()) {
        qDebug() << "The specified file does not exist or is not a valid file: " << file_path;
        return false;
    }

    if (file_info.suffix().compare(suffix, Qt::CaseInsensitive) != 0) {
        qDebug() << "The file extension does not match the expected type: " << file_path;
        return false;
    }

    return true;
}

void MainWindowUtils::ExportExcel(CString& table, QSharedPointer<YXlsx::Worksheet> worksheet, bool where)
{
    if (!worksheet) {
        return;
    }

    // QSqlDatabase source_db { PublicUtils::GetDatabase(kSourceConnection) };
    // if (!source_db.isValid())
    //     return;

    // QSqlQuery source_query(source_db);
    // QString select_query { QString("SELECT * FROM %1 WHERE is_valid = TRUE;").arg(table) };

    // if (!where)
    //     select_query = QString("SELECT * FROM %1;").arg(table);

    // if (!source_query.exec(select_query)) {
    //     qDebug() << "Failed to execute SELECT query for table in ExportExcel" << source_query.lastError().text();
    //     return;
    // }

    // const int column { source_query.record().count() };
    // QList<QVariantList> list(column);

    // while (source_query.next()) {
    //     for (int col = 0; col != column; ++col) {
    //         list[col].append(source_query.value(col));
    //     }
    // }

    // for (int col = 0; col != column; ++col) {
    //     worksheet->WriteColumn(2, col + 1, list.at(col));
    // }
}

void MainWindowUtils::Message(QMessageBox::Icon icon, CString& title, CString& text, int timeout)
{
    auto* box { new QMessageBox(icon, title, text, QMessageBox::NoButton) };
    QTimer::singleShot(timeout, box, &QMessageBox::accept);
    QObject::connect(box, &QMessageBox::finished, box, &QMessageBox::deleteLater);

    box->setModal(false);
    box->show();
}

void MainWindowUtils::SwitchDialog(const SectionContext* sc, bool enable)
{
    if (!sc)
        return;

    const auto& list { sc->dialog_list };
    for (auto dialog : list) {
        if (dialog) {
            dialog->setVisible(enable);
        }
    }
}

int MainWindowUtils::CompareVersion(const QString& v1, const QString& v2)
{
    const QStringList parts1 { v1.split('.') };
    const QStringList parts2 { v2.split('.') };

    const long long n { qMax(parts1.size(), parts2.size()) };

    for (long long i = 0; i != n; ++i) {
        const int num1 { i < parts1.size() ? parts1[i].toInt() : 0 };
        const int num2 { i < parts2.size() ? parts2[i].toInt() : 0 };

        if (num1 < num2)
            return -1;

        if (num1 > num2)
            return 1;
    }

    return 0; // equal
}

QString MainWindowUtils::SectionFile(const QString& email, const QString& workspace)
{
    QString email_prefix { email.section('@', 0, 0) };

    QString file_name { email_prefix + "_" + workspace };

    static QRegularExpression invalid_chars(R"([\\/:*?"<>| \t]+)");
    file_name.replace(invalid_chars, "_");

    return file_name;
}

void MainWindowUtils::AppendEntry(TableWidget* widget, Section section)
{
    assert(widget);

    auto* model { widget->Model() };
    assert(model);

    const int new_row { model->rowCount() };
    if (!model->insertRows(new_row, 1))
        return;

    const int linked_node_col { EntryUtils::LinkedNodeColumn(section) };
    QModelIndex target_index { model->index(new_row, linked_node_col) };

    if (target_index.isValid()) {
        widget->View()->setCurrentIndex(target_index);
    }
}

void MainWindowUtils::RemoveEntry(TableWidget* widget)
{
    assert(widget);

    auto* view { widget->View() };
    assert(view);

    if (!TemplateUtils::HasSelection(view))
        return;

    const QModelIndex current_index { view->currentIndex() };
    if (!current_index.isValid())
        return;

    auto* model { widget->Model() };
    assert(model);

    const int current_row { current_index.row() };
    if (!model->removeRows(current_row, 1)) {
        qDebug() << "Failed to remove row:" << current_row;
        return;
    }

    const int new_row_count { model->rowCount() };
    if (new_row_count == 0)
        return;

    QModelIndex new_index {};
    if (current_row < new_row_count) {
        new_index = model->index(current_row, 0);
    } else {
        new_index = model->index(new_row_count - 1, 0);
    }

    if (new_index.isValid()) {
        view->setCurrentIndex(new_index);
        view->closePersistentEditor(new_index);
    }
}

void MainWindowUtils::SetupHeaderStatus(QHeaderView* header, QSharedPointer<QSettings> settings, Section section, const QString& key)
{
    assert(header && settings);

    const auto section_name { SectionToString(section) };

    TemplateUtils::ReadConfig(header, &QHeaderView::restoreState, settings, section_name, key);

    QObject::connect(header, &QHeaderView::sectionMoved,
        [header, settings, section_name, key]() { TemplateUtils::WriteConfig(header, &QHeaderView::saveState, settings, section_name, key); });
}
