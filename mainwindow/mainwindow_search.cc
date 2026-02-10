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
#include "websocket/jsongen.h"
#include "websocket/websocket.h"

void MainWindow::on_actionSearch_triggered()
{
    qInfo() << "[UI]" << "on_actionSearch_triggered";

    if (!section_settings_)
        return;

    SearchNodeModel* node {};
    SearchEntryModel* entry {};
    SearchDialog* dialog {};

    switch (start_) {
    case Section::kFinance:
        node = new SearchNodeModelF(sc_->info, sc_->tree_model, this);
        entry = new SearchEntryModelF(sc_->info, sc_->tag_hash, this);
        dialog = new SearchDialogF(sc_, node, entry, this);
        break;
    case Section::kInventory:
        node = new SearchNodeModelI(sc_->info, sc_->tree_model, this);
        entry = new SearchEntryModelI(sc_->info, sc_->tag_hash, this);
        dialog = new SearchDialogI(sc_, node, entry, this);
        break;
    case Section::kTask:
        node = new SearchNodeModelT(sc_->info, sc_->tree_model, this);
        entry = new SearchEntryModelT(sc_->info, sc_->tag_hash, this);
        dialog = new SearchDialogT(sc_, node, entry, this);
        break;
    case Section::kPartner:
        node = new SearchNodeModelP(sc_->info, sc_->tree_model, this);
        entry = new SearchEntryModelP(sc_->entry_hub, sc_->info, sc_->tag_hash, this);
        dialog = new SearchDialogP(sc_, node, entry, sc_i_.tree_model, this);
        break;
    case Section::kSale:
    case Section::kPurchase:
        node = new SearchNodeModelO(sc_->info, sc_->tree_model, this);
        entry = new SearchEntryModelO(sc_->info, sc_->tag_hash, this);
        dialog = new SearchDialogO(sc_, node, entry, sc_i_.tree_model, sc_p_.tree_model, this);
        connect(WebSocket::Instance(), &WebSocket::SNodeSearch, node, &SearchNodeModel::RNodeSearch);
        break;
    default:
        break;
    }

    Utils::ManageDialog(sc_->dialog_hash, dialog);

    connect(dialog, &SearchDialog::SNodeLocation, this, &MainWindow::RNodeLocation);
    connect(dialog, &SearchDialog::SEntryLocation, this, &MainWindow::REntryLocation);
    connect(sc_->entry_hub, &EntryHub::SSearchEntry, entry, &SearchEntryModel::RSearchEntry);

    dialog->show();
}
