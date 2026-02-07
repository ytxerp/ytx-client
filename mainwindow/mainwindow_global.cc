#include <QJsonArray>

#include "mainwindow.h"
#include "utils/mainwindowutils.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::RSharedConfig(const QJsonArray& arr)
{
    for (const QJsonValue& val : arr) {
        if (!val.isObject()) {
            qWarning() << "Invalid item in GlobalConfig array:" << val;
            continue;
        }

        const QJsonObject obj { val.toObject() };
        const Section section { obj.value("section").toInt() };
        const int default_unit { obj.value("default_unit").toInt() };
        const QString document_dir { obj.value("document_dir").toString() };

        auto* sc = GetSectionContex(section);
        if (!sc) {
            continue;
        }

        sc->shared_config.default_unit = default_unit;
        sc->shared_config.document_dir = document_dir;
    }
}

void MainWindow::RDocumentDir(Section section, const QString& document_dir)
{
    auto* sc { GetSectionContex(section) };
    sc->shared_config.document_dir = document_dir;
}

void MainWindow::RDefaultUnit(Section section, int unit)
{
    auto* sc { GetSectionContex(section) };
    sc->shared_config.default_unit = unit;

    if (section == Section::kFinance)
        sc_->tree_widget->RInitStatus();
}

void MainWindow::RUpdateDefaultUnitFailed(const QString& /*section*/)
{
    Utils::ShowNotification(QMessageBox::Warning, tr("Update Failed"),
        tr("Cannot change the base unit for section Finance because related entries already exist."), kThreeThousand);
}

void MainWindow::UpdateSharedConfig(CSharedConfig& shared)
{
    if (!section_settings_)
        return;

    auto& current_shared { sc_->shared_config };
    if (current_shared == shared)
        return;

    if (current_shared.document_dir != shared.document_dir) {
        const auto message { JsonGen::DocumentDir(sc_->info.section, shared.document_dir) };
        WebSocket::Instance()->SendMessage(kDocumentDir, message);
        current_shared.document_dir = shared.document_dir;
    }

    if (current_shared.default_unit != shared.default_unit) {
        const auto message { JsonGen::DefaultUnit(sc_->info.section, shared.default_unit) };
        WebSocket::Instance()->SendMessage(kDefaultUnit, message);
    }
}
