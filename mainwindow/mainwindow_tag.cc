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

    InvalidateTagIconCache(tag->id);
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
        InvalidateTagIconCache(tag->id);
        ResourcePool<Tag>::Instance().Recycle(tag);
    } else {
        qWarning() << "RDeleteTag: tag not found";
    }
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
    menu->setObjectName("tagMenu");

    if (!tag_hash.isEmpty()) {
        auto* tag_menu = menu->addMenu(tr("Tags"));

        QList<Tag*> sorted_tags = tag_hash.values();
        std::sort(sorted_tags.begin(), sorted_tags.end(), [](const Tag* a, const Tag* b) { return a->name < b->name; });

        for (const auto* tag : std::as_const(sorted_tags)) {
            if (!tag || tag->id.isNull())
                continue;

            auto* tag_action { tag_menu->addAction(tag->name) };

            const bool is_checked { entry->tag.contains(tag->id.toString(QUuid::WithoutBraces)) };

            tag_action->setIcon(GetTagIcon(tag, is_checked));
            tag_action->setIconVisibleInMenu(true);

            connect(tag_action, &QAction::triggered, this, [=, this]() {
                if (is_checked) {
                    RRemoveTagFromCurrentRow(tag, model, entry);
                } else {
                    RInsertTagIntoCurrentRow(tag, model, entry);
                }
            });
        }

        menu->addSeparator();
    }

    menu->addAction(ui->actionDelete);

    menu->exec(QCursor::pos());
}

QIcon MainWindow::GetTagIcon(const Tag* tag, bool checked)
{
    auto& cache = checked ? tag_icon_checked_cache_ : tag_icon_cache_;

    if (cache.contains(tag->id)) {
        return cache[tag->id];
    }

    const qreal dpr { qApp->devicePixelRatio() };
    QPixmap pixmap(static_cast<int>(16 * dpr), static_cast<int>(16 * dpr));
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(QColor(tag->color)));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(1, 1, 15, 15, 3, 3);

    if (checked) {
        painter.setPen(Utils::GetContrastColor(tag->color));
        painter.setBrush(Qt::NoBrush);
        painter.drawLine(QPointF(4, 8), QPointF(7, 11));
        painter.drawLine(QPointF(7, 11), QPointF(12, 5));
    }

    painter.end();

    QIcon icon(pixmap);
    cache[tag->id] = icon;
    return icon;
}

void MainWindow::RInsertTagIntoCurrentRow(const Tag* tag, TableModel* model, const Entry* entry)
{
    qDebug() << "RInsertTagIntoCurrentRow";
    auto list { entry->tag };
    list.emplaceBack(tag->id.toString(QUuid::WithoutBraces));

    auto index { model->GetIndex(entry->id) };
    if (!index.isValid()) {
        qWarning() << "Invalid index for entry:" << entry->id;
        return;
    }

    const int column { Utils::EntryTagColumn(start_) };

    QModelIndex tag_index = model->index(index.row(), column);
    model->setData(tag_index, list);
}

void MainWindow::RRemoveTagFromCurrentRow(const Tag* tag, TableModel* model, const Entry* entry)
{
    qDebug() << "RRemoveTagFromCurrentRow";
    auto list { entry->tag };
    list.removeAll(tag->id.toString(QUuid::WithoutBraces));

    auto index { model->GetIndex(entry->id) };
    if (!index.isValid()) {
        qWarning() << "Invalid index for entry:" << entry->id;
        return;
    }

    const int column { Utils::EntryTagColumn(start_) };

    QModelIndex tag_index = model->index(index.row(), column);
    model->setData(tag_index, list);
}
