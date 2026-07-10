#include "databaseroledelegate.h"

#include <QStandardItem>

#include "global/userprofile.h"
#include "widget/combobox.h"
#include "workspace/databaserole.h"

DatabaseRoleDelegate::DatabaseRoleDelegate(QObject* parent)
    : StyledItemDelegate { parent }
{
}

QWidget* DatabaseRoleDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    auto* editor { new ComboBox(parent) };

    auto* model { new QStandardItemModel(editor) };
    editor->setModel(model);

    UserProfile& profile { UserProfile::Instance() };
    const auto database_roles { profile.GetDatabaseRoles() };

    for (const auto& item : database::RoleItemList()) {
        if ((database_roles & item.role) != item.role) {
            continue;
        }

        auto* model_item { new QStandardItem(item.text) };

        model_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        model_item->setData(static_cast<int>(item.role), Qt::UserRole);
        model_item->setCheckState(Qt::Unchecked);

        model->appendRow(model_item);
    }

    return editor;
}

void DatabaseRoleDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };

    const int value { index.data().toInt() };
    const database::Roles roles(value);

    auto* model { qobject_cast<QStandardItemModel*>(cast_editor->model()) };

    for (int i = 0; i != model->rowCount(); ++i) {
        auto* item { model->item(i) };

        const int role { item->data(Qt::UserRole).toInt() };
        const bool checked { static_cast<int>(roles & role) == role };
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }

    // Set line edit text to show all selected roles
    cast_editor->setEditText(database::RolesDisplay(roles));
}

void DatabaseRoleDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };
    auto* cast_model { qobject_cast<QStandardItemModel*>(cast_editor->model()) };

    int result { 0 };

    for (int i = 0; i != cast_model->rowCount(); ++i) {
        auto* item { cast_model->item(i) };

        if (item->checkState() == Qt::Checked) {
            result |= item->data(Qt::UserRole).toInt();
        }
    }

    model->setData(index, result);
}

void DatabaseRoleDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int value { index.data().toInt() };
    if (value == 0)
        return PaintEmpty(painter, option, index);

    const QString text { database::RolesDisplay(database::Roles(value)) };
    PaintText(text, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize DatabaseRoleDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int value { index.data().toInt() };
    const QString text { database::RolesDisplay(database::Roles(value)) };

    return CalculateTextSize(text, option);
}
