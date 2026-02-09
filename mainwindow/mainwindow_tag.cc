#include <QJsonArray>

#include "dialog/tagmanagerdlg.h"
#include "mainwindow.h"
#include "tag/tagmodel.h"
#include "utils/mainwindowutils.h"

void MainWindow::on_actionTags_triggered()
{
    qInfo() << "[UI]" << "on_actionTags_triggered";

    static QPointer<TagManagerDlg> dialog {};

    if (!dialog) {
        auto* model { new TagModel(start_, sc_->raw_tags, this) };

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

        Tag tag {};
        tag.ReadJson(tag_obj);

        if (tag.id.isNull()) {
            qWarning() << "RApplyTag: tag id is null, skip entry";
            continue;
        }

        sc->raw_tags.insert(tag.id, tag);
    }
}

void MainWindow::RInsertTag(const QJsonObject& obj)
{
    const Section section { obj.value(kSection).toInt() };

    auto* sc { GetSectionContex(section) };
    Q_ASSERT(sc);

    if (!obj.value(kTag).isObject()) {
        qWarning() << "RInsertTag: 'tag' is not an object";
        return;
    }

    const QJsonObject tag_obj { obj.value(kTag).toObject() };

    Tag tag {};
    tag.ReadJson(tag_obj);

    if (tag.id.isNull()) {
        qWarning() << "RInsertTag: tag id is null";
        return;
    }

    sc->raw_tags.insert(tag.id, tag);
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

    auto it = sc->raw_tags.find(id);
    if (it == sc->raw_tags.end()) {
        qWarning() << "RUpdateTag: tag not found, id =" << id;
        return;
    }

    if (!obj.value(kUpdate).isObject()) {
        qWarning() << "RUpdateTag: 'update' is not an object";
        return;
    }

    const QJsonObject update_obj { obj.value(kUpdate).toObject() };
    it->ReadJson(update_obj);
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

    if (!sc->raw_tags.remove(id)) {
        qWarning() << "RDeleteTag: tag not found, id =" << id;
    }
}
