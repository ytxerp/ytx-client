#include <QtCore/qdir.h>
#include <QtCore/qstandardpaths.h>

#include "global/collator.h"
#include "global/logininfo.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils/mainwindowutils.h"
#include "websocket/websocket.h"

void MainWindow::ReadLocalConfig()
{
    app_settings_ = QSharedPointer<QSettings>::create(
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() + kYTX + kDotSuffixINI, QSettings::IniFormat);

    app_settings_->beginGroup(kUi);
    app_config_.language = app_settings_->value(kLanguage, QLocale::system().name()).toString();
    app_config_.theme = app_settings_->value(kTheme, kSolarizedDark).toString();
    app_config_.separator = app_settings_->value(kSeparator, kDash).toString();
    app_settings_->endGroup();

    app_settings_->beginGroup(kPrint);
    app_config_.printer = app_settings_->value(kPrinter).toString();
    app_settings_->endGroup();

    app_settings_->beginGroup(kExport);
    app_config_.company_name = app_settings_->value(kCompanyName).toString();
    app_settings_->endGroup();

    LoginInfo::Instance().ReadConfig(app_settings_);
    WebSocket::Instance()->ReadConfig(app_settings_);
    WebSocket::Instance()->Connect();

    LoadAndInstallTranslator(app_config_.language);

    const QString theme { QStringLiteral("file:///:/theme/theme/%1.qss").arg(app_config_.theme) };
    qApp->setStyleSheet(theme);
}

void MainWindow::UpdateAppConfig(CAppConfig& app)
{
    if (app_config_ == app)
        return;

    auto new_separator { app.separator };
    auto old_separator { app_config_.separator };

    if (old_separator != new_separator) {
        sc_f_.tree_model->UpdateSeparator(old_separator, new_separator);
        sc_p_.tree_model->UpdateSeparator(old_separator, new_separator);
        sc_i_.tree_model->UpdateSeparator(old_separator, new_separator);
        sc_t_.tree_model->UpdateSeparator(old_separator, new_separator);

        auto* widget { ui->tabWidget };
        int count { ui->tabWidget->count() };

        for (int index = 0; index != count; ++index)
            widget->setTabToolTip(index, widget->tabToolTip(index).replace(old_separator, new_separator));
    }

    if (app_config_.language != app.language) {
        Utils::Message(QMessageBox::Information, tr("Language Changed"),
            tr("The language has been changed. Please restart the application for the changes to take effect."), kThreeThousand);
    }

    app_config_ = app;

    app_settings_->beginGroup(kUi);
    app_settings_->setValue(kLanguage, app.language);
    app_settings_->setValue(kSeparator, app.separator);
    app_settings_->setValue(kTheme, app.theme);
    app_settings_->endGroup();

    app_settings_->beginGroup(kPrint);
    app_settings_->setValue(kPrinter, app.printer);
    app_settings_->endGroup();

    app_settings_->beginGroup(kExport);
    app_settings_->setValue(kCompanyName, app.company_name);
    app_settings_->endGroup();
}

void MainWindow::ReadSectionConfig(SectionConfig& section_config, CString& section_name)
{
    section_settings_->beginGroup(section_name);
    const Section section { kStringSection.value(section_name) };

    if (IsDoubleEntry(section)) {
        section_config.static_label = section_settings_->value(kStaticLabel, {}).toString();
        section_config.static_node = section_settings_->value(kStaticNode, QUuid()).toUuid();
        section_config.dynamic_label = section_settings_->value(kDynamicLabel, {}).toString();
        section_config.dynamic_node_lhs = section_settings_->value(kDynamicNodeLhs, QUuid()).toUuid();
        section_config.operation = section_settings_->value(kOperation, kPlus).toString();
        section_config.dynamic_node_rhs = section_settings_->value(kDynamicNodeRhs, QUuid()).toUuid();
    }

    section_config.date_format = section_settings_->value(kDateFormat, kDateTimeFST).toString();
    section_config.amount_decimal = section_settings_->value(kAmountDecimal, 2).toInt();
    section_config.rate_decimal = section_settings_->value(kRateDecimal, 2).toInt();
    section_config.quantity_decimal = section_settings_->value(kQuantityDecimal, 2).toInt();

    section_settings_->endGroup();
}

void MainWindow::UpdateSectionConfig(CSectionConfig& section)
{
    auto& current_section { sc_->section_config };
    if (current_section == section)
        return;

    const bool resize_column { current_section.amount_decimal != section.amount_decimal || current_section.rate_decimal != section.rate_decimal
        || current_section.date_format != section.date_format };

    current_section = section;

    const QString text { kSectionString.value(start_) };
    section_settings_->beginGroup(text);

    if (start_ == Section::kFinance)
        sc_->tree_widget->RInitStatus();

    if (IsDoubleEntry(start_)) {
        section_settings_->setValue(kStaticLabel, section.static_label);
        section_settings_->setValue(kStaticNode, section.static_node);
        section_settings_->setValue(kDynamicLabel, section.dynamic_label);
        section_settings_->setValue(kDynamicNodeLhs, section.dynamic_node_lhs);
        section_settings_->setValue(kOperation, section.operation);
        section_settings_->setValue(kDynamicNodeRhs, section.dynamic_node_rhs);
    }

    section_settings_->setValue(kDateFormat, section.date_format);
    section_settings_->setValue(kAmountDecimal, section.amount_decimal);
    section_settings_->setValue(kRateDecimal, section.rate_decimal);
    section_settings_->setValue(kQuantityDecimal, section.quantity_decimal);

    section_settings_->endGroup();

    if (resize_column) {
        auto* current_widget { ui->tabWidget->currentWidget() };

        if (const auto* leaf_widget = dynamic_cast<TableWidget*>(current_widget)) {
            auto* header { leaf_widget->View()->horizontalHeader() };

            int column { std::to_underlying(EntryEnum::kDescription) };
            ResizeColumn(header, column);
            return;
        }

        if (dynamic_cast<TreeWidget*>(current_widget)) {
            auto* header { sc_->tree_view->header() };
            ResizeColumn(header, Utils::NodeDescriptionColumn(start_));
        }
    }
}

void MainWindow::LoadAndInstallTranslator(CString& language)
{
    const QString lang { kSupportedLanguages.contains(language) ? language : kEnUS };

    QLocale locale(lang);
    QLocale::setDefault(locale);
    Collator::SetLanguage(locale);

    if (lang.startsWith("en", Qt::CaseInsensitive))
        return;

    const QString ytx_language { QStringLiteral(":/I18N/I18N/ytx_%1.qm").arg(lang) };
    if (ytx_translator_.load(ytx_language))
        qApp->installTranslator(&ytx_translator_);

    const QString qt_language { QStringLiteral(":/I18N/I18N/qt_%1.qm").arg(lang) };
    if (qt_translator_.load(qt_language))
        qApp->installTranslator(&qt_translator_);
}
