#include <mainwindow/ui_mainwindow.h>

#include <QJsonArray>
#include <QPainter>

#include "mainwindow.h"
#include "tag/tagdialog.h"
#include "tag/tagenum.h"
#include "tag/tagmodel.h"
#include "utils/entryutils.h"
#include "utils/mainwindowutils.h"
#include "utils/nodeutils.h"
#include "utils/tagutils.h"

void MainWindow::on_actionTags_triggered()
{
    qInfo() << Q_FUNC_INFO;

    auto* model { new tag::Model(start_, header_info_.tag, this) };

    auto* dialog { new TagDialog(model) };

    utils::ManageDialog(sc_->widget_hash, dialog);
    dialog->setModal(true);

    auto* view { dialog->View() };
    InitTableView(view, std::to_underlying(tag::RowField::kColor));
    DelegateTag(view);

    // Most models are populated asynchronously after receiving data from the server.
    // Tags are a special case: data is already available locally, so rebuild after
    // view initialization to avoid the view's sorting state overriding the initial order.
    model->Rebuild(sc_->tag_hash);

    dialog->show();
}

void MainWindow::RApplyTag(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    auto* sc { GetSectionContext(section) };

    const QJsonValue tags { obj.value(kTagArray) };
    Q_ASSERT(tags.isArray());

    const QJsonArray tag_array { tags.toArray() };

    for (const QJsonValue& value : tag_array) {
        Q_ASSERT(value.isObject());

        const QJsonObject tag_obj { value.toObject() };
        const QUuid id { tag_obj.value(kId).toString() };

        Q_ASSERT(!id.isNull());

        tag::Row tag {};
        tag.ReadJson(tag_obj);

        sc->tag_hash.insert(id, tag);
        UpdateTagIcon(sc, tag);
    }
}

void MainWindow::RInsertTag(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    auto* sc { GetSectionContext(section) };

    const QJsonValue value { obj.value(kTag) };
    Q_ASSERT(value.isObject());

    const QJsonObject tag_obj { value.toObject() };
    const QUuid id { tag_obj.value(kId).toString() };

    Q_ASSERT(!id.isNull());
    Q_ASSERT(!sc->tag_hash.contains(id));

    tag::Row tag {};

    tag.ReadJson(tag_obj);
    sc->tag_hash.insert(tag.id, tag);
    UpdateTagIcon(sc, tag);
}

void MainWindow::RUpdateTag(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    auto* sc { GetSectionContext(section) };

    const QUuid id { obj.value(kId).toString() };
    Q_ASSERT(!id.isNull());

    auto it { sc->tag_hash.find(id) };
    if (it == sc->tag_hash.end())
        return;

    const QJsonValue value { obj.value(kUpdate) };
    Q_ASSERT(value.isObject());

    const QJsonObject update_obj { value.toObject() };
    const int version { obj.value(kVersion).toInt() };

    auto& tag { it.value() };
    tag.ReadJson(update_obj);
    tag.version = version;

    UpdateTagIcon(sc, tag);
}

void MainWindow::RDeleteTag(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };
    auto* sc { GetSectionContext(section) };

    const QUuid id { obj.value(kId).toString() };
    Q_ASSERT(!id.isNull());

    sc->tag_icon_hash.remove(id);
    sc->tag_hash.remove(id);
}

void MainWindow::RTreeViewCustomContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos);

    const auto index { sc_->tree_view->currentIndex() };
    if (!index.isValid())
        return;

    auto model { sc_->tree_model };
    const auto* node { static_cast<Node*>(index.internalPointer()) };

    const auto& tag_hash { sc_->tag_hash };

    auto* menu = new QMenu(this);

    auto* tag_menu = menu->addMenu(tr("Tags"));
    tag_menu->setIcon(ui->actionTags->icon());

    if (!tag_hash.isEmpty()) {
        QList<tag::Row> list = tag_hash.values();
        std::ranges::sort(list, [](const auto& lhs, const auto& rhs) { return utils::CompareString(lhs.name, rhs.name, Qt::AscendingOrder); });

        for (const auto& tag : std::as_const(list)) {
            Q_ASSERT(!tag.id.isNull());

            auto* tag_action { tag_menu->addAction(tag.name) };

            const bool is_checked { node->tag.contains(tag.id.toString(QUuid::WithoutBraces)) };

            tag_action->setIcon(GetTagIcon(sc_, tag, is_checked));
            tag_action->setIconVisibleInMenu(true);

            connect(tag_action, &QAction::triggered, this, [this, is_checked, model, tag, node]() {
                if (is_checked) {
                    RRemoveNodeTag(tag, model, node);
                } else {
                    RInsertNodeTag(tag, model, node);
                }
            });
        }

        tag_menu->addSeparator();
    }

    auto* manage_action = tag_menu->addAction(tr("Manage..."));
    // manage_action->setIcon(QIcon(":/icons/settings.png"));
    connect(manage_action, &QAction::triggered, this, [this]() { on_actionTags_triggered(); });

    menu->addSeparator();
    menu->addAction(ui->actionInsertNode);
    menu->addAction(ui->actionAppendNode);

    menu->addSeparator();
    menu->addAction(ui->actionRename);
    menu->addAction(ui->actionClearColor);

    menu->exec(QCursor::pos());
}

void MainWindow::RInsertNodeTag(const tag::Row& tag, TreeModel* model, const Node* node)
{
    qDebug() << Q_FUNC_INFO;

    auto list { node->tag };
    list.emplaceFront(tag.id.toString(QUuid::WithoutBraces));

    auto index { model->GetIndex(node->id) };
    if (!index.isValid()) {
        qWarning() << "Invalid index for node:" << node->name;
        return;
    }

    const int column { node::TagColumn(start_) };
    const QModelIndex tag_index { model->index(index.row(), column, index.parent()) };

    model->setData(tag_index, list);
}

void MainWindow::RRemoveNodeTag(const tag::Row& tag, TreeModel* model, const Node* node)
{
    qDebug() << Q_FUNC_INFO;

    auto list { node->tag };
    list.removeOne(tag.id.toString(QUuid::WithoutBraces));

    auto index { model->GetIndex(node->id) };
    if (!index.isValid()) {
        qWarning() << "Invalid index for node:" << node->name;
        return;
    }

    const int column { node::TagColumn(start_) };
    const QModelIndex tag_index { model->index(index.row(), column, index.parent()) };

    model->setData(tag_index, list);
}

void MainWindow::RTableViewCustomContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos);

    auto* widget { qobject_cast<TableWidget*>(sc_->tab_widget->currentWidget()) };
    if (!widget)
        return;

    const auto index { widget->View()->currentIndex() };
    if (!index.isValid())
        return;

    auto* model { widget->Model() };
    const auto* entry { model->GetEntry(index) };

    const auto& tag_hash { sc_->tag_hash };

    auto* menu = new QMenu(this);
    auto* tag_menu = menu->addMenu(tr("Tags"));

    if (!tag_hash.isEmpty()) {
        QList<tag::Row> list = tag_hash.values();
        std::ranges::sort(list, [](const auto& lhs, const auto& rhs) { return utils::CompareString(lhs.name, rhs.name, Qt::AscendingOrder); });

        for (const auto& tag : std::as_const(list)) {
            Q_ASSERT(!tag.id.isNull());

            auto* tag_action { tag_menu->addAction(tag.name) };

            const bool is_checked { entry->tag.contains(tag.id.toString(QUuid::WithoutBraces)) };

            tag_action->setIcon(GetTagIcon(sc_, tag, is_checked));
            tag_action->setIconVisibleInMenu(true);

            connect(tag_action, &QAction::triggered, this, [this, is_checked, entry, model, tag]() {
                if (is_checked) {
                    RRemoveEntryTag(tag, model, entry);
                } else {
                    RInsertEntryTag(tag, model, entry);
                }
            });
        }

        tag_menu->addSeparator();
    }

    auto* manage_action { tag_menu->addAction(tr("Manage...")) };
    // manage_action->setIcon(QIcon(":/icons/settings.png"));
    connect(manage_action, &QAction::triggered, this, [this]() { on_actionTags_triggered(); });

    if (IsDoubleEntry(start_)) {
        menu->addSeparator();
        menu->addAction(ui->actionJumpEntry);
    }

    menu->exec(QCursor::pos());
}

void MainWindow::UpdateTagIcon(SectionContext* sc, const tag::Row& tag)
{
    Q_ASSERT(sc);

    const QUuid tag_id { tag.id };
    Q_ASSERT(!tag_id.isNull());

    tag::Icon icon {};
    icon.pixmap = tag::CreatePixmap(tag);
    icon.icon = tag::CreateIcon(tag, /*checked=*/false);
    icon.icon_checked = tag::CreateIcon(tag, /*checked=*/true);

    sc->tag_icon_hash.insert(tag_id, icon);
}

QIcon MainWindow::GetTagIcon(SectionContext* sc, const tag::Row& tag, bool checked)
{
    Q_ASSERT(sc);

    const QUuid tag_id { tag.id };
    Q_ASSERT(!tag_id.isNull());

    auto it = sc->tag_icon_hash.find(tag_id);
    if (it == sc->tag_icon_hash.end()) {
        tag::Icon icon {};
        icon.pixmap = tag::CreatePixmap(tag);
        icon.icon = tag::CreateIcon(tag, false);
        icon.icon_checked = tag::CreateIcon(tag, true);

        it = sc->tag_icon_hash.insert(tag_id, icon);
    }

    return checked ? it->icon_checked : it->icon;
}

void MainWindow::RInsertEntryTag(const tag::Row& tag, TableModel* model, const Entry* entry)
{
    qDebug() << Q_FUNC_INFO;
    auto list { entry->tag };
    list.emplaceFront(tag.id.toString(QUuid::WithoutBraces));

    auto index { model->GetIndex(entry->id) };
    if (!index.isValid()) {
        qWarning() << "Invalid index for entry:" << entry->id;
        return;
    }

    const int column { entry::TagColumn(start_) };

    QModelIndex tag_index = model->index(index.row(), column);
    model->setData(tag_index, list);
}

void MainWindow::RRemoveEntryTag(const tag::Row& tag, TableModel* model, const Entry* entry)
{
    qDebug() << Q_FUNC_INFO;
    auto list { entry->tag };
    list.removeOne(tag.id.toString(QUuid::WithoutBraces));

    auto index { model->GetIndex(entry->id) };
    if (!index.isValid()) {
        qWarning() << "Invalid index for entry";
        return;
    }

    const int column { entry::TagColumn(start_) };

    QModelIndex tag_index = model->index(index.row(), column);
    model->setData(tag_index, list);
}
