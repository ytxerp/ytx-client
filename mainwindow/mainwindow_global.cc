#include <QJsonArray>
#include <QMessageBox>

#include "mainwindow.h"
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::RSharedConfig(const QJsonArray& arr)
{
    for (const QJsonValue& val : arr) {
        if (!val.isObject()) {
            qWarning() << "Invalid item in GlobalConfig array:" << val;
            continue;
        }

        QJsonObject obj { val.toObject() };
        Section section { obj.value("section").toInt() };
        int default_unit { obj.value("default_unit").toInt() };
        QString document_dir { obj.value("document_dir").toString() };

        auto* section_contex = GetSectionContex(section);
        if (!section_contex) {
            continue;
        }

        section_contex->shared_config.default_unit = default_unit;
        section_contex->shared_config.document_dir = document_dir;
        section_contex->tree_widget->InitStatus();

        section_contex->tree_model->UpdateDefaultUnit(default_unit);
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
        sc_->tree_widget->InitStatus();
}

void MainWindow::RUpdateDefaultUnitFailed(const QString& /*section*/)
{
    QMessageBox::warning(this, tr("Update Failed"), tr("Cannot change the base unit for section Finance because related entries already exist."));
}

void MainWindow::UpdateSharedConfig(CSharedConfig& shared)
{
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
