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

    const auto current_role { static_cast<int>(UserProfile::Instance().GetWorkspaceRole()) };

    for (const auto& [key, value] : workspace::RoleList()) {
        if (key < current_role)
            editor->addItem(value, key);
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
    const int key { index.data().toInt() };
    if (!workspace::RoleHash().contains(key))
        return PaintEmpty(painter, option, index);

    const QString text { workspace::RoleHash().value(key) };
    PaintText(text, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize WorkspaceRoleDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int key { index.data().toInt() };
    const QString text { workspace::RoleHash().value(key) };
    return CalculateTextSize(text, option);
}

void WorkspaceRoleDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int bar_width { QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) };
    const int key { index.data().toInt() };
    const QString text { workspace::RoleHash().value(key) };

    const QSize text_size { CalculateTextSize(text, option) };

    const int width { std::max(option.rect.width(), text_size.width() + bar_width) };
    const int height { std::max(option.rect.height(), text_size.height()) };

    QRect geom { option.rect };
    geom.setWidth(width);
    geom.setHeight(height);

    editor->setGeometry(geom);
}
