#include "mainwindowutils.h"

#include <QtWidgets/qpushbutton.h>
#include <zstd.h>

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QHeaderView>
#include <QPainter>

#include "global/resourcepool.h"
#include "utils/templateutils.h"

bool utils::PrepareNewFile(QString& file_path, CString& suffix)
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

void utils::ExportExcel(CString& table, const QSharedPointer<YXlsx::Worksheet>& worksheet, bool where)
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

void utils::ShowNotification(QMessageBox::Icon icon, CString& title, CString& text, int duration_ms, QMessageBox::StandardButtons buttons, QWidget* parent)
{
    assert(duration_ms >= 0);

    auto* box { new QMessageBox(icon, title, text, buttons, parent) };
    box->setAttribute(Qt::WA_DeleteOnClose);

    if (duration_ms > 0) {
        QTimer::singleShot(duration_ms, box, &QMessageBox::accept);
    }

    box->show();
}

QMessageBox* utils::CreateMessageBox(QMessageBox::Icon icon, CString& title, CString& text, bool modal, QMessageBox::StandardButtons buttons, QWidget* parent)
{
    auto* box { new QMessageBox(icon, title, text, buttons, parent) };
    box->setAttribute(Qt::WA_DeleteOnClose);

    if (modal && parent) {
        box->setWindowModality(Qt::WindowModal);
    }

    return box;
}

void utils::SwitchDialog(const SectionContext* sc, bool enable)
{
    if (!sc)
        return;

    const auto& widget_hash { sc->widget_hash };
    for (const auto& wc : widget_hash) {
        if (wc.widget && wc.role == WidgetRole::kDialog) {
            wc.widget->setVisible(enable);
        }
    }
}

int utils::CompareVersion(const QString& v1, const QString& v2)
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

QString utils::AccountIniFileName(const QString& email, const QString& workspace)
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

void utils::SetupHeaderStatus(QHeaderView* header, const QSharedPointer<QSettings>& settings, Section section, const QString& key)
{
    Q_ASSERT(header && settings);

    const auto section_name { kSectionString.value(section) };

    utils::ReadConfig(header, &QHeaderView::restoreState, settings, section_name, key);

    QObject::connect(header, &QHeaderView::sectionMoved,
        [header, settings, section_name, key]() { utils::WriteConfig(header, &QHeaderView::saveState, settings, section_name, key); });
}

void utils::ResetSectionContext(SectionContext& sc)
{
    Q_ASSERT(sc.tree_widget);
    Q_ASSERT(sc.entry_hub);
    Q_ASSERT(sc.tree_model);

    {
        auto tab_widget { sc.tab_widget };
        auto* tab_bar { tab_widget->tabBar() };
        const int count { tab_widget->count() };

        for (int index = count - 1; index >= 0; --index) {
            const auto id { tab_bar->tabData(index).toUuid() };
            if (!id.isNull())
                tab_widget->removeTab(index);
        }
    }

    utils::CloseWidgets(sc.widget_hash);

    sc.tree_widget->Reset();
    sc.tree_model->Reset();
    sc.entry_hub->Reset();

    sc.section_config = SectionConfig {};
    sc.shared_config = SharedConfig {};

    ResourcePool<Tag>::Instance().Recycle(sc.tag_hash);
    sc.tag_icon_hash.clear();
}

void utils::SetConnectionStatus(QLabel* label, ConnectionStatus status)
{
    Q_ASSERT(label != nullptr);

    switch (status) {
    case ConnectionStatus::Connecting:
        label->setText("…");
        label->setToolTip(QObject::tr("Connecting to server..."));
        label->setStyleSheet("QLabel {color: orange;}");
        return;
    case ConnectionStatus::Connected:
        label->setText("●");
        label->setToolTip(QObject::tr("Connected to server"));
        label->setStyleSheet("QLabel {color: green;}");
        return;
    case ConnectionStatus::Disconnected:
        label->setText("○");
        label->setToolTip(QObject::tr("Disconnected from server"));
        label->setStyleSheet("QLabel {color: red;}");
        return;
    }

    Q_UNREACHABLE();
}

void utils::SetLoginStatus(QLabel* label, LoginStatus status)
{
    Q_ASSERT(label != nullptr);

    switch (status) {
    case LoginStatus::LoggedIn:
        label->setText("●");
        label->setToolTip(QObject::tr("Logged in"));
        label->setStyleSheet("QLabel { color: green; }");
        return;
    case LoginStatus::LoggedOut:
        label->setText("○");
        label->setToolTip(QObject::tr("Logged out"));
        label->setStyleSheet("QLabel { color: red; }");
        return;
    }

    Q_UNREACHABLE();
}

void utils::SetPushButton(QPushButton* btn, const QKeySequence& ks)
{
    Q_ASSERT(btn != nullptr);

    btn->setShortcut(ks);
    btn->setToolTip(QString("%1 (%2)").arg(btn->text(), ks.toString()));
}

void utils::SetRadioButton(QRadioButton* btn, const QKeySequence& ks)
{
    Q_ASSERT(btn != nullptr);
    btn->setShortcut(ks);
    btn->setToolTip(QString("%1 (%2)").arg(btn->text(), ks.toString()));
}

/**
 * Convert UUID v7 to a short display code for orders
 *
 * @param uuid The UUID v7 to convert
 * @param length Total length of the short code (excluding separator)
 * @return Formatted short code (e.g., "AB12-3XYZ45" for length 10)
 *
 * Note: This is one-way conversion for display purposes only.
 * The original UUID should be stored in database for lookups.
 */
QString utils::UuidToShortCode(const QUuid& uuid, int length)
{
    // Extract 16 bytes from UUID
    const QByteArray bytes { uuid.toRfc4122() };
    QString base32 {};

    // Bit manipulation for Base32 encoding
    int bit_buffer { 0 };
    int bits_left { 0 };

    // Process each byte
    for (char byte : bytes) {
        bit_buffer = (bit_buffer << 8) | static_cast<unsigned char>(byte);
        bits_left += 8;

        // Extract 5-bit chunks for Base32
        while (bits_left >= 5) {
            bits_left -= 5;
            int index { (bit_buffer >> bits_left) & 0x1F };
            base32.append(kBase32Crockford[index]);
        }
    }

    // Handle remaining bits (if any)
    if (bits_left > 0) {
        int index { (bit_buffer << (5 - bits_left)) & 0x1F };
        base32.append(kBase32Crockford[index]);
    }

    // Return plain code (add separator only when displaying/printing)
    return base32.left(length);
}

QUuid utils::ManageDialog(QHash<QUuid, WidgetContext>& widget_hash, QDialog* dialog)
{
    Q_ASSERT(dialog);

    dialog->setAttribute(Qt::WA_DeleteOnClose);

    const QUuid id { QUuid::createUuidV7() };
    WidgetContext wc { dialog, id, WidgetRole::kDialog };

    widget_hash.insert(id, wc);
    return id;
}

QByteArray utils::ZstdDecompress(const QByteArray& data)
{
    if (!data.startsWith("\x28\xB5\x2F\xFD"))
        return data;

    // Attempt to retrieve the exact decompressed size from the zstd frame header.
    // This is available in the vast majority of zstd frames.
    unsigned long long const content_size { ZSTD_getFrameContentSize(data.constData(), static_cast<size_t>(data.size())) };

    if (content_size == ZSTD_CONTENTSIZE_ERROR) {
        qWarning() << "[ZstdDecompress] Not a valid zstd frame";
        return {};
    }

    if (content_size == ZSTD_CONTENTSIZE_UNKNOWN) {
        qWarning() << "[ZstdDecompress] Content size unknown";
        return {};
    }

    // Allocate the output buffer without zero-initialisation for performance.
    QByteArray result(static_cast<qsizetype>(content_size), Qt::Uninitialized);

    static ZSTD_DCtx* dctx { ZSTD_createDCtx() };
    if (!dctx) {
        qWarning() << "[ZstdDecompress] Failed to create DCtx";
        return {};
    }

    ZSTD_DCtx_reset(dctx, ZSTD_reset_session_only);

    size_t const ret { ZSTD_decompressDCtx(dctx, result.data(), static_cast<size_t>(result.size()), data.constData(), static_cast<size_t>(data.size())) };

    if (ZSTD_isError(ret)) {
        qWarning() << "[ZstdDecompress] Error:" << ZSTD_getErrorName(ret);
        return {};
    }

    if (ret != static_cast<size_t>(result.size())) {
        qWarning() << "[ZstdDecompress] Size mismatch:" << ret << "!=" << result.size();
        return {};
    }

    return result;
}

QByteArray utils::ZstdCompress(const QByteArray& data)
{
    // Skip compression for small payloads, as zstd overhead may increase size.
    if (data.size() < ZstdConst::kCompressThreshold)
        return data;

    size_t const bound { ZSTD_compressBound(static_cast<size_t>(data.size())) };
    QByteArray result(static_cast<qsizetype>(bound), Qt::Uninitialized);

    static ZSTD_CCtx* const cctx { ZSTD_createCCtx() };
    if (!cctx) {
        qWarning() << "Zstd compress error: failed to create CCtx";
        return {};
    }

    ZSTD_CCtx_reset(cctx, ZSTD_reset_session_only);
    ZSTD_CCtx_setParameter(cctx, ZSTD_c_contentSizeFlag, 1);
    ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, 3);
    ZSTD_CCtx_setPledgedSrcSize(cctx, static_cast<unsigned long long>(data.size()));

    size_t const compressed_size { ZSTD_compress2(cctx, result.data(), bound, data.constData(), static_cast<size_t>(data.size())) };

    if (ZSTD_isError(compressed_size)) {
        qWarning() << "Zstd compress error:" << ZSTD_getErrorName(compressed_size);
        return {};
    }

    result.resize(static_cast<qsizetype>(compressed_size));

    return result;
}
