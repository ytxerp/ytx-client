#include <mainwindow/ui_mainwindow.h>

#include <QJsonArray>
#include <QPainter>

#include "dialog/tagmanagerdlg.h"
#include "global/resourcepool.h"
#include "mainwindow.h"
#include "tag/tagmodel.h"
#include "utils/entryutils.h"
#include "utils/mainwindowutils.h"

void MainWindow::on_actionTags_triggered()
{
    qInfo() << "[UI]" << "on_actionTags_triggered";

    static QPointer<TagManagerDlg> dialog {};

    if (!dialog) {
        auto* model { new TagModel(start_, sc_->tag_hash, this) };
        connect(model, &TagModel::SInsertingTag, this, &MainWindow::RInsertingTag);

        dialog = new TagManagerDlg(this);

        Utils::ManageDialog(sc_->dialog_hash, dialog);

        dialog->SetModel(model);

        auto* view { dialog->View() };
        SetTagView(view);
        DelegateTagView(view);
    }

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void MainWindow::RApplyTag(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };

    auto* sc { GetSectionContex(section) };
    Q_ASSERT(sc);

    if (!obj.value(kTagArray).isArray()) {
        qWarning() << "RApplyTag: 'tags' is not an array";
        return;
    }

    const QJsonArray tag_array { obj.value(kTagArray).toArray() };

    for (const QJsonValue& value : tag_array) {
        if (!value.isObject()) {
            qWarning() << "RApplyTag: tag is not an object";
            continue;
        }

        const QJsonObject tag_obj { value.toObject() };
        const QUuid id { tag_obj.value(kId).toString() };

        if (id.isNull()) {
            qWarning() << "RApplyTag: tag id is null, skip entry";
            continue;
        }

        auto* tag { ResourcePool<Tag>::Instance().Allocate() };
        tag->ReadJson(tag_obj);
        tag->state = Tag::State::SYNCED;

        sc->tag_hash.insert(id, tag);
        UpdateTagIcon(sc, tag);
    }
}

void MainWindow::RInsertTag(const QJsonObject& obj, bool is_same_session)
{
    const Section section { obj.value(kSection).toInt() };

    auto* sc { GetSectionContex(section) };
    Q_ASSERT(sc);

    if (!obj.value(kTag).isObject()) {
        qWarning() << "RInsertTag: 'tag' is not an object";
        return;
    }

    const QJsonObject tag_obj { obj.value(kTag).toObject() };
    const QUuid id { tag_obj.value(kId).toString() };

    if (id.isNull()) {
        qWarning() << "RInsertTag: tag id is null";
        return;
    }

    Tag* tag {};

    if (is_same_session) {
        tag = inserting_tag_.take(id);
        if (!tag) {
            qWarning() << "RInsertTag: pending tag not found" << id;
            return;
        }
    } else {
        tag = ResourcePool<Tag>::Instance().Allocate();
    }

    tag->ReadJson(tag_obj);
    tag->state = Tag::State::SYNCED;
    sc->tag_hash.insert(tag->id, tag);
    UpdateTagIcon(sc, tag);
}

void MainWindow::RUpdateTag(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };

    auto* sc { GetSectionContex(section) };
    Q_ASSERT(sc);

    const QUuid id { obj.value(kId).toString() };
    if (id.isNull()) {
        qWarning() << "RUpdateTag: tag id is null";
        return;
    }

    auto it = sc->tag_hash.find(id);
    if (it == sc->tag_hash.end()) {
        qWarning() << "RUpdateTag: tag not found, id =" << id;
        return;
    }

    if (!obj.value(kUpdate).isObject()) {
        qWarning() << "RUpdateTag: 'update' is not an object";
        return;
    }

    auto* tag { it.value() };

    const QJsonObject update_obj { obj.value(kUpdate).toObject() };
    tag->ReadJson(update_obj);

    UpdateTagIcon(sc, tag);
}

void MainWindow::RDeleteTag(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };

    auto* sc { GetSectionContex(section) };
    Q_ASSERT(sc);

    const QUuid id { obj.value(kId).toString() };
    if (id.isNull()) {
        qWarning() << "RDeleteTag: tag id is null";
        return;
    }

    auto* tag { sc->tag_hash.take(id) };
    if (tag) {
        InvalidateTagIconCache(sc, tag->id);
        ResourcePool<Tag>::Instance().Recycle(tag);
    } else {
        qWarning() << "RDeleteTag: tag not found";
    }
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

    if (!tag_hash.isEmpty()) {
        auto* tag_menu = menu->addMenu(tr("Tags"));

        QList<Tag*> sorted_tags = tag_hash.values();
        std::sort(sorted_tags.begin(), sorted_tags.end(), [](const Tag* a, const Tag* b) { return a->name < b->name; });

        for (const auto* tag : std::as_const(sorted_tags)) {
            if (!tag || tag->id.isNull())
                continue;

            auto* tag_action { tag_menu->addAction(tag->name) };

            const bool is_checked { node->tag.contains(tag->id.toString(QUuid::WithoutBraces)) };

            tag_action->setIcon(GetTagIcon(sc_, tag, is_checked));
            tag_action->setIconVisibleInMenu(true);

            connect(tag_action, &QAction::triggered, this, [=, this]() {
                if (is_checked) {
                    RRemoveNodeTag(tag, model, node);
                } else {
                    RInsertNodeTag(tag, model, node);
                }
            });
        }

        menu->addSeparator();
    }

    menu->addAction(ui->actionInsertNode);
    menu->addAction(ui->actionAppendNode);
    menu->addSeparator();
    menu->addAction(ui->actionRename);
    menu->addAction(ui->actionClearColor);
    menu->addSeparator();
    menu->addAction(ui->actionDelete);

    menu->exec(QCursor::pos());
}

void MainWindow::RInsertNodeTag(const Tag* tag, TreeModel* model, const Node* node)
{
    qDebug() << "RInsertNodeTag";
    auto list { node->tag };
    list.emplaceFront(tag->id.toString(QUuid::WithoutBraces));

    auto index { model->GetIndex(node->id) };
    if (!index.isValid()) {
        qWarning() << "Invalid index for node:" << node->name;
        return;
    }

    const int column { Utils::NodeTagColumn(start_) };
    const QModelIndex tag_index { model->index(index.row(), column) };

    model->setData(tag_index, list);
}

void MainWindow::RRemoveNodeTag(const Tag* tag, TreeModel* model, const Node* node)
{
    qDebug() << "RRemoveNodeTag";

    auto list { node->tag };
    list.removeAll(tag->id.toString(QUuid::WithoutBraces));

    auto index { model->GetIndex(node->id) };
    if (!index.isValid()) {
        qWarning() << "Invalid index for node:" << node->name;
        return;
    }

    const int column { Utils::NodeTagColumn(start_) };
    const QModelIndex tag_index { model->index(index.row(), column) };

    model->setData(tag_index, list);
}

void MainWindow::RTableViewCustomContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos);

    auto* widget { qobject_cast<TableWidget*>(ui->tabWidget->currentWidget()) };
    if (!widget)
        return;

    const auto index { widget->View()->currentIndex() };
    if (!index.isValid())
        return;

    auto* model { widget->Model() };
    const auto* entry { model->GetEntry(index) };

    const auto& tag_hash { sc_->tag_hash };

    auto* menu = new QMenu(this);

    if (!tag_hash.isEmpty()) {
        auto* tag_menu = menu->addMenu(tr("Tags"));

        QList<Tag*> sorted_tags = tag_hash.values();
        std::sort(sorted_tags.begin(), sorted_tags.end(), [](const Tag* a, const Tag* b) { return a->name < b->name; });

        for (const auto* tag : std::as_const(sorted_tags)) {
            if (!tag || tag->id.isNull())
                continue;

            auto* tag_action { tag_menu->addAction(tag->name) };

            const bool is_checked { entry->tag.contains(tag->id.toString(QUuid::WithoutBraces)) };

            tag_action->setIcon(GetTagIcon(sc_, tag, is_checked));
            tag_action->setIconVisibleInMenu(true);

            connect(tag_action, &QAction::triggered, this, [=, this]() {
                if (is_checked) {
                    RRemoveEntryTag(tag, model, entry);
                } else {
                    RInsertEntryTag(tag, model, entry);
                }
            });
        }

        menu->addSeparator();
    }

    menu->addAction(ui->actionDelete);

    menu->exec(QCursor::pos());
}

void MainWindow::UpdateTagIcon(SectionContext* sc, const Tag* tag)
{
    Q_ASSERT(sc);
    Q_ASSERT(tag);

    const QUuid& tag_id { tag->id };

    // Rebuild pixmap (used by delegates)
    const QPixmap pixmap { Utils::CreateTagPixmap(tag) };
    if (!pixmap.isNull()) {
        sc->tag_pixmap.insert(tag_id, pixmap);
    }

    // Rebuild icons (used by menus / actions)
    sc->tag_icon.insert(tag_id, Utils::CreateTagIcon(tag, /*checked=*/false));
    sc->tag_icon_checked.insert(tag_id, Utils::CreateTagIcon(tag, /*checked=*/true));
}

QIcon MainWindow::GetTagIcon(SectionContext* sc, const Tag* tag, bool checked)
{
    auto& cache = checked ? sc->tag_icon_checked : sc->tag_icon;

    auto it = cache.find(tag->id);
    if (it == cache.end()) {
        it = cache.insert(tag->id, Utils::CreateTagIcon(tag, checked));
    }

    return it.value();
}

QPixmap MainWindow::GetTagPixmap(SectionContext* sc, const Tag* tag)
{
    auto it = sc->tag_pixmap.find(tag->id);
    if (it == sc->tag_pixmap.end()) {
        it = sc->tag_pixmap.insert(tag->id, Utils::CreateTagPixmap(tag));
    }

    return it.value();
}

void MainWindow::RInsertEntryTag(const Tag* tag, TableModel* model, const Entry* entry)
{
    qDebug() << "RInsertEntryTag";
    auto list { entry->tag };
    list.emplaceFront(tag->id.toString(QUuid::WithoutBraces));

    auto index { model->GetIndex(entry->id) };
    if (!index.isValid()) {
        qWarning() << "Invalid index for entry:" << entry->id;
        return;
    }

    const int column { Utils::EntryTagColumn(start_) };

    QModelIndex tag_index = model->index(index.row(), column);
    model->setData(tag_index, list);
}

void MainWindow::RRemoveEntryTag(const Tag* tag, TableModel* model, const Entry* entry)
{
    qDebug() << "RRemoveEntryTag";
    auto list { entry->tag };
    list.removeAll(tag->id.toString(QUuid::WithoutBraces));

    auto index { model->GetIndex(entry->id) };
    if (!index.isValid()) {
        qWarning() << "Invalid index for entry";
        return;
    }

    const int column { Utils::EntryTagColumn(start_) };

    QModelIndex tag_index = model->index(index.row(), column);
    model->setData(tag_index, list);
}
