#include "mainwindow.h"
#include "search/dialog/searchdialog.h"
#include "search/dialog/searchdialogf.h"
#include "search/dialog/searchdialogi.h"
#include "search/dialog/searchdialogo.h"
#include "search/dialog/searchdialogp.h"
#include "search/dialog/searchdialogt.h"
#include "search/entry/searchentrymodelf.h"
#include "search/entry/searchentrymodeli.h"
#include "search/entry/searchentrymodelo.h"
#include "search/entry/searchentrymodelp.h"
#include "search/entry/searchentrymodelt.h"
#include "search/node/searchnodemodelf.h"
#include "search/node/searchnodemodeli.h"
#include "search/node/searchnodemodelo.h"
#include "search/node/searchnodemodelp.h"
#include "search/node/searchnodemodelt.h"
#include "utils/mainwindowutils.h"

void MainWindow::on_actionSearch_triggered()
{
    qInfo() << Q_FUNC_INFO;

    if (!section_settings_)
        return;

    search::NodeModel* node {};
    search::EntryModel* entry {};
    SearchDialog* dialog {};

    const auto& info { sc_->info };
    const Section section { sc_->section };
    const QUuid widget_id { QUuid::createUuidV7() };

    switch (section) {
    case Section::kFinance:
        node = new search::NodeModelF(section, info.node_header, sc_->tree_model, sc_->tag_hash, this);
        entry = new search::EntryModelF(section, info.full_entry_header, sc_->tag_hash, this);
        dialog = new SearchDialogF(sc_, node, entry, widget_id, nullptr);
        break;
    case Section::kInventory:
        node = new search::NodeModelI(section, info.node_header, sc_->tree_model, sc_->tag_hash, this);
        entry = new search::EntryModelI(section, info.full_entry_header, sc_->tag_hash, this);
        dialog = new SearchDialogI(sc_, node, entry, widget_id, nullptr);
        break;
    case Section::kTask:
        node = new search::NodeModelT(section, info.node_header, sc_->tree_model, sc_->tag_hash, this);
        entry = new search::EntryModelT(section, info.full_entry_header, sc_->tag_hash, this);
        dialog = new SearchDialogT(sc_, node, entry, widget_id, nullptr);
        break;
    case Section::kPartner:
        node = new search::NodeModelP(section, info.node_header, sc_->tree_model, sc_->tag_hash, this);
        entry = new search::EntryModelP(section, info.full_entry_header, sc_->tag_hash, this);
        dialog = new SearchDialogP(sc_, node, entry, sc_i_.tree_model, widget_id, nullptr);
        break;
    case Section::kSale:
    case Section::kPurchase:
        node = new search::NodeModelO(section, info.node_header, sc_->tree_model, sc_->tag_hash, this);
        entry = new search::EntryModelO(section, info.full_entry_header, sc_->tag_hash, this);
        dialog = new SearchDialogO(sc_, node, entry, sc_i_.tree_model, sc_p_.tree_model, widget_id, nullptr);
        break;
    default:
        break;
    }

    utils::ManageDialog(sc_->widget_hash, dialog, widget_id);

    connect(dialog, &SearchDialog::SNodeLocate, this, &MainWindow::RNodeLocate);
    connect(dialog, &SearchDialog::SEntryLocate, this, &MainWindow::REntryLocate);

    dialog->show();
}

void MainWindow::ROrderSearch(Section section, QUuid widget_id, const QJsonArray& node_array)
{
    auto* sc { GetSectionContext(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* ptr { widget.data() };
    Q_ASSERT(qobject_cast<SearchDialogO*>(ptr));

    auto* d_widget { static_cast<SearchDialogO*>(ptr) };

    auto* model { d_widget->NodeModel() };
    model->Rebuild(node_array);
}

void MainWindow::REntrySearch(Section section, QUuid widget_id, const QJsonArray& entry_array)
{
    auto* sc { GetSectionContext(section) };

    auto widget { sc->widget_hash.value(widget_id).widget };
    if (!widget)
        return;

    auto* ptr { widget.data() };
    Q_ASSERT(qobject_cast<SearchDialog*>(ptr));

    auto* d_widget { static_cast<SearchDialog*>(ptr) };

    auto* model { d_widget->EntryModel() };
    model->Rebuild(entry_array);
}
