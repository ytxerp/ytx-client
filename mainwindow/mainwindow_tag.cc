#include <QJsonArray>

#include "dialog/tagmanagerdlg.h"
#include "global/resourcepool.h"
#include "mainwindow.h"
#include "tag/tagmodel.h"
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

    const QJsonObject update_obj { obj.value(kUpdate).toObject() };
    it.value()->ReadJson(update_obj);
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
        ResourcePool<Tag>::Instance().Recycle(tag);
    } else {
        qWarning() << "RDeleteTag: tag not found";
    }
}
