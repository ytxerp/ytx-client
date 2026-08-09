#include "workspaceroledelegate.h"

#include "global/userprofile.h"
#include "widget/combobox.h"

WorkspaceRoleDelegate::WorkspaceRoleDelegate(QObject* parent)
    : StyledItemDelegate { parent }
{
}

QWidget* WorkspaceRoleDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto* editor { new ComboBox(parent) };

    const auto current_role { UserProfile::Instance().WorkspaceRole() };

    for (const auto& item : workspace::RoleItems()) {
        if (workspace::CanAssignRole(current_role, item.role)) {
            editor->addItem(item.text, static_cast<int>(item.role));
        }
    }

    return editor;
}

void WorkspaceRoleDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };

    const int key { index.data().toInt() };
    const int idx { cast_editor->findData(key) };

    if (idx != -1)
        cast_editor->setCurrentIndex(idx);
}

void WorkspaceRoleDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* cast_editor { static_cast<ComboBox*>(editor) };

    const int key { cast_editor->currentData().toInt() };
    model->setData(index, key);
}

void WorkspaceRoleDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const auto role { static_cast<workspace::Role>(index.data().toInt()) };
    const QString text { workspace::RoleDisplay(role) };

    if (text.isEmpty())
        return PaintEmpty(painter, option, index);

    PaintText(text, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize WorkspaceRoleDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const auto role { static_cast<workspace::Role>(index.data().toInt()) };
    const QString text { workspace::RoleDisplay(role) };

    return CalculateTextSize(text, option);
}
