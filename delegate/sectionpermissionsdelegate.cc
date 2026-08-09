#include "sectionpermissionsdelegate.h"

#include <QStandardItem>

#include "global/userprofile.h"
#include "widget/combobox.h"
#include "workspace/sectionpermissions.h"

SectionPermissionsDelegate::SectionPermissionsDelegate(QObject* parent)
    : StyledItemDelegate { parent }
{
}

QWidget* SectionPermissionsDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    auto* editor { new ComboBox(parent) };

    auto* model { new QStandardItemModel(editor) };
    editor->setModel(model);

    UserProfile& profile { UserProfile::Instance() };
    const auto permissions { profile.SectionPermissions() };

    for (const auto& item : section::PermissionItems()) {
        if ((permissions & item.permission) != item.permission) {
            continue;
        }

        auto* model_item { new QStandardItem(item.text) };

        model_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        model_item->setData(static_cast<int>(item.permission), Qt::UserRole);
        model_item->setCheckState(Qt::Unchecked);

        model->appendRow(model_item);
    }

    return editor;
}

void SectionPermissionsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };

    const int value { index.data().toInt() };
    const section::Permissions permissions(value);

    auto* model { qobject_cast<QStandardItemModel*>(cast_editor->model()) };

    for (int i = 0; i != model->rowCount(); ++i) {
        auto* item { model->item(i) };

        const int permission { item->data(Qt::UserRole).toInt() };
        const bool checked { static_cast<int>(permissions & permission) == permission };
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }

    // Set line edit text to show all selected roles
    cast_editor->setEditText(section::PermissionsDisplay(permissions));
}

void SectionPermissionsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
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

void SectionPermissionsDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int value { index.data().toInt() };
    if (value == 0)
        return PaintEmpty(painter, option, index);

    const QString text { section::PermissionsDisplay(section::Permissions(value)) };
    PaintText(text, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize SectionPermissionsDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int value { index.data().toInt() };
    const QString text { section::PermissionsDisplay(section::Permissions(value)) };

    return CalculateTextSize(text, option);
}
