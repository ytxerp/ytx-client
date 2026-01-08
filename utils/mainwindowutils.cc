#include "mainwindowutils.h"

#include <QCoreApplication>
#include <QDir>
#include <QHeaderView>

#include "utils/templateutils.h"

bool Utils::PrepareNewFile(QString& file_path, CString& suffix)
{
    if (file_path.isEmpty())
        return false;

    if (!file_path.endsWith(suffix, Qt::CaseInsensitive))
        file_path += suffix;

    if (QFile::exists(file_path)) {
        QFile::remove(file_path);
    }

    return true;
}

void Utils::ExportExcel(CString& table, const QSharedPointer<YXlsx::Worksheet>& worksheet, bool where)
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

void Utils::Message(QMessageBox::Icon icon, CString& title, CString& text, int timeout)
{
    auto* box { new QMessageBox(icon, title, text, QMessageBox::NoButton) };
    QTimer::singleShot(timeout, box, &QMessageBox::accept);
    QObject::connect(box, &QMessageBox::finished, box, &QMessageBox::deleteLater);

    box->setModal(false);
    box->show();
}

void Utils::SwitchDialog(const SectionContext* sc, bool enable)
{
    if (!sc)
        return;

    const auto& list { sc->dialog_list };
    for (const auto& dialog : list) {
        if (dialog) {
            dialog->setVisible(enable);
        }
    }
}

int Utils::CompareVersion(const QString& v1, const QString& v2)
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

QString Utils::AccountIniFileName(const QString& email, const QString& workspace)
{
    // Extract email parts
    QString email_user { email.section('@', 0, 0) };
    QString domain_main { email.section('@', 1, 1).section('.', 0, 0) };

    // Regular expression to match invalid filename characters
    static const QRegularExpression invalid_chars { R"([\\/:*?"<>|\[\]\s.]+)" };

    // Sanitize all parts by replacing invalid characters with underscore
    email_user.replace(invalid_chars, "_");
    domain_main.replace(invalid_chars, "_");

    QString sanitized_workspace { workspace };
    sanitized_workspace.replace(invalid_chars, "_");

    // Combine: username_domain_workspace
    QString file_name { email_user + "_" + domain_main + "_" + sanitized_workspace };

    // Clean up: remove consecutive underscores and trim
    file_name.replace(QRegularExpression("_+"), "_");
    while (file_name.startsWith('_'))
        file_name.remove(0, 1);
    while (file_name.endsWith('_'))
        file_name.chop(1);

    return file_name;
}

void Utils::SetupHeaderStatus(QHeaderView* header, const QSharedPointer<QSettings>& settings, Section section, const QString& key)
{
    assert(header && settings);

    const auto section_name { kSectionString.value(section) };

    Utils::ReadConfig(header, &QHeaderView::restoreState, settings, section_name, key);

    QObject::connect(header, &QHeaderView::sectionMoved,
        [header, settings, section_name, key]() { Utils::WriteConfig(header, &QHeaderView::saveState, settings, section_name, key); });
}

void Utils::ResetSectionContext(SectionContext& ctx)
{
    if (ctx.tree_widget)
        ctx.tree_widget->Reset();

    if (ctx.entry_hub)
        ctx.entry_hub->Reset();

    if (ctx.tree_model)
        ctx.tree_model->Reset();

    ctx.section_config = SectionConfig {};
    ctx.shared_config = SharedConfig {};

    Utils::CloseWidgets(ctx.dialog_list);
    Utils::CloseWidgets(ctx.table_wgt_hash);
    Utils::CloseWidgets(ctx.widget_hash);
}
