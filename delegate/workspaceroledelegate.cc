#include "workspaceroledelegate.h"

#include "global/userprofile.h"
#include "widget/combobox.h"

WorkspaceRoleDelegate::WorkspaceRoleDelegate(
    const QHash<int, QString>& workspace_role_name, const QList<QPair<int, QString>>& workspace_role_list, QObject* parent)
    : StyledItemDelegate { parent }
    , workspace_role_name_ { workspace_role_name }
    , workspace_role_list_ { workspace_role_list }
{
}

QWidget* WorkspaceRoleDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto* editor { new ComboBox(parent) };

    const auto current_role { static_cast<int>(UserProfile::Instance().GetWorkspaceRole()) };

    for (const auto& [key, value] : workspace_role_list_) {
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
    if (!workspace_role_name_.contains(key))
        return PaintEmpty(painter, option, index);

    const QString text { workspace_role_name_.value(key) };
    PaintText(text, painter, option, index, Qt::AlignLeft | Qt::AlignVCenter);
}

QSize WorkspaceRoleDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int key { index.data().toInt() };
    const QString text { workspace_role_name_.value(key) };
    return CalculateTextSize(text, option);
}

void WorkspaceRoleDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int bar_width { QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) };
    const int key { index.data().toInt() };
    const QString text { workspace_role_name_.value(key) };

    const QSize text_size { CalculateTextSize(text, option) };

    const int width { std::max(option.rect.width(), text_size.width() + bar_width) };
    const int height { std::max(option.rect.height(), text_size.height()) };

    QRect geom { option.rect };
    geom.setWidth(width);
    geom.setHeight(height);

    editor->setGeometry(geom);
}
